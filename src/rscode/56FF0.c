/* 56FF0.c -- results overlay */

#include "common.h"
#include "libultra.h"
#include "rscode/rscode.h"

/*
================
func_80112350_rscode

Entry of the overlay: starts the first state and the mode's results.
================
*/
void func_80112350_rscode( void ) {
    func_80037B18();
    D_80114250_rscode = 0;
    D_80114252_rscode = 0;
    D_80114253_rscode = 0;
    D_801142AC_rscode = func_80112B60_rscode;
    D_801142A8_rscode = D_8005C0B6;
    func_80112574_rscode();
}

/*
================
func_801123A8_rscode

Frame of the overlay: reads the input, draws the background, runs the current state and pulses the boxes'
alpha.
================
*/
void func_801123A8_rscode( void ) {
    if ( !D_8005C0BF && D_80114250_rscode ) {
        D_8005C0BF = 1;
        func_80037870( 50, 0 );
    }
    if ( D_8005C0BF == 1 && D_8005C0C0 == 255 ) {
        D_8005C0BF = 2;
        D_8005C0C0 = 263;
        D_8005C0B6 = D_801142A8_rscode;
    }
    D_801142A6_rscode = D_8005C0BF ? 0 : D_800E6758;
    D_801142A0_rscode = D_8005C0BF ? 0 : D_800E4D62;
    func_80028EA8( &D_800F8094 );
    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    D_801142AC_rscode();
    if ( D_80114253_rscode == 1 ) {
        D_80114252_rscode -= 8;
        if ( D_80114252_rscode <= 0 ) {
            D_80114252_rscode = 0;
            D_80114253_rscode = 0;
        }
    } else {
        D_80114252_rscode += 8;
        if ( D_80114252_rscode >= 0x80 ) {
            D_80114252_rscode = 0x80;
            D_80114253_rscode = 1;
        }
    }
    func_80028A0C( &D_800F8094 );
    func_80028950( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
}

/*
================
func_80112574_rscode

Picks the first state and the BGM for the game mode.
================
*/
void func_80112574_rscode( void ) {
    switch ( D_8005C160 & 0xF ) {
    case 0:
        func_80112674_rscode();
        func_800379C8( 10 );
        break;
    case 1:
        D_801142AC_rscode = func_8011311C_rscode;
        func_800379C8( D_800EA090[0] <= D_800EA090[1] ? 15 : 14 );
        func_80037870( D_800EA090[0] <= D_800EA090[1] ? 0x6D : 0x6C, 0 );
        break;
    case 2:
        break;
    case 3:
        D_801142AC_rscode = D_801142B0_rscode = func_80113564_rscode;
        func_800379C8( 18 );
        break;
    }
}

/*
================
func_80112674_rscode

Picks the state of mode 0's results by its sub-mode; a perfect game (bit 0x200 of D_8009D098.unk_04) or a
finished game (bit 0x100) first shows its message, once.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: control flow; retail's switch on D_8005C160 >> 6 keeps a case 1 that goes to the end (its
// decision tree tests 1, then < 2, then 0), which an empty case here does not keep
void func_80112674_rscode( void ) {
    s32 found = 0;
    s32 i;

    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        func_801129CC_rscode();
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
                D_801142AC_rscode = func_8011291C_rscode;
                D_8009D098.unk_04 |= 0x200;
            }
            break;
        case 1:
            break;
        }
        break;
    case 1:
        D_801142AC_rscode = func_80113030_rscode;
        break;
    case 2:
        D_801142AC_rscode = func_801130AC_rscode;
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
            D_801142AC_rscode = func_8011286C_rscode;
            D_8009D098.unk_04 |= 0x100;
        }
        break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80112674_rscode);
#endif

/*
================
func_8011286C_rscode

Shows message 0x88C in a box for 300 frames or until A or Z, then goes to func_801130AC_rscode.
================
*/
void func_8011286C_rscode( void ) {
    func_8002C950( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, D_80114252_rscode + 0x40 );
    func_8002DF3C( 0x88C, 0x26, 0x50 );
    if ( ++D_80114254_rscode >= 300 || ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_801130AC_rscode;
    }
}

/*
================
func_8011291C_rscode

Shows message 0xAE3 in a box for 300 frames or until A or Z, then goes to func_80112B60_rscode.
================
*/
void func_8011291C_rscode( void ) {
    func_8002C950( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, D_80114252_rscode + 0x40 );
    func_8002DF3C( 0xAE3, 0x26, 0x50 );
    if ( ++D_80114254_rscode >= 300 || ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80112B60_rscode;
    }
}

/*
================
func_801129CC_rscode

Ranks the players by score; outside a sub-mode, adds the game to the records of each player's character
and saves them.
================
*/
void func_801129CC_rscode( void ) {
    s16 scores[4];
    s32 i;
    s32 character;

    D_801142B4_rscode = D_8005C160 >> 6;
    for ( i = 0; i < D_8005C161; i++ ) {
        scores[i] = D_800DF630[i][9];
    }
    func_80113FCC_rscode( D_801142A2_rscode, D_8005C161, scores, 0, 300 );
    if ( !D_801142B4_rscode ) {
        for ( i = 0; i < D_8005C161; i++ ) {
            character = ( ( D_8005C0E4[i] >> 4 ) & 7 ) - 1;
            if ( character >= 0 ) {
                func_80113EC0_rscode( &D_800E44E8[character], i );
                func_80113EC0_rscode( &D_8009D098.unk_08[character], i );
            }
        }
        for ( character = 0; character < 4; character++ ) {
            func_8003B034( &D_800E44E8[character], character, 0 );
            func_8003B034( &D_8009D098.unk_08[character], character, 1 );
        }
    }
    D_801142AC_rscode = func_80112B60_rscode;
}

/*
================
func_80112B60_rscode

Mode 0's results: the score sheet or the pin diagrams (left and right choose), then, unless every record is
empty, down goes to the records of func_80112E04_rscode.
================
*/
void func_80112B60_rscode( void ) {
    u16 images[2] = { 0x87A, 0x898 };
    s16 y = 0x10;
    s16 empty = 0;
    s16 i;

    for ( i = 0; i < 4; i++ ) {
        empty += D_800E44E8[i].unk_0C[0] ? 0 : 1;
    }
    if ( ( D_801142A6_rscode & L_JPAD ) && D_80114251_rscode == 1 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114251_rscode = 0;
    }
    if ( ( D_801142A6_rscode & R_JPAD ) && D_80114251_rscode == 0 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114251_rscode = 1;
    }
    func_8002DF3C( 0x88D, 0x64, y );
    y += 0x18;
    switch ( D_80114251_rscode ) {
    case 0:
        func_80039700( D_801142B4_rscode, 1, 0x18, y );
        break;
    case 1:
        func_80113904_rscode( 0x18, y );
        break;
    }
    y += 0x8C;
    func_80113DC8_rscode( D_80114251_rscode, images, y, 0x50, 0x10 );
    y += 0x1C;
    if ( !D_801142B4_rscode && empty != 4 ) {
        func_8002E4B0( 0xA82, 0x48, y, 0x60, 1.0f, 1.0f );
        func_8002E4B0( 0xA81, 0xA8, y, 0x60, 1.0f, 1.0f );
    }
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80112B60_rscode;
    }
    if ( ( D_801142A6_rscode & D_JPAD ) && !D_801142B4_rscode && empty != 4 ) {
        func_80037870( SE_CURSOR, 0 );
        D_801142AC_rscode = func_80112E04_rscode;
        D_80114251_rscode += 2;
    }
}

