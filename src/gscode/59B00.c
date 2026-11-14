/* 59B00.c -- game-select overlay: a menu of three choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80112F10_gscode

Frame of a three-choice menu built like func_80113990_gscode: two choices side by side and a third
below them. Taking a choice sets the sub-mode of D_8005C160 to it (bits 4-5); the third also sets up
one player on one team and clears the argument. Returns 0 while the menu runs; once it has slid out,
1 for a choice taken and 2 for backing out.
================
*/
s32 func_80112F10_gscode( void ) {
    u8 choice;

    if ( D_8011B739_gscode == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( D_8011B73A_gscode ) {
        case 0:
            D_8005C160 = D_8005C160 & 0xCF;
            break;
        case 1:
            D_8005C160 = ( D_8005C160 & 0xCF ) | 0x10;
            break;
        case 2:
            D_8005C161 = 1;
            D_8005C162 = 0;
            D_8005C160 = ( ( D_8005C160 & 0xCF ) | 0x20 ) & 0x3F;
            break;
        }
        D_8011B739_gscode = CHOICE_TAKEN;
    }
    if ( D_8011B739_gscode == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        func_80037870( 2, 0 );
        D_8011B739_gscode = CHOICE_BACK;
    }

    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_8003893C( D_8011B6B0_gscode, 0, D_8011B714_gscode[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( D_8011B6B0_gscode, 1, D_8011B714_gscode[1][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( D_8011B6B0_gscode, 2, D_8011B714_gscode[2][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( D_8011B739_gscode == CHOICE_RUN ) {
        if ( ++D_8011B73B_gscode == 30 ) {
            D_8011B73B_gscode = 0;
        }
        choice = D_8011B73A_gscode;
        switch ( choice ) {
        case 0:
            func_8003893C( D_8011B6B0_gscode, 0, D_8011B714_gscode[0][D_8011B73B_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0x28, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                D_8011B73A_gscode = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B73A_gscode = 2;
            }
            break;
        case 1:
            func_8003893C( D_8011B6B0_gscode, 1, D_8011B714_gscode[1][D_8011B73B_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0xB8, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B73A_gscode = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B73A_gscode = 2;
            }
            break;
        case 2:
            func_8003893C( D_8011B6B0_gscode, 2, D_8011B714_gscode[2][D_8011B73B_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0x70, 0x90, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B73A_gscode = 0;
            }
            if ( D_800E6758 & R_JPAD ) {
                D_8011B73A_gscode = 1;
            }
            break;
        }
        if ( D_8011B73A_gscode != choice ) {
            D_8011B73B_gscode = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( D_8011B6B0_gscode, D_8011B738_gscode );
    switch ( D_8011B739_gscode ) {
    case CHOICE_IN:
        D_8011B6B0_gscode[5].slide = SLIDE_LEFT;
        D_8011B738_gscode -= 2;
        if ( D_8011B738_gscode == 0 ) {
            D_8011B739_gscode = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011B6B0_gscode[5].slide = SLIDE_RIGHT;
        D_8011B738_gscode += 2;
        if ( D_8011B738_gscode == 30 ) {
            if ( D_8011B739_gscode == CHOICE_TAKEN ) {
                D_8011B739_gscode = CHOICE_IN;
                return 1;
            }
            D_8011B739_gscode = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
