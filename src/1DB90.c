/*
 * 1DB90.c -- n_audio: building the synthesizer, making a frame, the free update and voice lists, and time
 * conversion.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_80042790

Builds the synthesizer from c on c's heap: the effect and main buses, the effect if c has one, the physical
voices on the free list, and the free parameter updates.
================
*/
void func_80042790( ALSynConfig *c ) {
    s32 i;
    N_PVoice *pv;
    N_PVoice *pvoices;
    ALHeap *hp;
    ALParam *params;
    ALParam *paramPtr;

    hp = c->heap;
    D_8005D114->head = NULL;
    D_8005D114->numPVoices = c->maxPVoices;
    D_8005D114->curSamples = 0;
    D_8005D114->paramSamples = 0;
    D_8005D114->outputRate = c->outputRate;
    D_8005D114->maxOutSamples = AL_MAX_RSP_SAMPLES;
    D_8005D114->dma = (ALDMANew)c->dmaproc;
    D_8005D114->unk_4C = 1;
    D_8005D114->outBuf = NULL;

    D_8005D114->auxBus = alHeapAlloc( hp, 1, sizeof( N_ALAuxBus ) );
    D_8005D114->auxBus->sourceCount = 0;
    D_8005D114->auxBus->maxSources = c->maxPVoices;
    D_8005D114->auxBus->sources = alHeapAlloc( hp, c->maxPVoices, sizeof( N_PVoice * ) );

    D_8005D114->mainBus = alHeapAlloc( hp, 1, sizeof( N_ALMainBus ) );
    if ( c->fxType != AL_FX_NONE ) {
        D_8005D114->auxBus->fx = func_80042D40( 0, c, hp );
        D_8005D114->mainBus->handler = func_80041760;
    } else {
        D_8005D114->mainBus->handler = func_8003FED0;
    }

    D_8005D114->pFreeList.next = NULL;
    D_8005D114->pFreeList.prev = NULL;
    D_8005D114->pLameList.next = NULL;
    D_8005D114->pLameList.prev = NULL;
    D_8005D114->pAllocList.next = NULL;
    D_8005D114->pAllocList.prev = NULL;

    pvoices = alHeapAlloc( hp, c->maxPVoices, sizeof( N_PVoice ) );
    for ( i = 0; i < c->maxPVoices; i++ ) {
        pv = &pvoices[i];
        alLink( &pv->node, &D_8005D114->pFreeList );
        pv->vvoice = NULL;
        func_8004035C( pv, D_8005D114->dma, hp );
        D_8005D114->auxBus->sources[D_8005D114->auxBus->sourceCount++] = pv;
    }

    params = alHeapAlloc( hp, c->maxUpdates, sizeof( ALParam ) );
    D_8005D114->paramList = NULL;
    for ( i = 0; i < c->maxUpdates; i++ ) {
        paramPtr = &params[i];
        paramPtr->next = D_8005D114->paramList;
        D_8005D114->paramList = paramPtr;
    }

    D_8005D114->heap = hp;
}

/*
================
func_80042A00

Makes outLen stereo samples of output into outBuf: runs the first client's handler until its next call lies
beyond this output, then builds the frame in passes of at most maxOutSamples samples at cmdList. Stores the
command count in *cmdLen, frees the retired physical voices, and returns the end of the commands.
================
*/
Acmd *func_80042A00( Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen ) {
    ALPlayer *client;
    Acmd *cmdlEnd = cmdList;
    s16 *outPtr = outBuf;
    s32 n;

    if ( D_8005D114->head == NULL ) {
        *cmdLen = 0;
        return cmdlEnd;
    }

    client = D_8005D114->head;
    while ( client->samplesLeft - D_8005D114->curSamples < outLen ) {
        D_8005D114->paramSamples = client->samplesLeft & ~15;
        client->samplesLeft += func_80042C44( client->handler( client ) );
    }
    D_8005D114->paramSamples &= ~15;

    while ( outLen > 0 ) {
        n = D_8005D114->maxOutSamples;
        if ( outLen < n ) {
            n = outLen;
        }
        D_8005D114->outBuf = outPtr;
        cmdlEnd = func_80042CF0( D_8005D114->curSamples, cmdlEnd );
        outLen -= n;
        outPtr += 2 * n;
        D_8005D114->curSamples += n;
    }

    *cmdLen = cmdlEnd - cmdList;
    func_80042BAC();
    return cmdlEnd;
}

/*
================
func_80042B68

Takes an update off the free list and returns it, NULL if none is left.
================
*/
ALParam *func_80042B68( void ) {
    ALParam *update = NULL;

    if ( D_8005D114->paramList != NULL ) {
        update = D_8005D114->paramList;
        D_8005D114->paramList = update->next;
        update->next = NULL;
    }
    return update;
}

/*
================
func_80042B94

Puts update back on the free list.
================
*/
void func_80042B94( ALParam *update ) {
    update->next = D_8005D114->paramList;
    D_8005D114->paramList = update;
}

/*
================
func_80042BAC

Moves every retired physical voice back to the free list.
================
*/
void func_80042BAC( void ) {
    N_PVoice *pv;

    while ( ( pv = (N_PVoice *)D_8005D114->pLameList.next ) != NULL ) {
        alUnlink( &pv->node );
        alLink( &pv->node, &D_8005D114->pFreeList );
    }
}

/*
================
func_80042C0C

Retires pv: moves it to the list freed at the end of the frame.
================
*/
void func_80042C0C( N_PVoice *pv ) {
    alUnlink( &pv->node );
    alLink( &pv->node, &D_8005D114->pLameList );
}

/*
================
func_80042C44

Returns t microseconds in output samples, rounded to the nearest.
================
*/
s32 func_80042C44( ALMicroTime t ) {
    f32 samples = (f32)t * D_8005D114->outputRate / 1000000.0 + 0.5;

    return samples;
}

/*
================
func_80042C94

Returns t microseconds in output samples, rounded to the nearest and then down to a multiple of 16.
================
*/
s32 func_80042C94( ALMicroTime t ) {
    f32 samples = (f32)t * D_8005D114->outputRate / 1000000.0 + 0.5;

    return (s32)samples & ~15;
}
