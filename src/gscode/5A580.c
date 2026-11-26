/* 5A580.c -- game-select overlay: the menu of one or two players */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the menu of one or two players; [5]: its title */
static menuItem_t gs_playersMenu[9] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x863, 72, 12, 0xFF, SLIDE_LEFT },
    { 0, 0x989, 40, 84, 0xFF, SLIDE_LEFT },
    { 1, 0x9B3, 184, 84, 0xFF, SLIDE_RIGHT },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per choice: images of its blink */
static s16 gs_playersBlink[2][6] = {
    { 0x989, 0x988, 0x987, 0x986, 0x985, 0x984 },
    { 0x9B3, 0x9B2, 0x9B1, 0x9B0, 0x9AF, 0x9AE },
};

static u8 gs_playersSlide = 30; /* slide step of the menu of one or two players */
static u8 gs_playersState = 0; /* CHOICE_IN etc. */
static u8 gs_playersCursor = 0; /* highlighted choice: 0 one player, 1 two */
static u8 gs_playersBlinkFrame = 0; /* blink frame, 0 to 29 */

/*
================
GS_PlayersMenu

Frame of the menu of one or two players, a two-choice menu like GS_TwoChoiceMenu: taking the left
choice sets up one player on one team, the right one two players on two teams (game-mode argument 1).
Returns 0 while the menu runs; once it has slid out, 1 for a choice taken and 2 for backing out.
================
*/
s32 GS_PlayersMenu( void ) {
    u8 choice;

    if ( gs_playersState == CHOICE_RUN && ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_playersCursor ) {
        case 0:
            game_numPlayers = 1;
            game_lastTeam = 0;
            game_mode = game_mode & 0x3F;
            break;
        case 1:
            game_numPlayers = 2;
            game_lastTeam = 1;
            game_mode = ( game_mode & 0x3F ) | 0x40;
            break;
        }
        gs_playersState = CHOICE_TAKEN;
    }
    if ( gs_playersState == CHOICE_RUN && ( cont_allPressed & B_BUTTON ) ) {
        Sound_PlaySe( 2, 0 );
        gs_playersState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_SetItem( gs_playersMenu, 0, gs_playersBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    Menu_SetItem( gs_playersMenu, 1, gs_playersBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_playersState == CHOICE_RUN ) {
        if ( ++gs_playersBlinkFrame == 30 ) {
            gs_playersBlinkFrame = 0;
        }
        choice = gs_playersCursor;
        switch ( choice ) {
        case 0:
            Menu_SetItem( gs_playersMenu, 0, gs_playersBlink[0][gs_playersBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_playersCursor = 1;
            }
            break;
        case 1:
            Menu_SetItem( gs_playersMenu, 1, gs_playersBlink[1][gs_playersBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_playersCursor = 0;
            }
            break;
        }
        if ( gs_playersCursor != choice ) {
            gs_playersBlinkFrame = 0;
            Game_StartPulse();
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_DrawItems( gs_playersMenu, gs_playersSlide );
    switch ( gs_playersState ) {
    case CHOICE_IN:
        gs_playersMenu[5].slide = SLIDE_LEFT;
        gs_playersSlide -= 2;
        if ( gs_playersSlide == 0 ) {
            gs_playersState = CHOICE_RUN;
            Game_StartPulse();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_playersMenu[5].slide = SLIDE_RIGHT;
        gs_playersSlide += 2;
        if ( gs_playersSlide == 30 ) {
            if ( gs_playersState == CHOICE_TAKEN ) {
                gs_playersState = CHOICE_IN;
                return 1;
            }
            gs_playersState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
