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
    if ( snd_globals == NULL ) {
        snd_globals = g;
        if ( snd_synth == NULL ) {
            Synth_UseCustomFx( c );
            snd_synth = synth_driver = &snd_globals->drvr;
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
        snd_synth->auxBus->fx = Synth_CreateFx( 0, c, hp );
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
Synth_CreateFx

Creates the effect of bus bus and returns it.
================
*/
ALFx *Synth_CreateFx( s16 bus, ALSynConfig *c, ALHeap *hp ) {
    Synth_AllocFx( &snd_synth->auxBus->fxSlots[bus], c, hp );
    return snd_synth->auxBus->fxSlots[bus];
}

/*
================
Synth_AllocFx

Allocates the effect into *fx, with storage for the largest preset in synth_fxPresets, and sets it up with the
preset in c.
================
*/
void Synth_AllocFx( ALFx **fx, ALSynConfig *c, ALHeap *hp ) {
    *fx = alHeapAlloc( hp, 1, sizeof( ALFx ) );
    ( *fx )->paramHdl = alFxParamHdl;
    synth_fxStorage.delays = alHeapAlloc( hp, Synth_MaxFxSections(), sizeof( ALDelay ) );
    synth_fxStorage.buffer = alHeapAlloc( hp, Synth_MaxFxLength(), sizeof( s16 ) );
    synth_fxStorage.resampler = alHeapAlloc( hp, 1, sizeof( ALResampler ) );
    synth_fxStorage.resamplerState = alHeapAlloc( hp, 1, sizeof( RESAMPLE_STATE ) );
    synth_fxStorage.lowPass = alHeapAlloc( hp, 1, sizeof( ALLowPass ) );
    synth_fxStorage.lowPassState = alHeapAlloc( hp, 1, sizeof( POLEF_STATE ) );
    Synth_SetupFx( c->params );
}

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

    r = synth_driver->auxBus->fxSlots[0];
    j = 0;
    r->section_count = param[j++];
    r->length = param[j++];
    r->delay = synth_fxStorage.delays;
    r->base = synth_fxStorage.buffer;
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
            d->rsinc = ( ( (f32)param[j++] / 1000 ) * N_FX_RANGE ) / synth_driver->outputRate;
            d->rsgain = ( (f32)param[j++] / N_FX_CONVERT ) * ( d->output - d->input );
            d->rsval = 1.0;
            d->rsdelta = 0.0;
            d->rs = synth_fxStorage.resampler;
            d->rs->state = synth_fxStorage.resamplerState;
            d->rs->delta = 0.0;
            d->rs->first = 1;
        } else {
            d->rs = NULL;
            j++;
            j++;
        }

        if ( param[j] ) {
            d->lp = synth_fxStorage.lowPass;
            d->lp->fstate = synth_fxStorage.lowPassState;
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
    c->params = synth_defaultFx;
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
    for ( i = 0; synth_fxPresets[i] != NULL; i++ ) {
        if ( most < synth_fxPresets[i][0] ) {
            most = synth_fxPresets[i][0];
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
    for ( i = 0; synth_fxPresets[i] != NULL; i++ ) {
        if ( longest < synth_fxPresets[i][1] ) {
            longest = synth_fxPresets[i][1];
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
    if ( index >= synth_numFxPresets ) {
        return 1;
    }
    Synth_SetupFx( synth_fxPresets[index] );
    alFxParam( synth_driver->auxBus->fx, AL_FILTER_SET_SOURCE, synth_driver->auxBus );
    return 0;
}
