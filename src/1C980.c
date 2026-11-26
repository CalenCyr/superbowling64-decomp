/*
 * 1C980.c -- n_audio: a physical voice's resampler.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_80041580

Makes one frame of pv's samples at its pitch ratio: decodes as many samples as the ratio needs and resamples
them, or, at unity pitch, decodes one frame and moves it to *outp.
================
*/
Acmd *func_80041580( N_PVoice *pv, s16 *outp, Acmd *p ) {
    s16 inp = N_DECODE_BUFFER;
    f32 finCount;
    s32 inCount;
    u16 ratio;

    if ( pv->unityPitch ) {
        p = func_80040E60( pv, &inp, N_FRAME_SAMPLES, p );
        aDMEMMove( p++, inp, *outp, N_FRAME_SAMPLES << 1 );
    } else {
        if ( pv->ratio > N_MAX_RATIO ) {
            pv->ratio = N_MAX_RATIO;
        }
        pv->ratio = (s32)( pv->ratio * N_UNITY_PITCH ) / (f32)N_UNITY_PITCH;
        finCount = pv->rsDelta + pv->ratio * N_FRAME_SAMPLES;
        inCount = (s32)finCount;
        pv->rsDelta = finCount - (f32)inCount;
        p = func_80040E60( pv, &inp, inCount, p );
        ratio = (s32)( pv->ratio * N_UNITY_PITCH );
        n_aResample( p++, pv->rsFirst, ratio, osVirtualToPhysical( pv->rsState ), inp );
        pv->rsFirst = FALSE;
    }
    return p;
}

/*
================
func_8004173C

The resampler's parameter handler: passes every message on to the decoder.
================
*/
s32 func_8004173C( N_PVoice *pv, s32 paramID, void *param ) {
    func_800412A8( pv, paramID, param );
    return 0;
}
