/* 56FF0.c -- results overlay */

#include "common.h"
#include "libultra.h"
#include "rscode/rscode.h"

/*
================
RS_Init

Entry of the overlay: starts the first state and the mode's results.
================
*/
void RS_Init( void ) {
    Sound_StopBgm();
    rs_leaving = 0;
    rs_boxAlpha = 0;
    rs_boxAlphaFalling = 0;
    rs_state = RS_ScoreSheet;
    rs_nextScene = game_scene;
    RS_StartMode();
}

/*
================
RS_Frame

Frame of the overlay: reads the input, draws the background, runs the current state and pulses the boxes'
alpha.
================
*/
void RS_Frame( void ) {
    if ( !game_leaveState && rs_leaving ) {
        game_leaveState = 1;
        Sound_PlaySe( 50, 0 );
    }
    if ( game_leaveState == 1 && game_fade == 255 ) {
        game_leaveState = 2;
        game_fade = 263;
        game_scene = rs_nextScene;
    }
    rs_dpadPressed = game_leaveState ? 0 : D_800E6758;
    rs_buttonsPressed = game_leaveState ? 0 : D_800E4D62;
    Txt_LoadGlyphSheet( &game_gfx );
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    rs_state();
    if ( rs_boxAlphaFalling == 1 ) {
        rs_boxAlpha -= 8;
        if ( rs_boxAlpha <= 0 ) {
            rs_boxAlpha = 0;
            rs_boxAlphaFalling = 0;
        }
    } else {
        rs_boxAlpha += 8;
        if ( rs_boxAlpha >= 0x80 ) {
            rs_boxAlpha = 0x80;
            rs_boxAlphaFalling = 1;
        }
    }
    Txt_LoadFont( &game_gfx );
    Txt_SetupFont( &game_gfx, 0 );
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
}

/*
================
RS_StartMode

Picks the first state and the BGM for the game mode.
================
*/
void RS_StartMode( void ) {
    switch ( game_mode & 0xF ) {
    case 0:
        RS_StartMode0();
        Sound_PlayBgm( 10 );
        break;
    case 1:
        rs_state = RS_DuelResults;
        Sound_PlayBgm( game_totals[0] <= game_totals[1] ? 15 : 14 );
        Sound_PlaySe( game_totals[0] <= game_totals[1] ? 0x6D : 0x6C, 0 );
        break;
    case 2:
        break;
    case 3:
        rs_state = rs_returnState = RS_Menu;
        Sound_PlayBgm( 18 );
        break;
    }
}

/*
================
RS_StartMode0

Picks the state of mode 0's results by its sub-mode; a perfect game (bit 0x200 of save_data.unlocks) or a
finished game (bit 0x100) first shows its message, once. Sub-mode 0 checks for a perfect game only with
argument 0 (bits 6-7 of game_mode); its switch lists the other three arguments, which do nothing.
================
*/
void RS_StartMode0( void ) {
    s32 found = 0;
    s32 i;

    switch ( ( game_mode >> 4 ) & 3 ) {
    case 0:
        RS_RankAndRecord();
        switch ( game_mode >> 6 ) {
        case 0:
            if ( game_stage ) {
                break;
            }
            for ( i = 0; i < game_numPlayers; i++ ) {
                if ( !( game_players[i] >> 7 ) && game_frameScores[i][9] == 300 ) {
                    found = 1;
                    break;
                }
            }
            if ( found && !( save_data.unlocks & 0x200 ) ) {
                Sound_PlaySe( 0x35, 0 );
                rs_state = RS_PerfectMessage;
                save_data.unlocks |= 0x200;
            }
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        }
        break;
    case 1:
        rs_state = RS_ResultsPage1;
        break;
    case 2:
        rs_state = RS_ResultsPage2;
        if ( game_stage ) {
            break;
        }
        for ( i = 0; i < game_numPlayers; i++ ) {
            if ( game_frame[i] == 10 ) {
                found = 1;
                break;
            }
        }
        if ( found && !( save_data.unlocks & 0x100 ) ) {
            Sound_PlaySe( 0x35, 0 );
            rs_state = RS_FinishedMessage;
            save_data.unlocks |= 0x100;
        }
        break;
    }
}

/*
================
RS_FinishedMessage

Shows message 0x88C in a box for 300 frames or until A or Z, then goes to RS_ResultsPage2.
================
*/
void RS_FinishedMessage( void ) {
    Draw_RoundedBox( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, rs_boxAlpha + 0x40 );
    Draw_Image( 0x88C, 0x26, 0x50 );
    if ( ++rs_messageFrames >= 300 || ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_ResultsPage2;
    }
}

