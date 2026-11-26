/* 5A990.c -- game-select overlay: a menu of two choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the two-choice menu; [5]: its title */
static menuItem_t gs_twoChoiceMenu[9] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x863, 72, 12, 0xFF, SLIDE_LEFT },
    { 0, 0x9A1, 40, 84, 0xFF, SLIDE_LEFT },
    { 1, 0x9C4, 184, 84, 0xFF, SLIDE_RIGHT },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per choice: images of its blink */
static s16 gs_twoChoiceBlink[2][6] = {
    { 0x9A1, 0x9A0, 0x99F, 0x99E, 0x99D, 0x99C },
    { 0x9C4, 0x9C3, 0x9C2, 0x9C1, 0x9C0, 0x9BF },
};

static u8 gs_twoChoiceSlide = 30; /* slide step of the two-choice menu */
static u8 gs_twoChoiceState = 0; /* CHOICE_IN etc. */
static u8 gs_twoChoiceCursor = 0; /* highlighted choice, 0 or 1 */
static u8 gs_twoChoiceBlinkFrame = 0; /* blink frame, 0 to 29 */

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
            Game_StartPulse();
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
            Game_StartPulse();
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
