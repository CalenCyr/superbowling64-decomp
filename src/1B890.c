/*
 * 1B890.c -- n_audio: a physical voice's pull, parameter handler and envelope mixer.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_80040490

Pulls one frame of the physical voice filter: mixes the samples up to each queued update due in this frame,
applies the update, and mixes the rest of the frame.
================
*/
Acmd *func_80040490( void *filter, s32 sampleOffset, Acmd *p ) {
    N_PVoice *pv = (N_PVoice *)filter;
    Acmd *ptr = p;
    s32 offset = sampleOffset;          /* the sample the next update falls on */
    s16 outp = 0;
    s16 inp = 0;
    s32 outCount = N_FRAME_SAMPLES;
    s32 lastOffset;
    s32 samples;
    N_ALStartParamAlt *start;
    ALParam *update;                    /* the update a case applies */
    ALParam *done;                      /* the update applied, back to the free list */

    while ( pv->ctrlList != NULL ) {
        lastOffset = offset;
        offset = pv->ctrlList->delta;
        samples = N_ROUND_FRAMES( offset - lastOffset );
        if ( samples == 0 ) {
            offset = lastOffset;
        }
        if ( samples > outCount ) {
            break;
        }

        switch ( pv->ctrlList->type ) {
        case N_PARAM_START_VOICE_ALT:
            start = (N_ALStartParamAlt *)pv->ctrlList;
            if ( start->unityPitch ) {
                pv->unityPitch = TRUE;
            }
            func_800412A8( pv, N_FILTER_SET_WAVETABLE, start->wavetable );
            pv->motion = N_MOTION_PLAYING;
            pv->envFirst = TRUE;
            pv->envDelta = 0;
            pv->envLength = N_ROUND_FRAMES( start->envLength );
            pv->volume = ( start->volume * start->volume ) >> 15;
            pv->pan = start->pan;
            pv->dryAmt = D_8005D000[start->fxMix];
            pv->wetAmt = D_8005D000[127 - start->fxMix];
            if ( start->envLength ) {
                pv->volL = 1;
                pv->volR = 1;
            } else {
                pv->volL = ( pv->volume * D_8005D000[pv->pan] ) >> 15;
                pv->volR = ( pv->volume * D_8005D000[127 - pv->pan] ) >> 15;
            }
            pv->ratio = start->pitch;
            break;

        case N_PARAM_SET_VOLUME:
        case N_PARAM_SET_PAN:
        case N_PARAM_SET_FXMIX:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            if ( pv->envDelta >= pv->envLength ) {
                pv->targetL = ( pv->volume * D_8005D000[pv->pan] ) >> 15;
                pv->targetR = ( pv->volume * D_8005D000[127 - pv->pan] ) >> 15;
                pv->envDelta = pv->envLength;
                pv->volL = pv->targetL;
                pv->volR = pv->targetR;
            } else {
                pv->volL = func_80040E0C( pv->volL, pv->envDelta, pv->rateL, pv->rateLFrac );
                pv->volR = func_80040E0C( pv->volR, pv->envDelta, pv->rateR, pv->rateRFrac );
            }
            if ( pv->volL == 0 ) {
                pv->volL = 1;
            }
            if ( pv->volR == 0 ) {
                pv->volR = 1;
            }
            if ( pv->ctrlList->type == N_PARAM_SET_PAN ) {
                pv->pan = pv->ctrlList->data.i;
            }
            if ( pv->ctrlList->type == N_PARAM_SET_VOLUME ) {
                pv->envDelta = 0;
                pv->volume = ( pv->ctrlList->data.i * pv->ctrlList->data.i ) >> 15;
                pv->envLength = N_ROUND_FRAMES( pv->ctrlList->moredata );
            }
            if ( pv->ctrlList->type == N_PARAM_SET_FXMIX ) {
                pv->dryAmt = D_8005D000[pv->ctrlList->data.i];
                pv->wetAmt = D_8005D000[127 - pv->ctrlList->data.i];
            }
            pv->envFirst = TRUE;
            break;

        case N_PARAM_START_VOICE:
            update = pv->ctrlList;
            if ( update->unityPitch ) {
                pv->unityPitch = TRUE;
            }
            func_800412A8( pv, N_FILTER_SET_WAVETABLE, update->data.p );
            pv->motion = N_MOTION_PLAYING;
            break;

        case N_PARAM_STOP_VOICE:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            func_80040A08( pv, N_FILTER_RESET, NULL );
            break;

        case N_PARAM_FREE_VOICE:
            update = pv->ctrlList;
            ( (N_PVoice *)update->data.p )->offset = 0;
            func_80042C0C( update->data.p );
            break;

        case N_PARAM_SET_PITCH:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            pv->ratio = pv->ctrlList->data.f;
            break;

        case N_PARAM_UNITY_PITCH:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            pv->unityPitch = TRUE;
            break;

        case N_PARAM_SET_WAVETABLE:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            func_800412A8( pv, N_FILTER_SET_WAVETABLE, pv->ctrlList->data.p );
            break;

        default:
            ptr = func_80040AAC( pv, &inp, &outp, samples, ptr );
            func_80040A08( pv, pv->ctrlList->type, pv->ctrlList->data.p );
            break;
        }

        outCount -= samples;
        outp += samples << 1;
        done = pv->ctrlList;
        pv->ctrlList = done->next;
        if ( pv->ctrlList == NULL ) {
            pv->ctrlTail = NULL;
        }
        func_80042B94( done );
    }

    ptr = func_80040AAC( pv, &inp, &outp, outCount, ptr );
    if ( pv->envDelta > pv->envLength ) {
        pv->envDelta = pv->envLength;
    }
    return ptr;
}

