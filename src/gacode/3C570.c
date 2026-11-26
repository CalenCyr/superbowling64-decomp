/* 3C570.c -- game overlay: the main loop states */

#include "common.h"
#include "libultra.h"
#include "dbgtext.h"
#include "archive.h"
#include "input.h"
#include "gacode/gacode.h"

/* ===== Data ===== */

s16 ga_blinkLevel = 0;
static s8 ga_blinkFalling = 0;  /* 1: ga_blinkLevel falls, else it rises */
static u8 ga_gameDone = 0;      /* what the main loop state returned: nonzero, the game is done */
static u8 ga_leaveGame = 0;     /* what the pause menu's state returned: nonzero, leave the game */
/* the pause menu's pages (GA_PageBrowser) */
static gaMenuPage_t ga_menuPages[15] = {
    { 0x6E, 0, 5, 0x970, 1, -1 },
    { 0x66, -1, 0, 0x96F, 0, -1 },
    { 0x4E, -1, 0, 0x96E, 0, -1 },
    { 0x4E, 0, 3, 0x96D, 6, 0 },
    { 0x8E, -1, 0, 0x965, 13, -1 },
    { 0x5E, -1, 0, 0x963, 14, -1 },
    { 0x3E, -1, 0, 0x96C, 9, -1 },
    { 0x3E, -1, 0, 0x96A, 10, -1 },
    { 0x7E, -1, 0, 0x967, 12, -1 },
    { 0x76, -1, 0, 0x96B, 3, -1 },
    { 0x76, -1, 0, 0x969, 11, -1 },
    { 0x4E, -1, 0, 0x968, 3, -1 },
    { 0x8E, -1, 0, 0x966, 3, -1 },
    { 0x8E, -1, 0, 0x964, 0, -1 },
    { 0x66, -1, 0, 0x962, 0, -1 },
};
static u8 ga_menuPage = 0;      /* current page of ga_menuPages */
/* per pair of players: the index the turn passes to next (GA_PairGameLoop) */
static u8 ga_pairNext[2] = { 1, 2 };

/*
================
GA_Init

The overlay's init: stops the BGM, resets the teams' current players and the pause menu, enters the pause
menu's state as the state to come back to, loads the stage's floor grid, places each player's character in
the stage's scene and creates it, starts the stage's sound, sets up the lanes, clears the games won and the
scores, sets up the game mode, takes two options from save_data.unlocks and starts the game's BGM.
================
*/
void GA_Init( void ) {
    s32 character;
    s32 i;
    objParams_t *params;

    Sound_StopBgm();
    Sound_PlaySe( 0x36, 0 );
    ga_teamCurrent[0] = ga_teamCurrent[1] = 0;
    ga_stage = &ga_stages[game_stage];
    ga_paused = 0;
    ga_parFrame = 0;
    ga_pauseCursor = 0;
    ga_state = GA_PauseMenu;
    Str_ReadFile( ga_stage->floorGridFile, (u8 *)&game_floorGrids[0] );
    params = ga_stage->scene;
    for ( i = 0; i < game_numPlayers; i++, params++ ) {
        character = game_players[i] & 0xF;
        params->args[0] = ga_characterFiles[character][0];
        params->args[1] = ga_characterFiles[character][1];
        params->args[4] = ga_characterData[character].unk_04;
    }
    Obj_CreateAll( ga_classes, ga_stage->scene );
    if ( ga_stage->ambientSound >= 0 ) {
        ga_sound = Sound_PlaySe( ga_stage->ambientSound, 0 );
    } else {
        ga_sound = 0;
    }
    GA_SetupLanes( game_lastTeam, ga_stage->laneOffset, ( game_mode & 0xF ) == 3 );
    GA_ClearScores();
    for ( i = 0; i < 2; i++ ) {
        game_gamesWon[i] = 0;
        game_duelScores[i][0] = game_duelScores[i][1] = game_duelScores[i][2] = 0;
    }
    GA_SetupMode();
    ga_earlyCommit = ( save_data.unlocks >> 9 ) & 1;
    ga_steerBall = ( save_data.unlocks >> 8 ) & 1;
    Sound_PlayBgm( ga_bgm );
}

