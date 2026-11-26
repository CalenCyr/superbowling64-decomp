/*
 * 1CB60.c -- n_audio: the effect, run over the effect bus each frame, and its parameter handler.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_80041760

The main bus's pull through the effect: pulls the effect bus, mixes its two channels into the left one and feeds
that into the effect's delay buffer, runs each delay line and mixes its output into the right channel, then copies
the right channel to the left.
================
*/
Acmd *func_80041760( s32 sampleOffset, Acmd *cmdList ) {
    Acmd *ptr;
    ALFx *fx = D_8005D114->auxBus->fx;
    ALDelay *delay;
    s16 *inPos;
    s16 *outPos;
    s16 *lastOut = NULL;
    s16 spareBuff = N_FX_BUFFER_A;
    s16 lineBuff = N_FX_BUFFER_B;
    s16 swap;
    s16 i;
    s16 outBuff = N_DMEM_AUX_R;   /* the effect's output: the effect bus's right channel */

    ptr = func_8003FED0( sampleOffset, cmdList );
    aMix( ptr++, 0, N_FX_INPUT_GAIN, N_DMEM_AUX_L, N_DMEM_AUX_L );
    aMix( ptr++, 0, N_FX_RIGHT_GAIN, outBuff, N_DMEM_AUX_L );
    ptr = func_80041F98( fx, fx->input, N_DMEM_AUX_L, ptr );
    aClearBuffer( ptr++, outBuff, N_FRAME_SAMPLES << 1 );

    for ( i = 0; i < fx->section_count; i++ ) {
        delay = &fx->delay[i];
        inPos = &fx->input[-delay->input];
        outPos = &fx->input[-delay->output];

        /* the line reads what the last one wrote: its frame is still in lineBuff */
        if ( inPos == lastOut ) {
            swap = lineBuff;
            lineBuff = spareBuff;
            spareBuff = swap;
        } else {
            ptr = func_80041E70( fx, inPos, spareBuff, N_FRAME_SAMPLES, ptr );
        }
        ptr = func_80041C60( fx, delay, lineBuff, ptr );

        if ( delay->ffcoef ) {
            aMix( ptr++, 0, (u16)delay->ffcoef, spareBuff, lineBuff );
            if ( !delay->rs && !delay->lp ) {
                ptr = func_80041F98( fx, outPos, lineBuff, ptr );
            }
        }
        if ( delay->fbcoef ) {
            aMix( ptr++, 0, (u16)delay->fbcoef, lineBuff, spareBuff );
            ptr = func_80041F98( fx, inPos, spareBuff, ptr );
        }
        if ( delay->lp ) {
            ptr = func_800420B0( delay->lp, lineBuff, ptr );
        }
        if ( !delay->rs ) {
            ptr = func_80041F98( fx, outPos, lineBuff, ptr );
        }
        if ( delay->gain ) {
            aMix( ptr++, 0, (u16)delay->gain, lineBuff, outBuff );
        }
        /* R36: the ROM keeps input + output here, not outPos (input - output), so the test above misses
           the last line's output */
        lastOut = &fx->input[delay->output];
    }

    fx->input += N_FRAME_SAMPLES;
    if ( fx->input > &fx->base[fx->length] ) {
        fx->input -= fx->length;
    }
    aDMEMMove( ptr++, outBuff, N_DMEM_AUX_L, N_FRAME_SAMPLES << 1 );
    return ptr;
}

/*
================
func_80041A70

The effect's parameter handler: paramID - 2 selects a delay line, eight parameters each; *param is the value.
================
*/
s32 func_80041A70( void *filter, s32 paramID, void *param ) {
    ALFx *fx = filter;
    s32 which = ( paramID - 2 ) % 8;
    s32 line = ( paramID - 2 ) / 8;
    s32 value = *(s32 *)param;

    switch ( which ) {
    case 0:
        fx->delay[line].input = value & ~7;
        break;
    case 1:
        fx->delay[line].output = value & ~7;
        break;
    case 3:
        fx->delay[line].ffcoef = value;
        break;
    case 2:
        fx->delay[line].fbcoef = value;
        break;
    case 4:
        fx->delay[line].gain = value;
        break;
    case 5:
        fx->delay[line].rsinc = ( (f32)value / 1000 ) * N_FX_RANGE / D_8005D114->outputRate;
        break;
    case 6:
        fx->delay[line].rsgain = ( (f32)value / N_FX_CONVERT ) * ( fx->delay[line].output - fx->delay[line].input );
        break;
    case 7:
        if ( fx->delay[line].lp ) {
            fx->delay[line].lp->fc = value;
            _init_lpfilter( fx->delay[line].lp );
        }
        break;
    }
    return 0;
}

