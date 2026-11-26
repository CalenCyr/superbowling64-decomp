/*
 * 1D8B0.c -- n_audio: a voice's pan.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynSetPan

Queues pan as voice's pan, if voice has a physical voice and an update is free.
================
*/
void n_alSynSetPan( N_ALVoice *voice, u8 pan ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_PAN;
            update->data.i = pan;
            update->next = NULL;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