/*
================
GA_Frame

The overlay's frame: leaves once main has faded out after a request, requests leaving when the game mode
is done or the pause menu quits, opens the pause menu on Start and, while C-up is held (not in mode 2),
GA_DrawScoreboard's view, runs the stage and the players, draws them, runs the main loop state (or, paused, the pause menu's state), and steps the
blink of ga_blinkLevel between 0 and 0x80.
================
*/
void GA_Frame( void ) {
    s32 team;
    s32 cUpHeld = FALSE;

    if ( game_leaveState == 1 && game_fade == 0xFF ) {
        GA_StopLaneSound( &ga_sound );
        game_leaveState = 2;
        game_fade = 0x107;
        game_scene = ga_gameDone == 1 ? 6 : 4;
    }
    if ( game_leaveState == 0 ) {
        if ( ga_gameDone ) {
            Sound_PlaySe( 0x7F, 0 );
            game_leaveState = 1;
        } else if ( ga_paused && ga_leaveGame ) {
            Sound_PlaySe( 0x32, 0 );
            game_replay = 0;
            game_leaveState = 1;
        }
        if ( !ga_paused ) {
            if ( cont_allPressed & START_BUTTON ) {
                Sound_PlaySe( 6, 0 );
                ga_paused = 1;
                cont_rumble[0] = cont_rumble[1] = 8;
                cont_rumbleFrames[0] = cont_rumbleFrames[1] = 0;
            }
            if ( ( game_mode & 0xF ) != 2 && ( ( cont_held[0] | cont_held[1] ) & U_CBUTTONS ) ) {
                cUpHeld = TRUE;
                cont_rumble[0] = cont_rumble[1] = 8;
                cont_rumbleFrames[0] = cont_rumbleFrames[1] = 0;
            }
        }
    }
    if ( !ga_paused ) {
        ga_pauseCursor = 0;
    }
    ga_stage->update();
    Obj_Frame();
    for ( team = 0; team <= game_lastTeam; team++ ) {
        GA_HoldBall( ga_teamPlayers[team][ga_teamCurrent[team]] );
    }
    Txt_LoadGlyphSheet( &game_gfx );
    Draw_FillRect( 0x9F, 0, 2, 0xF0, 0, 0, 0, 0xFF );
    if ( !ga_paused && !cUpHeld ) {
        GA_RunPanelFunc( ga_teamPlayers[0][ga_teamCurrent[0]] );
        if ( game_lastTeam ) {
            GA_RunPanelFunc( ga_teamPlayers[1][ga_teamCurrent[1]] );
        } else {
            Draw_ImageScaled( 0x926, 0xD2, 0x10, (u8)ga_blinkLevel + 0x40, 1.0f, 1.0f );
        }
    }
    if ( !ga_paused && !cUpHeld ) {
        ga_gameDone = ga_modeLoop();
        if ( !game_lastTeam ) {
            GA_UpdateLaneCamera( ga_teamPlayers[0][ga_teamCurrent[0]] );
        }
    } else {
        for ( team = 0; team <= game_lastTeam; team++ ) {
            GA_FreezePlayerObject( ga_teamPlayers[team][ga_teamCurrent[team]] );
        }
    }
    if ( ga_paused ) {
        Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x10, 0x10, 0x10, 0x80 );
        ga_leaveGame = ga_state();
    }
    if ( cUpHeld ) {
        GA_DrawScoreboard();
    }
    if ( ga_blinkFalling == 1 ) {
        ga_blinkLevel -= 8;
        if ( ga_blinkLevel <= 0 ) {
            ga_blinkLevel = 0;
            ga_blinkFalling = 0;
        }
    } else {
        ga_blinkLevel += 8;
        if ( ga_blinkLevel >= 0x80 ) {
            ga_blinkLevel = 0x80;
            ga_blinkFalling = 1;
        }
    }
    Txt_LoadFont( &game_gfx );
    Txt_SetupFont( &game_gfx, 0 );
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
}

/*
================
GA_FreezeScene
================
*/
void GA_FreezeScene( void ) {
    s32 team;

    GA_StopLaneSound( &ga_sound );
    for ( team = 0; team <= game_lastTeam; team++ ) {
        GA_FreezePlayerObject( ga_teamPlayers[team][ga_teamCurrent[team]] );
    }
}