/*
================
RS_PerfectMessage

Shows message 0xAE3 in a box for 300 frames or until A or Z, then goes to RS_ScoreSheet.
================
*/
void RS_PerfectMessage( void ) {
    Draw_RoundedBox( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, rs_boxAlpha + 0x40 );
    Draw_Image( 0xAE3, 0x26, 0x50 );
    if ( ++rs_messageFrames >= 300 || ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_ScoreSheet;
    }
}

/*
================
RS_RankAndRecord

Ranks the players by score; outside a sub-mode, adds the game to the records of each player's character
and saves them.
================
*/
void RS_RankAndRecord( void ) {
    s16 scores[4];
    s32 i;
    s32 character;

    D_801142B4_rscode = game_mode >> 6;
    for ( i = 0; i < game_numPlayers; i++ ) {
        scores[i] = game_frameScores[i][9];
    }
    RS_RankHighest( rs_ranks, game_numPlayers, scores, 0, 300 );
    if ( !D_801142B4_rscode ) {
        for ( i = 0; i < game_numPlayers; i++ ) {
            character = ( ( game_players[i] >> 4 ) & 7 ) - 1;
            if ( character >= 0 ) {
                RS_AddToRecord( &game_sessionRecords[character], i );
                RS_AddToRecord( &save_data.records[character], i );
            }
        }
        for ( character = 0; character < 4; character++ ) {
            Score_SetRecordFigures( &game_sessionRecords[character], character, 0 );
            Score_SetRecordFigures( &save_data.records[character], character, 1 );
        }
    }
    rs_state = RS_ScoreSheet;
}

/*
================
RS_ScoreSheet

Mode 0's results: the score sheet or the pin diagrams (left and right choose), then, unless every record is
empty, down goes to the records of RS_Records.
================
*/
void RS_ScoreSheet( void ) {
    u16 images[2] = { 0x87A, 0x898 };
    s16 y = 0x10;
    s16 empty = 0;
    s16 i;

    for ( i = 0; i < 4; i++ ) {
        empty += game_sessionRecords[i].games ? 0 : 1;
    }
    if ( ( rs_dpadPressed & L_JPAD ) && rs_sheetPage == 1 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_sheetPage = 0;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_sheetPage == 0 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_sheetPage = 1;
    }
    Draw_Image( 0x88D, 0x64, y );
    y += 0x18;
    switch ( rs_sheetPage ) {
    case 0:
        Score_DrawTeamBoard( D_801142B4_rscode, 1, 0x18, y );
        break;
    case 1:
        RS_DrawPinDiagrams( 0x18, y );
        break;
    }
    y += 0x8C;
    RS_DrawChoices( rs_sheetPage, images, y, 0x50, 0x10 );
    y += 0x1C;
    if ( !D_801142B4_rscode && empty != 4 ) {
        Draw_ImageScaled( 0xA82, 0x48, y, 0x60, 1.0f, 1.0f );
        Draw_ImageScaled( 0xA81, 0xA8, y, 0x60, 1.0f, 1.0f );
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ScoreSheet;
    }
    if ( ( rs_dpadPressed & D_JPAD ) && !D_801142B4_rscode && empty != 4 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_state = RS_Records;
        rs_sheetPage += 2;
    }
}

/*
================
RS_Records

Records of the characters, of this session or saved (left and right choose); up goes back to
RS_ScoreSheet.
================
*/
void RS_Records( void ) {
    u16 images[2] = { 0xA82, 0xA81 };
    s16 y = 0x10;

    if ( ( rs_dpadPressed & L_JPAD ) && rs_sheetPage == 3 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_sheetPage = 2;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_sheetPage == 2 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_sheetPage = 3;
    }
    Draw_Image( 0x88D, 0x64, y );
    y += 0x18;
    switch ( rs_sheetPage ) {
    case 2:
        Score_DrawRecordTable( 4, game_sessionRecords, 0, 0x18, y );
        break;
    case 3:
        Score_DrawRecordTable( 4, save_data.records, 1, 0x18, y );
        break;
    }
    y += 0x8C;
    Draw_ImageScaled( 0x87A, 0x48, y, 0x60, 1.0f, 1.0f );
    Draw_ImageScaled( 0x898, 0xA8, y, 0x60, 1.0f, 1.0f );
    y += 0x1C;
    RS_DrawChoices( rs_sheetPage - 2, images, y, 0x50, 0x10 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_Records;
    }
    if ( rs_dpadPressed & U_JPAD ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_state = RS_ScoreSheet;
        rs_sheetPage -= 2;
    }
}

