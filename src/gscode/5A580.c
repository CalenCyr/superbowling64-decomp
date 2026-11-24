/* 5A580.c -- game-select overlay: the menu of one or two players */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

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

    if ( gs_playersState == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( gs_playersCursor ) {
        case 0:
            D_8005C161 = 1;
            D_8005C162 = 0;
            D_8005C160 = D_8005C160 & 0x3F;
            break;
        case 1:
            D_8005C161 = 2;
            D_8005C162 = 1;
            D_8005C160 = ( D_8005C160 & 0x3F ) | 0x40;
            break;
        }
        gs_playersState = CHOICE_TAKEN;
    }
    if ( gs_playersState == CHOICE_RUN && ( D_800E4D62 & B_BUTTON ) ) {
        func_80037870( 2, 0 );
        gs_playersState = CHOICE_BACK;
    }

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_8003893C( gs_playersMenu, 0, gs_playersBlink[0][0], MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_playersMenu, 1, gs_playersBlink[1][0], MENU_NONE, MENU_NONE, MENU_NONE );

    if ( gs_playersState == CHOICE_RUN ) {
        if ( ++gs_playersBlinkFrame == 30 ) {
            gs_playersBlinkFrame = 0;
        }
        choice = gs_playersCursor;
        switch ( choice ) {
        case 0:
            func_8003893C( gs_playersMenu, 0, gs_playersBlink[0][gs_playersBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0x28, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                gs_playersCursor = 1;
            }
            break;
        case 1:
            func_8003893C( gs_playersMenu, 1, gs_playersBlink[1][gs_playersBlinkFrame / 5],
                           MENU_NONE, MENU_NONE, MENU_NONE );
            Draw_RoundedBox( 0xB8, 0x54, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                gs_playersCursor = 0;
            }
            break;
        }
        if ( gs_playersCursor != choice ) {
            gs_playersBlinkFrame = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( gs_playersMenu, gs_playersSlide );
    switch ( gs_playersState ) {
    case CHOICE_IN:
        gs_playersMenu[5].slide = SLIDE_LEFT;
        gs_playersSlide -= 2;
        if ( gs_playersSlide == 0 ) {
            gs_playersState = CHOICE_RUN;
            func_80026B28();
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