/*
================
func_80040A08

A physical voice's parameter handler: queues updates, resets and starts the voice, and passes the rest on to the
resampler's chain (the decoder).
================
*/
s32 func_80040A08( N_PVoice *pv, s32 paramID, void *param ) {
    switch ( paramID ) {
    case AL_FILTER_ADD_UPDATE:
        if ( pv->ctrlTail != NULL ) {
            pv->ctrlTail->next = param;
        } else {
            pv->ctrlList = param;
        }
        pv->ctrlTail = param;
        break;
    case N_FILTER_RESET:
        pv->envFirst = TRUE;
        pv->motion = N_MOTION_STOPPED;
        pv->volume = 1;
        pv->envLength = 0;
        pv->rsDelta = 0.0f;
        pv->rsFirst = TRUE;
        pv->unityPitch = FALSE;
        func_800412A8( pv, paramID, param );
        break;
    case N_FILTER_START:
        pv->motion = N_MOTION_PLAYING;
        break;
    default:
        func_800412A8( pv, paramID, param );
        break;
    }
    return 0;
}

/*
================
func_80040AAC

Mixes one frame of the physical voice filter into the dry and wet mix: resamples it to *inp, sets the volume
ramps when they changed, and runs the envelope mixer. outp is not used. Nothing is made unless pv plays and outCount is not 0.
================
*/
Acmd *func_80040AAC( void *filter, s16 *inp, s16 *outp, s32 outCount, Acmd *p ) {
    Acmd *ptr = p;
    N_PVoice *pv = (N_PVoice *)filter;

    if ( pv->motion != N_MOTION_PLAYING || outCount == 0 ) {
        return ptr;
    }
    ptr = func_80041580( pv, inp, ptr );
    if ( pv->envFirst ) {
        pv->envFirst = FALSE;
        pv->targetL = ( pv->volume * D_8005D000[pv->pan] ) >> 15;
        pv->rateL = func_80040CCC( pv->volL, pv->targetL, pv->envLength, &pv->rateLFrac );
        pv->targetR = ( pv->volume * D_8005D000[127 - pv->pan] ) >> 15;
        pv->rateR = func_80040CCC( pv->volR, pv->targetR, pv->envLength, &pv->rateRFrac );
        aSetVolume( ptr++, A_RATE, pv->targetL, pv->rateL, pv->rateLFrac );
        aSetVolume( ptr++, A_VOL | A_LEFT, pv->volL, pv->dryAmt, pv->wetAmt );
        aSetVolume( ptr++, A_VOL | A_RIGHT, pv->targetR, pv->rateR, pv->rateRFrac );
        n_aEnvMixer( ptr++, A_INIT, pv->volR, osVirtualToPhysical( pv->emState ) );
    } else {
        n_aEnvMixer( ptr++, A_CONTINUE, 0, osVirtualToPhysical( pv->emState ) );
    }
    *inp += N_FRAME_SAMPLES << 1;
    pv->envDelta += N_FRAME_SAMPLES;
    return ptr;
}

/*
================
func_80040CCC

Returns the whole part of the per-8-sample step that ramps a volume from vol to tgt over count samples, and puts
its fraction in *ratel. With no count the step jumps all the way, up or down.
================
*/
s16 func_80040CCC( f64 vol, f64 tgt, s32 count, u16 *ratel ) {
    s16 s;                              /* the whole part of the step */
    s16 c;                              /* the carry out of the borrowed fraction */
    f64 invn;
    f64 step;                           /* the step per 8 samples */
    f64 frac;                           /* its fraction, plus the one borrowed from s */
    f64 low;                            /* the fraction in 16 bits */

    if ( count == 0 ) {
        if ( tgt >= vol ) {
            *ratel = 0xFFFF;
            return 0x7FFF;
        } else {
            *ratel = 0;
            return -0x8000;
        }
    }
    invn = 1.0 / count;
    if ( tgt < 1.0 ) {
        tgt = 1.0;
    }
    if ( vol <= 0.0 ) {
        vol = 1.0;
    }
    step = ( tgt - vol ) * invn * 8.0;
    s = (s16)step;
    /* borrow one so the fraction is positive, and give back its carry */
    frac = ( step - s ) + 1.0;
    s--;
    c = (s16)frac;
    low = ( frac - c ) * 65535.0;
    s += c;
    *ratel = (u16)low;
    return s;
}

/*
================
func_80040E0C

Returns vol after count samples of the ramp with step ratem.ratel (per 8 samples).
================
*/
s16 func_80040E0C( s16 vol, s32 count, s16 ratem, u16 ratel ) {
    count >>= 3;
    if ( count == 0 ) {
        return vol;
    }
    return vol + ( ( ( ratel * count ) >> 16 ) + ratem * count );
}
