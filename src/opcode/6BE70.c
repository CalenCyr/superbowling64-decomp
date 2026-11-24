/* 6BE70.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "opcode/opcode.h"

/*
================
OP_Init

Entry of the overlay: in mode 0xA, Start held selects mode 0xFE, and no controller in port 1 mode 0xFD.
================
*/
void OP_Init( void ) {
    op_modeFrames = 0;
    if ( D_8005C0E8 == 10 ) {
        if ( D_800994BC & START_BUTTON ) {
            D_8005C0E8 = 0xFE;
        }
        if ( D_8005B194 == 1 ) {
            D_8005C0E8 = 0xFD;
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

    switch ( D_8005C0E8 ) {
    case 10:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xB58, 0, 0 );
        if ( ++op_modeFrames >= 150 ) {
            game_leaveState = 1;
            if ( game_fade == 0xFF ) {
                game_fade = 0x107;
                game_leaveState = 2;
                D_8005C0E8 = 0;
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
                D_8005C0E8 = 1;
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
        Draw_ImageScaled( 0x890, 0x44, 0xA0, D_8009CFB0, 1.0f, 1.0f );
        if ( ++op_modeFrames >= 450 ) {
            game_leaveState = 1;
            if ( game_fade == 0xFF ) {
                game_fade = 0x107;
                game_scene = 0;
                game_leaveState = 2;
                D_800693B0 = 1200;
                D_8005C0EC = 1;
                D_8009D098.music = 0;
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
        if ( ( D_800E44E0[0] & START_BUTTON ) && !game_leaveState ) {
            Sound_PlaySe( 1, 0 );
            D_8005C0E8 = 2;
            op_modeFrames = 0;
        }
        break;
    case 2:
        Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
        Draw_Image( 0xADD, 0, 0 );
        if ( !OP_CheckSettings() ) {
            game_leaveState = 1;
            D_8005C0E8 = 3;
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
            D_8005C0E8 = 10;
        }
        break;
    }
}

