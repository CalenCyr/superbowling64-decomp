/* 6BE70.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "opcode/opcode.h"

/* The unit's uninitialised state (it has no .data). */
static u16 op_modeFrames; /* frames in the current mode */

/*
================
OP_Init

Entry of the overlay: in mode 0xA, Start held selects mode 0xFE, and no controller in port 1 mode 0xFD.
================
*/
void OP_Init( void ) {
    op_modeFrames = 0;
    if ( op_nextMode == 10 ) {
        if ( cont_allHeld & START_BUTTON ) {
            op_nextMode = 0xFE;
        }
        if ( cont_noController == 1 ) {
            op_nextMode = 0xFD;
        }
    }
}

/*
================
OP_Frame

The opening's modes: two logo screens, the title with its demo games, then OP_CheckSettings's and
OP_PakScreen's screens. Each logo and the title count frames in op_modeFrames, then ask to leave.
================
*/
// RULE-EXCEPTION: s32 with no return value: retail keeps v0 live at the exit
s32 OP_Frame( void ) {
    s32 i;

    switch ( op_nextMode ) {
    case 10:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xB58, 0, 0 );
        if ( ++op_modeFrames >= 150 ) {
            game_leaveState = 1;
            if ( game_fade == 0xFF ) {
                game_fade = 0x107;
                game_leaveState = 2;
                op_nextMode = 0;
                op_modeFrames = 0;
                Sound_PlaySe( 0x43, 0 );
            }
        }
        break;
    case 0:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0x925, 0x3C, 0x58 );
        if ( ++op_modeFrames >= 70 ) {
            game_leaveState = 1;
            if ( game_fade == 0xFF ) {
                game_fade = 0x107;
                game_leaveState = 2;
                op_nextMode = 1;
                op_modeFrames = 0;
            }
        }
        break;
    case 1:
        if ( op_modeFrames == 6 ) {
            Sound_PlayBgm( 0 );
        }
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xADD, 0, 0 );
        Draw_ImageScaled( 0x890, 0x44, 0xA0, game_pulseAlpha, 1.0f, 1.0f );
        if ( ++op_modeFrames >= 450 ) {
            game_leaveState = 1;
            if ( game_fade == 0xFF ) {
                game_fade = 0x107;
                game_scene = 0;
                game_leaveState = 2;
                cont_demoFrames = 1200;
                cont_demoMode = 1;
                save_data.music = 0;
                if ( ++game_demo == 4 ) {
                    game_demo = 0;
                }
                game_mode = 0;
                switch ( game_demo ) {
                case 0:
                    game_lastTeam = guRandom() % 2;
                    game_numPlayers = game_lastTeam + 1;
                    break;
                case 1:
                    game_mode = 0x10;
                    game_lastTeam = guRandom() % 2;
                    game_numPlayers = game_lastTeam + 1;
                    break;
                case 2:
                    game_mode = 0x20;
                    game_lastTeam = guRandom() % 2;
                    game_numPlayers = game_lastTeam + 1;
                    break;
                case 3:
                    game_mode = game_demo;   /* demo 3 plays game mode 3 */
                    game_lastTeam = 1;
                    game_numPlayers = 2;
                    break;
                }
                game_stage = guRandom() % 80 / 10;
                for ( i = 0; i < game_numPlayers; i++ ) {
                    game_players[i] = ( game_demoCharacter++ & 7 ) | 0x80;
                }
                for ( i = 0; i < game_numPlayers; i++ ) {
                    if ( ( game_players[i] >> 7 ) || !( ( game_players[i] >> 4 ) & 7 ) ) {
                        game_playerNames[i] = game_defaultNames[game_players[i] & 0xF];
                    }
                }
            }
        }
        if ( ( cont_pressed[0] & START_BUTTON ) && !game_leaveState ) {
            Sound_PlaySe( 1, 0 );
            op_nextMode = 2;
            op_modeFrames = 0;
        }
        break;
    case 2:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xADD, 0, 0 );
        if ( !OP_CheckSettings() ) {
            game_leaveState = 1;
            op_nextMode = 3;
        }
        break;
    case 3:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xADD, 0, 0 );
        if ( game_fade == 0xFF ) {
            game_fade = 0x107;
            game_leaveState = 2;
            game_scene = 4;
        }
        break;
    case 0xFD:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xB5A, 0x48, 0x60 );
        break;
    case 0xFE:
        if ( !OP_PakScreen() ) {
            op_nextMode = 10;
        }
        break;
    }
}