/*
================
func_80112E04_rscode

Records of the characters, of this session or saved (left and right choose); up goes back to
func_80112B60_rscode.
================
*/
void func_80112E04_rscode( void ) {
    u16 images[2] = { 0xA82, 0xA81 };
    s16 y = 0x10;

    if ( ( D_801142A6_rscode & L_JPAD ) && D_80114251_rscode == 3 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114251_rscode = 2;
    }
    if ( ( D_801142A6_rscode & R_JPAD ) && D_80114251_rscode == 2 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114251_rscode = 3;
    }
    func_8002DF3C( 0x88D, 0x64, y );
    y += 0x18;
    switch ( D_80114251_rscode ) {
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
    func_80113DC8_rscode( D_80114251_rscode - 2, images, y, 0x50, 0x10 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80112E04_rscode;
    }
    if ( D_801142A6_rscode & U_JPAD ) {
        func_80037870( SE_CURSOR, 0 );
        D_801142AC_rscode = func_80112B60_rscode;
        D_80114251_rscode -= 2;
    }
}

/*
================
func_80113030_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_80113030_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_80039CBC( D_8005C160 >> 6, 1, 0x20, 0x28 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80113030_rscode;
    }
}

/*
================
func_801130AC_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_801130AC_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_8003A3F0( 1, 0x18, 0x28 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_801130AC_rscode;
    }
}

/*
================
func_8011311C_rscode

Results of a duel: the score of each game, the higher one marked, the games won, and the winner; A or Z
leaves when player 1 won (setting the bit of D_8005C163 in D_8009D098.unk_04), else opens the menu.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail passes x to func_800389D0 zero-extended and not folded to the constant 0x40 (then forms
// x + 0x38 with ori), and allocates the first loop's registers differently (x in s1, i in s2)
void func_8011311C_rscode( void ) {
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
        func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player], x, y );
        x += 0x38;
        opponent = !player;
        for ( i = 0; i < games; i++ ) {
            func_8002DF3C( 0x86D, x, y );
            func_800393D8( D_8009D030[player][i], x + 6, y + 6 );
            if ( D_8009D030[player][i] > D_8009D030[opponent][i] ) {
                func_8002E4B0( 0x86C, x, y, D_80114252_rscode + 0x20, 1.0f, 1.0f );
            }
            x += 0x20;
        }
        func_8002DF3C( 0x86B, x, y );
        func_8003A8B4( player, D_800693DC[player], 1, x + 4, y + 2 );
        y += 0x14;
    }
    if ( D_800693DC[0] > D_800693DC[1] ) {
        func_8002E4B0( 0x869, 0x2C, y + 8, D_80114252_rscode + 0x7F, 1.0f, 1.0f );
        if ( !( D_8009D098.unk_04 & ( 1 << D_8005C163 ) ) ) {
            func_8002DF3C( 0x870, 0x38, 0xAC );
        }
    } else {
        func_8002E4B0( 0x86A, 0x2C, y + 8, D_80114252_rscode + 0x7F, 1.0f, 1.0f );
    }
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        if ( D_800693DC[0] > D_800693DC[1] ) {
            D_801142AC_rscode = func_8011377C_rscode;
            D_8009D098.unk_04 |= 1 << D_8005C163;
        } else {
            D_801142AC_rscode = func_80113564_rscode;
            D_801142B0_rscode = func_8011311C_rscode;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_8011311C_rscode);
INCLUDE_RODATA("asm/nonmatchings/rscode/56FF0", D_80114290_rscode);
#endif

/*
================
func_80113504_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_80113504_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80113504_rscode;
    }
}

/*
================
func_80113564_rscode

Menu of two choices: leave the results, or func_8011372C_rscode's; B goes back to the page it came from.
================
*/
void func_80113564_rscode( void ) {
    u16 images[2] = { 0xAB5, 0xA57 };

    if ( ( D_801142A6_rscode & L_JPAD ) && D_80114256_rscode == 1 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114256_rscode = 0;
    }
    if ( ( D_801142A6_rscode & R_JPAD ) && D_80114256_rscode == 0 ) {
        func_80037870( SE_CURSOR, 0 );
        D_80114256_rscode = 1;
    }
    func_80113DC8_rscode( D_80114256_rscode, images, 0x68, 0x50, 0x10 );
    func_8002E4B0( D_80114256_rscode ? 0x84C : 0x84B, 0x98, 0x68, D_80114252_rscode + 0x40, 1.0f, 1.0f );
    if ( ( D_801142A0_rscode & B_BUTTON ) && D_801142B0_rscode != func_80113564_rscode ) {
        func_80037870( 2, 0 );
        D_80114256_rscode = 0;
        D_801142AC_rscode = D_801142B0_rscode;
    }
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        switch ( D_80114256_rscode ) {
        case 0:
            D_801142A8_rscode = 4;
            D_8005CB60 = 1;
            D_801142AC_rscode = func_801137CC_rscode;
            D_80114250_rscode = 1;
            break;
        case 1:
            func_80037870( SE_DECIDE, 0 );
            D_801142AC_rscode = func_8011372C_rscode;
            D_80114256_rscode = 0;
            break;
        }
    }
}

