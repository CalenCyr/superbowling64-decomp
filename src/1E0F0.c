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
func_80042CF0

Mixes the frame through func_80042D90, interleaves the two channels and saves them to the output buffer.
================
*/
Acmd *func_80042CF0( s32 sampleOffset, Acmd *cmdList ) {
    Acmd *ptr = func_80042D90( sampleOffset, cmdList );

    aInterleaveFrame( ptr++ );
    aSaveFrame( ptr++, N_DMEM_FRAME_BYTES, D_8005D114->unk_48 );
    return ptr;
}