/*
================
GA_PauseMenu

Pause menu: draws it with the cursor on one of its four items and the on/off mark of save_data.music,
moves the cursor with the D-pad, and on A or Z runs the item; B closes the menu.
================
*/
s32 GA_PauseMenu( void ) {
    s16 cursorY[4] = { 0x8A, 0x5A, 0x6A, 0x7A };
    s16 markX[2] = { 0xB6, 0x9C };
    u8 isMode2;

    isMode2 = ( game_mode & 0xF ) == 2;
    Draw_Image( isMode2 ? 0x882 : 0x89E, 0x50, 0x40 );
    Draw_ImageScaled( 0x8A0, 0x66, cursorY[ga_pauseCursor], (u8)ga_blinkLevel + 0x40, 1.0f, 1.0f );
    Draw_ImageScaled( 0x89F, markX[save_data.music], 0x6C, (u8)ga_blinkLevel + 0x40, 1.0f, 1.0f );
    if ( cont_allDpadRepeat & U_JPAD ) {
        Sound_PlaySe( 0, 0 );
        if ( ga_pauseCursor == 0 ) {
            ga_pauseCursor = 3;
        } else {
            ga_pauseCursor--;
        }
    }
    if ( cont_allDpadRepeat & D_JPAD ) {
        Sound_PlaySe( 0, 0 );
        if ( ga_pauseCursor == 3 ) {
            ga_pauseCursor = 0;
        } else {
            ga_pauseCursor++;
        }
    }
    if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
        switch ( ga_pauseCursor ) {
        case 0:
            ga_paused = 0;
            Sound_PlaySe( 1, 0 );
            break;
        case 1:
            ga_state = isMode2 ? GA_PageBrowser : GA_PauseScoreboard;
            Sound_PlaySe( 1, 0 );
            break;
        case 2:
            save_data.music = !save_data.music;
            if ( save_data.music ) {
                Sound_PlayBgm( ga_bgm );
            } else {
                Sound_StopBgm();
            }
            break;
        case 3:
            ga_state = GA_PauseQuitPrompt;
            Sound_PlaySe( 1, 0 );
            break;
        }
    } else if ( cont_allPressed & B_BUTTON ) {
        ga_paused = 0;
        Sound_PlaySe( 2, 0 );
    }
    return 0;
}

/*
================
GA_PauseScoreboard
================
*/
s32 GA_PauseScoreboard( void ) {
    Draw_ImageScaled( 0x89E, 0x50, 0x40, 0x40, 1.0f, 1.0f );
    GA_DrawScoreboard();
    if ( cont_allPressed & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( 11, 0 );
        ga_state = GA_PauseMenu;
    }
    return 0;
}

/*
================
GA_DrawScoreboard
================
*/
void GA_DrawScoreboard( void ) {
    if ( ( game_mode & 0xF ) == 3 ) {
        GA_DrawDistancePanels();
        return;
    }
    switch ( ( game_mode >> 4 ) & 3 ) {
    case 0:
        Score_DrawTeamBoard( game_mode >> 6, 0, 0x18, 0x30 );
        break;
    case 1:
        Score_DrawParCard( game_mode >> 6, 0, 0x20, 0x30 );
        break;
    case 2:
        Score_DrawAllBoards( 0, 0x18, 0x30 );
        break;
    }
}

