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

Takes the synthesizer down for Synth_Teardown: drops every client player.
================
*/
void Synth_Destroy( void ) {
    snd_synth->head = NULL;
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
        update = Synth_AllocUpdate();
        if ( update != NULL ) {
            update->delta = snd_synth->paramSamples + voice->pvoice->offset;
            update->type = N_PARAM_SET_FXMIX;
            if ( fxmix > 127 ) {
                fxmix = 127;
            }
            update->data.i = fxmix;
            update->next = NULL;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
        }
    }
}
