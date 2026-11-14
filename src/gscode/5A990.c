/* 5A990.c -- game-select overlay: a menu of two choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80113DA0_gscode

Frame of a two-choice menu that slides in and out: left and right move the cursor between the two
blinking images, A or Z takes the highlighted choice and B backs out. Returns 0 while the menu runs;
once it has slid out, 1 for the left choice, 3 for the right one and 2 for backing out.
================
*/
s32 func_80113DA0_gscode( void ) {
    u8 choice;

    if ( D_8011B8C5_gscode == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( D_8011B8C6_gscode ) {
        case 0:
            D_8011B8C5_gscode = CHOICE_TAKEN;
            break;
        case 1:
            D_8011B8C5_gscode = CHOICE_TAKEN;
            break;
        }
    }
    if ( D_8011B8C5_gscode == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        func_80037870( 2, 0 );
        D_8011B8C5_gscode = CHOICE_BACK;
    }

    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_8003893C( D_8011B850_gscode, 0, D_8011B8AC_gscode[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( D_8011B850_gscode, 1, D_8011B8AC_gscode[1][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( D_8011B8C5_gscode == CHOICE_RUN ) {
        if ( ++D_8011B8C7_gscode == 30 ) {
            D_8011B8C7_gscode = 0;
        }
        choice = D_8011B8C6_gscode;
        switch ( choice ) {
        case 0:
            func_8003893C( D_8011B850_gscode, 0, D_8011B8AC_gscode[0][D_8011B8C7_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0x28, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                D_8011B8C6_gscode = 1;
            }
            break;
        case 1:
            func_8003893C( D_8011B850_gscode, 1, D_8011B8AC_gscode[1][D_8011B8C7_gscode / 5], MENU_NONE, MENU_NONE, MENU_NONE );
            func_8002C950( 0xB8, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B8C6_gscode = 0;
            }
            break;
        }
        if ( D_8011B8C6_gscode != choice ) {
            D_8011B8C7_gscode = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( D_8011B850_gscode, D_8011B8C4_gscode );
    switch ( D_8011B8C5_gscode ) {
    case CHOICE_IN:
        D_8011B850_gscode[5].slide = SLIDE_LEFT;
        D_8011B8C4_gscode -= 2;
        if ( D_8011B8C4_gscode == 0 ) {
            D_8011B8C5_gscode = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011B850_gscode[5].slide = SLIDE_RIGHT;
        D_8011B8C4_gscode += 2;
        if ( D_8011B8C4_gscode == 30 ) {
            if ( D_8011B8C5_gscode == CHOICE_TAKEN ) {
                D_8011B8C5_gscode = CHOICE_IN;
                return ( D_8011B8C6_gscode << 1 ) | 1;
            }
            D_8011B8C5_gscode = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
