/*
 * 1DA80.c -- n_audio: starting a voice.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynStartVoice

Queues the start of table on voice, if voice has a physical voice and an update is free.
================
*/
void n_alSynStartVoice( N_ALVoice *voice, ALWaveTable *table ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_START_VOICE;
            update->data.p = table;
            update->next = NULL;
            update->unityPitch = voice->unityPitch;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
