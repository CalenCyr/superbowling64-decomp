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
        update = func_80042B68();
        if ( update != NULL ) {
            update->delta = D_8005D114->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_VOLUME;
            update->data.i = volume;
            update->moredata = func_80042C94( t );
            update->next = NULL;
            func_80040A08( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
