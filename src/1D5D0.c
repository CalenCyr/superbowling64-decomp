/*
 * 1D5D0.c -- n_audio: registering a client player with the synthesizer.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
n_alSynAddPlayer

Puts client at the head of the synthesizer's client list, its first call due now, with interrupts off so that
the audio thread never sees the list half linked.
================
*/
void n_alSynAddPlayer( ALPlayer *client ) {
    OSIntMask mask;

    mask = osSetIntMask( OS_IM_NONE );
    client->samplesLeft = snd_synth->curSamples;
    client->next = snd_synth->head;
    snd_synth->head = client;
    osSetIntMask( mask );
}
