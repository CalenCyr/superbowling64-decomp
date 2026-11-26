/* 5A020.c -- game-select overlay: a second menu of three choices */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the second three-choice menu; [5]: its title */
static menuItem_t gs_teamSizeMenu[10] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x863, 72, 12, 0xFF, SLIDE_LEFT },
    { 0, 0x995, 40, 56, 0xFF, SLIDE_LEFT },
    { 1, 0x983, 184, 56, 0xFF, SLIDE_RIGHT },
    { 2, 0x9BE, 112, 144, 0xFF, SLIDE_BOTTOM },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per choice: images of its blink */
static s16 gs_teamSizeBlink[3][6] = {
    { 0x995, 0x994, 0x993, 0x992, 0x991, 0x990 },
    { 0x983, 0x982, 0x981, 0x980, 0x97F, 0x97E },
    { 0x9BE, 0x9BD, 0x9BC, 0x9BB, 0x9BA, 0x9B9 },
};

static u8 gs_teamSizeSlide = 30; /* slide step of the second three-choice menu */
static u8 gs_teamSizeState = 0; /* CHOICE_IN etc. */
static u8 gs_teamSizeCursor = 0; /* highlighted choice: 0 left, 1 right, 2 below */
static u8 gs_teamSizeBlinkFrame = 0; /* blink frame, 0 to 29 */

/*
================
GS_TeamSizeMenu

Frame of a three-choice menu like GS_SubModeMenu, choosing the number of players on one team:
the left choice sets up one player, the other two set up two players, with argument 2 (right) or
1 (below) in bits 6-7 of game_mode. Up from the lower choice moves to the right one. Returns 0
while the menu runs; once it has slid out, 1 for a choice taken and 2 for backing out.
================
*/
s32 GS_TeamSizeMenu( void ) {
    u8 choice;

    if ( gs_teamSizeState == CHOICE_RUN && ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_teamSizeCursor ) {
        case 0:
            game_numPlayers = 1;
            game_lastTeam = 0;
            game_mode = game_mode & 0x3F;
            break;
        case 1:
            game_numPlayers = 2;
            game_lastTeam = 0;
            game_mode = ( game_mode & 0x3F ) | 0x80;
            break;
        case 2:
            game_numPlayers = 2;
            game_lastTeam = 0;
            game_mode = ( game_mode & 0x3F ) | 0x40;
            break;
        }
        gs_teamSizeState = CHOICE_TAKEN;
    }
    if ( gs_teamSizeState == CHOICE_RUN && ( cont_allPressed & B_BUTTON ) ) {
        Sound_PlaySe( 2, 0 );
        gs_teamSizeState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_SetItem( gs_teamSizeMenu, 0, gs_teamSizeBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    Menu_SetItem( gs_teamSizeMenu, 1, gs_teamSizeBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );
    Menu_SetItem( gs_teamSizeMenu, 2, gs_teamSizeBlink[2][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_teamSizeState == CHOICE_RUN ) {
        if ( ++gs_teamSizeBlinkFrame == 30 ) {
            gs_teamSizeBlinkFrame = 0;
        }
        choice = gs_teamSizeCursor;
        switch ( choice ) {
        case 0:
            Menu_SetItem( gs_teamSizeMenu, 0, gs_teamSizeBlink[0][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            if ( cont_allDpadRepeat & D_JPAD ) {
                gs_teamSizeCursor = 2;
            }
            break;
        case 1:
            Menu_SetItem( gs_teamSizeMenu, 1, gs_teamSizeBlink[1][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x38, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_teamSizeCursor = 0;
            }
            if ( cont_allDpadRepeat & D_JPAD ) {
                gs_teamSizeCursor = 2;
            }
            break;
        case 2:
            Menu_SetItem( gs_teamSizeMenu, 2, gs_teamSizeBlink[2][gs_teamSizeBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x70, 0x90, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_teamSizeCursor = 0;
            }
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            if ( cont_allDpadRepeat & U_JPAD ) {
                gs_teamSizeCursor = 1;
            }
            break;
        }
        if ( gs_teamSizeCursor != choice ) {
            gs_teamSizeBlinkFrame = 0;
            Game_StartPulse();
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_DrawItems( gs_teamSizeMenu, gs_teamSizeSlide );
    switch ( gs_teamSizeState ) {
    case CHOICE_IN:
        gs_teamSizeMenu[5].slide = SLIDE_LEFT;
        gs_teamSizeSlide -= 2;
        if ( gs_teamSizeSlide == 0 ) {
            gs_teamSizeState = CHOICE_RUN;
            Game_StartPulse();
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
