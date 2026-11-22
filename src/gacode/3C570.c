/* 3C570.c -- game overlay: the main loop states */

#include "common.h"
#include "libultra.h"
#include "input.h"
#include "gacode/gacode.h"

/*
================
func_80112350_gacode

The overlay's init: stops the BGM, resets the teams' current players and the pause menu, enters the pause
menu's state as the state to come back to, loads the stage's floor grid, places each player's character in
the stage's scene and creates it, starts the stage's sound, sets up the lanes, clears the games won and the
scores, sets up the game mode, takes two options from D_8009D098.unk_04 and starts the game's BGM.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: loop strength reduction only: retail's pointer walks &params[i].unk_08[4] (stores at -0x10,
// -0xC, 0), the draft's walks &params[i] (8, 0xC, 0x18); statement order and a per-iteration params pointer
// were tried
void func_80112350_gacode( void ) {
    s32 character;
    s32 i;
    objParams_t *params;

    func_80037B18();
    func_80037870( 0x36, 0 );
    D_8012CDF8_gacode[0] = D_8012CDF8_gacode[1] = 0;
    D_8012D220_gacode = &D_80123EB0_gacode[D_8005C163];
    D_8012D650_gacode = 0;
    D_8012D238_gacode = 0;
    D_8012D239_gacode = 0;
    D_8012CDF0_gacode = func_80112AF0_gacode;
    func_80029AF4( D_8012D220_gacode->unk_18, (u8 *)&D_80100100 );
    params = D_8012D220_gacode->unk_00;
    for ( i = 0; i < D_8005C161; i++ ) {
        character = D_8005C0E4[i] & 0xF;
        params[i].unk_08[0] = D_80123A14_gacode[character][0];
        params[i].unk_08[1] = D_80123A14_gacode[character][1];
        params[i].unk_08[4] = D_80123AA0_gacode[character].unk_04;
    }
    func_8002A1EC( D_80124050_gacode, D_8012D220_gacode->unk_00 );
    if ( D_8012D220_gacode->unk_1C >= 0 ) {
        D_8012CDE8_gacode = func_80037870( D_8012D220_gacode->unk_1C, 0 );
    } else {
        D_8012CDE8_gacode = 0;
    }
    func_80114140_gacode( D_8005C162, D_8012D220_gacode->unk_04, ( D_8005C160 & 0xF ) == 3 );
    func_80121E34_gacode();
    for ( i = 0; i < 2; i++ ) {
        D_800693DC[i] = 0;
        D_8009D030[i][0] = D_8009D030[i][1] = D_8009D030[i][2] = 0;
    }
    func_801132B0_gacode();
    D_8012D225_gacode = ( D_8009D098.unk_04 >> 9 ) & 1;
    D_8012D224_gacode = ( D_8009D098.unk_04 >> 8 ) & 1;
    func_800379C8( D_8012CE00_gacode );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112350_gacode);
#endif

/*
================
func_801125A0_gacode

The overlay's frame: leaves once main has faded out after a request, requests leaving when the game mode
is done or the pause menu quits, opens the pause menu on Start and, while C-up is held (not in mode 2),
func_80112E04_gacode's view, runs the stage and the players, draws them, runs the main loop state (or, paused, the pause menu's state), and steps the
blink of D_80123600_gacode between 0 and 0x80.
================
*/
void func_801125A0_gacode( void ) {
    s32 team;
    s32 cUpHeld = FALSE;

    if ( D_8005C0BF == 1 && D_8005C0C0 == 0xFF ) {
        func_80120D7C_gacode( &D_8012CDE8_gacode );
        D_8005C0BF = 2;
        D_8005C0C0 = 0x107;
        D_8005C0B6 = D_80123603_gacode == 1 ? 6 : 4;
    }
    if ( D_8005C0BF == 0 ) {
        if ( D_80123603_gacode ) {
            func_80037870( 0x7F, 0 );
            D_8005C0BF = 1;
        } else if ( D_8012D650_gacode && D_80123604_gacode ) {
            func_80037870( 0x32, 0 );
            D_8005CB60 = 0;
            D_8005C0BF = 1;
        }
        if ( !D_8012D650_gacode ) {
            if ( D_800E4D62 & START_BUTTON ) {
                func_80037870( 6, 0 );
                D_8012D650_gacode = 1;
                D_800DF680[0] = D_800DF680[1] = 8;
                D_8006940D[0] = D_8006940D[1] = 0;
            }
            if ( ( D_8005C160 & 0xF ) != 2 && ( ( D_800E57E4[0] | D_800E57E4[1] ) & U_CBUTTONS ) ) {
                cUpHeld = TRUE;
                D_800DF680[0] = D_800DF680[1] = 8;
                D_8006940D[0] = D_8006940D[1] = 0;
            }
        }
    }
    if ( !D_8012D650_gacode ) {
        D_8012D239_gacode = 0;
    }
    D_8012D220_gacode->update();
    func_8002A2F0();
    for ( team = 0; team <= D_8005C162; team++ ) {
        func_80114EC4_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] );
    }
    func_80028EA8( &D_800F8094 );
    func_8002FDE4( 0x9F, 0, 2, 0xF0, 0, 0, 0, 0xFF );
    if ( !D_8012D650_gacode && !cUpHeld ) {
        func_80114FC4_gacode( D_8012CE02_gacode[0][D_8012CDF8_gacode[0]] );
        if ( D_8005C162 ) {
            func_80114FC4_gacode( D_8012CE02_gacode[1][D_8012CDF8_gacode[1]] );
        } else {
            func_8002E4B0( 0x926, 0xD2, 0x10, (u8)D_80123600_gacode + 0x40, 1.0f, 1.0f );
        }
    }
    if ( !D_8012D650_gacode && !cUpHeld ) {
        D_80123603_gacode = D_8012CDFC_gacode();
        if ( !D_8005C162 ) {
            func_801150B4_gacode( D_8012CE02_gacode[0][D_8012CDF8_gacode[0]] );
        }
    } else {
        for ( team = 0; team <= D_8005C162; team++ ) {
            func_80114FFC_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] );
        }
    }
    if ( D_8012D650_gacode ) {
        func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x10, 0x10, 0x10, 0x80 );
        D_80123604_gacode = D_8012CDF0_gacode();
    }
    if ( cUpHeld ) {
        func_80112E04_gacode();
    }
    if ( D_80123602_gacode == 1 ) {
        D_80123600_gacode -= 8;
        if ( D_80123600_gacode <= 0 ) {
            D_80123600_gacode = 0;
            D_80123602_gacode = 0;
        }
    } else {
        D_80123600_gacode += 8;
        if ( D_80123600_gacode >= 0x80 ) {
            D_80123600_gacode = 0x80;
            D_80123602_gacode = 1;
        }
    }
    func_80028A0C( &D_800F8094 );
    func_80028950( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
}

