/* 59490.c -- game-select overlay: the menu of the five game modes */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the menu of the five game modes, its background; [5]: its title */
static menuItem_t gs_modeMenu[12] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { 0, 0xA45, 76, 12, 0xFF, SLIDE_LEFT },
    { 4, 0x9A7, 112, 112, 0xFF, SLIDE_BOTTOM },
    { 1, 0x97D, 56, 48, 0xFF, SLIDE_LEFT },
    { 2, 0x98F, 168, 48, 0xFF, SLIDE_RIGHT },
    { 3, 0x99B, 24, 152, 0xFF, SLIDE_LEFT },
    { 5, 0x977, 200, 152, 0xFF, SLIDE_RIGHT },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* its choices, ids 1 to 5 */
static menuItem_t gs_modeChoices[6] = {
    { 4, 0x9A7, 112, 112, 0xFF, SLIDE_BOTTOM },
    { 1, 0x97D, 56, 48, 0xFF, SLIDE_LEFT },
    { 2, 0x98F, 168, 48, 0xFF, SLIDE_RIGHT },
    { 3, 0x99B, 24, 152, 0xFF, SLIDE_LEFT },
    { 5, 0x977, 200, 152, 0xFF, SLIDE_RIGHT },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per item of gs_modeChoices: images of its blink */
static s16 gs_modeBlink[5][6] = {
    { 0x9A7, 0x9A6, 0x9A5, 0x9A4, 0x9A3, 0x9A2 },
    { 0x97D, 0x97C, 0x97B, 0x97A, 0x979, 0x978 },
    { 0x98F, 0x98E, 0x98D, 0x98C, 0x98B, 0x98A },
    { 0x99B, 0x99A, 0x999, 0x998, 0x997, 0x996 },
    { 0x977, 0x976, 0x975, 0x974, 0x973, 0x972 },
};

static u8 gs_modeSlide = 30; /* slide step of the menu of the five game modes */
static u8 gs_modeState = 0; /* CHOICE_IN etc. */
static u8 gs_modeCursor = 0; /* highlighted choice, 0 to 4 */
static u8 gs_modeBlinkFrame = 0; /* blink frame, 0 to 29 */

/*
================
GS_ModeMenu

Frame of the first menu, five choices laid out as two rows of two around one in the middle (choice 4):
the pad moves the cursor between neighbours and A or Z takes the highlighted choice, which sets the
game mode in the low nibble of game_mode. There is no backing out. Once the menu has slid in it
resets the game setup to one player with no characters. Returns 0 while the menu runs and 1 once it
has slid out after a choice.
================
*/
s32 GS_ModeMenu( void ) {
    s32 i;
    u8 choice;
    // R36: unused in the original: retail's frame reserves 0x30 bytes no instruction touches (type unknown)
    s32 unused[12];

    if ( gs_modeState == CHOICE_RUN && ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_modeCursor ) {
        case 0:
            gs_modeState = CHOICE_TAKEN;
            game_mode = game_mode & 0xF0;
            break;
        case 1:
            game_mode = 1;
            goto taken;
        case 2:
            game_mode = 2;
            goto taken;
        case 3:
            game_mode = 3;
        taken:
            /* the three plain modes share this store; it is not repeated in each case */
            gs_modeState = CHOICE_TAKEN;
            break;
        case 4:
            gs_modeState = CHOICE_TAKEN;
            game_mode = ( game_mode & 0xF0 ) | 4;
            break;
        }
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_DrawItems( gs_modeMenu, gs_modeSlide );
    for ( i = 0; i < 5; i++ ) {
        Menu_SetItem( gs_modeChoices, i + 1, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }

    if ( gs_modeState == CHOICE_RUN ) {
        if ( ++gs_modeBlinkFrame == 30 ) {
            gs_modeBlinkFrame = 0;
        }
        choice = gs_modeCursor;
        switch ( choice ) {
        case 0:
            Menu_SetItem( gs_modeChoices, 1, gs_modeBlink[1][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0x38, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_modeCursor = 1;
            }
            if ( cont_allDpadRepeat & D_JPAD ) {
                gs_modeCursor = 2;
            }
            break;
        case 1:
            Menu_SetItem( gs_modeChoices, 2, gs_modeBlink[2][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0xA8, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_modeCursor = 0;
            }
            if ( cont_allDpadRepeat & D_JPAD ) {
                gs_modeCursor = 3;
            }
            break;
        case 2:
            Menu_SetItem( gs_modeChoices, 3, gs_modeBlink[3][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0x18, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & R_JPAD ) {
                gs_modeCursor = 4;
            }
            if ( cont_allDpadRepeat & U_JPAD ) {
                gs_modeCursor = 0;
            }
            break;
        case 3:
            Menu_SetItem( gs_modeChoices, 5, gs_modeBlink[4][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0xC8, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_modeCursor = 4;
            }
            if ( cont_allDpadRepeat & U_JPAD ) {
                gs_modeCursor = 1;
            }
            break;
        case 4:
            Menu_SetItem( gs_modeChoices, 4, gs_modeBlink[0][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0x70, 0x70, 0x60, 0x48, 0xE0, 0xE0, 0xE0, game_pulseAlpha );
            if ( cont_allDpadRepeat & L_JPAD ) {
                gs_modeCursor = 2;
            } else if ( cont_allDpadRepeat & R_JPAD ) {
                gs_modeCursor = 3;
            } else if ( cont_allDpadRepeat & U_JPAD ) {
                gs_modeCursor = 0;
            } else if ( cont_allDpadRepeat & D_JPAD ) {
                gs_modeCursor = 3;
            }
            break;
        }
        if ( gs_modeCursor != choice ) {
            gs_modeBlinkFrame = 0;
            Game_StartPulse();
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_DrawItems( gs_modeChoices, gs_modeSlide );
    switch ( gs_modeState ) {
    case CHOICE_IN:
        gs_modeMenu[5].slide = SLIDE_LEFT;
        gs_modeSlide -= 2;
        if ( gs_modeSlide == 0 ) {
            gs_modeState = CHOICE_RUN;
            Game_StartPulse();
            game_mode = 0;
            game_numPlayers = 1;
            for ( i = 0; i < 4; i++ ) {
                game_players[i] = 0;
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_modeMenu[5].slide = SLIDE_RIGHT;
        gs_modeSlide += 2;
        if ( gs_modeSlide == 30 ) {
            if ( gs_modeState == CHOICE_TAKEN ) {
                gs_modeState = CHOICE_IN;
                return 1;
            }
            gs_modeState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
