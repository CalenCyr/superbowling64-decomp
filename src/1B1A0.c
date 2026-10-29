/*
 * 1B1A0.c -- audio frame sizing: how many samples each audio frame synthesizes, to keep the AI queue steady.
 */

/* This unit has no INCLUDE_ASM: no asm prelude, so its first function keeps the original hazard nop. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Snd_InitFrameSizes

Sets the audio frame sizes for ticks ticks of tickRate per second at outputRate: snd_frameSize is the samples
of those ticks, rounded up past a multiple of AUDIO_FRAME_SAMPLES, with one AUDIO_FRAME_SAMPLES shorter and
longer sizes for Snd_NextFrameSize and extraPercent percent of a frame as the target queue. Returns the largest
frame plus the queue: the samples an output buffer must hold.
================
*/
u32 Snd_InitFrameSizes( u32 ticks, u32 outputRate, u32 tickRate, u32 extraPercent ) {
    u32 samples;

    samples = ( ticks * outputRate + tickRate - 1 ) / tickRate;
    snd_frameSize = ( samples / AUDIO_FRAME_SAMPLES + 1 ) * AUDIO_FRAME_SAMPLES;
    snd_minFrameSize = snd_frameSize - AUDIO_FRAME_SAMPLES;
    snd_maxFrameSize = snd_frameSize + AUDIO_FRAME_SAMPLES;
    snd_extraSamples = snd_frameSize * extraPercent / 100;
    return snd_frameSize + AUDIO_FRAME_SAMPLES + snd_extraSamples;
}

/*
================
Snd_NextFrameSize

Returns the samples of the next audio frame, given queued samples still waiting in the AI: one short frame if
the queue has grown past its target by a frame, one long frame if it has drained below it, else the usual size.
A short or long frame is only made once the queue has been back in range since the last one.
================
*/
u32 Snd_NextFrameSize( u32 queued ) {
    if ( queued > snd_extraSamples + AUDIO_FRAME_SAMPLES ) {
        if ( snd_frameSettled ) {
            snd_frameSettled = 0;
            return snd_minFrameSize;
        }
    } else if ( queued < snd_extraSamples ) {
        if ( snd_frameSettled ) {
            snd_frameSettled = 0;
            return snd_maxFrameSize;
        }
    } else {
        snd_frameSettled = 1;
    }
    return snd_frameSize;
}
