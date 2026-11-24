/* 2DF0.c -- controllers: set-up, Rumble Paks, and the per-frame pad state */

#include "common.h"
#include "libultra.h"
#include "input.h"
#include "gamesnd.h"

#define STICK_DEADZONE      6       /* stick values up to this far from 0 count as 0 */
#define STICK_DPAD          30      /* stick values beyond this count as a D-pad direction */
#define DPAD_BUTTONS        ( U_JPAD | L_JPAD | R_JPAD | D_JPAD )
#define REPEAT_DELAY        10      /* frames a direction is held before it repeats */

/*
================
Cont_Init

Sets up the controllers and the Rumble Paks of both ports.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R17): byte-exact only with the unused local pads, which supplies retail's 0x18
// bytes of unused frame; without it the frame is 0x18 instead of 0x30
void Cont_Init( void ) {
    OSContPad pads[MAXCONTROLLERS];

    osCreateMesgQueue( &cont_siQueue, &cont_siMsgs, 1 );
    osSetEventMesg( OS_EVENT_SI, &cont_siQueue, (OSMesg)D_800619B8 );
    osContInit( &cont_siQueue, &cont_standardMask, cont_status );
    osContSetCh( NUM_PLAYERS );
    cont_hasRumble[1] = FALSE;
    cont_hasRumble[0] = FALSE;
    Rumble_Probe( 0 );
    Rumble_Probe( 1 );
}
#else
INCLUDE_ASM("asm/nonmatchings/2DF0", Cont_Init);
#endif

/*
================
Rumble_Probe

Looks for a Rumble Pak on port chan, and stops it if there is one.
================
*/
void Rumble_Probe( s32 chan ) {
    if ( osMotorInit( &cont_siQueue, &cont_rumblePfs[chan], chan ) == 0 ) {
        cont_hasRumble[chan] = TRUE;
        Rumble_Stop( chan );
    } else {
        cont_hasRumble[chan] = FALSE;
    }
    D_80066C24[chan] = 0;
    cont_rumble[chan] = 0;
}

/*
================
Rumble_Start

Starts the Rumble Pak of port chan, if it has one.
================
*/
void Rumble_Start( s32 chan ) {
    if ( cont_hasRumble[chan] == TRUE ) {
        osMotorStart( &cont_rumblePfs[chan] );
    }
}

/*
================
Rumble_Stop

Stops the Rumble Pak of port chan, if it has one.
================
*/
void Rumble_Stop( s32 chan ) {
    if ( cont_hasRumble[chan] == TRUE ) {
        osMotorStop( &cont_rumblePfs[chan] );
    }
}

