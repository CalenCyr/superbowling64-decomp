/*
 * 1A7A0.c -- the synthesizer's ROM sample cache, and the audio thread's scheduler client.
 *
 * The synthesizer reads samples through func_8003F5F0, which serves them from a cache of fixed-size ROM DMA
 * buffers: a read inside a buffer already loaded uses it, any other starts a DMA into a free buffer. The DMA
 * completes before the frame's RSP task runs; func_8003F514 collects the completions each frame and frees the
 * buffers that went unused.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

#define SAMPLE_ADDR_TAG     0xFF000000  /* sample addresses with this top byte are read through D_800693D8 */
#define SAMPLE_ADDR_MASK    0x00FFFFFF
#define SAMPLE_ADDR_BASE    0x140000    /* where tagged addresses start on D_800693D8's device */
#define SAMPLES_IN_RAM      0x00000001  /* D_800DF6B0: untagged samples are already in RAM */
#define DMA_BUFFER_FRAMES   2           /* frames a buffer stays loaded after its last use */

/*
================
func_8003F3A0

Sets the cache up with count buffers of size bytes; returns the synthesizer's DMA set-up routine.
================
*/
ALDMANew func_8003F3A0( s32 count, s32 size ) {
    s32 i;

    D_800643C8 = osCartRomInit();
    D_8006439C = Snd_Alloc( count * 2 * sizeof( OSIoMesg ) );
    D_800643A0 = Snd_Alloc( count * 2 * sizeof( OSMesg ) );
    D_80064398 = Snd_Alloc( count * sizeof( dmaBuffer_t ) );
    Snd_MemSet( D_80064398, 0, count * sizeof( dmaBuffer_t ) );
    for ( i = 0; i < count - 1; i++ ) {
        D_80064398[i].next = &D_80064398[i] + 1;
        D_80064398[i].next->prev = &D_80064398[i];
        D_80064398[i].ptr = Snd_Alloc( size );
        D_80064398[i].startAddr = -1;
    }
    D_80064398[i].ptr = Snd_Alloc( size );
    D_80064398[i].startAddr = -1;
    D_800643A4 = size;
    D_800643A8 = 0;
    D_80064390 = NULL;
    D_80064394 = D_80064398;
    osCreateMesgQueue( &D_800643B0, D_800643A0, count * 2 );
    return func_8003F5E4;
}

/*
================
func_8003F514

Once a frame: collects the completions of the frame's DMAs, and frees the buffers no read has used for
DMA_BUFFER_FRAMES frames.
================
*/
void func_8003F514( void ) {
    OSMesg msg;
    dmaBuffer_t *buf;
    dmaBuffer_t *next;

    while ( D_800643A8 != 0 ) {
        osRecvMesg( &D_800643B0, &msg, OS_MESG_NOBLOCK );
        D_800643A8--;
    }

    buf = D_80064390;
    while ( buf != NULL ) {
        if ( --buf->framesLeft == 0 ) {
            next = buf->next;
            if ( next != NULL ) {
                next->prev = buf->prev;
            }
            if ( buf->prev != NULL ) {
                buf->prev->next = buf->next;
            } else {
                D_80064390 = buf->next;
            }
            buf->prev = NULL;
            buf->next = D_80064394;
            D_80064394 = buf;
            buf = next;
        } else {
            buf = buf->next;
        }
    }
}

/*
================
func_8003F5E4

The synthesizer's DMA set-up routine: every voice reads through func_8003F5F0.
================
*/
ALDMAproc func_8003F5E4( void *state ) {
    return func_8003F5F0;
}

