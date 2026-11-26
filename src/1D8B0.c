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
        update = Synth_AllocUpdate();
        if ( update != NULL ) {
            update->delta = snd_synth->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_PAN;
            update->data.i = pan;
            update->next = NULL;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
