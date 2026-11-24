/* 61CA0.c -- game-select overlay: the controller pak check */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_StartPakCheck

Starts the pak check: gs_secondController is set when the second team has a controller; step 0 asks for
the paks, 1 has them, 2 skips the check.
================
*/
void GS_StartPakCheck( void ) {
    s32 i;

    gs_secondController = 0;
    if ( game_lastTeam == 1 ) {
        switch ( game_numPlayers ) {
        case 2:
            if ( !( game_players[1] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 3:
            if ( !( game_players[2] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( game_players[i] >> 7 ) ) {
                    gs_secondController = 1;
                }
            }
            break;
        }
    }
    gs_lastContStatus[1] = 0;
    gs_lastContStatus[0] = 0;
    if ( save_data.rumble ) {
        if ( !cont_hasRumble[0] || ( gs_secondController == 1 && !cont_hasRumble[1] ) ) {
            gs_pakCheckStep = 0;
            return;
        }
        if ( cont_hasRumble[0] == 1 || ( gs_secondController == 1 && cont_hasRumble[1] == 1 ) ) {
            gs_pakCheckStep = 1;
            return;
        }
    }
    gs_pakCheckStep = 2;
}

/*
================
GS_PakCheck

Frame of the pak check, over a dimmed screen; returns 1 when it is over.
================
*/
s32 GS_PakCheck( void ) {
    s32 i;

    D_8005B191 = 1;
    gs_secondController = 0;
    if ( game_lastTeam == 1 ) {
        switch ( game_numPlayers ) {
        case 2:
            if ( !( game_players[1] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 3:
            if ( !( game_players[2] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( game_players[i] >> 7 ) ) {
                    gs_secondController = 1;
                }
            }
            break;
        }
    }
    Draw_FillRect( 0, 0, 0x140, 0xF0, 0, 0, 0, 0x80 );
    switch ( gs_pakCheckStep ) {
    case 0:
        gs_pakCheckFrames++;
        for ( i = 0; i < 2; i++ ) {
            if ( ( cont_status[i].status ^ gs_lastContStatus[i] ) & ( CONT_CARD_ON | CONT_CARD_PULL )
                 & cont_status[i].status ) {
                if ( !cont_rumble[i] ) {
                    cont_rumble[i] = 8;
                }
            }
            gs_lastContStatus[i] = cont_status[i].status;
        }
        if ( Save_ShowPrompt( 0x948, 1 ) ) {
            if ( cont_hasRumble[0] == 1 || ( gs_secondController == 1 && cont_hasRumble[1] == 1 ) ) {
                gs_pakCheckStep = 1;
                return 0;
            }
            gs_pakCheckStep = 2;
            return 0;
        }
        if ( save_promptState ) {
            return 0;
        }
        if ( cont_hasRumble[0] ) {
            Draw_Image( 0x945, 0xD0, 0x65 );
        } else {
            Draw_Image( 0x947, 0xD0, 0x65 );
        }
        if ( gs_secondController == 1 ) {
            if ( cont_hasRumble[1] ) {
                Draw_Image( 0x944, 0xD0, 0x75 );
            } else {
                Draw_Image( 0x946, 0xD0, 0x75 );
            }
        }
        if ( cont_hasRumble[0] || ( gs_secondController == 1 && cont_hasRumble[1] ) ) {
            Draw_Image( D_80120660_gscode[gs_pakCheckFrames % 4], 0xB0, 0x65 );
        } else {
            Draw_Image( D_80120660_gscode[0], 0xB0, 0x65 );
        }
        return 0;
    case 1:
        if ( Save_ShowPrompt( 0x940, 0 ) ) {
            gs_pakCheckStep = 2;
        }
        return 0;
    case 2:
        D_8005B191 = 0;
        return 1;
    }
    return 0;
}
