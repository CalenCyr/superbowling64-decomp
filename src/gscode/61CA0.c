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
    if ( D_8005C162 == 1 ) {
        switch ( D_8005C161 ) {
        case 2:
            if ( !( D_8005C0E4[1] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 3:
            if ( !( D_8005C0E4[2] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) ) {
                    gs_secondController = 1;
                }
            }
            break;
        }
    }
    gs_lastContStatus[1] = 0;
    gs_lastContStatus[0] = 0;
    if ( D_8009D098.unk_7B ) {
        if ( !D_800E4FD0[0] || ( gs_secondController == 1 && !D_800E4FD0[1] ) ) {
            gs_pakCheckStep = 0;
            return;
        }
        if ( D_800E4FD0[0] == 1 || ( gs_secondController == 1 && D_800E4FD0[1] == 1 ) ) {
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
    if ( D_8005C162 == 1 ) {
        switch ( D_8005C161 ) {
        case 2:
            if ( !( D_8005C0E4[1] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 3:
            if ( !( D_8005C0E4[2] >> 7 ) ) {
                gs_secondController = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) ) {
                    gs_secondController = 1;
                }
            }
            break;
        }
    }
    func_8002FDE4( 0, 0, 0x140, 0xF0, 0, 0, 0, 0x80 );
    switch ( gs_pakCheckStep ) {
    case 0:
        gs_pakCheckFrames++;
        for ( i = 0; i < 2; i++ ) {
            if ( ( D_800F7BE8[i].status ^ gs_lastContStatus[i] ) & ( CONT_CARD_ON | CONT_CARD_PULL )
                 & D_800F7BE8[i].status ) {
                if ( !D_800DF680[i] ) {
                    D_800DF680[i] = 8;
                }
            }
            gs_lastContStatus[i] = D_800F7BE8[i].status;
        }
        if ( func_80036E60( 0x948, 1 ) ) {
            if ( D_800E4FD0[0] == 1 || ( gs_secondController == 1 && D_800E4FD0[1] == 1 ) ) {
                gs_pakCheckStep = 1;
                return 0;
            }
            gs_pakCheckStep = 2;
            return 0;
        }
        if ( D_8005C9BC ) {
            return 0;
        }
        if ( D_800E4FD0[0] ) {
            func_8002DF3C( 0x945, 0xD0, 0x65 );
        } else {
            func_8002DF3C( 0x947, 0xD0, 0x65 );
        }
        if ( gs_secondController == 1 ) {
            if ( D_800E4FD0[1] ) {
                func_8002DF3C( 0x944, 0xD0, 0x75 );
            } else {
                func_8002DF3C( 0x946, 0xD0, 0x75 );
            }
        }
        if ( D_800E4FD0[0] || ( gs_secondController == 1 && D_800E4FD0[1] ) ) {
            func_8002DF3C( D_80120660_gscode[gs_pakCheckFrames % 4], 0xB0, 0x65 );
        } else {
            func_8002DF3C( D_80120660_gscode[0], 0xB0, 0x65 );
        }
        return 0;
    case 1:
        if ( func_80036E60( 0x940, 0 ) ) {
            gs_pakCheckStep = 2;
        }
        return 0;
    case 2:
        D_8005B191 = 0;
        return 1;
    }
    return 0;
}