/*
================
GA_PageBrowser

Main loop state of the pause menu's page browser (mode 2): draws the current page of ga_menuPages with
the cursor on an item or on the back item, moves the cursor with the D-pad, and on A or Z opens the item's
page or goes back to the parent page (from the first page, to the pause menu); B goes back to the pause menu.
================
*/
s32 GA_PageBrowser( void ) {
    s16 x = 0x28;
    s16 y = 0x28;

    Draw_Image( ga_menuPages[ga_menuPage].image, x, y );
    if ( ga_menuPages[ga_menuPage].cursor >= 0
        && ga_menuPages[ga_menuPage].cursor < ga_menuPages[ga_menuPage].backItem ) {
        Draw_ImageScaled( 0x885, x + 8, y + 0x18 + ga_menuPages[ga_menuPage].cursor * 16,
                       (u8)ga_blinkLevel + 0x7F, 1.0f, 1.0f );
    } else {
        Draw_ImageScaled( 0x884, x + 0xBB, y + ga_menuPages[ga_menuPage].backY,
                       (u8)ga_blinkLevel + 0x10, 1.0f, 1.0f );
    }
    if ( ga_menuPages[ga_menuPage].cursor >= 0 ) {
        if ( cont_allDpadRepeat & U_JPAD ) {
            Sound_PlaySe( 0, 0 );
            if ( ga_menuPages[ga_menuPage].cursor != 0 ) {
                ga_menuPages[ga_menuPage].cursor--;
            } else {
                ga_menuPages[ga_menuPage].cursor = ga_menuPages[ga_menuPage].backItem;
            }
        }
        if ( cont_allDpadRepeat & D_JPAD ) {
            Sound_PlaySe( 0, 0 );
            if ( ga_menuPages[ga_menuPage].cursor != ga_menuPages[ga_menuPage].backItem ) {
                ga_menuPages[ga_menuPage].cursor++;
            } else {
                ga_menuPages[ga_menuPage].cursor = 0;
            }
        }
        if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
            if ( ga_menuPages[ga_menuPage].cursor == ga_menuPages[ga_menuPage].backItem ) {
                ga_menuPages[ga_menuPage].cursor = 0;
                if ( ga_menuPages[ga_menuPage].parent < 0 ) {
                    Sound_PlaySe( 11, 0 );
                    ga_menuPage = 0;
                    ga_state = GA_PauseMenu;
                } else {
                    Sound_PlaySe( 0x33, 0 );
                    ga_menuPage = ga_menuPages[ga_menuPage].parent;
                }
            } else {
                Sound_PlaySe( 0x33, 0 );
                ga_menuPage = ga_menuPages[ga_menuPage].firstPage
                                  + ga_menuPages[ga_menuPage].cursor;
            }
        }
    } else if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( 0x33, 0 );
        ga_menuPage = ga_menuPages[ga_menuPage].firstPage;
    }
    if ( cont_allPressed & B_BUTTON ) {
        Sound_PlaySe( 2, 0 );
        ga_menuPage = 0;
        ga_state = GA_PauseMenu;
    }
    return 0;
}

/*
================
GA_PauseQuitPrompt
================
*/
s32 GA_PauseQuitPrompt( void ) {
    switch ( Menu_TwoChoicePrompt() ) {
    case 1:
        ga_state = GA_PauseSaveSettings;
        break;
    case 2:
        ga_state = GA_PauseMenu;
        break;
    }
    return 0;
}

/*
================
GA_PauseSaveSettings
================
*/
s32 GA_PauseSaveSettings( void ) {
    if ( Save_SaveSettings() ) {
        return 0;
    }
    ga_state = GA_PauseQuitDone;
    return 1;
}

/*
================
GA_PauseQuitDone
================
*/
s32 GA_PauseQuitDone( void ) {
    return 0;
}

/*
================
GA_SetupMode

Sets up the game for the mode in the low nibble of game_mode, with its BGM.
================
*/
void GA_SetupMode( void ) {
    switch ( game_mode & 0xF ) {
    case 0:
        GA_SetupStandardGame();
        break;
    case 1:
        GA_StartMode( GA_DuelLoop, GA_ScoreStandard );
        ga_bgm = ga_stage->bgm;
        break;
    case 3:
        GA_StartMode( GA_DistanceLoop, GA_ScoreBattle );
        ga_bgm = 17;
        break;
    case 2:
        GA_StartMode( GA_RunFirstPlayer, GA_ScorePinEditor );
        ga_bgm = 16;
        break;
    }
    GA_FillTeams();
}

