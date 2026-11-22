/* 3C570.c -- game overlay: the main loop states */

#include "common.h"
#include "libultra.h"
#include "input.h"
#include "gacode/gacode.h"

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112350_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_801125A0_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112AF0_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112EBC_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113384_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113AD4_gacode);

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

INCLUDE_RODATA("asm/nonmatchings/gacode/3C570", D_8012CA00_gacode);