/*
================
func_80112A74_gacode
================
*/
void func_80112A74_gacode( void ) {
    s32 team;

    func_80120D7C_gacode( &D_8012CDE8_gacode );
    for ( team = 0; team <= D_8005C162; team++ ) {
        func_80114FFC_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] );
    }
}

/*
================
func_80112AF0_gacode

Pause menu: draws it with the cursor on one of its four items and the on/off mark of D_8009D098.unk_79,
moves the cursor with the D-pad, and on A or Z runs the item; B closes the menu.
================
*/
s32 func_80112AF0_gacode( void ) {
    s16 cursorY[4] = { 0x8A, 0x5A, 0x6A, 0x7A };
    s16 markX[2] = { 0xB6, 0x9C };
    u8 isMode2;

    isMode2 = ( D_8005C160 & 0xF ) == 2;
    func_8002DF3C( isMode2 ? 0x882 : 0x89E, 0x50, 0x40 );
    func_8002E4B0( 0x8A0, 0x66, cursorY[D_8012D239_gacode], (u8)D_80123600_gacode + 0x40, 1.0f, 1.0f );
    func_8002E4B0( 0x89F, markX[D_8009D098.unk_79], 0x6C, (u8)D_80123600_gacode + 0x40, 1.0f, 1.0f );
    if ( D_800E6758 & U_JPAD ) {
        func_80037870( 0, 0 );
        if ( D_8012D239_gacode == 0 ) {
            D_8012D239_gacode = 3;
        } else {
            D_8012D239_gacode--;
        }
    }
    if ( D_800E6758 & D_JPAD ) {
        func_80037870( 0, 0 );
        if ( D_8012D239_gacode == 3 ) {
            D_8012D239_gacode = 0;
        } else {
            D_8012D239_gacode++;
        }
    }
    if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
        switch ( D_8012D239_gacode ) {
        case 0:
            D_8012D650_gacode = 0;
            func_80037870( 1, 0 );
            break;
        case 1:
            D_8012CDF0_gacode = isMode2 ? func_80112EBC_gacode : func_80112D94_gacode;
            func_80037870( 1, 0 );
            break;
        case 2:
            D_8009D098.unk_79 = !D_8009D098.unk_79;
            if ( D_8009D098.unk_79 ) {
                func_800379C8( D_8012CE00_gacode );
            } else {
                func_80037B18();
            }
            break;
        case 3:
            D_8012CDF0_gacode = func_80113210_gacode;
            func_80037870( 1, 0 );
            break;
        }
    } else if ( D_800E4D62 & B_BUTTON ) {
        D_8012D650_gacode = 0;
        func_80037870( 2, 0 );
    }
    return 0;
}

