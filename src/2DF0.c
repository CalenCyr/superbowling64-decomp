/* 2DF0.c -- controllers: set-up, Rumble Paks, and the per-frame pad state */

#include "common.h"
#include "libultra.h"
#include "input.h"

/*
================
func_800279F0

Sets up the controllers and the Rumble Paks of both ports.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R17): byte-exact only with the unused local pads, which supplies retail's 0x18
// bytes of unused frame; without it the frame is 0x18 instead of 0x30
void func_800279F0( void ) {
    OSContPad pads[MAXCONTROLLERS];

    osCreateMesgQueue( &D_80066B38, &D_800E4FCC, 1 );
    osSetEventMesg( OS_EVENT_SI, &D_80066B38, (OSMesg)D_800619B8 );
    osContInit( &D_80066B38, &D_8005B190, D_800F7BE8 );
    osContSetCh( NUM_PLAYERS );
    D_800E4FD0[1] = FALSE;
    D_800E4FD0[0] = FALSE;
    func_80027A7C( 0 );
    func_80027A7C( 1 );
}
#else
INCLUDE_ASM("asm/nonmatchings/2DF0", func_800279F0);
#endif

/*
================
func_80027A7C

Looks for a Rumble Pak on port chan, and stops it if there is one.
================
*/
void func_80027A7C( s32 chan ) {
    if ( osMotorInit( &D_80066B38, &D_800E6400[chan], chan ) == 0 ) {
        D_800E4FD0[chan] = TRUE;
        func_80027B6C( chan );
    } else {
        D_800E4FD0[chan] = FALSE;
    }
    D_80066C24[chan] = 0;
    D_800DF680[chan] = 0;
}

/*
================
func_80027B18

Starts the Rumble Pak of port chan, if it has one.
================
*/
void func_80027B18( s32 chan ) {
    if ( D_800E4FD0[chan] == TRUE ) {
        osMotorStart( &D_800E6400[chan] );
    }
}

/*
================
func_80027B6C

Stops the Rumble Pak of port chan, if it has one.
================
*/
void func_80027B6C( s32 chan ) {
    if ( D_800E4FD0[chan] == TRUE ) {
        osMotorStop( &D_800E6400[chan] );
    }
}

INCLUDE_ASM("asm/nonmatchings/2DF0", func_80027BC0);

/*
================
func_80027FF0

Sets a bit of D_8005B190 for each port that holds a standard controller.
================
*/
void func_80027FF0( void ) {
    s32 i;

    D_8005B190 = 0;
    for ( i = 0; i < NUM_PLAYERS; i++ ) {
        if ( D_800F7BE8[i].errno == 0 && ( D_800F7BE8[i].type & CONT_TYPE_MASK ) == CONT_TYPE_NORMAL ) {
            D_8005B190 |= 1 << i;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/2DF0", func_80028064);