/*
================
func_8003F5F0

Returns the physical address of the samples at addr, len bytes: in a cache buffer, or where they are if
they need no DMA.
================
*/
s32 func_8003F5F0( s32 addr, s32 len, void *state ) {
    dmaBuffer_t *buf;

    buf = func_8003F65C( addr, len );
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
func_8003F65C

Returns the cache buffer holding addr to addr + len, starting a DMA into a free buffer if none does; NULL
if the samples are in RAM. With no buffer free, returns the first buffer in use (the wrong data).
================
*/
dmaBuffer_t *func_8003F65C( u32 addr, s32 len ) {
    OSPiHandle *handle;
    dmaBuffer_t *buf;
    dmaBuffer_t *prev;
    dmaBuffer_t *newBuf;
    OSIoMesg *ioMsg;
    u32 end;

    if ( ( addr & SAMPLE_ADDR_TAG ) == SAMPLE_ADDR_TAG ) {
        handle = D_800693D8;
        addr &= SAMPLE_ADDR_MASK;
        addr += SAMPLE_ADDR_BASE;
    } else {
        if ( D_800DF6B0 & SAMPLES_IN_RAM ) {
            return NULL;
        }
        handle = D_800643C8;
    }

    end = addr + len;
    prev = NULL;
    for ( buf = D_80064390; buf != NULL; buf = buf->next ) {
        if ( addr < buf->startAddr ) {
            break;
        }
        if ( end <= buf->startAddr + D_800643A4 ) {
            return buf;
        }
        prev = buf;
    }

    newBuf = D_80064394;
    if ( newBuf == NULL ) {
        return D_80064390;
    }
    D_80064394 = newBuf->next;
    if ( prev != NULL ) {
        newBuf->next = prev->next;
        if ( newBuf->next != NULL ) {
            newBuf->next->prev = newBuf;
        }
        newBuf->prev = prev;
        prev->next = newBuf;
    } else {
        newBuf->prev = NULL;
        newBuf->next = D_80064390;
        if ( D_80064390 != NULL ) {
            D_80064390->prev = newBuf;
        }
        D_80064390 = newBuf;
    }

    newBuf->startAddr = addr & ~1;
    newBuf->framesLeft = DMA_BUFFER_FRAMES;
    ioMsg = &D_8006439C[D_800643A8++];
    ioMsg->hdr.pri = OS_MESG_PRI_NORMAL;
    ioMsg->hdr.retQueue = &D_800643B0;
    ioMsg->dramAddr = newBuf->ptr;
    ioMsg->devAddr = newBuf->startAddr;
    ioMsg->size = D_800643A4;
    osEPiStartDma( handle, ioMsg, OS_READ );
    return newBuf;
}

/*
================
func_8003F7F0

Sets the scheduler the audio thread's client registers with.
================
*/
void func_8003F7F0( OSSched *sc ) {
    D_800643D0 = sc;
}

/*
================
func_8003F7FC

D_8005CE40's init: registers the audio thread's client with the scheduler.
================
*/
void func_8003F7FC( void ) {
    D_800643D4 = Snd_Alloc( sizeof( audioClient_t ) );
    osCreateMesgQueue( &D_800643D4->msgQ, D_800643D4->msgBuf, AUDIO_CLIENT_MESGS );
    osCreateMesgQueue( &D_800643D4->taskDoneQ, D_800643D4->taskDoneMsgBuf, AUDIO_CLIENT_MESGS );
    osScAddClient( D_800643D0, &D_800643D4->client, &D_800643D4->msgQ );
}

/*
================
func_8003F860

D_8005CE40's waitFrame: waits for the next retrace message, dropping any other.
================
*/
void func_8003F860( void ) {
    OSScMsg *msg;

    do {
        osRecvMesg( &D_800643D4->msgQ, (OSMesg *)&msg, OS_MESG_BLOCK );
        osRecvMesg( &D_800643D4->msgQ, NULL, OS_MESG_NOBLOCK );
    } while ( msg->type != OS_SC_RETRACE_MSG );
}

/*
================
func_8003F8C0

D_8005CE40's runTask: runs audio on the RSP through the scheduler and waits until it is done.
================
*/
void func_8003F8C0( audioTask_t *audio ) {
    OSScTask task;
    OSScMsg doneMsg;

    task.next = NULL;
    task.msg = (OSMesg)&doneMsg;
    task.flags = OS_SC_NEEDS_RSP;
    task.msgQ = &D_800643D4->taskDoneQ;
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
    osSendMesg( osScGetCmdQ( D_800643D0 ), (OSMesg)&task, OS_MESG_BLOCK );
    osRecvMesg( &D_800643D4->taskDoneQ, NULL, OS_MESG_BLOCK );
}