/*
================
func_80112D94_gacode
================
*/
s32 func_80112D94_gacode( void ) {
    func_8002E4B0( 0x89E, 0x50, 0x40, 0x40, 1.0f, 1.0f );
    func_80112E04_gacode();
    if ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
        func_80037870( 11, 0 );
        D_8012CDF0_gacode = func_80112AF0_gacode;
    }
    return 0;
}

/*
================
func_80112E04_gacode
================
*/
void func_80112E04_gacode( void ) {
    if ( ( D_8005C160 & 0xF ) == 3 ) {
        func_8011A26C_gacode();
        return;
    }
    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        func_80039700( D_8005C160 >> 6, 0, 0x18, 0x30 );
        break;
    case 1:
        func_80039CBC( D_8005C160 >> 6, 0, 0x20, 0x30 );
        break;
    case 2:
        func_8003A3F0( 0, 0x18, 0x30 );
        break;
    }
}

/*
================
func_80112EBC_gacode

Main loop state of the pause menu's page browser (mode 2): draws the current page of D_80123608_gacode with
the cursor on an item or on the back item, moves the cursor with the D-pad, and on A or Z opens the item's
page or goes back to the parent page (from the first page, to the pause menu); B goes back to the pause menu.
================
*/
s32 func_80112EBC_gacode( void ) {
    s16 x = 0x28;
    s16 y = 0x28;

    func_8002DF3C( D_80123608_gacode[D_80123680_gacode].image, x, y );
    if ( D_80123608_gacode[D_80123680_gacode].cursor >= 0
        && D_80123608_gacode[D_80123680_gacode].cursor < D_80123608_gacode[D_80123680_gacode].backItem ) {
        func_8002E4B0( 0x885, x + 8, y + 0x18 + D_80123608_gacode[D_80123680_gacode].cursor * 16,
                       (u8)D_80123600_gacode + 0x7F, 1.0f, 1.0f );
    } else {
        func_8002E4B0( 0x884, x + 0xBB, y + D_80123608_gacode[D_80123680_gacode].backY,
                       (u8)D_80123600_gacode + 0x10, 1.0f, 1.0f );
    }
    if ( D_80123608_gacode[D_80123680_gacode].cursor >= 0 ) {
        if ( D_800E6758 & U_JPAD ) {
            func_80037870( 0, 0 );
            if ( D_80123608_gacode[D_80123680_gacode].cursor != 0 ) {
                D_80123608_gacode[D_80123680_gacode].cursor--;
            } else {
                D_80123608_gacode[D_80123680_gacode].cursor = D_80123608_gacode[D_80123680_gacode].backItem;
            }
        }
        if ( D_800E6758 & D_JPAD ) {
            func_80037870( 0, 0 );
            if ( D_80123608_gacode[D_80123680_gacode].cursor != D_80123608_gacode[D_80123680_gacode].backItem ) {
                D_80123608_gacode[D_80123680_gacode].cursor++;
            } else {
                D_80123608_gacode[D_80123680_gacode].cursor = 0;
            }
        }
        if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
            if ( D_80123608_gacode[D_80123680_gacode].cursor == D_80123608_gacode[D_80123680_gacode].backItem ) {
                D_80123608_gacode[D_80123680_gacode].cursor = 0;
                if ( D_80123608_gacode[D_80123680_gacode].parent < 0 ) {
                    func_80037870( 11, 0 );
                    D_80123680_gacode = 0;
                    D_8012CDF0_gacode = func_80112AF0_gacode;
                } else {
                    func_80037870( 0x33, 0 );
                    D_80123680_gacode = D_80123608_gacode[D_80123680_gacode].parent;
                }
            } else {
                func_80037870( 0x33, 0 );
                D_80123680_gacode = D_80123608_gacode[D_80123680_gacode].firstPage
                                  + D_80123608_gacode[D_80123680_gacode].cursor;
            }
        }
    } else if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( 0x33, 0 );
        D_80123680_gacode = D_80123608_gacode[D_80123680_gacode].firstPage;
    }
    if ( D_800E4D62 & B_BUTTON ) {
        func_80037870( 2, 0 );
        D_80123680_gacode = 0;
        D_8012CDF0_gacode = func_80112AF0_gacode;
    }
    return 0;
}

