/* 5A020.c -- game-select overlay: a second menu of three choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80113430_gscode

Frame of a three-choice menu like func_80112F10_gscode, choosing the number of players on one team:
the left choice sets up one player, the other two set up two players, with argument 2 (right) or
1 (below) in bits 6-7 of D_8005C160. Up from the lower choice moves to the right one. Returns 0
while the menu runs; once it has slid out, 1 for a choice taken and 2 for backing out.
================
*/
s32 func_80113430_gscode( void ) {
    u8 choice;

    if ( D_8011B7C9_gscode == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( D_8011B7CA_gscode ) {
        case 0:
            D_8005C161 = 1;
            D_8005C162 = 0;
            D_8005C160 = D_8005C160 & 0x3F;
            break;
        case 1:
            D_8005C161 = 2;
            D_8005C162 = 0;
            D_8005C160 = ( D_8005C160 & 0x3F ) | 0x80;
            break;
        case 2:
            D_8005C161 = 2;
            D_8005C162 = 0;
            D_8005C160 = ( D_8005C160 & 0x3F ) | 0x40;
            break;
        }
        D_8011B7C9_gscode = CHOICE_TAKEN;
    }
    if ( D_8011B7C9_gscode == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        func_80037870( 2, 0 );
        D_8011B7C9_gscode = CHOICE_BACK;
    }

    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_8003893C( D_8011B740_gscode, 0, D_8011B7A4_gscode[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( D_8011B740_gscode, 1, D_8011B7A4_gscode[1][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( D_8011B740_gscode, 2, D_8011B7A4_gscode[2][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( D_8011B7C9_gscode == CHOICE_RUN ) {
        if ( ++D_8011B7CB_gscode == 30 ) {
            D_8011B7CB_gscode = 0;
        }
        choice = D_8011B7CA_gscode;
        switch ( choice ) {
        case 0:
            func_8003893C( D_8011B740_gscode, 0, D_8011B7A4_gscode[0][D_8011B7CB_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0x28, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                D_8011B7CA_gscode = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B7CA_gscode = 2;
            }
            break;
        case 1:
            func_8003893C( D_8011B740_gscode, 1, D_8011B7A4_gscode[1][D_8011B7CB_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0xB8, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B7CA_gscode = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B7CA_gscode = 2;
            }
            break;
        case 2:
            func_8003893C( D_8011B740_gscode, 2, D_8011B7A4_gscode[2][D_8011B7CB_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0x70, 0x90, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B7CA_gscode = 0;
            }
            if ( D_800E6758 & R_JPAD ) {
                D_8011B7CA_gscode = 1;
            }
            if ( D_800E6758 & U_JPAD ) {
                D_8011B7CA_gscode = 1;
            }
            break;
        }
        if ( D_8011B7CA_gscode != choice ) {
            D_8011B7CB_gscode = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( D_8011B740_gscode, D_8011B7C8_gscode );
    switch ( D_8011B7C9_gscode ) {
    case CHOICE_IN:
        D_8011B740_gscode[5].slide = SLIDE_LEFT;
        D_8011B7C8_gscode -= 2;
        if ( D_8011B7C8_gscode == 0 ) {
            D_8011B7C9_gscode = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011B740_gscode[5].slide = SLIDE_RIGHT;
        D_8011B7C8_gscode += 2;
        if ( D_8011B7C8_gscode == 30 ) {
            if ( D_8011B7C9_gscode == CHOICE_TAKEN ) {
                D_8011B7C9_gscode = CHOICE_IN;
                return 1;
            }
            D_8011B7C9_gscode = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
