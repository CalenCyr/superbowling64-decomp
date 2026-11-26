/*
 * 1B890.c -- n_audio: a physical voice's pull, parameter handler and envelope mixer.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/* ===== Data ===== */

static s16 snd_panGains[128] = {            /* gains by pan and by effect mix, 0x7FFF down; index 127 - x for the other side */
    0x7FFF, 0x7FFC, 0x7FF5, 0x7FE8, 0x7FD7, 0x7FC0, 0x7FA5, 0x7F84,
    0x7F5F, 0x7F34, 0x7F05, 0x7ED0, 0x7E97, 0x7E58, 0x7E15, 0x7DCD,
    0x7D7F, 0x7D2D, 0x7CD6, 0x7C7A, 0x7C1A, 0x7BB4, 0x7B49, 0x7ADA,
    0x7A66, 0x79ED, 0x796F, 0x78ED, 0x7866, 0x77DA, 0x7749, 0x76B4,
    0x761A, 0x757B, 0x74D8, 0x7430, 0x7384, 0x72D3, 0x721E, 0x7164,
    0x70A6, 0x6FE3, 0x6F1C, 0x6E51, 0x6D81, 0x6CAD, 0x6BD5, 0x6AF9,
    0x6A18, 0x6933, 0x684A, 0x675D, 0x666C, 0x6577, 0x647E, 0x6381,
    0x6280, 0x617C, 0x6073, 0x5F67, 0x5E57, 0x5D43, 0x5C2C, 0x5B11,
    0x59F2, 0x58D0, 0x57AA, 0x5681, 0x5555, 0x5425, 0x52F2, 0x51BC,
    0x5082, 0x4F46, 0x4E06, 0x4CC3, 0x4B7D, 0x4A35, 0x48E9, 0x479B,
    0x4649, 0x44F5, 0x439E, 0x4245, 0x40E9, 0x3F8A, 0x3E29, 0x3CC6,
    0x3B60, 0x39F8, 0x388D, 0x3721, 0x35B2, 0x3441, 0x32CE, 0x3159,
    0x2FE2, 0x2E69, 0x2CEF, 0x2B72, 0x29F4, 0x2875, 0x26F3, 0x2570,
    0x23EC, 0x2266, 0x20DF, 0x1F57, 0x1DCE, 0x1C43, 0x1AB7, 0x192A,
    0x179C, 0x160E, 0x147E, 0x12ED, 0x115C, 0x0FCA, 0x0E38, 0x0CA5,
    0x0B11, 0x097D, 0x07E9, 0x0654, 0x04C0, 0x032A, 0x0195, 0x0000,
};