/*
================
func_80113210_gacode
================
*/
s32 func_80113210_gacode( void ) {
    switch ( func_8003B204() ) {
    case 1:
        D_8012CDF0_gacode = func_80113270_gacode;
        break;
    case 2:
        D_8012CDF0_gacode = func_80112AF0_gacode;
        break;
    }
    return 0;
}

/*
================
func_80113270_gacode
================
*/
s32 func_80113270_gacode( void ) {
    if ( func_80037410() ) {
        return 0;
    }
    D_8012CDF0_gacode = func_801132A8_gacode;
    return 1;
}

/*
================
func_801132A8_gacode
================
*/
s32 func_801132A8_gacode( void ) {
    return 0;
}

/*
================
func_801132B0_gacode

Sets up the game for the mode in the low nibble of D_8005C160, with its BGM.
================
*/
void func_801132B0_gacode( void ) {
    switch ( D_8005C160 & 0xF ) {
    case 0:
        func_80113384_gacode();
        break;
    case 1:
        func_80113594_gacode( func_80113EE4_gacode, func_801151A0_gacode );
        D_8012CE00_gacode = D_8012D220_gacode->unk_1A;
        break;
    case 3:
        func_80113594_gacode( func_80114024_gacode, func_8011582C_gacode );
        D_8012CE00_gacode = 17;
        break;
    case 2:
        func_80113594_gacode( func_80114108_gacode, func_801157F8_gacode );
        D_8012CE00_gacode = 16;
        break;
    }
    func_801136FC_gacode();
}

/*
================
func_80113384_gacode

Sets up the game of mode 0 by its sub-mode (bits 4-5 of D_8005C160): its main loop state and handler, the
frames' setups (the par game also sums its pars into D_8009CF80[0]) and the BGM.
================
*/
void func_80113384_gacode( void ) {
    s32 i;

    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        /* the arguments that share a state still have a case each (retail keeps one block per case) */
        switch ( D_8005C160 >> 6 ) {
        case 0:
            func_80113594_gacode( func_80113968_gacode, func_801151A0_gacode );
            break;
        case 2:
            func_80113594_gacode( func_80113968_gacode, func_801151A0_gacode );
            break;
        case 1:
            func_80113594_gacode( func_80113AD4_gacode, func_801151A0_gacode );
            break;
        }
        D_8012CE00_gacode = D_8012D220_gacode->unk_1A;
        break;
    case 1:
        func_801209C0_gacode( D_800DF59C.unk_04, 25, 0 );
        for ( i = 0; i < 9; i++ ) {
            D_80099458[0][i] = D_80123950_gacode[D_800DF59C.unk_04[i]].unk_00;
            D_8009CF80[0] += D_80099458[0][i];
        }
        /* the arguments that share a state still have a case each (retail keeps one block per case) */
        switch ( D_8005C160 >> 6 ) {
        case 0:
            func_80113594_gacode( func_80113D48_gacode, func_801155BC_gacode );
            break;
        case 1:
            func_80113594_gacode( func_80113D48_gacode, func_801155BC_gacode );
            break;
        }
        D_8012CE00_gacode = D_8012D220_gacode->unk_1A;
        break;
    case 2:
        func_801209C0_gacode( &D_800DF59C.unk_04[0], 5, 0 );
        func_801209C0_gacode( &D_800DF59C.unk_04[3], 5, 5 );
        func_801209C0_gacode( &D_800DF59C.unk_04[6], 5, 10 );
        D_800DF59C.unk_04[9] = 15;
        for ( i = 0; i < D_8005C161; i++ ) {
            D_80068A50[i] = 5;
        }
        func_80113594_gacode( func_80113968_gacode, func_801156D8_gacode );
        D_8012CE00_gacode = 11;
        break;
    }
}