/*
================
GA_SetupStandardGame

Sets up the game of mode 0 by its sub-mode (bits 4-5 of game_mode): its main loop state and handler, the
frames' setups (the par game also sums its pars into game_splits[0]) and the BGM.
================
*/
void GA_SetupStandardGame( void ) {
    s32 i;

    switch ( ( game_mode >> 4 ) & 3 ) {
    case 0:
        /* the arguments that share a state still have a case each (retail keeps one block per case) */
        switch ( game_mode >> 6 ) {
        case 0:
            GA_StartMode( GA_TeamGameLoop, GA_ScoreStandard );
            break;
        case 2:
            GA_StartMode( GA_TeamGameLoop, GA_ScoreStandard );
            break;
        case 1:
            GA_StartMode( GA_PairGameLoop, GA_ScoreStandard );
            break;
        }
        ga_bgm = ga_stage->bgm;
        break;
    case 1:
        GA_ShuffleOrder( game_setup.frameSetups, 25, 0 );
        for ( i = 0; i < 9; i++ ) {
            game_frameFlags[0][i] = ga_parPinSets[game_setup.frameSetups[i]].par;
            game_splits[0] += game_frameFlags[0][i];
        }
        /* the arguments that share a state still have a case each (retail keeps one block per case) */
        switch ( game_mode >> 6 ) {
        case 0:
            GA_StartMode( GA_ParGameLoop, GA_ScorePar );
            break;
        case 1:
            GA_StartMode( GA_ParGameLoop, GA_ScorePar );
            break;
        }
        ga_bgm = ga_stage->bgm;
        break;
    case 2:
        GA_ShuffleOrder( &game_setup.frameSetups[0], 5, 0 );
        GA_ShuffleOrder( &game_setup.frameSetups[3], 5, 5 );
        GA_ShuffleOrder( &game_setup.frameSetups[6], 5, 10 );
        game_setup.frameSetups[9] = 15;
        for ( i = 0; i < game_numPlayers; i++ ) {
            game_throw[i] = 5;
        }
        GA_StartMode( GA_TeamGameLoop, GA_ScoreSetupGame );
        ga_bgm = 11;
        break;
    }
}

/*
================
GA_StartMode

Starts a game mode: sets the main loop state and the mode's handler, picks the handler that starts a turn,
and sets up each player on the lane of their team (with two players the second, with three or four players
2 and up, on the second lane, when there are two teams); players without a controller get port -1.
================
*/
void GA_StartMode( gaState_t state, gaPlayerFunc_t mode ) {
    s32 player;
    u8 team;
    u8 lane;

    ga_modeLoop = state;
    ga_gameMode = mode;
    if ( mode == GA_ScorePinEditor ) {
        ga_endThrowFunc = GA_EndThrowNextTurn;
    } else if ( mode == GA_ScorePar ) {
        ga_endThrowFunc = GA_EndThrowFrame;
    } else if ( mode == GA_ScoreSetupGame ) {
        ga_endThrowFunc = GA_EndThrowNextTurn;
    } else if ( mode == GA_ScoreBattle ) {
        ga_endThrowFunc = GA_EndThrowFrame;
    } else {
        ga_endThrowFunc = GA_EndTurn;
    }
    for ( player = 0; player < game_numPlayers; player++ ) {
        /* team stays unset with one player (or more than four); game_lastTeam is then 0 */
        switch ( game_numPlayers ) {
        case 2:
            team = player > 0;
            break;
        case 3:
        case 4:
            team = player >= 2;
            break;
        }
        lane = game_lastTeam ? team : 0;
        GA_SetupPlayer( player, ( game_players[player] >> 7 ) ? -1 : lane, game_players[player] & 0xF, lane );
    }
    if ( !game_lastTeam ) {
        GA_ResetViews();
    }
}

/*
================
GA_FillTeams

Fills the teams' player lists: with one team every player in order; with two, players 0 and 1 apart with
two players, players 0 and 1 against the rest with three or four.
================
*/
void GA_FillTeams( void ) {
    s32 i;

    if ( !game_lastTeam ) {
        for ( i = 0; i < game_numPlayers; i++ ) {
            ga_teamPlayers[0][i] = i;
        }
        for ( ; i < 5; i++ ) {
            ga_teamPlayers[0][i] = -1;
        }
        return;
    }
    switch ( game_numPlayers ) {
    case 2:
        ga_teamPlayers[0][0] = 0;
        for ( i = 1; i < 5; i++ ) {
            ga_teamPlayers[0][i] = -1;
        }
        ga_teamPlayers[1][0] = 1;
        for ( i = 1; i < 5; i++ ) {
            ga_teamPlayers[1][i] = -1;
        }
        break;
    case 3:
        ga_teamPlayers[0][0] = 0;
        ga_teamPlayers[0][1] = 1;
        for ( i = 2; i < 5; i++ ) {
            ga_teamPlayers[0][i] = -1;
        }
        ga_teamPlayers[1][0] = 2;
        for ( i = 1; i < 5; i++ ) {
            ga_teamPlayers[1][i] = -1;
        }
        break;
    case 4:
        ga_teamPlayers[0][0] = 0;
        ga_teamPlayers[0][1] = 1;
        for ( i = 2; i < 5; i++ ) {
            ga_teamPlayers[0][i] = -1;
        }
        ga_teamPlayers[1][0] = 2;
        ga_teamPlayers[1][1] = 3;
        for ( i = 2; i < 5; i++ ) {
            ga_teamPlayers[1][i] = -1;
        }
        break;
    }
}

