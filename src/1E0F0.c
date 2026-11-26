/*
 * 1E0F0.c -- n_audio: one frame's output.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_PullOutput

Mixes the frame through Synth_PullMix, interleaves the two channels and saves them to the output buffer.
================
*/
Acmd *Synth_PullOutput( s32 sampleOffset, Acmd *cmdList ) {
    Acmd *ptr = Synth_PullMix( sampleOffset, cmdList );

    aInterleaveFrame( ptr++ );
    aSaveFrame( ptr++, N_DMEM_FRAME_BYTES, snd_synth->outBuf );
    return ptr;
}