/*
================
func_80113594_gacode

Starts a game mode: sets the main loop state and the mode's handler, picks the handler that starts a turn,
and sets up each player on the lane of their team (with two players the second, with three or four players
2 and up, on the second lane, when there are two teams); players without a controller get port -1.
================
*/
void func_80113594_gacode( gaState_t state, gaPlayerFunc_t mode ) {
    s32 player;
    u8 team;
    u8 lane;

    D_8012CDFC_gacode = state;
    D_8012CDF4_gacode = mode;
    if ( mode == func_801157F8_gacode ) {
        D_8012D23C_gacode = func_80115D38_gacode;
    } else if ( mode == func_801155BC_gacode ) {
        D_8012D23C_gacode = func_80115CDC_gacode;
    } else if ( mode == func_801156D8_gacode ) {
        D_8012D23C_gacode = func_80115D38_gacode;
    } else if ( mode == func_8011582C_gacode ) {
        D_8012D23C_gacode = func_80115CDC_gacode;
    } else {
        D_8012D23C_gacode = func_80115B9C_gacode;
    }
    for ( player = 0; player < D_8005C161; player++ ) {
        /* team stays unset with one player (or more than four); D_8005C162 is then 0 */
        switch ( D_8005C161 ) {
        case 2:
            team = player > 0;
            break;
        case 3:
        case 4:
            team = player >= 2;
            break;
        }
        lane = D_8005C162 ? team : 0;
        func_80114550_gacode( player, ( D_8005C0E4[player] >> 7 ) ? -1 : lane, D_8005C0E4[player] & 0xF, lane );
    }
    if ( !D_8005C162 ) {
        func_80115054_gacode();
    }
}

/*
================
func_801136FC_gacode

Fills the teams' player lists: with one team every player in order; with two, players 0 and 1 apart with
two players, players 0 and 1 against the rest with three or four.
================
*/
void func_801136FC_gacode( void ) {
    s32 i;

    if ( !D_8005C162 ) {
        for ( i = 0; i < D_8005C161; i++ ) {
            D_8012CE02_gacode[0][i] = i;
        }
        for ( ; i < 5; i++ ) {
            D_8012CE02_gacode[0][i] = -1;
        }
        return;
    }
    switch ( D_8005C161 ) {
    case 2:
        D_8012CE02_gacode[0][0] = 0;
        for ( i = 1; i < 5; i++ ) {
            D_8012CE02_gacode[0][i] = -1;
        }
        D_8012CE02_gacode[1][0] = 1;
        for ( i = 1; i < 5; i++ ) {
            D_8012CE02_gacode[1][i] = -1;
        }
        break;
    case 3:
        D_8012CE02_gacode[0][0] = 0;
        D_8012CE02_gacode[0][1] = 1;
        for ( i = 2; i < 5; i++ ) {
            D_8012CE02_gacode[0][i] = -1;
        }
        D_8012CE02_gacode[1][0] = 2;
        for ( i = 1; i < 5; i++ ) {
            D_8012CE02_gacode[1][i] = -1;
        }
        break;
    case 4:
        D_8012CE02_gacode[0][0] = 0;
        D_8012CE02_gacode[0][1] = 1;
        for ( i = 2; i < 5; i++ ) {
            D_8012CE02_gacode[0][i] = -1;
        }
        D_8012CE02_gacode[1][0] = 2;
        D_8012CE02_gacode[1][1] = 3;
        for ( i = 2; i < 5; i++ ) {
            D_8012CE02_gacode[1][i] = -1;
        }
        break;
    }
}

