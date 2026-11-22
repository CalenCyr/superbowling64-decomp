/* 5EC00.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80118010_gscode

Flag 4 adds 0x18, flag 8 adds 0x20 and flag 0x10 adds 0xC0; flag 4 is always set in the result.
================
*/
u8 func_80118010_gscode( u8 flags ) {
    u8 result;

    result = flags | 4;
    if ( flags & 4 ) {
        result = flags | 0x1C;
    }
    if ( flags & 8 ) {
        result |= 0x20;
    }
    if ( flags & 0x10 ) {
        result |= 0xC0;
    }
    return result;
}

/*
================
func_80118040_gscode

One frame of the team select: in turn, each team's players take a character with the D-pad and A or Start (B steps
back), while the characters' models and their three figure bars are drawn; the menu slides in and out around it.
Returns 1 once every team is done, 2 when backed out, else 0.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: complete, same instructions and branches; registers only: retail keeps `character` in s1 and `pad`
// in s0 in the team loop and `character` in s4 in the model and bar loops (the draft: s0, s1, s0), and allocates the
// model loop's pointers s0/s1/s3 differently; relocation addends to D_8005C0E5-7, D_8005C0D4, D_8011FABB are splat's
// names for D_8005C0E4[] and D_8011FA80_gscode[5].slide. Permuter (target-only scoring, 5 min at -O3):
// nothing below the base score 1355.
s32 func_80118040_gscode( void ) {
    u8 slots[2][4];         /* per team: its players in turn order; 0xFF: none */
    u8 shown[4];            /* per player: has taken, or is taking, a character */
    u8 unlocked;            /* bit per character that can be taken */
    s32 i;
    s32 j;
    u16 pad;
    s32 character;
    u8 player;

    unlocked = D_8009D098.unk_04;
    memset( slots, 0xFF, sizeof( slots ) );

    if ( D_8005C162 == 0 ) {
        for ( i = 0; i < D_8005C161; i++ ) {
            slots[0][i] = i;
        }
        func_8003893C( D_8011FA80_gscode, 12, 0x8CF, MENU_NONE, MENU_NONE, MENU_NONE );
        func_8003893C( D_8011FA80_gscode, 13, 0x8D0, 0x78, 0x30, MENU_NONE );
        func_8003893C( D_8011FA80_gscode, 14, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        func_8003893C( D_8011FA80_gscode, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        switch ( D_8005C161 ) {
        case 1:
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x5A, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 2:
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x46, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x6E, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 4:
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, 0x78, 0xFF );
            break;
        }
    } else {
        func_8003893C( D_8011FA80_gscode, 12, 0x8A9, MENU_NONE, MENU_NONE, MENU_NONE );
        func_8003893C( D_8011FA80_gscode, 13, 0x8D1, 0x28, 0x2C, MENU_NONE );
        func_8003893C( D_8011FA80_gscode, 14, 0x8A8, MENU_NONE, MENU_NONE, 0x99 );
        func_8003893C( D_8011FA80_gscode, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        switch ( D_8005C161 ) {
        case 2:
            if ( ( D_8005C160 & 0xF ) != 1 ) {
                slots[0][0] = 0;
                if ( !( D_8005C0E4[1] >> 7 ) ) {
                    slots[1][0] = 1;
                } else {
                    slots[0][1] = 1;
                }
            } else {
                slots[0][0] = 0;
                slots[0][1] = 1;
                if ( D_8011FC84_gscode[0] ) {
                    unlocked = func_80118010_gscode( D_8009D098.unk_04 );
                }
            }
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            slots[0][0] = 0;
            slots[0][1] = 1;
            if ( !( D_8005C0E4[2] >> 7 ) ) {
                slots[1][0] = 2;
            } else {
                slots[0][2] = 2;
            }
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 4:
            slots[0][0] = 0;
            slots[0][1] = 1;
            if ( !( D_8005C0E4[2] >> 7 ) || !( D_8005C0E4[3] >> 7 ) ) {
                slots[1][0] = 2;
                slots[1][1] = 3;
            } else {
                slots[0][2] = 2;
                slots[0][3] = 3;
            }
            func_8003893C( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            func_8003893C( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            func_8003893C( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x7A, 0xFF );
            func_8003893C( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, 0x8A, 0xFF );
            break;
        }
    }

    switch ( D_8005C161 ) {
    case 1:
        D_80120828_gscode[0][0] = 0x38;
        D_80120840_gscode[0][0] = 0x7A;
        D_80120828_gscode[0][1] = 0x40;
        D_80120840_gscode[0][1] = 0x77;
        D_80120828_gscode[0][2] = 0x48;
        D_80120840_gscode[0][2] = 0x74;
        D_80120800_gscode[0][0] = -290;
        D_80120800_gscode[0][1] = -50;
        D_80120800_gscode[0][2] = 200;
        break;
    case 2:
        D_80120828_gscode[0][0] = 0x38;
        D_80120840_gscode[0][0] = 0x7A;
        D_80120828_gscode[0][1] = 0x40;
        D_80120840_gscode[0][1] = 0x77;
        D_80120828_gscode[0][2] = 0x48;
        D_80120840_gscode[0][2] = 0x74;
        D_80120828_gscode[1][0] = 0xF0;
        D_80120840_gscode[1][0] = 0x74;
        D_80120828_gscode[1][1] = 0xF8;
        D_80120840_gscode[1][1] = 0x77;
        D_80120828_gscode[1][2] = 0x100;
        D_80120840_gscode[1][2] = 0x7A;
        D_80120800_gscode[0][0] = -290;
        D_80120800_gscode[0][1] = -50;
        D_80120800_gscode[0][2] = 200;
        D_80120800_gscode[1][0] = 290;
        D_80120800_gscode[1][1] = -50;
        D_80120800_gscode[1][2] = 200;
        break;
    case 3:
        D_80120828_gscode[0][0] = 0x20;
        D_80120840_gscode[0][0] = 0x82;
        D_80120828_gscode[0][1] = 0x28;
        D_80120840_gscode[0][1] = 0x7F;
        D_80120828_gscode[0][2] = 0x30;
        D_80120840_gscode[0][2] = 0x7C;
        D_80120828_gscode[1][0] = 0x50;
        D_80120840_gscode[1][0] = 0x72;
        D_80120828_gscode[1][1] = 0x58;
        D_80120840_gscode[1][1] = 0x6F;
        D_80120828_gscode[1][2] = 0x60;
        D_80120840_gscode[1][2] = 0x6C;
        D_80120828_gscode[2][0] = 0xF0;
        D_80120840_gscode[2][0] = 0x74;
        D_80120828_gscode[2][1] = 0xF8;
        D_80120840_gscode[2][1] = 0x77;
        D_80120828_gscode[2][2] = 0x100;
        D_80120840_gscode[2][2] = 0x7A;
        D_80120800_gscode[0][0] = -320;
        D_80120800_gscode[0][1] = -50;
        D_80120800_gscode[0][2] = 300;
        D_80120800_gscode[1][0] = -220;
        D_80120800_gscode[1][1] = -50;
        D_80120800_gscode[1][2] = 100;
        D_80120800_gscode[2][0] = 290;
        D_80120800_gscode[2][1] = -50;
        D_80120800_gscode[2][2] = 200;
        break;
    case 4:
        D_80120828_gscode[0][0] = 0x20;
        D_80120840_gscode[0][0] = 0x82;
        D_80120828_gscode[0][1] = 0x28;
        D_80120840_gscode[0][1] = 0x7F;
        D_80120828_gscode[0][2] = 0x30;
        D_80120840_gscode[0][2] = 0x7C;
        D_80120828_gscode[1][0] = 0x50;
        D_80120840_gscode[1][0] = 0x72;
        D_80120828_gscode[1][1] = 0x58;
        D_80120840_gscode[1][1] = 0x6F;
        D_80120828_gscode[1][2] = 0x60;
        D_80120840_gscode[1][2] = 0x6C;
        D_80120828_gscode[2][0] = 0xD8;
        D_80120840_gscode[2][0] = 0x6C;
        D_80120828_gscode[2][1] = 0xE0;
        D_80120840_gscode[2][1] = 0x6F;
        D_80120828_gscode[2][2] = 0xE8;
        D_80120840_gscode[2][2] = 0x72;
        D_80120828_gscode[3][0] = 0x108;
        D_80120840_gscode[3][0] = 0x7C;
        D_80120828_gscode[3][1] = 0x110;
        D_80120840_gscode[3][1] = 0x7F;
        D_80120828_gscode[3][2] = 0x118;
        D_80120840_gscode[3][2] = 0x82;
        D_80120800_gscode[0][0] = -320;
        D_80120800_gscode[0][1] = -50;
        D_80120800_gscode[0][2] = 300;
        D_80120800_gscode[1][0] = -220;
        D_80120800_gscode[1][1] = -50;
        D_80120800_gscode[1][2] = 100;
        D_80120800_gscode[2][0] = 220;
        D_80120800_gscode[2][1] = -50;
        D_80120800_gscode[2][2] = 100;
        D_80120800_gscode[3][0] = 320;
        D_80120800_gscode[3][1] = -50;
        D_80120800_gscode[3][2] = 300;
        break;
    }

    if ( D_8011FC81_gscode == CHOICE_RUN ) {
        switch ( D_8011FC82_gscode ) {
        case 0:
            for ( i = 0; i < 2; i++ ) {
                if ( !( D_8011FC84_gscode[i] & 0x80 ) ) {
                    player = slots[i][D_8011FC84_gscode[i]];
                    if ( player != 0xFF ) {
                    character = D_8005C0E4[player] & 0xF;
                    func_8003893C( D_8011FA80_gscode, player + 16, MENU_NONE, MENU_NONE, MENU_NONE, D_8009CFB0 );

                    if ( D_8011FC84_gscode[1] == 0x80 ) {
                        pad = D_800E6758;
                    } else {
                        pad = D_800E6760[i];
                    }
                    if ( pad & L_JPAD ) {
                        do {
                            character--;
                            if ( character < 0 ) {
                                character = 7;
                            }
                        } while ( !( ( unlocked >> character ) & 1 ) );
                        D_8005C0E4[slots[i][D_8011FC84_gscode[i]]] =
                            ( D_8005C0E4[slots[i][D_8011FC84_gscode[i]]] & 0xF0 ) | character;
                        func_80037870( SE_CURSOR, 0 );
                    }
                    if ( pad & R_JPAD ) {
                        do {
                            character++;
                            if ( character == 8 ) {
                                character = 0;
                            }
                        } while ( !( ( unlocked >> character ) & 1 ) );
                        D_8005C0E4[slots[i][D_8011FC84_gscode[i]]] =
                            ( D_8005C0E4[slots[i][D_8011FC84_gscode[i]]] & 0xF0 ) | character;
                        func_80037870( SE_CURSOR, 0 );
                    }

                    if ( D_8011FC84_gscode[1] == 0x80 ) {
                        pad = D_800E4D62;
                    } else {
                        pad = D_800E44E0[i];
                    }
                    if ( pad & ( A_BUTTON | Z_TRIG ) ) {
                        func_80037870( SE_DECIDE, 0 );
                        D_8011FC84_gscode[i]++;
                        if ( ( D_8005C160 & 0xF ) == 1 && D_8011FC84_gscode[i] == 1 ) {
                            D_8005C0E4[1] = ( D_8005C0E4[1] & 0xF0 ) | 1;
                            unlocked = func_80118010_gscode( D_8009D098.unk_04 );
                            /* BUG: this reuses the team loop's counter, so the team loop goes on from the character
                               found (or ends) */
                            for ( i = 0; i < 8; i++ ) {
                                if ( ( unlocked ^ D_8009D098.unk_04 ) & ( 1 << i ) ) {
                                    D_8005C0E4[1] = ( D_8005C0E4[1] & 0xF0 ) | i;
                                    break;
                                }
                            }
                        }
                    }
                    if ( pad & B_BUTTON ) {
                        if ( D_8005CB60 ) {
                            if ( D_8011FC84_gscode[i] ) {
                                func_80037870( 2, 0 );
                                D_8011FC84_gscode[i]--;
                            }
                        } else {
                            func_80037870( 2, 0 );
                            if ( D_8011FC84_gscode[i] == 0 ) {
                                D_8011FC81_gscode = CHOICE_BACK;
                                break;
                            }
                            D_8011FC84_gscode[i]--;
                        }
                    }
                    } else {
                        D_8011FC84_gscode[i] |= 0x80;
                    }
                } else {
                    if ( D_8011FC84_gscode[1] == 0x80 ) {
                        pad = D_800E4D62;
                    } else {
                        pad = D_800E44E0[i];
                    }
                    if ( ( pad & B_BUTTON ) && ( D_8011FC84_gscode[i] & 0x7F ) ) {
                        func_80037870( 2, 0 );
                        D_8011FC84_gscode[i] = ( D_8011FC84_gscode[i] & 0x7F ) - 1;
                    }
                }
            }
            if ( ( D_8011FC84_gscode[0] & 0x80 ) && ( D_8011FC84_gscode[1] & 0x80 ) ) {
                D_8011FC82_gscode = 1;
                D_801207B2_gscode = 5;
            }
            break;
        case 1:
            if ( --D_801207B2_gscode == 0 ) {
                if ( ( D_8005C160 & 0xF ) == 1 ) {
                    if ( D_8009D098.unk_7B ) {
                        func_8011B0B0_gscode();
                        D_8011FC81_gscode = 4;
                    } else {
                        D_8005C0BF = 1;
                        D_8011FC81_gscode = CHOICE_TAKEN;
                    }
                } else {
                    D_8011FC81_gscode = CHOICE_TAKEN;
                }
                for ( i = 0; i < D_8005C161; i++ ) {
                    if ( ( D_8005C0E4[i] >> 7 ) || !( ( D_8005C0E4[i] >> 4 ) & 7 ) ) {
                        D_80068218[i] = D_8005C0F8[D_8005C0E4[i] & 0xF];
                    }
                }
            }
            break;
        }
    }

    memset( shown, 0, sizeof( shown ) );
    for ( i = 0; i < 2; i++ ) {
        for ( j = 0; j <= ( D_8011FC84_gscode[i] & 0x7F ); j++ ) {
            if ( slots[i][j] != 0xFF ) {
                shown[slots[i][j]] = 1;
            }
        }
    }

    for ( i = 0; i < 4; i++ ) {
        func_8003893C( D_8011FA80_gscode, i + 8, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < 8; i++ ) {
        func_8003893C( D_8011FB98_gscode, i + 24, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < D_8005C161; i++ ) {
        if ( shown[i] ) {
            if ( !( D_8005C0E4[i] >> 7 ) ) {
                func_8003893C( D_8011FA80_gscode, i + 8, D_8011FC68_gscode[i], ( D_8005C0E4[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            } else {
                func_8003893C( D_8011FA80_gscode, i + 8, D_8011FC70_gscode[i], ( D_8005C0E4[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            }
            func_8003893C( D_8011FA80_gscode, i + 16, D_8011FC34_gscode[D_8005C0E4[i] & 0xF],
                           D_8011FC24_gscode[D_8005C0E4[i] & 0xF], MENU_NONE, MENU_NONE );
        } else {
            func_8003893C( D_8011FA80_gscode, i + 16, D_8011FC78_gscode[i], 0x82, MENU_NONE, MENU_NONE );
        }
    }
    for ( i = 0; i < 8; i++ ) {
        if ( ( unlocked >> i ) & 1 ) {
            func_8003893C( D_8011FA80_gscode, i, D_8011FC14_gscode[i], MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            func_8003893C( D_8011FA80_gscode, i, 0x8BC, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }
    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_800386E0( D_8011FA80_gscode, D_8011FC80_gscode );
    for ( i = 0; i < 4; i++ ) {
    }

    if ( D_8011FC81_gscode == CHOICE_RUN ) {
        for ( i = 0; i < 4; i++ ) {
            if ( i < D_8005C161 && shown[i] ) {
                character = D_8005C0E4[i] & 0xF;
                ( (sceneObject_t *)D_800EA120[10 + i] )->viewMask = 1;
                if ( D_801207B4_gscode[i] != character ) {
                    D_801207B4_gscode[i] = character;
                    ( (sceneObject_t *)D_800EA120[10 + i] )->frame = 0;
                    ( (sceneObject_t *)D_800EA120[10 + i] )->model = D_801207C0_gscode[character];
                    ( (sceneObject_t *)D_800EA120[10 + i] )->anim = D_801207E0_gscode[character];
                    if ( character == 6 ) {
                        D_80120800_gscode[i][1] += 40;
                    }
                    func_8002AAD0( ( (sceneObject_t *)D_800EA120[10 + i] )->mtx, 0.0f, 0.0f, 0.0f,
                                   D_8011F8C0_gscode[character], D_80120800_gscode[i][0], D_80120800_gscode[i][1],
                                   D_80120800_gscode[i][2] );
                }
            } else {
                ( (sceneObject_t *)D_800EA120[10 + i] )->viewMask = 0;
            }
        }
        func_8002A2F0();
        for ( i = 0; i < D_8005C161; i++ ) {
            if ( shown[i] ) {
                character = D_8005C0E4[i] & 0xF;
                for ( j = 0; j < 3; j++ ) {
                    if ( D_80120818_gscode[i][j] < D_8011FC50_gscode[character][j] ) {
                        D_80120818_gscode[i][j]++;
                    }
                    if ( D_80120818_gscode[i][j] > D_8011FC50_gscode[character][j] ) {
                        D_80120818_gscode[i][j]--;
                    }
                    func_8002FDE4( D_80120828_gscode[i][j], D_80120840_gscode[i][j], 3, 40 - D_80120818_gscode[i][j],
                                   0xB0, 0xB0, 0xB0, 0xC0 );
                    func_8002FDE4( D_80120828_gscode[i][j], D_80120840_gscode[i][j] - ( D_80120818_gscode[i][j] - 40 ),
                                   3, D_80120818_gscode[i][j], D_8011FC44_gscode[j][0], D_8011FC44_gscode[j][1],
                                   D_8011FC44_gscode[j][2], 0xC0 );
                }
                func_8003893C( D_8011FB98_gscode, i + 24, D_8011FC04_gscode[character], D_80120828_gscode[i][0] - 16,
                               D_80120840_gscode[i][0] + 18, D_8009CFB0 );
                func_8003893C( D_8011FB98_gscode, i + 28, D_8011FBF4_gscode[character], D_80120828_gscode[i][0] - 16,
                               D_80120840_gscode[i][0] + 26, D_8009CFB0 );
            }
        }
    }

    func_800386E0( D_8011FB98_gscode, D_8011FC80_gscode );

    switch ( D_8011FC81_gscode ) {
    case CHOICE_IN:
        D_8011FA80_gscode[5].slide = SLIDE_LEFT;
        D_8011FC80_gscode -= 2;
        if ( D_8011FC80_gscode == 0 ) {
            D_8011FC81_gscode = CHOICE_RUN;
            func_80026B28();
            func_8002A1EC( D_8011B540_gscode, D_8011F900_gscode );
            D_8009CFA0[0]->path->unk_38 |= 4;
            for ( i = 0; i < 4; i++ ) {
                D_801207B4_gscode[i] = 0xFFFF;
                for ( j = 0; j < 3; j++ ) {
                    D_80120818_gscode[i][j] = 0;
                }
            }
        } else if ( D_8011FC80_gscode == 28 ) {
            for ( i = 0; i < 8; i++ ) {
                D_801207C0_gscode[i] = func_80030C04( D_8011F8E0_gscode[i], FILE_TYPE1 );
                D_801207E0_gscode[i] = func_80030C04( D_8011F8F0_gscode[i], 2 );
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011FA80_gscode[5].slide = SLIDE_RIGHT;
        D_8011FC80_gscode += 2;
        if ( D_8011FC80_gscode == 30 ) {
            if ( D_8005C0BF == 1 ) {
                if ( D_8005C0C0 != 0xFF ) {
                    D_8011FC80_gscode = 28;
                    return 0;
                }
                D_8005C0BF = 2;
                D_8005C0C0 = 0x107;
            }
            for ( i = 0; i < 4; i++ ) {
                shown[i] = 0;
            }
            func_8002A67C();
            D_8011FC82_gscode = 0;
            func_800311D0( 0x825 );
            D_8011FC84_gscode[0] = D_8011FC84_gscode[1] = 0;
            if ( D_8011FC81_gscode == CHOICE_TAKEN ) {
                D_8011FC81_gscode = CHOICE_IN;
                return 1;
            }
            D_8011FC81_gscode = CHOICE_IN;
            return 2;
        }
        break;
    case 4:
        if ( func_8011B214_gscode() ) {
            D_8005C0BF = 1;
            D_8011FC81_gscode = CHOICE_TAKEN;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/5EC00", func_80118040_gscode);
#endif
