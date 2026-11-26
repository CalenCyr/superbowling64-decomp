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
        update = Synth_AllocUpdate();
        if ( update != NULL ) {
            update->delta = snd_synth->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_PITCH;
            update->data.f = pitch;
            update->next = NULL;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
