/* 58F40.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* The classes of the overlay's scenes, by objParams_t.type: 0 and 10 scene objects, 1 and 11 cameras; the rest
   unused. Units 5E2B0, 5EC00 and 61640 create their scenes with them. */
objClass_t gs_classes[] = {
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_Draw },
    { Cam_Create, (objUpdateFunc_t)Cam_UpdateAndAdvance },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { (objCreateFunc_t)GS_NullCreate, (objUpdateFunc_t)GS_NullHandler },
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_AddToDrawList },
    { Cam_Create, (objUpdateFunc_t)Cam_Update },
};

static s32 gs_screenDepth = 0; /* entries in gs_screenStack */

/* The unit's uninitialised state: file-scope statics, which the assembler allocates in .bss in definition order. */
static u8 gs_screen;            /* current screen */
static u8 gs_screenStack[15];   /* screens to go back to, a stack (its size is unknown: up to the end of the unit's .bss) */

/*
================
GS_Init

Entry of the overlay: starts BGM 1; coming back from a game (game_replay set) opens screen 6, else clears
the players and opens screen 0.
================
*/
void GS_Init( void ) {
    s32 i;

    Sound_PlayBgm( 1 );
    if ( !game_replay ) {
        for ( i = 0; i < 4; i++ ) {
            game_players[i] = 0;
            score_plateNumbers[i] = 0;
        }
        game_stage = 0;
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
            switch ( game_mode & 0xF ) {
            case 0:
                gs_screen = 1;
                break;
            case 1:
                game_lastTeam = 1;
                game_numPlayers = 2;
                game_players[1] = 0x81;
                game_players[0] = 0;
                gs_screen = 4;
                break;
            case 2:
                game_numPlayers = 1;
                game_lastTeam = 0;
                game_players[0] = 0;
                gs_screen = 6;
                break;
            case 3:
                game_lastTeam = 1;
                game_numPlayers = 2;
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
            switch ( ( game_mode >> 4 ) & 3 ) {
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
            switch ( game_mode & 0xF ) {
            case 0:
            case 2:
            case 3:
                gs_screen = 7;
                break;
            case 1:
                game_leaveState = 2;
                game_scene = 0;
                game_stage = game_players[1] & 0xF;
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
            game_leaveState = 2;
            game_scene = 0;
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