/*
================
RS_ResultsPage1

Results page; A or Z opens the menu of RS_Menu, which comes back here.
================
*/
void RS_ResultsPage1( void ) {
    Draw_Image( 0x88D, 0x64, 0x10 );
    Score_DrawParCard( game_mode >> 6, 1, 0x20, 0x28 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ResultsPage1;
    }
}

/*
================
RS_ResultsPage2

Results page; A or Z opens the menu of RS_Menu, which comes back here.
================
*/
void RS_ResultsPage2( void ) {
    Draw_Image( 0x88D, 0x64, 0x10 );
    Score_DrawAllBoards( 1, 0x18, 0x28 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ResultsPage2;
    }
}

/*
================
RS_DuelResults

Results of a duel: the score of each game, the higher one marked, the games won, and the winner; A or Z
leaves when player 1 won (setting the bit of game_stage in save_data.unlocks), else opens the menu.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail passes x to Menu_DrawNamePlate zero-extended and not folded to the constant 0x40 (then forms
// x + 0x38 with ori), and allocates the first loop's registers differently (x in s1, i in s2)
// The zero extension fits a call with no prototype in scope; the project keeps every call prototyped, so it stays
// parked.
void RS_DuelResults( void ) {
    u16 images[3] = { 0x874, 0x873, 0x872 };
    s16 x;
    s16 y;
    s32 games;
    s32 player;
    s32 opponent;
    s32 i;

    Draw_Image( 0x88D, 0x64, 0x10 );
    y = 0x28;
    x = 0x78;
    games = game_gamesWon[0] + game_gamesWon[1];
    for ( i = 0; i < games; i++ ) {
        Draw_Image( images[i], x, y );
        x += 0x20;
    }
    Draw_Image( 0x871, x, y );
    y += 9;
    for ( player = 0; player < 2; player++ ) {
        x = 0x40;
        Menu_DrawNamePlate( player, game_players[player] >> 7, game_playerNames[player].chars, x, y );
        x += 0x38;
        opponent = !player;
        for ( i = 0; i < games; i++ ) {
            Draw_Image( 0x86D, x, y );
            Score_DrawNumber( game_duelScores[player][i], x + 6, y + 6 );
            if ( game_duelScores[player][i] > game_duelScores[opponent][i] ) {
                Draw_ImageScaled( 0x86C, x, y, rs_boxAlpha + 0x20, 1.0f, 1.0f );
            }
            x += 0x20;
        }
        Draw_Image( 0x86B, x, y );
        Menu_DrawBox( player, game_gamesWon[player], 1, x + 4, y + 2 );
        y += 0x14;
    }
    if ( game_gamesWon[0] > game_gamesWon[1] ) {
        Draw_ImageScaled( 0x869, 0x2C, y + 8, rs_boxAlpha + 0x7F, 1.0f, 1.0f );
        if ( !( save_data.unlocks & ( 1 << game_stage ) ) ) {
            Draw_Image( 0x870, 0x38, 0xAC );
        }
    } else {
        Draw_ImageScaled( 0x86A, 0x2C, y + 8, rs_boxAlpha + 0x7F, 1.0f, 1.0f );
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        if ( game_gamesWon[0] > game_gamesWon[1] ) {
            rs_state = RS_Leave;
            save_data.unlocks |= 1 << game_stage;
        } else {
            rs_state = RS_Menu;
            rs_returnState = RS_DuelResults;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", RS_DuelResults);
INCLUDE_RODATA("asm/nonmatchings/rscode/56FF0", D_80114290_rscode);
#endif

/*
================
RS_ResultsPage

Results page; A or Z opens the menu of RS_Menu, which comes back here.
================
*/
void RS_ResultsPage( void ) {
    Draw_Image( 0x88D, 0x64, 0x10 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        Sound_PlaySe( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ResultsPage;
    }
}

/*
================
RS_Menu

Menu of two choices: leave the results, or RS_MenuConfirm's; B goes back to the page it came from.
================
*/
void RS_Menu( void ) {
    u16 images[2] = { 0xAB5, 0xA57 };

    if ( ( rs_dpadPressed & L_JPAD ) && rs_menuChoice == 1 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_menuChoice = 0;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_menuChoice == 0 ) {
        Sound_PlaySe( SE_CURSOR, 0 );
        rs_menuChoice = 1;
    }
    RS_DrawChoices( rs_menuChoice, images, 0x68, 0x50, 0x10 );
    Draw_ImageScaled( rs_menuChoice ? 0x84C : 0x84B, 0x98, 0x68, rs_boxAlpha + 0x40, 1.0f, 1.0f );
    if ( ( rs_buttonsPressed & B_BUTTON ) && rs_returnState != RS_Menu ) {
        Sound_PlaySe( 2, 0 );
        rs_menuChoice = 0;
        rs_state = rs_returnState;
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        switch ( rs_menuChoice ) {
        case 0:
            rs_nextScene = 4;
            game_replay = 1;
            rs_state = RS_Idle;
            rs_leaving = 1;
            break;
        case 1:
            Sound_PlaySe( SE_DECIDE, 0 );
            rs_state = RS_MenuConfirm;
            rs_menuChoice = 0;
            break;
        }
    }
}

/*
================
RS_MenuConfirm
================
*/
void RS_MenuConfirm( void ) {
    s32 choice;

    choice = Menu_TwoChoicePrompt();
    if ( choice == 2 ) {
        rs_state = RS_Menu;
    } else if ( choice == 1 ) {
        rs_state = RS_Leave;
    }
}

/*
================
RS_Leave

Waits for Save_SaveSettings, then leaves the overlay.
================
*/
void RS_Leave( void ) {
    if ( Save_SaveSettings() ) {
        return;
    }
    rs_leaving = 1;
    rs_nextScene = 4;
    game_replay = 0;
    rs_state = RS_Idle;
}

/*
================
RS_Idle

Last state: does nothing while the overlay is left.
================
*/
void RS_Idle( void ) {
}

/*
================
RS_DrawPinDiagram

Draws the pin diagram of a player's frame at (x, y): image 0x89D for a pin of the first mask (D_800A3050),
else 0x89C for a pin of the second (D_800F7C18). Declared inline: RS_DrawPinDiagrams repeats its body.
================
*/
inline void RS_DrawPinDiagram( s32 player, u8 frame, s16 x, s16 y ) {
    s32 pin;

    Draw_Image( 0x899, x, y );
    for ( pin = 0; pin < 10; pin++ ) {
        if ( ( D_800A3050[player][frame] >> ( 9 - pin ) ) & 1 ) {
            Draw_Image( 0x89D, x + rs_pinPos[pin][0], y + rs_pinPos[pin][1] );
        } else if ( ( D_800F7C18[player][frame] >> ( 9 - pin ) ) & 1 ) {
            Draw_Image( 0x89C, x + rs_pinPos[pin][0], y + rs_pinPos[pin][1] );
        }
    }
}

/*
================
RS_DrawPinDiagrams

Pin diagrams of every player's frames at (x, y), in teams in sub-mode 1; C left and right turn the page.
================
*/
void RS_DrawPinDiagrams( s16 x, s16 y ) {
    s16 rowX;
    s16 pageX;
    s32 players;
    s32 player;
    s32 frame;
    s32 end;
    s32 mate;

    if ( ( rs_buttonsPressed & R_CBUTTONS ) && !rs_pinPage ) {
        Sound_PlaySe( 0x33, 0 );
        rs_pinPage = 1;
    }
    if ( ( rs_buttonsPressed & L_CBUTTONS ) && rs_pinPage == 1 ) {
        Sound_PlaySe( 0x33, 0 );
        rs_pinPage = 0;
    }
    Draw_Image( rs_pinPage ? 0x89B : 0x89A, 0xF8, 0x18 );
    if ( D_801142B4_rscode == 2 ) {
        Draw_Image( 0xA12, x, y - 3 );
    }
    Score_DrawHeaders( rs_pinPage, x + 0x2C, y );
    y += 9;
    players = D_801142B4_rscode == 1 ? game_numPlayers >> 1 : game_numPlayers;
    for ( player = 0; player < players; player++ ) {
        rowX = x;
        if ( D_801142B4_rscode == 2 && player == 2 ) {
            Draw_Image( 0x9FB, rowX, y + 4 );
            y += 0x10;
        }
        if ( D_801142B4_rscode == 1 ) {
            mate = ( player == 1 ) * 2;
            Menu_DrawSmallNamePlate( mate, game_players[mate] >> 7, game_playerNames[mate].chars, 0, rowX, y );
            Menu_DrawSmallNamePlate( mate | 1, game_players[mate + 1] >> 7, game_playerNames[mate + 1].chars, 0, rowX, y + 0x1C );
        } else {
            Menu_DrawSmallNamePlate( player, game_players[player] >> 7, game_playerNames[player].chars, 0, rowX, y );
        }
        rowX += 0x2C;
        if ( D_801142B4_rscode == 1 ) {
            y += 0xE;
        }
        frame = rs_pinPage ? 9 : 0;
        end = rs_pinPage ? D_80066C00.unk_20[player] : 9;
        pageX = rowX;
        rowX += rs_pinPage ? 0x10 : 0;
        for ( ; frame < end; frame++ ) {
            RS_DrawPinDiagram( player, frame, rowX, y );
            rowX += 0x18;
        }
        if ( rs_pinPage ) {
            rowX = pageX;
        }
        Draw_ImageScaled( rs_pinPage ? 0x84C : 0x84B, rowX, y + 4, rs_boxAlpha + 0x40, 1.0f, 1.0f );
        y += D_801142B4_rscode == 1 ? 0x38 : 0x1C;
    }
}

/*
================
RS_DrawChoices

Draws two choices side by side, the chosen one in a box, the other faded.
================
*/
void RS_DrawChoices( u8 choice, u16 *images, s16 y, u16 w, u16 h ) {
    s16 x[2] = { 0x48, 0xA8 };

    Draw_RoundedBox( x[choice], y, w, h, 0xE0, 0xE0, 0xE0, rs_boxAlpha + 0x40 );
    Draw_Image( images[choice], x[choice], y );
    Draw_ImageScaled( images[!choice], x[!choice], y, 0x60, 1.0f, 1.0f );
}

/*
================
Max

The greater of a and b.
================
*/
static inline s32 Max( s32 a, s32 b ) {
    return a > b ? a : b;
}

/*
================
Min

The lesser of a and b.
================
*/
static inline s32 Min( s32 a, s32 b ) {
    return a < b ? a : b;
}

/*
================
RS_AddToRecord

Adds the player's game to a character's record: one more game (to 255), the total score (to 0xFFFF), the
high and the low score, and three more counts (to 9999).
================
*/
void RS_AddToRecord( playerRecord_t *record, s32 player ) {
    if ( record->games + 1 < 256 ) {
        record->games++;
    }
    if ( record->total + game_frameScores[player][9] <= 0xFFFF ) {
        record->total += game_frameScores[player][9];
    }
    record->high = Max( game_frameScores[player][9], record->high );
    record->low = Min( game_frameScores[player][9], record->low );
    if ( record->strikes + game_strikes[player] < 10000 ) {
        record->strikes += game_strikes[player];
    }
    if ( record->spares + game_spares[player] < 10000 ) {
        record->spares += game_spares[player];
    }
    if ( record->splits + game_splits[player] < 10000 ) {
        record->splits += game_splits[player];
    }
}

/*
================
RS_RankHighest

Ranks the count scores, each between low and high, from the highest: ranks[i] is 1 plus the number of
higher scores. Bug (kept): the loops run to i == count, one entry past the end; RS_RankAndRecord's
call ranks a fifth, stale score and writes its rank over rs_dpadPressed.
================
*/
void RS_RankHighest( u8 *ranks, u8 count, s16 *scores, s16 low, s16 high ) {
    s32 tally[302];
    s32 i;

    for ( i = low; i <= high; i++ ) {
        tally[i] = 0;
    }
    for ( i = 0; i <= count; i++ ) {
        tally[scores[i]]++;
    }
    tally[high + 1] = 1;
    for ( i = high; i >= low; i-- ) {
        tally[i] += tally[i + 1];
    }
    for ( i = 0; i <= count; i++ ) {
        ranks[i] = tally[scores[i] + 1];
    }
}

/*
================
RS_RankLowest

Ranks the count values, each between low and high, from the lowest: ranks[i] is 1 plus the number of
lower values. As RS_RankHighest, the loops run one entry past the end. Nothing calls it.
================
*/
void RS_RankLowest( u8 *ranks, u8 count, s16 *values, s16 low, s16 high ) {
    s32 tally[12];
    s16 base = 1 - low;
    s32 value;
    s32 i;

    for ( value = low + base; value <= high + base; value++ ) {
        tally[value] = 0;
    }
    for ( i = 0; i <= count; i++ ) {
        tally[values[i] + base]++;
    }
    tally[0] = 1;
    for ( value = low + base; value <= high + base; value++ ) {
        tally[value] += tally[value - 1];
    }
    for ( i = 0; i <= count; i++ ) {
        ranks[i] = tally[values[i] + base - 1];
    }
}