/*
================
GA_NextTeamPlayer

Advances *cur to the next player of the team, wrapping at the end of the list, and on past each player
whose game_playerDone flag is set, but stops at start.
================
*/
inline void GA_NextTeamPlayer( u8 *cur, s32 start, s32 team ) {
    if ( ga_teamPlayers[team][++( *cur )] == -1 ) {
        *cur = 0;
    }
    if ( *cur != start && game_playerDone[ga_teamPlayers[team][*cur]] ) {
        GA_NextTeamPlayer( cur, start, team );
    }
}

/*
================
GA_TeamGameLoop

Main loop state of a team game: runs the current player of each team; when one is done, ends the game for
the team if every player has the game_playerDone flag set, else passes the turn to the team's next player.
================
*/
// The turn pass is GA_NextTeamPlayer inlined (its inline is what puts the body here at -O2).
s32 GA_TeamGameLoop( void ) {
    s32 team;
    s32 flagged;
    s32 players;
    s32 i;

    for ( team = 0; team <= game_lastTeam; team++ ) {
        if ( GA_RunPlayer( ga_teamPlayers[team][ga_teamCurrent[team]] ) ) {
            flagged = 0;
            players = 0;
            for ( i = 0; ga_teamPlayers[team][i] != -1; i++ ) {
                players++;
                flagged += game_playerDone[ga_teamPlayers[team][i]] != 0;
            }
            if ( flagged == players ) {
                GA_ParkLane( ga_teamPlayers[team][0], 0 );
            } else {
                GA_NextTeamPlayer( &ga_teamCurrent[team], ga_teamCurrent[team], team );
            }
        }
    }
    return GA_StopViewHandlers( game_lastTeam );
}

/*
================
GA_PairGameLoop

Main loop state of the pair game: runs the current player of each team; with three or more players the
turn alternates between the pair of players 0 and 1 and the pair 2 and 3 (ga_pairNext keeps each
pair's next index), else it passes to the team's next player. Ends the game for the team once the flag in
game_playerDone of the current player's pair is set (with four players on one team, once both pairs' flags are).
================
*/
s32 GA_PairGameLoop( void ) {
    s32 team;
    s32 result;
    s32 done;
    s32 pair;

    done = 0;
    for ( team = 0; team <= game_lastTeam; team++ ) {
        result = GA_RunPlayer( ga_teamPlayers[team][ga_teamCurrent[team]] );
        if ( result < 0 ) {
            if ( ga_teamPlayers[0][2] != -1 ) {
                if ( ga_teamCurrent[team] >= 2 ) {
                    ga_pairNext[1] = ga_teamCurrent[team];
                    ga_teamCurrent[team] = ga_teamCurrent[team] == 3 ? 2 : 3;
                } else {
                    ga_pairNext[0] = ga_teamCurrent[team];
                    ga_teamCurrent[team] = ga_teamCurrent[team] == 1 ? 0 : 1;
                }
            } else if ( ga_teamPlayers[team][++ga_teamCurrent[team]] == -1 ) {
                ga_teamCurrent[team] = 0;
            }
        } else if ( result > 0 ) {
            if ( ga_teamPlayers[0][2] != -1 ) {
                if ( ga_teamCurrent[team] >= 2 ) {
                    ga_teamCurrent[team] = ga_pairNext[0];
                    ga_pairNext[0] = ga_pairNext[0] == 1 ? 0 : 1;
                } else {
                    ga_teamCurrent[team] = ga_pairNext[1];
                    ga_pairNext[1] = ga_pairNext[1] == 3 ? 2 : 3;
                }
            } else if ( ga_teamPlayers[team][++ga_teamCurrent[team]] == -1 ) {
                ga_teamCurrent[team] = 0;
            }
        }
        if ( result != 0 ) {
            if ( !game_lastTeam && game_numPlayers == 4 ) {
                if ( game_playerDone[0] && game_playerDone[1] ) {
                    done = 1;
                }
            } else {
                pair = ga_teamPlayers[team][ga_teamCurrent[team]] >= 2;
                if ( game_playerDone[pair] ) {
                    done = 1;
                }
            }
            if ( done ) {
                GA_ParkLane( ga_teamPlayers[team][ga_teamCurrent[team]], 0 );
            }
        }
    }
    return GA_StopViewHandlers( game_lastTeam );
}

