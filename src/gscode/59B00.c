/* 59B00.c -- game-select overlay: a menu of three choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_SubModeMenu

Frame of a three-choice menu built like GS_PlayersMenu: two choices side by side and a third
below them. Taking a choice sets the sub-mode of D_8005C160 to it (bits 4-5); the third also sets up
one player on one team and clears the argument. Returns 0 while the menu runs; once it has slid out,
1 for a choice taken and 2 for backing out.
================
*/
s32 GS_SubModeMenu( void ) {
    u8 choice;

    if ( gs_subModeState == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_subModeCursor ) {
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
        gs_subModeState = CHOICE_TAKEN;
    }
    if ( gs_subModeState == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        Sound_PlaySe( 2, 0 );
        gs_subModeState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    func_8003893C( gs_subModeMenu, 0, gs_subModeBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_subModeMenu, 1, gs_subModeBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_subModeMenu, 2, gs_subModeBlink[2][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_subModeState == CHOICE_RUN ) {
        if ( ++gs_subModeBlinkFrame == 30 ) {
            gs_subModeBlinkFrame = 0;
        }
        choice = gs_subModeCursor;
        switch ( choice ) {
        case 0:
            func_8003893C( gs_subModeMenu, 0, gs_subModeBlink[0][gs_subModeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                gs_subModeCursor = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_subModeCursor = 2;
            }
            break;
        case 1:
            func_8003893C( gs_subModeMenu, 1, gs_subModeBlink[1][gs_subModeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_subModeCursor = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_subModeCursor = 2;
            }
            break;
        case 2:
            func_8003893C( gs_subModeMenu, 2, gs_subModeBlink[2][gs_subModeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x70, 0x90, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_subModeCursor = 0;
            }
            if ( D_800E6758 & R_JPAD ) {
                gs_subModeCursor = 1;
            }
            break;
        }
        if ( gs_subModeCursor != choice ) {
            gs_subModeBlinkFrame = 0;
            func_80026B28();
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( gs_subModeMenu, gs_subModeSlide );
    switch ( gs_subModeState ) {
    case CHOICE_IN:
        gs_subModeMenu[5].slide = SLIDE_LEFT;
        gs_subModeSlide -= 2;
        if ( gs_subModeSlide == 0 ) {
            gs_subModeState = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_subModeMenu[5].slide = SLIDE_RIGHT;
        gs_subModeSlide += 2;
        if ( gs_subModeSlide == 30 ) {
            if ( gs_subModeState == CHOICE_TAKEN ) {
                gs_subModeState = CHOICE_IN;
                return 1;
            }
            gs_subModeState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