/*
================
Cont_ReadPads

Reads the pads of both ports into the per-port state: buttons held and pressed, the stick outside its dead
zone and as D-pad bits, and D-pad presses that repeat every 4 frames once held for REPEAT_DELAY; then merges
the two ports.
================
*/
void Cont_ReadPads( void ) {
    s32 i;
    OSContPad *pad;

    for ( i = 0; i < NUM_PLAYERS; i++ ) {
        if ( ( ( cont_standardMask >> i ) & 1 ) && ( cont_status[i].type & CONT_TYPE_MASK ) == CONT_TYPE_NORMAL ) {
            pad = &cont_pads[i];
            cont_stickX[i] = 0;
            cont_stickY[i] = 0;
            if ( pad->errno != 0 ) {
                cont_held[i] = 0;
            } else {
                cont_held[i] = pad->button;
                if ( pad->stick_x < -STICK_DEADZONE || pad->stick_x > STICK_DEADZONE ) {
                    cont_stickX[i] = pad->stick_x;
                }
                if ( pad->stick_y < -STICK_DEADZONE || pad->stick_y > STICK_DEADZONE ) {
                    cont_stickY[i] = pad->stick_y;
                }
            }
            cont_pressed[i] = cont_held[i] & ( cont_held[i] ^ cont_lastHeld[i] );
            cont_lastHeld[i] = cont_held[i];
            cont_lastStickDpad[i] = cont_stickDpad[i];
            cont_stickDpad[i] = 0;
            if ( cont_stickX[i] > STICK_DPAD ) {
                cont_stickDpad[i] = R_JPAD;
            }
            if ( cont_stickX[i] < -STICK_DPAD ) {
                cont_stickDpad[i] |= L_JPAD;
            }
            if ( cont_stickY[i] > STICK_DPAD ) {
                cont_stickDpad[i] |= U_JPAD;
            }
            if ( cont_stickY[i] < -STICK_DPAD ) {
                cont_stickDpad[i] |= D_JPAD;
            }
            cont_dpadRepeat[i] = cont_stickDpad[i] & ( cont_stickDpad[i] ^ cont_lastStickDpad[i] );
            cont_dpadRepeat[i] |= cont_pressed[i] & DPAD_BUTTONS;
            if ( ( cont_held[i] & DPAD_BUTTONS ) || cont_stickDpad[i] != 0 ) {
                cont_holdFrames[i]++;
                if ( cont_dpadRepeat[i] == 0 && cont_holdFrames[i] > REPEAT_DELAY && ( cont_holdFrames[i] & 3 ) == 0 ) {
                    if ( cont_held[i] & DPAD_BUTTONS ) {
                        cont_dpadRepeat[i] = cont_held[i] & DPAD_BUTTONS;
                    } else {
                        cont_dpadRepeat[i] = cont_stickDpad[i];
                    }
                }
            } else {
                cont_holdFrames[i] = 0;
            }
        } else {
            cont_pressed[i] = 0;
            cont_held[i] = 0;
            if ( i == 0 ) {
                cont_noController = 1;
            }
        }
    }
    if ( cont_held[0] != 0 ) {
        cont_allHeld = cont_held[0];
    } else {
        cont_allHeld = cont_held[1];
    }
    if ( cont_pressed[0] != 0 ) {
        cont_allPressed = cont_pressed[0];
    } else {
        cont_allPressed = cont_pressed[1];
    }
    if ( cont_dpadRepeat[0] != 0 ) {
        cont_allDpadRepeat = cont_dpadRepeat[0];
    } else {
        cont_allDpadRepeat = cont_dpadRepeat[1];
    }
    if ( cont_stickX[0] >= -5 && cont_stickX[0] <= 5 ) {
        cont_allStickX = cont_stickX[1];
    } else {
        cont_allStickX = cont_stickX[0];
    }
    if ( cont_stickY[0] >= -5 && cont_stickY[0] <= 5 ) {
        cont_allStickY = cont_stickY[1];
    } else {
        cont_allStickY = cont_stickY[0];
    }
    if ( cont_demoMode != 0 ) {
        if ( ( cont_allPressed & START_BUTTON ) && cont_demoFrames > 40 ) {
            Sound_PlaySe( 1, 0 );
            cont_demoFrames = 40;
        }
        cont_allPressed &= ~START_BUTTON;
        cont_held[0] &= ~U_CBUTTONS;
        cont_held[1] &= ~U_CBUTTONS;
        if ( ( sched_frameCount & 0x7E ) == 0 ) {
            cont_pressed[0] = R_TRIG;
        } else {
            cont_pressed[0] = 0;
        }
        if ( ( sched_frameCount & 0x7E ) == 0x3E ) {
            cont_pressed[1] |= R_TRIG;
        } else {
            cont_pressed[1] = 0;
        }
    }
}

/*
================
Cont_FindStandard

Sets a bit of cont_standardMask for each port that holds a standard controller.
================
*/
void Cont_FindStandard( void ) {
    s32 i;

    cont_standardMask = 0;
    for ( i = 0; i < NUM_PLAYERS; i++ ) {
        if ( cont_status[i].errno == 0 && ( cont_status[i].type & CONT_TYPE_MASK ) == CONT_TYPE_NORMAL ) {
            cont_standardMask |= 1 << i;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/2DF0", func_80028064);
