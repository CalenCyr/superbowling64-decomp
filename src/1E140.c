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
func_80042D40

Creates the effect of bus bus and returns it.
================
*/
ALFx *func_80042D40( s16 bus, ALSynConfig *c, ALHeap *hp ) {
    func_8003FF70( &D_8005D114->auxBus->fxSlots[bus], c, hp );
    return D_8005D114->auxBus->fxSlots[bus];
}

/*
================
func_80042D90

Clears the main bus's buffers, pulls the main bus into them, and mixes the effect bus's output in at full gain.
================
*/
Acmd *func_80042D90( s32 sampleOffset, Acmd *cmdList ) {
    Acmd *ptr = cmdList;

    aClearBuffer( ptr++, N_DMEM_MAIN_L, N_DMEM_FRAME_BYTES );
    ptr = D_8005D114->mainBus->handler( sampleOffset, ptr );
    aMix( ptr++, 0, 0x7FFF, N_DMEM_AUX_L, N_DMEM_MAIN_L );
    aMix( ptr++, 0, 0x7FFF, N_DMEM_AUX_R, N_DMEM_MAIN_R );
    return ptr;
}
