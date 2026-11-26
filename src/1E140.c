/*
 * 1E140.c -- n_audio: an effect bus's effect, and the main bus's frame mix.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_NewFx

Creates the effect of bus bus and returns it.
================
*/
ALFx *Synth_NewFx( s16 bus, ALSynConfig *c, ALHeap *hp ) {
    Synth_BuildFx( &snd_synth->auxBus->fxSlots[bus], c, hp );
    return snd_synth->auxBus->fxSlots[bus];
}

/*
================
Synth_PullMix

Clears the main bus's buffers, pulls the main bus into them, and mixes the effect bus's output in at full gain.
================
*/
Acmd *Synth_PullMix( s32 sampleOffset, Acmd *cmdList ) {
    Acmd *ptr = cmdList;

    aClearBuffer( ptr++, N_DMEM_MAIN_L, N_DMEM_FRAME_BYTES );
    ptr = snd_synth->mainBus->handler( sampleOffset, ptr );
    aMix( ptr++, 0, 0x7FFF, N_DMEM_AUX_L, N_DMEM_MAIN_L );
    aMix( ptr++, 0, 0x7FFF, N_DMEM_AUX_R, N_DMEM_MAIN_R );
    return ptr;
}
