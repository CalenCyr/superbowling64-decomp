/* 5A990.c -- game-select overlay: a menu of two choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_TwoChoiceMenu

Frame of a two-choice menu that slides in and out: left and right move the cursor between the two
blinking images, A or Z takes the highlighted choice and B backs out. Returns 0 while the menu runs;
once it has slid out, 1 for the left choice, 3 for the right one and 2 for backing out.
================
*/
s32 GS_TwoChoiceMenu( void ) {
    u8 choice;

    if ( gs_twoChoiceState == CHOICE_RUN && ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_twoChoiceCursor ) {
        case 0:
            gs_twoChoiceState = CHOICE_TAKEN;
            break;
        case 1:
            gs_twoChoiceState = CHOICE_TAKEN;
            break;
        }
    }
    if ( gs_twoChoiceState == CHOICE_RUN && ( cont_allPressed & B_BUTTON ) ) {
        Sound_PlaySe( 2, 0 );
        gs_twoChoiceState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_SetItem( gs_twoChoiceMenu, 0, gs_twoChoiceBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    Menu_SetItem( gs_twoChoiceMenu, 1, gs_twoChoiceBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_twoChoiceState == CHOICE_RUN ) {
        if ( ++gs_twoChoiceBlinkFrame == 30 ) {
            gs_twoChoiceBlinkFrame = 0;
        }
        choice = gs_twoChoiceCursor;
        switch ( choice ) {
        case 0:
            Menu_SetItem( gs_twoChoiceMenu, 0, gs_twoChoiceBlink[0][gs_twoChoiceBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_twoChoiceCursor = 1;
            }
            break;
        case 1:
            Menu_SetItem( gs_twoChoiceMenu, 1, gs_twoChoiceBlink[1][gs_twoChoiceBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_twoChoiceCursor = 0;
            }
            break;
        }
        if ( gs_twoChoiceCursor != choice ) {
            gs_twoChoiceBlinkFrame = 0;
            func_80026B28();
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_DrawItems( gs_twoChoiceMenu, gs_twoChoiceSlide );
    switch ( gs_twoChoiceState ) {
    case CHOICE_IN:
        gs_twoChoiceMenu[5].slide = SLIDE_LEFT;
        gs_twoChoiceSlide -= 2;
        if ( gs_twoChoiceSlide == 0 ) {
            gs_twoChoiceState = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_twoChoiceMenu[5].slide = SLIDE_RIGHT;
        gs_twoChoiceSlide += 2;
        if ( gs_twoChoiceSlide == 30 ) {
            if ( gs_twoChoiceState == CHOICE_TAKEN ) {
                gs_twoChoiceState = CHOICE_IN;
                return ( gs_twoChoiceCursor << 1 ) | 1;
            }
            gs_twoChoiceState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