/*
================
func_801138EC_gacode

Advances *cur to the next player of the team, wrapping at the end of the list, and on past each player
whose D_800994BE flag is set, but stops at start.
================
*/
inline void func_801138EC_gacode( u8 *cur, s32 start, s32 team ) {
    if ( D_8012CE02_gacode[team][++( *cur )] == -1 ) {
        *cur = 0;
    }
    if ( *cur != start && D_800994BE[D_8012CE02_gacode[team][*cur]] ) {
        func_801138EC_gacode( cur, start, team );
    }
}

/*
================
func_80113968_gacode

Main loop state of a team game: runs the current player of each team; when one is done, ends the game for
the team if every player has the D_800994BE flag set, else passes the turn to the team's next player.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: schedule only: in the flag-count loop retail increments i before the row pointer, the draft
// after it (tried: if/++ count, i++ in the index, local order; 20k permuter iterations found nothing). The
// turn pass is func_801138EC_gacode inlined (its inline is what puts the body here at -O2).
s32 func_80113968_gacode( void ) {
    s32 team;
    s32 flagged;
    s32 i;

    for ( team = 0; team <= D_8005C162; team++ ) {
        if ( func_80114640_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] ) ) {
            flagged = 0;
            for ( i = 0; D_8012CE02_gacode[team][i] != -1; i++ ) {
                flagged += D_800994BE[D_8012CE02_gacode[team][i]] != 0;
            }
            if ( flagged == i ) {
                func_80115D70_gacode( D_8012CE02_gacode[team][0], 0 );
            } else {
                func_801138EC_gacode( &D_8012CDF8_gacode[team], D_8012CDF8_gacode[team], team );
            }
        }
    }
    return func_80115E24_gacode( D_8005C162 );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113968_gacode);
#endif

/*
================
func_80113AD4_gacode

Main loop state of the pair game: runs the current player of each team; with three or more players the
turn alternates between the pair of players 0 and 1 and the pair 2 and 3 (D_80123684_gacode keeps each
pair's next index), else it passes to the team's next player. Ends the game for the team once the flag in
D_800994BE of the current player's pair is set (with four players on one team, once both pairs' flags are).
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: allocation only: retail reads the D_800994BE[pair] flag through its absolute address and keeps
// the array's base (in fp) for the four-player test alone; the draft reuses the hoisted base for both, which
// renumbers s5-s8 (tried: the test as a ternary, inverted test order; the permuter only helped a little, with
// the pair index in a local).
s32 func_80113AD4_gacode( void ) {
    s32 team;
    s32 result;
    s32 done;

    done = 0;
    for ( team = 0; team <= D_8005C162; team++ ) {
        result = func_80114640_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] );
        if ( result < 0 ) {
            if ( D_8012CE02_gacode[0][2] != -1 ) {
                if ( D_8012CDF8_gacode[team] >= 2 ) {
                    D_80123684_gacode[1] = D_8012CDF8_gacode[team];
                    D_8012CDF8_gacode[team] = D_8012CDF8_gacode[team] == 3 ? 2 : 3;
                } else {
                    D_80123684_gacode[0] = D_8012CDF8_gacode[team];
                    D_8012CDF8_gacode[team] = D_8012CDF8_gacode[team] == 1 ? 0 : 1;
                }
            } else if ( D_8012CE02_gacode[team][++D_8012CDF8_gacode[team]] == -1 ) {
                D_8012CDF8_gacode[team] = 0;
            }
        } else if ( result > 0 ) {
            if ( D_8012CE02_gacode[0][2] != -1 ) {
                if ( D_8012CDF8_gacode[team] >= 2 ) {
                    D_8012CDF8_gacode[team] = D_80123684_gacode[0];
                    D_80123684_gacode[0] = D_80123684_gacode[0] == 1 ? 0 : 1;
                } else {
                    D_8012CDF8_gacode[team] = D_80123684_gacode[1];
                    D_80123684_gacode[1] = D_80123684_gacode[1] == 3 ? 2 : 3;
                }
            } else if ( D_8012CE02_gacode[team][++D_8012CDF8_gacode[team]] == -1 ) {
                D_8012CDF8_gacode[team] = 0;
            }
        }
        if ( result != 0 ) {
            if ( !D_8005C162 && D_8005C161 == 4 ) {
                if ( D_800994BE[0] && D_800994BE[1] ) {
                    done = 1;
                }
            } else if ( D_800994BE[D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] >= 2] ) {
                done = 1;
            }
            if ( done ) {
                func_80115D70_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]], 0 );
            }
        }
    }
    return func_80115E24_gacode( D_8005C162 );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113AD4_gacode);
#endif

/*
================
func_80113D48_gacode

Main loop state of the par game: runs the current player of each team, passes the turn on when one is
done, and ends the game for the team when every player has the D_800994BE flag set (or, without that,
when the turn comes back to the first player). Then advances the frame when the lanes are done.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: schedule only, the same residue as func_80113968_gacode: retail increments i before the
// row pointer in the flag-count loop (tried an inline helper over the row, pointer-walk counts).
s32 func_80113D48_gacode( void ) {
    s32 team;
    s32 i;
    s32 flagged;
    s32 showFrame;

    for ( team = 0; team <= D_8005C162; team++ ) {
        if ( func_80114640_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] ) ) {
            if ( D_8012CE02_gacode[team][++D_8012CDF8_gacode[team]] == -1 ) {
                D_8012CDF8_gacode[team] = 0;
            }
            flagged = 0;
            for ( i = 0; D_8012CE02_gacode[team][i] != -1; i++ ) {
                flagged += D_800994BE[D_8012CE02_gacode[team][i]] != 0;
            }
            if ( flagged == i ) {
                func_80115D70_gacode( D_8012CE02_gacode[team][0], 0 );
            } else if ( D_8012CDF8_gacode[team] == 0 ) {
                func_80115D70_gacode( D_8012CE02_gacode[team][0], 1 );
            }
        }
    }
    showFrame = ( D_8005C160 >> 6 ) == 1;
    if ( D_8012D238_gacode < 9 && func_80115ED4_gacode( D_8005C162 ) ) {
        if ( showFrame ) {
            func_8012224C_gacode( D_8012D238_gacode );
        }
        D_8012D238_gacode++;
    }
    return func_80115E24_gacode( D_8005C162 ) != 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113D48_gacode);
#endif

/*
================
func_80113EE4_gacode

Main loop state of a two-lane game: updates both lanes; when both are done, counts a game won for the lane
func_80115FCC_gacode returns. On A or Z with both lanes at -1, starts the next game (skipping when a lane has
won two).
================
*/
s32 func_80113EE4_gacode( void ) {
    s32 results[2];
    s32 winner;
    s32 i;

    for ( i = 0; i < 2; i++ ) {
        results[i] = func_80114640_gacode( i );
        if ( results[i] > 0 && D_800994BE[i] ) {
            func_80115D70_gacode( i, 1 );
        }
    }
    if ( func_80115E24_gacode( 1 ) ) {
        return 1;
    }
    if ( func_80115DF4_gacode( 0 ) && func_80115DF4_gacode( 1 ) ) {
        winner = func_80115FCC_gacode();
        if ( winner >= 0 ) {
            D_800693DC[winner]++;
        }
        return 0;
    }
    if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) && results[0] + results[1] == -2 ) {
        func_80037870( SE_DECIDE, 0 );
        if ( D_800693DC[0] == 2 || D_800693DC[1] == 2 ) {
            func_80116170_gacode( 1 );
        } else {
            func_80116170_gacode( 0 );
        }
    }
    return 0;
}

/*
================
func_80114024_gacode

Main loop state of mode 3: updates both lanes, and ends the turn when both are done, on A or Z, or when
both lanes return -1.
================
*/
s32 func_80114024_gacode( void ) {
    s32 results[2];
    s32 i;

    for ( i = 0; i < 2; i++ ) {
        results[i] = func_80114640_gacode( i );
        if ( results[i] > 0 && D_800994BE[i] ) {
            func_80115D70_gacode( i, 1 );
        }
    }
    if ( func_80115E24_gacode( 1 ) ) {
        return 1;
    }
    if ( func_80115DF4_gacode( 0 ) && func_80115DF4_gacode( 1 ) ) {
        func_80115FCC_gacode();
        return 0;
    }
    if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || results[0] + results[1] == -2 ) {
        func_80116170_gacode( 1 );
    }
    return 0;
}

/*
================
func_80114108_gacode
================
*/
s32 func_80114108_gacode( void ) {
    func_80114640_gacode( 0 );
    return 0;
}

