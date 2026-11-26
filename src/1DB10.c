/*
 * 1DB10.c -- n_audio: stopping a voice.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynStopVoice

Queues the stop of voice, if voice has a physical voice and an update is free.
================
*/
void n_alSynStopVoice( N_ALVoice *voice ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_STOP_VOICE;
            update->next = NULL;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
