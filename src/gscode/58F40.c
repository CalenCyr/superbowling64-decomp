/* 58F40.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_Init

Entry of the overlay: starts BGM 1; coming back from a game (D_8005CB60 set) opens screen 6, else clears
the players and opens screen 0.
================
*/
void GS_Init( void ) {
    s32 i;

    Sound_PlayBgm( 1 );
    if ( !D_8005CB60 ) {
        for ( i = 0; i < 4; i++ ) {
            D_8005C0E4[i] = 0;
            D_800DF5D0[i] = 0;
        }
        D_8005C163 = 0;
        gs_screen = 0;
    } else {
        gs_screen = 6;
    }
}

/*
================
GS_PushScreen

Pushes a screen to go back to.
================
*/
void GS_PushScreen( u8 screen ) {
    gs_screenStack[gs_screenDepth++] = screen;
}

/*
================
GS_PopScreen

Goes back to the last pushed screen.
================
*/
void GS_PopScreen( void ) {
    gs_screen = gs_screenStack[--gs_screenDepth];
}

/*
================
GS_Frame

Frame of the game-select overlay: runs the current screen and, on its result, pushes it and opens the
next screen, or goes back to the last pushed one. The screens after the first depend on the game mode.
Declared s32 but returns nothing (the caller ignores it): the original's return register is live at the
exit, which keeps the delay slot of the game-mode switch's range check empty.
================
*/
s32 GS_Frame( void ) {
    switch ( gs_screen ) {
    case 0:
        if ( GS_ModeMenu() == 1 ) {
            GS_PushScreen( 0 );
            switch ( D_8005C160 & 0xF ) {
            case 0:
                gs_screen = 1;
                break;
            case 1:
                D_8005C162 = 1;
                D_8005C161 = 2;
                D_8005C0E4[1] = 0x81;
                D_8005C0E4[0] = 0;
                gs_screen = 4;
                break;
            case 2:
                D_8005C161 = 1;
                D_8005C162 = 0;
                D_8005C0E4[0] = 0;
                gs_screen = 6;
                break;
            case 3:
                D_8005C162 = 1;
                D_8005C161 = 2;
                gs_screen = 5;
                break;
            case 4:
                gs_screen = 8;
                break;
            }
        }
        break;
    case 1:
        switch ( GS_SubModeMenu() ) {
        case 1:
            GS_PushScreen( 1 );
            switch ( ( D_8005C160 >> 4 ) & 3 ) {
            case 0:
                gs_screen = 2;
                break;
            case 1:
                gs_screen = 3;
                break;
            case 2:
                gs_screen = 5;
                break;
            }
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 2:
        switch ( GS_TeamSizeMenu() ) {
        case 1:
            GS_PushScreen( 2 );
            gs_screen = 5;
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 3:
        switch ( GS_PlayersMenu() ) {
        case 1:
            GS_PushScreen( 3 );
            gs_screen = 5;
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 4:
        switch ( GS_TwoChoiceMenu() ) {
        case 1:
            GS_PushScreen( 4 );
            gs_screen = 6;
            break;
        case 2:
            GS_PopScreen();
            break;
        case 3:
            GS_PushScreen( 4 );
            gs_screen = 9;
            Sound_PlayBgm( 13 );
            break;
        }
        break;
    case 5:
        switch ( GS_PlayerSetup() ) {
        case 1:
            GS_PushScreen( 5 );
            gs_screen = 6;
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 6:
        switch ( GS_TeamSelect() ) {
        case 1:
            GS_PushScreen( 6 );
            switch ( D_8005C160 & 0xF ) {
            case 0:
            case 2:
            case 3:
                gs_screen = 7;
                break;
            case 1:
                D_8005C0BF = 2;
                D_8005C0B6 = 0;
                D_8005C163 = D_8005C0E4[1] & 0xF;
                break;
            }
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 7:
        switch ( GS_CharacterSelect() ) {
        case 1:
            D_8005C0BF = 2;
            D_8005C0B6 = 0;
            break;
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 8:
        switch ( GS_Options() ) {
        case 1:
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    case 9:
        switch ( GS_CharacterMenu() ) {
        case 1:
        case 2:
            GS_PopScreen();
            break;
        }
        break;
    }
}

/*
================
GS_NullCreate
================
*/
void GS_NullCreate( void ) {
}

/*
================
GS_NullHandler
================
*/
void GS_NullHandler( void ) {
}
