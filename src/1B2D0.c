/*
 * 1B2D0.c -- n_audio: the effect bus's pull, the library's effect set-up, and a physical voice's set-up.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_8003FED0

The effect bus's pull: clears the dry and wet mix, then pulls one frame of each of the bus's physical voices.
================
*/
Acmd *func_8003FED0( s32 sampleOffset, Acmd *cmdList ) {
    N_ALAuxBus *bus = D_8005D114->auxBus;
    N_PVoice **sources = bus->sources;
    s32 i;

    aClearBuffer( cmdList++, N_DMEM_AUX_L, N_DMEM_FRAME_BYTES );
    for ( i = 0; i < bus->sourceCount; i++ ) {
        cmdList = func_80040490( sources[i], sampleOffset, cmdList );
    }
    return cmdList;
}

/*
================
func_8003FF70

Creates the effect of c's fxType into *fx, with its delay lines, buffer, modulation and low-pass on hp.
================
*/
void func_8003FF70( ALFx **fx, ALSynConfig *c, ALHeap *hp ) {
    ALFx *r;
    ALDelay *d;
    s32 *param;
    u16 i;
    u16 j;
    u16 n;

    r = alHeapAlloc( hp, 1, sizeof( ALFx ) );
    *fx = r;

    switch ( c->fxType ) {
    case AL_FX_SMALLROOM:
        param = D_8005CE70;
        break;
    case AL_FX_BIGROOM:
        param = D_8005CED8;
        break;
    case AL_FX_ECHO:
        param = D_8005CF60;
        break;
    case AL_FX_CHORUS:
        param = D_8005CF88;
        break;
    case AL_FX_FLANGE:
        param = D_8005CFB0;
        break;
    case AL_FX_CUSTOM:
        param = c->params;
        break;
    default:
        param = D_8005CFD8;
        break;
    }

    j = 0;
    r->section_count = param[j++];
    r->length = param[j++];
    r->delay = alHeapAlloc( hp, r->section_count, sizeof( ALDelay ) );
    r->base = alHeapAlloc( hp, r->length, sizeof( s16 ) );
    r->input = r->base;

    for ( n = 0; n < r->length; n++ ) {
        r->base[n] = 0;
    }

    for ( i = 0; i < r->section_count; i++ ) {
        d = &r->delay[i];
        d->input = param[j++];
        d->output = param[j++];
        d->fbcoef = param[j++];
        d->ffcoef = param[j++];
        d->gain = param[j++];

        if ( param[j] ) {
            d->rsinc = ( ( (f32)param[j++] / 1000 ) * N_FX_RANGE ) / c->outputRate;
            d->rsgain = ( (f32)param[j++] / N_FX_CONVERT ) * ( d->output - d->input );
            d->rsval = 1.0;
            d->rsdelta = 0.0;
            d->rs = alHeapAlloc( hp, 1, sizeof( ALResampler ) );
            d->rs->state = alHeapAlloc( hp, 1, sizeof( RESAMPLE_STATE ) );
            d->rs->delta = 0.0;
            d->rs->first = 1;
        } else {
            d->rs = NULL;
            j++;
            j++;
        }

        if ( param[j] ) {
            d->lp = alHeapAlloc( hp, 1, sizeof( ALLowPass ) );
            d->lp->fstate = alHeapAlloc( hp, 1, sizeof( POLEF_STATE ) );
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
func_8004035C

Sets up pv's decoder, resampler and envelope mixer, with their states on hp and a DMA routine from dmaNew.
================
*/
void func_8004035C( N_PVoice *pv, ALDMANew dmaNew, ALHeap *hp ) {
    pv->decState = alHeapAlloc( hp, 1, sizeof( ADPCM_STATE ) );
    pv->loopState = alHeapAlloc( hp, 1, sizeof( ADPCM_STATE ) );
    pv->dma = dmaNew( &pv->dmaState );
    pv->lastsam = 0;
    pv->first = TRUE;
    pv->memin = 0;

    pv->rsState = alHeapAlloc( hp, 1, sizeof( RESAMPLE_STATE ) );
    pv->rsDelta = 0.0f;
    pv->rsFirst = TRUE;
    pv->unityPitch = FALSE;
    pv->ratio = 1.0f;

    pv->emState = alHeapAlloc( hp, 1, sizeof( ENVMIX_STATE ) );
    pv->envFirst = TRUE;
    pv->motion = N_MOTION_STOPPED;
    pv->volume = 1;
    pv->targetL = 1;
    pv->targetR = 1;
    pv->volL = 1;
    pv->volR = 1;
    pv->dryAmt = 0;
    pv->wetAmt = 0;
    pv->rateL = 1;
    pv->rateLFrac = 0;
    /* R36: the right ramp's step was meant here; the left one is set twice, and the right one never */
    pv->rateL = 1;
    pv->rateLFrac = 0;
    pv->envDelta = 0;
    pv->envLength = 0;
    pv->pan = 0;
    pv->ctrlList = NULL;
    pv->ctrlTail = NULL;
}
