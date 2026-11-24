/* 59490.c -- game-select overlay: the menu of the five game modes */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

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
#ifdef NON_MATCHING
// NON_MATCHING: retail's frame is 0x60, this draft's 0x30 (0x30 bytes of unused stack, likely an inlined helper's locals); cases 1 to 3 of the first switch share one tail in retail. Banking it needs symbol_addrs entries gs_modeMenu size:0x78 and gs_modeBlink size:0x3C.
// An unused 48-byte local gives retail's frame (not adopted, R17); no callee here is a same-unit inline whose frame
// could carry it in.
s32 GS_ModeMenu( void ) {
    s32 i;
    u8 choice;

    if ( gs_modeState == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        switch ( gs_modeCursor ) {
        case 0:
            gs_modeState = CHOICE_TAKEN;
            game_mode = game_mode & 0xF0;
            break;
        case 1:
            game_mode = 1;
            gs_modeState = CHOICE_TAKEN;
            break;
        case 2:
            game_mode = 2;
            gs_modeState = CHOICE_TAKEN;
            break;
        case 3:
            game_mode = 3;
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
            Draw_RoundedBox( 0x38, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                gs_modeCursor = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_modeCursor = 2;
            }
            break;
        case 1:
            Menu_SetItem( gs_modeChoices, 2, gs_modeBlink[2][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0xA8, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_modeCursor = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                gs_modeCursor = 3;
            }
            break;
        case 2:
            Menu_SetItem( gs_modeChoices, 3, gs_modeBlink[3][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0x18, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                gs_modeCursor = 4;
            }
            if ( D_800E6758 & U_JPAD ) {
                gs_modeCursor = 0;
            }
            break;
        case 3:
            Menu_SetItem( gs_modeChoices, 5, gs_modeBlink[4][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0xC8, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_modeCursor = 4;
            }
            if ( D_800E6758 & U_JPAD ) {
                gs_modeCursor = 1;
            }
            break;
        case 4:
            Menu_SetItem( gs_modeChoices, 4, gs_modeBlink[0][gs_modeBlinkFrame / 5], MENU_NONE, MENU_NONE, 0xFF );
            Draw_RoundedBox( 0x70, 0x70, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_modeCursor = 2;
            } else if ( D_800E6758 & R_JPAD ) {
                gs_modeCursor = 3;
            } else if ( D_800E6758 & U_JPAD ) {
                gs_modeCursor = 0;
            } else if ( D_800E6758 & D_JPAD ) {
                gs_modeCursor = 3;
            }
            break;
        }
        if ( gs_modeCursor != choice ) {
            gs_modeBlinkFrame = 0;
            func_80026B28();
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
            func_80026B28();
            game_mode = 0;
            game_numPlayers = 1;
            for ( i = 3; i >= 0; i-- ) {
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
#else
INCLUDE_ASM("asm/nonmatchings/gscode/59490", GS_ModeMenu);
#endif
