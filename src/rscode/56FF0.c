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
    rs_nextScene = D_8005C0B6;
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
    if ( !D_8005C0BF && rs_leaving ) {
        D_8005C0BF = 1;
        func_80037870( 50, 0 );
    }
    if ( D_8005C0BF == 1 && D_8005C0C0 == 255 ) {
        D_8005C0BF = 2;
        D_8005C0C0 = 263;
        D_8005C0B6 = rs_nextScene;
    }
    rs_dpadPressed = D_8005C0BF ? 0 : D_800E6758;
    rs_buttonsPressed = D_8005C0BF ? 0 : D_800E4D62;
    Txt_LoadGlyphSheet( &D_800F8094 );
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
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
    Txt_LoadFont( &D_800F8094 );
    Txt_SetupFont( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
}

/*
================
RS_StartMode

Picks the first state and the BGM for the game mode.
================
*/
void RS_StartMode( void ) {
    switch ( D_8005C160 & 0xF ) {
    case 0:
        RS_StartMode0();
        Sound_PlayBgm( 10 );
        break;
    case 1:
        rs_state = RS_DuelResults;
        Sound_PlayBgm( D_800EA090[0] <= D_800EA090[1] ? 15 : 14 );
        func_80037870( D_800EA090[0] <= D_800EA090[1] ? 0x6D : 0x6C, 0 );
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

Picks the state of mode 0's results by its sub-mode; a perfect game (bit 0x200 of D_8009D098.unk_04) or a
finished game (bit 0x100) first shows its message, once. Sub-mode 0 checks for a perfect game only with
argument 0 (bits 6-7 of D_8005C160); its switch lists the other three arguments, which do nothing.
================
*/
void RS_StartMode0( void ) {
    s32 found = 0;
    s32 i;

    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        RS_RankAndRecord();
        switch ( D_8005C160 >> 6 ) {
        case 0:
            if ( D_8005C163 ) {
                break;
            }
            for ( i = 0; i < D_8005C161; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) && D_800DF630[i][9] == 300 ) {
                    found = 1;
                    break;
                }
            }
            if ( found && !( D_8009D098.unk_04 & 0x200 ) ) {
                func_80037870( 0x35, 0 );
                rs_state = RS_PerfectMessage;
                D_8009D098.unk_04 |= 0x200;
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
        if ( D_8005C163 ) {
            break;
        }
        for ( i = 0; i < D_8005C161; i++ ) {
            if ( D_8009D19C[i] == 10 ) {
                found = 1;
                break;
            }
        }
        if ( found && !( D_8009D098.unk_04 & 0x100 ) ) {
            func_80037870( 0x35, 0 );
            rs_state = RS_FinishedMessage;
            D_8009D098.unk_04 |= 0x100;
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
    func_8002DF3C( 0x88C, 0x26, 0x50 );
    if ( ++rs_messageFrames >= 300 || ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
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
    func_8002DF3C( 0xAE3, 0x26, 0x50 );
    if ( ++rs_messageFrames >= 300 || ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
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

    D_801142B4_rscode = D_8005C160 >> 6;
    for ( i = 0; i < D_8005C161; i++ ) {
        scores[i] = D_800DF630[i][9];
    }
    RS_RankHighest( rs_ranks, D_8005C161, scores, 0, 300 );
    if ( !D_801142B4_rscode ) {
        for ( i = 0; i < D_8005C161; i++ ) {
            character = ( ( D_8005C0E4[i] >> 4 ) & 7 ) - 1;
            if ( character >= 0 ) {
                RS_AddToRecord( &D_800E44E8[character], i );
                RS_AddToRecord( &D_8009D098.unk_08[character], i );
            }
        }
        for ( character = 0; character < 4; character++ ) {
            func_8003B034( &D_800E44E8[character], character, 0 );
            func_8003B034( &D_8009D098.unk_08[character], character, 1 );
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
        empty += D_800E44E8[i].games ? 0 : 1;
    }
    if ( ( rs_dpadPressed & L_JPAD ) && rs_sheetPage == 1 ) {
        func_80037870( SE_CURSOR, 0 );
        rs_sheetPage = 0;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_sheetPage == 0 ) {
        func_80037870( SE_CURSOR, 0 );
        rs_sheetPage = 1;
    }
    func_8002DF3C( 0x88D, 0x64, y );
    y += 0x18;
    switch ( rs_sheetPage ) {
    case 0:
        func_80039700( D_801142B4_rscode, 1, 0x18, y );
        break;
    case 1:
        RS_DrawPinDiagrams( 0x18, y );
        break;
    }
    y += 0x8C;
    RS_DrawChoices( rs_sheetPage, images, y, 0x50, 0x10 );
    y += 0x1C;
    if ( !D_801142B4_rscode && empty != 4 ) {
        func_8002E4B0( 0xA82, 0x48, y, 0x60, 1.0f, 1.0f );
        func_8002E4B0( 0xA81, 0xA8, y, 0x60, 1.0f, 1.0f );
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ScoreSheet;
    }
    if ( ( rs_dpadPressed & D_JPAD ) && !D_801142B4_rscode && empty != 4 ) {
        func_80037870( SE_CURSOR, 0 );
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
        func_80037870( SE_CURSOR, 0 );
        rs_sheetPage = 2;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_sheetPage == 2 ) {
        func_80037870( SE_CURSOR, 0 );
        rs_sheetPage = 3;
    }
    func_8002DF3C( 0x88D, 0x64, y );
    y += 0x18;
    switch ( rs_sheetPage ) {
    case 2:
        func_8003A970( 4, D_800E44E8, 0, 0x18, y );
        break;
    case 3:
        func_8003A970( 4, D_8009D098.unk_08, 1, 0x18, y );
        break;
    }
    y += 0x8C;
    func_8002E4B0( 0x87A, 0x48, y, 0x60, 1.0f, 1.0f );
    func_8002E4B0( 0x898, 0xA8, y, 0x60, 1.0f, 1.0f );
    y += 0x1C;
    RS_DrawChoices( rs_sheetPage - 2, images, y, 0x50, 0x10 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_Records;
    }
    if ( rs_dpadPressed & U_JPAD ) {
        func_80037870( SE_CURSOR, 0 );
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
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_80039CBC( D_8005C160 >> 6, 1, 0x20, 0x28 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
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
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_8003A3F0( 1, 0x18, 0x28 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        rs_state = RS_Menu;
        rs_returnState = RS_ResultsPage2;
    }
}

/*
================
RS_DuelResults

Results of a duel: the score of each game, the higher one marked, the games won, and the winner; A or Z
leaves when player 1 won (setting the bit of D_8005C163 in D_8009D098.unk_04), else opens the menu.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail passes x to func_800389D0 zero-extended and not folded to the constant 0x40 (then forms
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

    func_8002DF3C( 0x88D, 0x64, 0x10 );
    y = 0x28;
    x = 0x78;
    games = D_800693DC[0] + D_800693DC[1];
    for ( i = 0; i < games; i++ ) {
        func_8002DF3C( images[i], x, y );
        x += 0x20;
    }
    func_8002DF3C( 0x871, x, y );
    y += 9;
    for ( player = 0; player < 2; player++ ) {
        x = 0x40;
        func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, x, y );
        x += 0x38;
        opponent = !player;
        for ( i = 0; i < games; i++ ) {
            func_8002DF3C( 0x86D, x, y );
            func_800393D8( D_8009D030[player][i], x + 6, y + 6 );
            if ( D_8009D030[player][i] > D_8009D030[opponent][i] ) {
                func_8002E4B0( 0x86C, x, y, rs_boxAlpha + 0x20, 1.0f, 1.0f );
            }
            x += 0x20;
        }
        func_8002DF3C( 0x86B, x, y );
        func_8003A8B4( player, D_800693DC[player], 1, x + 4, y + 2 );
        y += 0x14;
    }
    if ( D_800693DC[0] > D_800693DC[1] ) {
        func_8002E4B0( 0x869, 0x2C, y + 8, rs_boxAlpha + 0x7F, 1.0f, 1.0f );
        if ( !( D_8009D098.unk_04 & ( 1 << D_8005C163 ) ) ) {
            func_8002DF3C( 0x870, 0x38, 0xAC );
        }
    } else {
        func_8002E4B0( 0x86A, 0x2C, y + 8, rs_boxAlpha + 0x7F, 1.0f, 1.0f );
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        if ( D_800693DC[0] > D_800693DC[1] ) {
            rs_state = RS_Leave;
            D_8009D098.unk_04 |= 1 << D_8005C163;
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
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
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
        func_80037870( SE_CURSOR, 0 );
        rs_menuChoice = 0;
    }
    if ( ( rs_dpadPressed & R_JPAD ) && rs_menuChoice == 0 ) {
        func_80037870( SE_CURSOR, 0 );
        rs_menuChoice = 1;
    }
    RS_DrawChoices( rs_menuChoice, images, 0x68, 0x50, 0x10 );
    func_8002E4B0( rs_menuChoice ? 0x84C : 0x84B, 0x98, 0x68, rs_boxAlpha + 0x40, 1.0f, 1.0f );
    if ( ( rs_buttonsPressed & B_BUTTON ) && rs_returnState != RS_Menu ) {
        func_80037870( 2, 0 );
        rs_menuChoice = 0;
        rs_state = rs_returnState;
    }
    if ( rs_buttonsPressed & ( A_BUTTON | Z_TRIG ) ) {
        switch ( rs_menuChoice ) {
        case 0:
            rs_nextScene = 4;
            D_8005CB60 = 1;
            rs_state = RS_Idle;
            rs_leaving = 1;
            break;
        case 1:
            func_80037870( SE_DECIDE, 0 );
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

    choice = func_8003B204();
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
    D_8005CB60 = 0;
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

    func_8002DF3C( 0x899, x, y );
    for ( pin = 0; pin < 10; pin++ ) {
        if ( ( D_800A3050[player][frame] >> ( 9 - pin ) ) & 1 ) {
            func_8002DF3C( 0x89D, x + rs_pinPos[pin][0], y + rs_pinPos[pin][1] );
        } else if ( ( D_800F7C18[player][frame] >> ( 9 - pin ) ) & 1 ) {
            func_8002DF3C( 0x89C, x + rs_pinPos[pin][0], y + rs_pinPos[pin][1] );
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
        func_80037870( 0x33, 0 );
        rs_pinPage = 1;
    }
    if ( ( rs_buttonsPressed & L_CBUTTONS ) && rs_pinPage == 1 ) {
        func_80037870( 0x33, 0 );
        rs_pinPage = 0;
    }
    func_8002DF3C( rs_pinPage ? 0x89B : 0x89A, 0xF8, 0x18 );
    if ( D_801142B4_rscode == 2 ) {
        func_8002DF3C( 0xA12, x, y - 3 );
    }
    func_80038D0C( rs_pinPage, x + 0x2C, y );
    y += 9;
    players = D_801142B4_rscode == 1 ? D_8005C161 >> 1 : D_8005C161;
    for ( player = 0; player < players; player++ ) {
        rowX = x;
        if ( D_801142B4_rscode == 2 && player == 2 ) {
            func_8002DF3C( 0x9FB, rowX, y + 4 );
            y += 0x10;
        }
        if ( D_801142B4_rscode == 1 ) {
            mate = ( player == 1 ) * 2;
            func_80038AE8( mate, D_8005C0E4[mate] >> 7, D_80068218[mate].chars, 0, rowX, y );
            func_80038AE8( mate | 1, D_8005C0E4[mate + 1] >> 7, D_80068218[mate + 1].chars, 0, rowX, y + 0x1C );
        } else {
            func_80038AE8( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, 0, rowX, y );
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
        func_8002E4B0( rs_pinPage ? 0x84C : 0x84B, rowX, y + 4, rs_boxAlpha + 0x40, 1.0f, 1.0f );
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
    func_8002DF3C( images[choice], x[choice], y );
    func_8002E4B0( images[!choice], x[!choice], y, 0x60, 1.0f, 1.0f );
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
void RS_AddToRecord( UnkRecord *record, s32 player ) {
    if ( record->games + 1 < 256 ) {
        record->games++;
    }
    if ( record->total + D_800DF630[player][9] <= 0xFFFF ) {
        record->total += D_800DF630[player][9];
    }
    record->high = Max( D_800DF630[player][9], record->high );
    record->low = Min( D_800DF630[player][9], record->low );
    if ( record->unk_14 + D_800DF6B4[player] < 10000 ) {
        record->unk_14 += D_800DF6B4[player];
    }
    if ( record->unk_16 + D_8009D088[player] < 10000 ) {
        record->unk_16 += D_8009D088[player];
    }
    if ( record->unk_18 + D_8009CF80[player] < 10000 ) {
        record->unk_18 += D_8009CF80[player];
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