/*
================
func_8011372C_rscode
================
*/
void func_8011372C_rscode( void ) {
    s32 choice;

    choice = func_8003B204();
    if ( choice == 2 ) {
        D_801142AC_rscode = func_80113564_rscode;
    } else if ( choice == 1 ) {
        D_801142AC_rscode = func_8011377C_rscode;
    }
}

/*
================
func_8011377C_rscode

Waits for func_80037410, then leaves the overlay.
================
*/
void func_8011377C_rscode( void ) {
    if ( func_80037410() ) {
        return;
    }
    D_80114250_rscode = 1;
    D_801142A8_rscode = 4;
    D_8005CB60 = 0;
    D_801142AC_rscode = func_801137CC_rscode;
}

/*
================
func_801137CC_rscode

Last state: does nothing while the overlay is left.
================
*/
void func_801137CC_rscode( void ) {
}

/*
================
func_801137D4_rscode

Draws the pin diagram of a player's frame at (x, y): image 0x89D for a pin of the first mask (D_800A3050),
else 0x89C for a pin of the second (D_800F7C18). Declared inline: func_80113904_rscode repeats its body.
================
*/
inline void func_801137D4_rscode( s32 player, u8 frame, s16 x, s16 y ) {
    s32 pin;

    func_8002DF3C( 0x899, x, y );
    for ( pin = 0; pin < 10; pin++ ) {
        if ( ( D_800A3050[player][frame] >> ( 9 - pin ) ) & 1 ) {
            func_8002DF3C( 0x89D, x + D_80114258_rscode[pin][0], y + D_80114258_rscode[pin][1] );
        } else if ( ( D_800F7C18[player][frame] >> ( 9 - pin ) ) & 1 ) {
            func_8002DF3C( 0x89C, x + D_80114258_rscode[pin][0], y + D_80114258_rscode[pin][1] );
        }
    }
}

