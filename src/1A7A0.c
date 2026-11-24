/*
 * 1A7A0.c -- the synthesizer's ROM sample cache, and the audio thread's scheduler client.
 *
 * The synthesizer reads samples through Audio_DmaRead, which serves them from a cache of fixed-size ROM DMA
 * buffers: a read inside a buffer already loaded uses it, any other starts a DMA into a free buffer. The DMA
 * completes before the frame's RSP task runs; Audio_UpdateDmaCache collects the completions each frame and frees the
 * buffers that went unused.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

#define SAMPLE_ADDR_TAG     0xFF000000  /* sample addresses with this top byte are read through audio_samplePiHandle */
#define SAMPLE_ADDR_MASK    0x00FFFFFF
#define SAMPLE_ADDR_BASE    0x140000    /* where tagged addresses start on audio_samplePiHandle's device */
#define SAMPLES_IN_RAM      0x00000001  /* audio_sampleFlags: untagged samples are already in RAM */
#define DMA_BUFFER_FRAMES   2           /* frames a buffer stays loaded after its last use */

/*
================
Audio_InitDmaCache

Sets the cache up with count buffers of size bytes; returns the synthesizer's DMA set-up routine.
================
*/
ALDMANew Audio_InitDmaCache( s32 count, s32 size ) {
    s32 i;

    audio_romHandle = osCartRomInit();
    audio_dmaRequests = Snd_Alloc( count * 2 * sizeof( OSIoMesg ) );
    audio_dmaMsgs = Snd_Alloc( count * 2 * sizeof( OSMesg ) );
    audio_dmaBuffers = Snd_Alloc( count * sizeof( dmaBuffer_t ) );
    Snd_MemSet( audio_dmaBuffers, 0, count * sizeof( dmaBuffer_t ) );
    for ( i = 0; i < count - 1; i++ ) {
        audio_dmaBuffers[i].next = &audio_dmaBuffers[i] + 1;
        audio_dmaBuffers[i].next->prev = &audio_dmaBuffers[i];
        audio_dmaBuffers[i].ptr = Snd_Alloc( size );
        audio_dmaBuffers[i].startAddr = -1;
    }
    audio_dmaBuffers[i].ptr = Snd_Alloc( size );
    audio_dmaBuffers[i].startAddr = -1;
    audio_dmaBufferSize = size;
    audio_dmaCount = 0;
    audio_dmaUsed = NULL;
    audio_dmaFree = audio_dmaBuffers;
    osCreateMesgQueue( &audio_dmaQueue, audio_dmaMsgs, count * 2 );
    return Audio_DmaSetup;
}

/*
================
Audio_UpdateDmaCache

Once a frame: collects the completions of the frame's DMAs, and frees the buffers no read has used for
DMA_BUFFER_FRAMES frames.
================
*/
void Audio_UpdateDmaCache( void ) {
    OSMesg msg;
    dmaBuffer_t *buf;
    dmaBuffer_t *next;

    while ( audio_dmaCount != 0 ) {
        osRecvMesg( &audio_dmaQueue, &msg, OS_MESG_NOBLOCK );
        audio_dmaCount--;
    }

    buf = audio_dmaUsed;
    while ( buf != NULL ) {
        if ( --buf->framesLeft == 0 ) {
            next = buf->next;
            if ( next != NULL ) {
                next->prev = buf->prev;
            }
            if ( buf->prev != NULL ) {
                buf->prev->next = buf->next;
            } else {
                audio_dmaUsed = buf->next;
            }
            buf->prev = NULL;
            buf->next = audio_dmaFree;
            audio_dmaFree = buf;
            buf = next;
        } else {
            buf = buf->next;
        }
    }
}

/*
================
Audio_DmaSetup

The synthesizer's DMA set-up routine: every voice reads through Audio_DmaRead.
================
*/
ALDMAproc Audio_DmaSetup( void *state ) {
    return Audio_DmaRead;
}

/*
================
Audio_DmaRead

Returns the physical address of the samples at addr, len bytes: in a cache buffer, or where they are if
they need no DMA.
================
*/
s32 Audio_DmaRead( s32 addr, s32 len, void *state ) {
    dmaBuffer_t *buf;

    buf = Audio_FindDmaBuffer( addr, len );
    if ( buf == NULL ) {
        return osVirtualToPhysical( (void *)addr );
    }
    if ( ( addr & SAMPLE_ADDR_TAG ) == SAMPLE_ADDR_TAG ) {
        addr &= SAMPLE_ADDR_MASK;
        addr += SAMPLE_ADDR_BASE;
    }
    return osVirtualToPhysical( buf->ptr + addr - buf->startAddr );
}