/*
================
GA_ParGameLoop

Main loop state of the par game: runs the current player of each team, passes the turn on when one is
done, and ends the game for the team when every player has the game_playerDone flag set (or, without that,
when the turn comes back to the first player). Then advances the frame when the lanes are done.
================
*/
s32 GA_ParGameLoop( void ) {
    s32 team;
    s32 i;
    s32 flagged;
    s32 players;
    s32 showFrame;

    for ( team = 0; team <= game_lastTeam; team++ ) {
        if ( GA_RunPlayer( ga_teamPlayers[team][ga_teamCurrent[team]] ) ) {
            if ( ga_teamPlayers[team][++ga_teamCurrent[team]] == -1 ) {
                ga_teamCurrent[team] = 0;
            }
            flagged = 0;
            players = 0;
            for ( i = 0; ga_teamPlayers[team][i] != -1; i++ ) {
                players++;
                flagged += game_playerDone[ga_teamPlayers[team][i]] != 0;
            }
            if ( flagged == players ) {
                GA_ParkLane( ga_teamPlayers[team][0], 0 );
            } else if ( ga_teamCurrent[team] == 0 ) {
                GA_ParkLane( ga_teamPlayers[team][0], 1 );
            }
        }
    }
    showFrame = ( game_mode >> 6 ) == 1;
    if ( ga_parFrame < 9 && GA_StopPanelHandlers( game_lastTeam ) ) {
        if ( showFrame ) {
            GA_DecideFrame( ga_parFrame );
        }
        ga_parFrame++;
    }
    return GA_StopViewHandlers( game_lastTeam ) != 0;
}

/*
================
GA_DuelLoop

Main loop state of a two-lane game: updates both lanes; when both are done, counts a game won for the lane
GA_EndDuelGame returns. On A or Z with both lanes at -1, starts the next game (skipping when a lane has
won two).
================
*/
s32 GA_DuelLoop( void ) {
    s32 results[2];
    s32 winner;
    s32 i;

    for ( i = 0; i < 2; i++ ) {
        results[i] = GA_RunPlayer( i );
        if ( results[i] > 0 && game_playerDone[i] ) {
            GA_ParkLane( i, 1 );
        }
    }
    if ( GA_StopViewHandlers( 1 ) ) {
        return 1;
    }
    if ( GA_IsParkedRestart( 0 ) && GA_IsParkedRestart( 1 ) ) {
        winner = GA_EndDuelGame();
        if ( winner >= 0 ) {
            game_gamesWon[winner]++;
        }
        return 0;
    }
    if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) && results[0] + results[1] == -2 ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        if ( game_gamesWon[0] == 2 || game_gamesWon[1] == 2 ) {
            GA_EndGameOverTurns( 1 );
        } else {
            GA_EndGameOverTurns( 0 );
        }
    }
    return 0;
}

/*
================
GA_DistanceLoop

Main loop state of mode 3: updates both lanes, and ends the turn when both are done, on A or Z, or when
both lanes return -1.
================
*/
s32 GA_DistanceLoop( void ) {
    s32 results[2];
    s32 i;

    for ( i = 0; i < 2; i++ ) {
        results[i] = GA_RunPlayer( i );
        if ( results[i] > 0 && game_playerDone[i] ) {
            GA_ParkLane( i, 1 );
        }
    }
    if ( GA_StopViewHandlers( 1 ) ) {
        return 1;
    }
    if ( GA_IsParkedRestart( 0 ) && GA_IsParkedRestart( 1 ) ) {
        GA_EndDuelGame();
        return 0;
    }
    if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) || results[0] + results[1] == -2 ) {
        GA_EndGameOverTurns( 1 );
    }
    return 0;
}

/*
================
GA_RunFirstPlayer
================
*/
s32 GA_RunFirstPlayer( void ) {
    GA_RunPlayer( 0 );
    return 0;
}