/*
================
Synth_PullPVoice

Pulls one frame of the physical voice filter: mixes the samples up to each queued update due in this frame,
applies the update, and mixes the rest of the frame.
================
*/
Acmd *Synth_PullPVoice( void *filter, s32 sampleOffset, Acmd *p ) {
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
            Synth_DecoderParam( pv, N_FILTER_SET_WAVETABLE, start->wavetable );
            pv->motion = N_MOTION_PLAYING;
            pv->envFirst = TRUE;
            pv->envDelta = 0;
            pv->envLength = N_ROUND_FRAMES( start->envLength );
            pv->volume = ( start->volume * start->volume ) >> 15;
            pv->pan = start->pan;
            pv->dryAmt = snd_panGains[start->fxMix];
            pv->wetAmt = snd_panGains[127 - start->fxMix];
            if ( start->envLength ) {
                pv->volL = 1;
                pv->volR = 1;
            } else {
                pv->volL = ( pv->volume * snd_panGains[pv->pan] ) >> 15;
                pv->volR = ( pv->volume * snd_panGains[127 - pv->pan] ) >> 15;
            }
            pv->ratio = start->pitch;
            break;

        case N_PARAM_SET_VOLUME:
        case N_PARAM_SET_PAN:
        case N_PARAM_SET_FXMIX:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            if ( pv->envDelta >= pv->envLength ) {
                pv->targetL = ( pv->volume * snd_panGains[pv->pan] ) >> 15;
                pv->targetR = ( pv->volume * snd_panGains[127 - pv->pan] ) >> 15;
                pv->envDelta = pv->envLength;
                pv->volL = pv->targetL;
                pv->volR = pv->targetR;
            } else {
                pv->volL = Synth_RampVolume( pv->volL, pv->envDelta, pv->rateL, pv->rateLFrac );
                pv->volR = Synth_RampVolume( pv->volR, pv->envDelta, pv->rateR, pv->rateRFrac );
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
                pv->dryAmt = snd_panGains[pv->ctrlList->data.i];
                pv->wetAmt = snd_panGains[127 - pv->ctrlList->data.i];
            }
            pv->envFirst = TRUE;
            break;

        case N_PARAM_START_VOICE:
            update = pv->ctrlList;
            if ( update->unityPitch ) {
                pv->unityPitch = TRUE;
            }
            Synth_DecoderParam( pv, N_FILTER_SET_WAVETABLE, update->data.p );
            pv->motion = N_MOTION_PLAYING;
            break;

        case N_PARAM_STOP_VOICE:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            Synth_PVoiceParam( pv, N_FILTER_RESET, NULL );
            break;

        case N_PARAM_FREE_VOICE:
            update = pv->ctrlList;
            ( (N_PVoice *)update->data.p )->offset = 0;
            Synth_RetirePVoice( update->data.p );
            break;

        case N_PARAM_SET_PITCH:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            pv->ratio = pv->ctrlList->data.f;
            break;

        case N_PARAM_UNITY_PITCH:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            pv->unityPitch = TRUE;
            break;

        case N_PARAM_SET_WAVETABLE:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            Synth_DecoderParam( pv, N_FILTER_SET_WAVETABLE, pv->ctrlList->data.p );
            break;

        default:
            ptr = Synth_MixFrame( pv, &inp, &outp, samples, ptr );
            Synth_PVoiceParam( pv, pv->ctrlList->type, pv->ctrlList->data.p );
            break;
        }

        outCount -= samples;
        outp += samples << 1;
        done = pv->ctrlList;
        pv->ctrlList = done->next;
        if ( pv->ctrlList == NULL ) {
            pv->ctrlTail = NULL;
        }
        Synth_FreeUpdate( done );
    }

    ptr = Synth_MixFrame( pv, &inp, &outp, outCount, ptr );
    if ( pv->envDelta > pv->envLength ) {
        pv->envDelta = pv->envLength;
    }
    return ptr;
}

/*
================
Synth_PVoiceParam

A physical voice's parameter handler: queues updates, resets and starts the voice, and passes the rest on to the
resampler's chain (the decoder).
================
*/
s32 Synth_PVoiceParam( N_PVoice *pv, s32 paramID, void *param ) {
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
        Synth_DecoderParam( pv, paramID, param );
        break;
    case N_FILTER_START:
        pv->motion = N_MOTION_PLAYING;
        break;
    default:
        Synth_DecoderParam( pv, paramID, param );
        break;
    }
    return 0;
}

/*
================
Synth_MixFrame

Mixes one frame of the physical voice filter into the dry and wet mix: resamples it to *inp, sets the volume
ramps when they changed, and runs the envelope mixer. outp is not used. Nothing is made unless pv plays and outCount is not 0.
================
*/
Acmd *Synth_MixFrame( void *filter, s16 *inp, s16 *outp, s32 outCount, Acmd *p ) {
    Acmd *ptr = p;
    N_PVoice *pv = (N_PVoice *)filter;

    if ( pv->motion != N_MOTION_PLAYING || outCount == 0 ) {
        return ptr;
    }
    ptr = Synth_ResampleFrame( pv, inp, ptr );
    if ( pv->envFirst ) {
        pv->envFirst = FALSE;
        pv->targetL = ( pv->volume * snd_panGains[pv->pan] ) >> 15;
        pv->rateL = Synth_RampStep( pv->volL, pv->targetL, pv->envLength, &pv->rateLFrac );
        pv->targetR = ( pv->volume * snd_panGains[127 - pv->pan] ) >> 15;
        pv->rateR = Synth_RampStep( pv->volR, pv->targetR, pv->envLength, &pv->rateRFrac );
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
Synth_RampStep

Returns the whole part of the per-8-sample step that ramps a volume from vol to tgt over count samples, and puts
its fraction in *ratel. With no count the step jumps all the way, up or down.
================
*/
s16 Synth_RampStep( f64 vol, f64 tgt, s32 count, u16 *ratel ) {
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
Synth_RampVolume

Returns vol after count samples of the ramp with step ratem.ratel (per 8 samples).
================
*/
s16 Synth_RampVolume( s16 vol, s32 count, s16 ratem, u16 ratel ) {
    count >>= 3;
    if ( count == 0 ) {
        return vol;
    }
    return vol + ( ( ( ratel * count ) >> 16 ) + ratem * count );
}