/*
================
func_80113904_rscode

Pin diagrams of every player's frames at (x, y), in teams in sub-mode 1; C left and right turn the page.
================
*/
void func_80113904_rscode( s16 x, s16 y ) {
    s16 rowX;
    s16 pageX;
    s32 players;
    s32 player;
    s32 frame;
    s32 end;
    s32 mate;

    if ( ( D_801142A0_rscode & R_CBUTTONS ) && !D_80114280_rscode ) {
        func_80037870( 0x33, 0 );
        D_80114280_rscode = 1;
    }
    if ( ( D_801142A0_rscode & L_CBUTTONS ) && D_80114280_rscode == 1 ) {
        func_80037870( 0x33, 0 );
        D_80114280_rscode = 0;
    }
    func_8002DF3C( D_80114280_rscode ? 0x89B : 0x89A, 0xF8, 0x18 );
    if ( D_801142B4_rscode == 2 ) {
        func_8002DF3C( 0xA12, x, y - 3 );
    }
    func_80038D0C( D_80114280_rscode, x + 0x2C, y );
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
            func_80038AE8( mate, D_8005C0E4[mate] >> 7, D_80068218[mate], 0, rowX, y );
            func_80038AE8( mate | 1, D_8005C0E4[mate + 1] >> 7, D_80068218[mate + 1], 0, rowX, y + 0x1C );
        } else {
            func_80038AE8( player, D_8005C0E4[player] >> 7, D_80068218[player], 0, rowX, y );
        }
        rowX += 0x2C;
        if ( D_801142B4_rscode == 1 ) {
            y += 0xE;
        }
        frame = D_80114280_rscode ? 9 : 0;
        end = D_80114280_rscode ? D_80066C00.unk_20[player] : 9;
        pageX = rowX;
        rowX += D_80114280_rscode ? 0x10 : 0;
        for ( ; frame < end; frame++ ) {
            func_801137D4_rscode( player, frame, rowX, y );
            rowX += 0x18;
        }
        if ( D_80114280_rscode ) {
            rowX = pageX;
        }
        func_8002E4B0( D_80114280_rscode ? 0x84C : 0x84B, rowX, y + 4, D_80114252_rscode + 0x40, 1.0f, 1.0f );
        y += D_801142B4_rscode == 1 ? 0x38 : 0x1C;
    }
}

/*
================
func_80113DC8_rscode

Draws two choices side by side, the chosen one in a box, the other faded.
================
*/
void func_80113DC8_rscode( u8 choice, u16 *images, s16 y, u16 w, u16 h ) {
    s16 x[2] = { 0x48, 0xA8 };

    func_8002C950( x[choice], y, w, h, 0xE0, 0xE0, 0xE0, D_80114252_rscode + 0x40 );
    func_8002DF3C( images[choice], x[choice], y );
    func_8002E4B0( images[!choice], x[!choice], y, 0x60, 1.0f, 1.0f );
}

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113EC0_rscode);

/*
================
func_80113FCC_rscode

Ranks the count scores, each between low and high, from the highest: ranks[i] is 1 plus the number of
higher scores. Bug (kept): the loops run to i == count, one entry past the end; func_801129CC_rscode's
call ranks a fifth, stale score and writes its rank over D_801142A6_rscode.
================
*/
void func_80113FCC_rscode( u8 *ranks, u8 count, s16 *scores, s16 low, s16 high ) {
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
func_801140EC_rscode

Ranks the count values, each between low and high, from the lowest: ranks[i] is 1 plus the number of
lower values. As func_80113FCC_rscode, the loops run one entry past the end. Nothing calls it.
================
*/
void func_801140EC_rscode( u8 *ranks, u8 count, s16 *values, s16 low, s16 high ) {
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
