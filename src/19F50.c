/*
 * 19F50.c -- synthesizer and effect setup: the game's own start-up of the n_audio synthesizer, with an effect
 * whose storage is sized for the largest of its presets so that the preset can change while sound plays.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_Init

Sets up the n_audio globals in g and the synthesizer from c, once.
================
*/
void Synth_Init( N_ALGlobals *g, ALSynConfig *c ) {
    if ( D_8005D110 == NULL ) {
        D_8005D110 = g;
        if ( D_8005D114 == NULL ) {
            Synth_UseCustomFx( c );
            D_8005D114 = D_800EA098 = &D_8005D110->drvr;
            Synth_Create( c );
        }
    }
}

/*
================
Synth_Create

Builds the synthesizer from c on c's heap: the effect and main buses, the effect if c has one, the physical
voices on the free list, and the free parameter updates.
================
*/
void Synth_Create( ALSynConfig *c ) {
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
    D_8005D114->unk_48 = 0;

    D_8005D114->auxBus = alHeapAlloc( hp, 1, sizeof( N_ALAuxBus ) );
    D_8005D114->auxBus->sourceCount = 0;
    D_8005D114->auxBus->maxSources = c->maxPVoices;
    D_8005D114->auxBus->sources = alHeapAlloc( hp, c->maxPVoices, sizeof( N_PVoice * ) );

    D_8005D114->mainBus = alHeapAlloc( hp, 1, sizeof( N_ALMainBus ) );
    if ( c->fxType != AL_FX_NONE ) {
        D_8005D114->auxBus->fx = Synth_CreateFx( 0, c, hp );
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
Synth_CreateFx

Creates the effect of bus bus and returns it.
================
*/
ALFx *Synth_CreateFx( s16 bus, ALSynConfig *c, ALHeap *hp ) {
    Synth_AllocFx( &D_8005D114->auxBus->fxSlots[bus], c, hp );
    return D_8005D114->auxBus->fxSlots[bus];
}

/*
================
Synth_AllocFx

Allocates the effect into *fx, with storage for the largest preset in D_8005CE24, and sets it up with the
preset in c.
================
*/
void Synth_AllocFx( ALFx **fx, ALSynConfig *c, ALHeap *hp ) {
    *fx = alHeapAlloc( hp, 1, sizeof( ALFx ) );
    ( *fx )->paramHdl = alFxParamHdl;
    D_800EA0A8.delays = alHeapAlloc( hp, Synth_MaxFxSections(), sizeof( ALDelay ) );
    D_800EA0A8.buffer = alHeapAlloc( hp, Synth_MaxFxLength(), sizeof( s16 ) );
    D_800EA0A8.resampler = alHeapAlloc( hp, 1, sizeof( ALResampler ) );
    D_800EA0A8.resamplerState = alHeapAlloc( hp, 1, sizeof( RESAMPLE_STATE ) );
    D_800EA0A8.lowPass = alHeapAlloc( hp, 1, sizeof( ALLowPass ) );
    D_800EA0A8.lowPassState = alHeapAlloc( hp, 1, sizeof( POLEF_STATE ) );
    Synth_SetupFx( c->params );
}

/* modulation rate range, and hundredths of a cent per natural-log unit of pitch ratio (120000 / ln 2) */
#define RANGE   2.0
#define CONVERT 173123.404906676

/*
================
Synth_SetupFx

Sets the effect up from preset param: the section count and buffer length, then for each section its delay
line, coefficients, and the optional modulation and low-pass filter.
================
*/
void Synth_SetupFx( s32 *param ) {
    ALFx *r;
    ALDelay *d;
    u16 i;
    u16 j;

    r = D_800EA098->auxBus->fxSlots[0];
    j = 0;
    r->section_count = param[j++];
    r->length = param[j++];
    r->delay = D_800EA0A8.delays;
    r->base = D_800EA0A8.buffer;
    r->input = r->base;

    for ( i = 0; i < r->length; i++ ) {
        r->base[i] = 0;
    }

    for ( i = 0; i < r->section_count; i++ ) {
        d = &r->delay[i];
        d->input = param[j++];
        d->output = param[j++];
        d->fbcoef = param[j++];
        d->ffcoef = param[j++];
        d->gain = param[j++];

        if ( param[j] ) {
            d->rsinc = ( ( (f32)param[j++] / 1000 ) * RANGE ) / D_800EA098->outputRate;
            d->rsgain = ( (f32)param[j++] / CONVERT ) * ( d->output - d->input );
            d->rsval = 1.0;
            d->rsdelta = 0.0;
            d->rs = D_800EA0A8.resampler;
            d->rs->state = D_800EA0A8.resamplerState;
            d->rs->delta = 0.0;
            d->rs->first = 1;
        } else {
            d->rs = NULL;
            j++;
            j++;
        }

        if ( param[j] ) {
            d->lp = D_800EA0A8.lowPass;
            d->lp->fstate = D_800EA0A8.lowPassState;
            d->lp->fc = param[j++];
            _init_lpfilter( d->lp );
        } else {
            d->lp = NULL;
            j++;
        }
    }
}

/*
================
Synth_UseCustomFx

Makes c's effect the custom one, with the default preset.
================
*/
void Synth_UseCustomFx( ALSynConfig *c ) {
    c->fxType = AL_FX_CUSTOM;
    c->params = D_8005CC98;
}

/*
================
Synth_MaxFxSections

Returns the most sections of any preset.
================
*/
s32 Synth_MaxFxSections( void ) {
    s32 i;
    s32 most;

    most = 0;
    for ( i = 0; D_8005CE24[i] != NULL; i++ ) {
        if ( most < D_8005CE24[i][0] ) {
            most = D_8005CE24[i][0];
        }
    }
    return most;
}

/*
================
Synth_MaxFxLength

Returns the longest buffer of any preset.
================
*/
s32 Synth_MaxFxLength( void ) {
    s32 i;
    s32 longest;

    longest = 0;
    for ( i = 0; D_8005CE24[i] != NULL; i++ ) {
        if ( longest < D_8005CE24[i][1] ) {
            longest = D_8005CE24[i][1];
        }
    }
    return longest;
}

/*
================
Synth_SetFxPreset

Switches the effect to preset index; returns 0, or 1 if there is no such preset.
================
*/
s32 Synth_SetFxPreset( s32 index ) {
    if ( index >= D_8005CE20 ) {
        return 1;
    }
    Synth_SetupFx( D_8005CE24[index] );
    alFxParam( D_800EA098->auxBus->fx, AL_FILTER_SET_SOURCE, D_800EA098->auxBus );
    return 0;
}
