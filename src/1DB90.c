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
Synth_BuildSynth

Builds the synthesizer from c on c's heap: the effect and main buses, the effect if c has one, the physical
voices on the free list, and the free parameter updates.
================
*/
void Synth_BuildSynth( ALSynConfig *c ) {
    s32 i;
    N_PVoice *pv;
    N_PVoice *pvoices;
    ALHeap *hp;
    ALParam *params;
    ALParam *paramPtr;

    hp = c->heap;
    snd_synth->head = NULL;
    snd_synth->numPVoices = c->maxPVoices;
    snd_synth->curSamples = 0;
    snd_synth->paramSamples = 0;
    snd_synth->outputRate = c->outputRate;
    snd_synth->maxOutSamples = AL_MAX_RSP_SAMPLES;
    snd_synth->dma = (ALDMANew)c->dmaproc;
    snd_synth->unk_4C = 1;
    snd_synth->outBuf = NULL;

    snd_synth->auxBus = alHeapAlloc( hp, 1, sizeof( N_ALAuxBus ) );
    snd_synth->auxBus->sourceCount = 0;
    snd_synth->auxBus->maxSources = c->maxPVoices;
    snd_synth->auxBus->sources = alHeapAlloc( hp, c->maxPVoices, sizeof( N_PVoice * ) );

    snd_synth->mainBus = alHeapAlloc( hp, 1, sizeof( N_ALMainBus ) );
    if ( c->fxType != AL_FX_NONE ) {
        snd_synth->auxBus->fx = Synth_NewFx( 0, c, hp );
        snd_synth->mainBus->handler = Synth_PullMainBus;
    } else {
        snd_synth->mainBus->handler = Synth_PullFxBus;
    }

    snd_synth->pFreeList.next = NULL;
    snd_synth->pFreeList.prev = NULL;
    snd_synth->pLameList.next = NULL;
    snd_synth->pLameList.prev = NULL;
    snd_synth->pAllocList.next = NULL;
    snd_synth->pAllocList.prev = NULL;

    pvoices = alHeapAlloc( hp, c->maxPVoices, sizeof( N_PVoice ) );
    for ( i = 0; i < c->maxPVoices; i++ ) {
        pv = &pvoices[i];
        alLink( &pv->node, &snd_synth->pFreeList );
        pv->vvoice = NULL;
        Synth_InitPVoice( pv, snd_synth->dma, hp );
        snd_synth->auxBus->sources[snd_synth->auxBus->sourceCount++] = pv;
    }

    params = alHeapAlloc( hp, c->maxUpdates, sizeof( ALParam ) );
    snd_synth->paramList = NULL;
    for ( i = 0; i < c->maxUpdates; i++ ) {
        paramPtr = &params[i];
        paramPtr->next = snd_synth->paramList;
        snd_synth->paramList = paramPtr;
    }

    snd_synth->heap = hp;
}

/*
================
Synth_MakeFrame

Makes outLen stereo samples of output into outBuf: runs the first client's handler until its next call lies
beyond this output, then builds the frame in passes of at most maxOutSamples samples at cmdList. Stores the
command count in *cmdLen, frees the retired physical voices, and returns the end of the commands.
================
*/
Acmd *Synth_MakeFrame( Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen ) {
    ALPlayer *client;
    Acmd *cmdlEnd = cmdList;
    s16 *outPtr = outBuf;
    s32 n;

    if ( snd_synth->head == NULL ) {
        *cmdLen = 0;
        return cmdlEnd;
    }

    client = snd_synth->head;
    while ( client->samplesLeft - snd_synth->curSamples < outLen ) {
        snd_synth->paramSamples = client->samplesLeft & ~15;
        client->samplesLeft += Synth_RoundTime( client->handler( client ) );
    }
    snd_synth->paramSamples &= ~15;

    while ( outLen > 0 ) {
        n = snd_synth->maxOutSamples;
        if ( outLen < n ) {
            n = outLen;
        }
        snd_synth->outBuf = outPtr;
        cmdlEnd = Synth_PullOutput( snd_synth->curSamples, cmdlEnd );
        outLen -= n;
        outPtr += 2 * n;
        snd_synth->curSamples += n;
    }

    *cmdLen = cmdlEnd - cmdList;
    Synth_RecycleRetired();
    return cmdlEnd;
}

/*
================
Synth_AllocUpdate

Takes an update off the free list and returns it, NULL if none is left.
================
*/
ALParam *Synth_AllocUpdate( void ) {
    ALParam *update = NULL;

    if ( snd_synth->paramList != NULL ) {
        update = snd_synth->paramList;
        snd_synth->paramList = update->next;
        update->next = NULL;
    }
    return update;
}

/*
================
Synth_FreeUpdate

Puts update back on the free list.
================
*/
void Synth_FreeUpdate( ALParam *update ) {
    update->next = snd_synth->paramList;
    snd_synth->paramList = update;
}

/*
================
Synth_RecycleRetired

Moves every retired physical voice back to the free list.
================
*/
void Synth_RecycleRetired( void ) {
    N_PVoice *pv;

    while ( ( pv = (N_PVoice *)snd_synth->pLameList.next ) != NULL ) {
        alUnlink( &pv->node );
        alLink( &pv->node, &snd_synth->pFreeList );
    }
}

/*
================
Synth_RetirePVoice

Retires pv: moves it to the list freed at the end of the frame.
================
*/
void Synth_RetirePVoice( N_PVoice *pv ) {
    alUnlink( &pv->node );
    alLink( &pv->node, &snd_synth->pLameList );
}

/*
================
Synth_RoundTime

Returns t microseconds in output samples, rounded to the nearest.
================
*/
s32 Synth_RoundTime( ALMicroTime t ) {
    f32 samples = (f32)t * snd_synth->outputRate / 1000000.0 + 0.5;

    return samples;
}

/*
================
Synth_FloorTime

Returns t microseconds in output samples, rounded to the nearest and then down to a multiple of 16.
================
*/
s32 Synth_FloorTime( ALMicroTime t ) {
    f32 samples = (f32)t * snd_synth->outputRate / 1000000.0 + 0.5;

    return (s32)samples & ~15;
}
