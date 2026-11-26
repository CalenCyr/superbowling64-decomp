/*
 * 1D9D0.c -- n_audio: a voice's volume.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynSetVol

Queues volume as voice's volume, reached over t microseconds, if voice has a physical voice and an update is
free.
================
*/
void n_alSynSetVol( N_ALVoice *voice, s16 volume, ALMicroTime t ) {
    ALParam *update;

    if ( voice->pvoice != NULL ) {
        update = Synth_AllocUpdate();
        if ( update != NULL ) {
            update->delta = snd_synth->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_VOLUME;
            update->data.i = volume;
            update->moredata = Synth_FloorTime( t );
            update->next = NULL;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