/*
================
func_80041C60

Reads one frame of delay line delay's output into buff: resampled by its modulation, if it has one.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: the logic and the command words match; retail keeps count, delay and ptr in s2, s3, s4 (here s4,
// s2, s3), takes count - N_FRAME_SAMPLES into count's own register, and computes the resample's buff >> 8 first
Acmd *func_80041C60( ALFx *fx, ALDelay *delay, s32 buff, Acmd *p ) {
    s32 count = N_FRAME_SAMPLES;
    s32 length;
    f32 offset;
    f32 pitch;
    f32 inCountF;
    s32 misalign;
    s16 *inPos;
    u16 ratio;

    if ( delay->rs ) {
        length = delay->output - delay->input;
        offset = _doModFunc( delay, N_FRAME_SAMPLES ) / length;
        pitch = 1.0 - (s32)( offset * N_UNITY_PITCH ) / (f32)N_UNITY_PITCH;
        inCountF = delay->rs->delta + pitch * count;
        count = (s32)inCountF;
        delay->rs->delta = inCountF - (f32)count;

        inPos = &fx->input[delay->rsdelta - delay->output];
        misalign = ( (s32)inPos & 7 ) >> 1;
        inPos -= misalign;
        p = func_80041E70( fx, inPos, N_FX_RESAMPLE_BUFFER, count + misalign, p );

        ratio = (s32)( pitch * N_UNITY_PITCH );
        n_aResampleOut( p++, delay->rs->first, ratio, osVirtualToPhysical( delay->rs->state ),
                        N_FX_RESAMPLE_BUFFER + ( misalign << 1 ), buff );
        delay->rs->first = FALSE;
        delay->rsdelta = ( delay->rsdelta - N_FRAME_SAMPLES ) + count;
    } else {
        p = func_80041E70( fx, &fx->input[-delay->output], buff, N_FRAME_SAMPLES, p );
    }
    return p;
}
#else
INCLUDE_ASM("asm/nonmatchings/1CB60", func_80041C60);
#endif

/*
================
func_80041E70

Loads count samples of the effect's delay buffer from pos into buff, in two parts when they wrap around the
buffer's end.
================
*/
Acmd *func_80041E70( ALFx *fx, s16 *pos, s32 buff, s32 count, Acmd *p ) {
    Acmd *ptr = p;
    s16 *bufEnd = &fx->base[fx->length];
    s16 *endPos;
    s32 headCount;
    s32 tailCount;

    if ( pos < fx->base ) {
        pos += fx->length;
    }
    endPos = pos + count;
    if ( endPos > bufEnd ) {
        headCount = bufEnd - pos;
        tailCount = endPos - bufEnd;
        n_aLoadBuffer( ptr++, headCount << 1, buff, osVirtualToPhysical( pos ) );
        n_aLoadBuffer( ptr++, tailCount << 1, buff + ( headCount << 1 ), osVirtualToPhysical( fx->base ) );
    } else {
        n_aLoadBuffer( ptr++, count << 1, buff, osVirtualToPhysical( pos ) );
    }
    return ptr;
}

/*
================
func_80041F98

Saves one frame from buff into the effect's delay buffer at pos, in two parts when it wraps around the
buffer's end.
================
*/
Acmd *func_80041F98( ALFx *fx, s16 *pos, s32 buff, Acmd *p ) {
    Acmd *ptr = p;
    s16 *bufEnd = &fx->base[fx->length];
    s16 *endPos;
    s32 headCount;
    s32 tailCount;

    if ( pos < fx->base ) {
        pos += fx->length;
    }
    endPos = pos + N_FRAME_SAMPLES;
    if ( endPos > bufEnd ) {
        headCount = bufEnd - pos;
        tailCount = endPos - bufEnd;
        n_aSaveBuffer( ptr++, headCount << 1, buff, osVirtualToPhysical( pos ) );
        n_aSaveBuffer( ptr++, tailCount << 1, buff + ( headCount << 1 ), osVirtualToPhysical( fx->base ) );
    } else {
        n_aSaveBuffer( ptr++, N_FRAME_SAMPLES << 1, buff, osVirtualToPhysical( pos ) );
    }
    return ptr;
}

/*
================
func_800420B0

Runs low-pass lp over the frame in buff: loads its coefficients, then filters from its saved state.
================
*/
Acmd *func_800420B0( ALLowPass *lp, s32 buff, Acmd *p ) {
    Acmd *ptr = p;
    s32 page = buff >> 8;

    aLoadADPCM( ptr++, 32, osVirtualToPhysical( lp->fcvec.fccoef ) );
    n_aPoleFilter( ptr++, lp->first, lp->fgain, page, osVirtualToPhysical( lp->fstate ) );
    lp->first = FALSE;
    return ptr;
}
