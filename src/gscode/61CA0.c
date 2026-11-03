/* 61CA0.c -- game-select overlay: the controller pak check */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_8011B0B0_gscode

Starts the pak check: D_80120A64_gscode is set when the second team has a controller; step 0 asks for
the paks, 1 has them, 2 skips the check.
================
*/
void func_8011B0B0_gscode( void ) {
    s32 i;

    D_80120A64_gscode = 0;
    if ( D_8005C162 == 1 ) {
        switch ( D_8005C161 ) {
        case 2:
            if ( !( D_8005C0E4[1] >> 7 ) ) {
                D_80120A64_gscode = 1;
            }
            break;
        case 3:
            if ( !( D_8005C0E4[2] >> 7 ) ) {
                D_80120A64_gscode = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) ) {
                    D_80120A64_gscode = 1;
                }
            }
            break;
        }
    }
    D_80120A68_gscode[1] = 0;
    D_80120A68_gscode[0] = 0;
    if ( D_8009D098.unk_7B ) {
        if ( !D_800E4FD0[0] || ( D_80120A64_gscode == 1 && !D_800E4FD0[1] ) ) {
            D_80120A60_gscode = 0;
            return;
        }
        if ( D_800E4FD0[0] == 1 || ( D_80120A64_gscode == 1 && D_800E4FD0[1] == 1 ) ) {
            D_80120A60_gscode = 1;
            return;
        }
    }
    D_80120A60_gscode = 2;
}

/*
================
func_8011B214_gscode

Frame of the pak check, over a dimmed screen; returns 1 when it is over.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: expression shape; retail computes (status ^ old) & (status & 3), which gcc folds here to
// status & ~old & 3 (or keeps as ((status ^ old) & status) & 3); every other instruction is the same
s32 func_8011B214_gscode( void ) {
    s32 i;

    D_8005B191 = 1;
    D_80120A64_gscode = 0;
    if ( D_8005C162 == 1 ) {
        switch ( D_8005C161 ) {
        case 2:
            if ( !( D_8005C0E4[1] >> 7 ) ) {
                D_80120A64_gscode = 1;
            }
            break;
        case 3:
            if ( !( D_8005C0E4[2] >> 7 ) ) {
                D_80120A64_gscode = 1;
            }
            break;
        case 4:
            for ( i = 2; i < 4; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) ) {
                    D_80120A64_gscode = 1;
                }
            }
            break;
        }
    }
    func_8002FDE4( 0, 0, 0x140, 0xF0, 0, 0, 0, 0x80 );
    switch ( D_80120A60_gscode ) {
    case 0:
        D_80120668_gscode++;
        for ( i = 0; i < 2; i++ ) {
            if ( ( D_800F7BE8[i].status ^ D_80120A68_gscode[i] ) & ( D_800F7BE8[i].status & 3 ) ) {
                if ( !D_800DF680[i] ) {
                    D_800DF680[i] = 8;
                }
            }
            D_80120A68_gscode[i] = D_800F7BE8[i].status;
        }
        if ( func_80036E60( 0x948, 1 ) ) {
            if ( D_800E4FD0[0] == 1 || ( D_80120A64_gscode == 1 && D_800E4FD0[1] == 1 ) ) {
                D_80120A60_gscode = 1;
                return 0;
            }
            D_80120A60_gscode = 2;
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
        if ( D_80120A64_gscode == 1 ) {
            if ( D_800E4FD0[1] ) {
                func_8002DF3C( 0x944, 0xD0, 0x75 );
            } else {
                func_8002DF3C( 0x946, 0xD0, 0x75 );
            }
        }
        if ( D_800E4FD0[0] || ( D_80120A64_gscode == 1 && D_800E4FD0[1] ) ) {
            func_8002DF3C( D_80120660_gscode[D_80120668_gscode % 4], 0xB0, 0x65 );
        } else {
            func_8002DF3C( D_80120660_gscode[0], 0xB0, 0x65 );
        }
        return 0;
    case 1:
        if ( func_80036E60( 0x940, 0 ) ) {
            D_80120A60_gscode = 2;
        }
        return 0;
    case 2:
        D_8005B191 = 0;
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/61CA0", func_8011B214_gscode);
#endif
