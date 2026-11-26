/*
 * 1D940.c -- n_audio: a voice's pitch.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynSetPitch

Queues pitch as voice's pitch ratio, if voice has a physical voice and an update is free.
================
*/
void n_alSynSetPitch( N_ALVoice *voice, f32 pitch ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_PITCH;
            update->data.f = pitch;
            update->next = NULL;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