/*
================
Audio_FindDmaBuffer

Returns the cache buffer holding addr to addr + len, starting a DMA into a free buffer if none does; NULL
if the samples are in RAM. With no buffer free, returns the first buffer in use (the wrong data).
================
*/
dmaBuffer_t *Audio_FindDmaBuffer( u32 addr, s32 len ) {
    OSPiHandle *handle;
    dmaBuffer_t *buf;
    dmaBuffer_t *prev;
    dmaBuffer_t *newBuf;
    OSIoMesg *ioMsg;
    u32 end;

    if ( ( addr & SAMPLE_ADDR_TAG ) == SAMPLE_ADDR_TAG ) {
        handle = audio_samplePiHandle;
        addr &= SAMPLE_ADDR_MASK;
        addr += SAMPLE_ADDR_BASE;
    } else {
        if ( audio_sampleFlags & SAMPLES_IN_RAM ) {
            return NULL;
        }
        handle = audio_romHandle;
    }

    end = addr + len;
    prev = NULL;
    for ( buf = audio_dmaUsed; buf != NULL; buf = buf->next ) {
        if ( addr < buf->startAddr ) {
            break;
        }
        if ( end <= buf->startAddr + audio_dmaBufferSize ) {
            return buf;
        }
        prev = buf;
    }

    newBuf = audio_dmaFree;
    if ( newBuf == NULL ) {
        return audio_dmaUsed;
    }
    audio_dmaFree = newBuf->next;
    if ( prev != NULL ) {
        newBuf->next = prev->next;
        if ( newBuf->next != NULL ) {
            newBuf->next->prev = newBuf;
        }
        newBuf->prev = prev;
        prev->next = newBuf;
    } else {
        newBuf->prev = NULL;
        newBuf->next = audio_dmaUsed;
        if ( audio_dmaUsed != NULL ) {
            audio_dmaUsed->prev = newBuf;
        }
        audio_dmaUsed = newBuf;
    }

    newBuf->startAddr = addr & ~1;
    newBuf->framesLeft = DMA_BUFFER_FRAMES;
    ioMsg = &audio_dmaRequests[audio_dmaCount++];
    ioMsg->hdr.pri = OS_MESG_PRI_NORMAL;
    ioMsg->hdr.retQueue = &audio_dmaQueue;
    ioMsg->dramAddr = newBuf->ptr;
    ioMsg->devAddr = newBuf->startAddr;
    ioMsg->size = audio_dmaBufferSize;
    osEPiStartDma( handle, ioMsg, OS_READ );
    return newBuf;
}

/*
================
Audio_SetScheduler

Sets the scheduler the audio thread's client registers with.
================
*/
void Audio_SetScheduler( OSSched *sc ) {
    audio_sched = sc;
}

/*
================
Audio_ClientInit

audio_defaultTaskFuncs's init: registers the audio thread's client with the scheduler.
================
*/
void Audio_ClientInit( void ) {
    audio_client = Snd_Alloc( sizeof( audioClient_t ) );
    osCreateMesgQueue( &audio_client->msgQ, audio_client->msgBuf, AUDIO_CLIENT_MESGS );
    osCreateMesgQueue( &audio_client->taskDoneQ, audio_client->taskDoneMsgBuf, AUDIO_CLIENT_MESGS );
    osScAddClient( audio_sched, &audio_client->client, &audio_client->msgQ );
}

/*
================
Audio_ClientWaitFrame

audio_defaultTaskFuncs's waitFrame: waits for the next retrace message, dropping any other.
================
*/
void Audio_ClientWaitFrame( void ) {
    OSScMsg *msg;

    do {
        osRecvMesg( &audio_client->msgQ, (OSMesg *)&msg, OS_MESG_BLOCK );
        osRecvMesg( &audio_client->msgQ, NULL, OS_MESG_NOBLOCK );
    } while ( msg->type != OS_SC_RETRACE_MSG );
}

/*
================
Audio_ClientRunTask

audio_defaultTaskFuncs's runTask: runs audio on the RSP through the scheduler and waits until it is done.
================
*/
void Audio_ClientRunTask( audioTask_t *audio ) {
    OSScTask task;
    OSScMsg doneMsg;

    task.next = NULL;
    task.msg = (OSMesg)&doneMsg;
    task.flags = OS_SC_NEEDS_RSP;
    task.msgQ = &audio_client->taskDoneQ;
    task.list.t.data_ptr = (u64 *)audio->cmdList;
    task.list.t.data_size = audio->cmdSize;
    task.list.t.type = M_AUDTASK;
    task.list.t.ucode_boot = (u64 *)rspbootTextStart;
    task.list.t.ucode_boot_size = (s32)rspbootTextEnd - (s32)rspbootTextStart;
    task.list.t.flags = 0;
    task.list.t.ucode = audio->ucode;
    task.list.t.ucode_data = audio->ucodeData;
    task.list.t.ucode_size = SP_UCODE_SIZE;
    task.list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
    task.list.t.dram_stack = NULL;
    task.list.t.dram_stack_size = 0;
    task.list.t.output_buff = NULL;
    task.list.t.output_buff_size = NULL;
    task.list.t.yield_data_ptr = NULL;
    task.list.t.yield_data_size = 0;
    osSendMesg( osScGetCmdQ( audio_sched ), (OSMesg)&task, OS_MESG_BLOCK );
    osRecvMesg( &audio_client->taskDoneQ, NULL, OS_MESG_BLOCK );
}
