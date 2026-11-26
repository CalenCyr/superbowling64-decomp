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
func_80042150

Sets up the n_audio globals in g and the synthesizer from c, once.
================
*/
void func_80042150( N_ALGlobals *g, ALSynConfig *c ) {
    if ( D_8005D110 == NULL ) {
        D_8005D110 = g;
        if ( D_8005D114 == NULL ) {
            D_8005D114 = &D_8005D110->drvr;
            func_80042790( c );
        }
    }
}

/*
================
func_80042194

Takes the synthesizer down and clears the n_audio globals.
================
*/
void func_80042194( void ) {
    if ( D_8005D110 != NULL ) {
        Synth_Destroy();
        D_8005D110 = NULL;
        D_8005D114 = NULL;
    }
}
