/*
 * 1D550.c -- n_audio: setting up and taking down the library.
 */

/* This unit has no INCLUDE_ASM: no asm prelude. */
#define NO_INCLUDE_ASM

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_InitGlobals

Sets up the n_audio globals in g and the synthesizer from c, once.
================
*/
void Synth_InitGlobals( N_ALGlobals *g, ALSynConfig *c ) {
    if ( snd_globals == NULL ) {
        snd_globals = g;
        if ( snd_synth == NULL ) {
            snd_synth = &snd_globals->drvr;
            Synth_BuildSynth( c );
        }
    }
}

/*
================
Synth_Teardown

Takes the synthesizer down and clears the n_audio globals.
================
*/
void Synth_Teardown( void ) {
    if ( snd_globals != NULL ) {
        Synth_Destroy();
        snd_globals = NULL;
        snd_synth = NULL;
    }
}
