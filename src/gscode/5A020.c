/* 5A020.c -- game-select overlay: a second menu of three choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_TeamSizeMenu

Frame of a three-choice menu like GS_SubModeMenu, choosing the number of players on one team:
the left choice sets up one player, the other two set up two players, with argument 2 (right) or
1 (below) in bits 6-7 of D_8005C160. Up from the lower choice moves to the right one. Returns 0
while the menu runs; once it has slid out, 1 for a choice taken and 2 for backing out.
================
*/
s32 GS_TeamSizeMenu( void ) {
    u8 choice;

    if ( gs_teamSizeState == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( gs_teamSizeCursor ) {
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
        gs_teamSizeState = CHOICE_TAKEN;
    }
    if ( gs_teamSizeState == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        func_80037870( 2, 0 );
        gs_teamSizeState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_8003893C( gs_teamSizeMenu, 0, gs_teamSizeBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_teamSizeMenu, 1, gs_teamSizeBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_teamSizeMenu, 2, gs_teamSizeBlink[2][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_teamSizeState == CHOICE_RUN ) {
        if ( ++gs_teamSizeBlinkFrame == 30 ) {
            gs_teamSizeBlinkFrame = 0;
        }
        choice = gs_teamSizeCursor;
        switch ( choice ) {
        case 0:
            func_8003893C( gs_teamSizeMenu, 0, gs_teamSizeBlink[0][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_teamSizeCursor = 2;
            }
            break;
        case 1:
            func_8003893C( gs_teamSizeMenu, 1, gs_teamSizeBlink[1][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_teamSizeCursor = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_teamSizeCursor = 2;
            }
            break;
        case 2:
            func_8003893C( gs_teamSizeMenu, 2, gs_teamSizeBlink[2][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x70, 0x90, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_teamSizeCursor = 0;
            }
            if ( D_800E6758 & R_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            if ( D_800E6758 & U_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            break;
        }
        if ( gs_teamSizeCursor != choice ) {
            gs_teamSizeBlinkFrame = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( gs_teamSizeMenu, gs_teamSizeSlide );
    switch ( gs_teamSizeState ) {
    case CHOICE_IN:
        gs_teamSizeMenu[5].slide = SLIDE_LEFT;
        gs_teamSizeSlide -= 2;
        if ( gs_teamSizeSlide == 0 ) {
            gs_teamSizeState = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_teamSizeMenu[5].slide = SLIDE_RIGHT;
        gs_teamSizeSlide += 2;
        if ( gs_teamSizeSlide == 30 ) {
            if ( gs_teamSizeState == CHOICE_TAKEN ) {
                gs_teamSizeState = CHOICE_IN;
                return 1;
            }
            gs_teamSizeState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
