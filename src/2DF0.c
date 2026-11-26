/* 2DF0.c -- controllers: set-up, Rumble Paks, and the per-frame pad state */

#include "common.h"
#include "libultra.h"
#include "input.h"
#include "gamesnd.h"
#include "boot.h"

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
void Cont_Init( void ) {
    // R36: unused in the original: retail's frame reserves 0x18 bytes no instruction touches
    OSContPad pads[MAXCONTROLLERS];

    osCreateMesgQueue( &cont_siQueue, &cont_siMsgs, 1 );
    osSetEventMesg( OS_EVENT_SI, &cont_siQueue, (OSMesg)cont_siEvent );
    osContInit( &cont_siQueue, &cont_standardMask, cont_status );
    osContSetCh( NUM_PLAYERS );
    cont_hasRumble[1] = FALSE;
    cont_hasRumble[0] = FALSE;
    Rumble_Probe( 0 );
    Rumble_Probe( 1 );
}

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
    cont_rumblePhase[chan] = 0;
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

/*
================
Cont_Thread

The controller thread. Each retrace: takes the pads read last frame, drives the Rumble Pak of one port (the
ports take turns by frame: the strength cont_rumble, for cont_rumbleFrames frames, runs as pulses of the
motor), serves one request of cont_requestQueue, queries the controller status while cont_pollStatus is set,
and starts the next read.
================
*/
void Cont_Thread( void *arg ) {
    OSMesg msg;
    u8 motorOn[NUM_PLAYERS] = { FALSE, FALSE };
    s32 chan;

    for ( ;; ) {
        osRecvMesg( &sched_retraceQueue, &msg, OS_MESG_BLOCK );
        osRecvMesg( &cont_siQueue, NULL, OS_MESG_BLOCK );
        osContGetReadData( cont_pads );
        if ( save_data.rumble ) {
            chan = sched_frameCount & 1;
            if ( cont_rumble[chan] == RUMBLE_REPROBE || sched_reset[0] == TRUE ) {
                Rumble_Probe( chan );
                motorOn[chan] = FALSE;
                cont_rumble[chan] = 0;
            } else {
                if ( cont_rumble[chan] != 0 ) {
                    if ( --cont_rumbleFrames[chan] == 0 ) {
                        cont_rumble[chan] = 0;
                    }
                } else if ( motorOn[chan] != 0 ) {
                    if ( motorOn[chan]++ >= 4 ) {
                        motorOn[chan] = FALSE;
                    }
                }
                cont_rumblePhase[chan] += cont_rumble[chan] << 5;
                if ( cont_rumblePhase[chan] >= RUMBLE_PERIOD ) {
                    if ( !motorOn[chan] ) {
                        Rumble_Start( chan );
                        motorOn[chan] = TRUE;
                    }
                    cont_rumblePhase[chan] -= RUMBLE_PERIOD;
                } else if ( motorOn[chan] ) {
                    Rumble_Stop( chan );
                    motorOn[chan] = FALSE;
                }
            }
        }
        if ( osRecvMesg( &cont_requestQueue, &msg, OS_MESG_NOBLOCK ) != -1 ) {
            switch ( *(s32 *)msg ) {
            case CONT_REQ_ALLOCATE_FILE:
                cont_result = osPfsAllocateFile( &cont_pakPfs, cont_noteCompany, cont_noteGameCode, cont_noteGameName,
                                                 cont_noteExtName, cont_noteSize, &cont_noteIndex );
                break;
            case CONT_REQ_DELETE_FILE:
                cont_result = osPfsDeleteFile( &cont_pakPfs, cont_deleteCompany, cont_deleteGameCode,
                                               cont_deleteGameName, cont_deleteExtName );
                break;
            case CONT_REQ_FIND_FILE:
                cont_result = osPfsFindFile( &cont_pakPfs, cont_noteCompany, cont_noteGameCode, cont_noteGameName,
                                             cont_noteExtName, &cont_noteIndex );
                break;
            case CONT_REQ_NUM_FILES:
                cont_result = osPfsNumFiles( &cont_pakPfs, &cont_maxNotes, &cont_usedNotes );
                break;
            case CONT_REQ_FREE_BLOCKS:
                cont_result = osPfsFreeBlocks( &cont_pakPfs, &cont_freeBytes );
                break;
            case CONT_REQ_READ_WRITE_FILE:
                cont_result = osPfsReadWriteFile( &cont_pakPfs, cont_noteIndex, cont_pakWrite, 0, cont_noteSize,
                                                  (u8 *)save_dataPtr );
                break;
            case CONT_REQ_INIT_PAK:
                cont_result = osPfsInitPak( &cont_siQueue, &cont_pakPfs, cont_pakPort );
                break;
            case CONT_REQ_REPAIR_ID:
                cont_result = osPfsRepairId( &cont_pakPfs );
                break;
            case CONT_REQ_IS_PLUG:
                cont_result = osPfsIsPlug( &cont_siQueue, &cont_pakPlugged );
                break;
            case CONT_REQ_MOTOR_INIT:
                // R36: the original probes port 1 into the OSPfs of port 2
                cont_result = osMotorInit( &cont_siQueue, &cont_rumblePfs[1], 0 );
                break;
            case CONT_REQ_FILE_STATE:
                cont_result = osPfsFileState( &cont_pakPfs, cont_noteIndex, &D_80066C00.note );
                break;
            }
            // the reply message is the address of msg, a local of this thread
            osSendMesg( &cont_replyQueue, &msg, OS_MESG_NOBLOCK );
        }
        if ( cont_pollStatus ) {
            osContStartQuery( &cont_siQueue );
            osRecvMesg( &cont_siQueue, &msg, OS_MESG_BLOCK );
            osContGetQuery( cont_status );
        }
        osContStartReadData( &cont_siQueue );
    }
}

// TODO(identity): 8 bytes of zeros after Cont_Thread's jump table in this unit's .rodata that no instruction
// references (53B0's .rodata starts after them); what the original compiled them from is unknown.
INCLUDE_RODATA("asm/nonmatchings/2DF0", D_800603B8);
