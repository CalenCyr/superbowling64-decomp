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
        update = Synth_AllocUpdate();
        if ( update != NULL ) {
            update->delta = snd_synth->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_START_VOICE;
            update->data.p = table;
            update->next = NULL;
            update->unityPitch = voice->unityPitch;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
