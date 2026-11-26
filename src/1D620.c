/*
 * 1D620.c -- n_audio: allocating a voice its physical voice, stealing a lower-priority one when none is free.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynAllocVoice

Sets voice up from vc and gives it a physical voice. A stolen physical voice first fades its old voice out
over N_PVOICE_STEAL_FADE samples and stops it; its updates for voice wait N_PVOICE_STEAL_DELAY samples
until then. Returns whether voice got a physical voice.
================
*/
s32 n_alSynAllocVoice( N_ALVoice *voice, ALVoiceConfig *vc ) {
    N_PVoice *pv = NULL;
    ALParam *update;
    s32 stolen;

    voice->priority = vc->priority;
    voice->unityPitch = vc->unityPitch;
    voice->table = NULL;
    voice->fxBus = vc->fxBus;
    voice->state = AL_STOPPED;
    voice->pvoice = NULL;

    stolen = Synth_AllocPVoice( &pv, vc->priority );
    if ( pv != NULL ) {
        if ( stolen ) {
            pv->offset = N_PVOICE_STEAL_DELAY;
            pv->vvoice->pvoice = NULL;
            pv->vvoice = voice;
            voice->pvoice = pv;

            /* R36: unlike the stop below, this update is used without a check for none left */
            update = Synth_AllocUpdate();
            update->delta = snd_synth->paramSamples;
            update->type = N_PARAM_SET_VOLUME;
            update->data.i = 0;
            update->moredata = N_PVOICE_STEAL_FADE;
            Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );

            update = Synth_AllocUpdate();
            if ( update != NULL ) {
                update->delta = snd_synth->paramSamples + pv->offset;
                update->type = N_PARAM_STOP_VOICE;
                update->next = NULL;
                Synth_PVoiceParam( voice->pvoice, AL_FILTER_ADD_UPDATE, update );
            }
        } else {
            pv->offset = 0;
            pv->vvoice = voice;
            voice->pvoice = pv;
        }
    }
    return pv != NULL;
}

/*
================
Synth_AllocPVoice

Takes a physical voice for a voice of the given priority onto the allocated list: one waiting to be freed,
else a free one. With none, picks the allocated one with the lowest priority not above priority (the last of
equals) that is not already being stolen, and leaves it where it is. Sets *pvoice to it, left alone when there
is none. Returns whether it must be stolen.
================
*/
s32 Synth_AllocPVoice( N_PVoice **pvoice, s16 priority ) {
    N_PVoice *pv;
    s32 stolen = FALSE;

    if ( ( pv = (N_PVoice *)snd_synth->pLameList.next ) != NULL
        || ( pv = (N_PVoice *)snd_synth->pFreeList.next ) != NULL ) {
        *pvoice = pv;
        alUnlink( &pv->node );
        alLink( &pv->node, &snd_synth->pAllocList );
    } else {
        for ( pv = (N_PVoice *)snd_synth->pAllocList.next; pv != NULL; pv = (N_PVoice *)pv->node.next ) {
            if ( pv->vvoice->priority <= priority && pv->offset == 0 ) {
                *pvoice = pv;
                priority = pv->vvoice->priority;
                stolen = TRUE;
            }
        }
    }
    return stolen;
}
