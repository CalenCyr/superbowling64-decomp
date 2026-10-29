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

/*
================
func_80027BC0

Reads the pads of both ports into the per-port state: buttons held and pressed, the stick outside its dead
zone and as D-pad bits, and D-pad presses that repeat every 4 frames once held for REPEAT_DELAY; then merges
the two ports.
================
*/
void func_80027BC0( void ) {
    s32 i;
    OSContPad *pad;

    for ( i = 0; i < NUM_PLAYERS; i++ ) {
        if ( ( ( D_8005B190 >> i ) & 1 ) && ( D_800F7BE8[i].type & CONT_TYPE_MASK ) == CONT_TYPE_NORMAL ) {
            pad = &D_80066B58[i];
            D_800692D0[i] = 0;
            D_800A2FD8[i] = 0;
            if ( pad->errno != 0 ) {
                D_800E57E4[i] = 0;
            } else {
                D_800E57E4[i] = pad->button;
                if ( pad->stick_x < -STICK_DEADZONE || pad->stick_x > STICK_DEADZONE ) {
                    D_800692D0[i] = pad->stick_x;
                }
                if ( pad->stick_y < -STICK_DEADZONE || pad->stick_y > STICK_DEADZONE ) {
                    D_800A2FD8[i] = pad->stick_y;
                }
            }
            D_800E44E0[i] = D_800E57E4[i] & ( D_800E57E4[i] ^ D_8009D198[i] );
            D_8009D198[i] = D_800E57E4[i];
            D_8005B1A0[i] = D_800619B0[i];
            D_800619B0[i] = 0;
            if ( D_800692D0[i] > STICK_DPAD ) {
                D_800619B0[i] = R_JPAD;
            }
            if ( D_800692D0[i] < -STICK_DPAD ) {
                D_800619B0[i] |= L_JPAD;
            }
            if ( D_800A2FD8[i] > STICK_DPAD ) {
                D_800619B0[i] |= U_JPAD;
            }
            if ( D_800A2FD8[i] < -STICK_DPAD ) {
                D_800619B0[i] |= D_JPAD;
            }
            D_800E6760[i] = D_800619B0[i] & ( D_800619B0[i] ^ D_8005B1A0[i] );
            D_800E6760[i] |= D_800E44E0[i] & DPAD_BUTTONS;
            if ( ( D_800E57E4[i] & DPAD_BUTTONS ) || D_800619B0[i] != 0 ) {
                D_8005B1A4[i]++;
                if ( D_800E6760[i] == 0 && D_8005B1A4[i] > REPEAT_DELAY && ( D_8005B1A4[i] & 3 ) == 0 ) {
                    if ( D_800E57E4[i] & DPAD_BUTTONS ) {
                        D_800E6760[i] = D_800E57E4[i] & DPAD_BUTTONS;
                    } else {
                        D_800E6760[i] = D_800619B0[i];
                    }
                }
            } else {
                D_8005B1A4[i] = 0;
            }
        } else {
            D_800E44E0[i] = 0;
            D_800E57E4[i] = 0;
            if ( i == 0 ) {
                D_8005B194 = 1;
            }
        }
    }
    if ( D_800E57E4[0] != 0 ) {
        D_800994BC = D_800E57E4[0];
    } else {
        D_800994BC = D_800E57E4[1];
    }
    if ( D_800E44E0[0] != 0 ) {
        D_800E4D62 = D_800E44E0[0];
    } else {
        D_800E4D62 = D_800E44E0[1];
    }
    if ( D_800E6760[0] != 0 ) {
        D_800E6758 = D_800E6760[0];
    } else {
        D_800E6758 = D_800E6760[1];
    }
    if ( D_800692D0[0] >= -5 && D_800692D0[0] <= 5 ) {
        D_8009D05C = D_800692D0[1];
    } else {
        D_8009D05C = D_800692D0[0];
    }
    if ( D_800A2FD8[0] >= -5 && D_800A2FD8[0] <= 5 ) {
        D_800DF628 = D_800A2FD8[1];
    } else {
        D_800DF628 = D_800A2FD8[0];
    }
    if ( D_8005C0EC != 0 ) {
        if ( ( D_800E4D62 & START_BUTTON ) && D_800693B0 > 40 ) {
            func_80037870( 1, 0 );
            D_800693B0 = 40;
        }
        D_800E4D62 &= ~START_BUTTON;
        D_800E57E4[0] &= ~U_CBUTTONS;
        D_800E57E4[1] &= ~U_CBUTTONS;
        if ( ( D_8005B0F4 & 0x7E ) == 0 ) {
            D_800E44E0[0] = R_TRIG;
        } else {
            D_800E44E0[0] = 0;
        }
        if ( ( D_8005B0F4 & 0x7E ) == 0x3E ) {
            D_800E44E0[1] |= R_TRIG;
        } else {
            D_800E44E0[1] = 0;
        }
    }
}

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
