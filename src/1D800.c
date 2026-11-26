/*
 * 1D800.c -- n_audio: the synthesizer's teardown, and a voice's effect mix.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_Destroy

Takes the synthesizer down for func_80042194: drops every client player.
================
*/
void Synth_Destroy( void ) {
    D_8005D114->head = NULL;
}

/*
================
n_alSynSetFXMix

Queues fxmix, at most 127, as voice's effect mix, if voice has a physical voice and an update is free.
================
*/
void n_alSynSetFXMix( N_ALVoice *voice, u8 fxmix ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_FXMIX;
            if ( fxmix > 127 ) {
                fxmix = 127;
            }
            update->data.i = fxmix;
            update->next = NULL;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
