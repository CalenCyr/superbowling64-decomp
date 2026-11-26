/* 5EC00.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* per character: scale of its model */
static f32 D_8011F8C0_gscode[8] = {
    1.0f, 1.0f, 1.0f, 1.2f, 1.2f, 0.7f, 1.1f, 1.0f,
};

/* per character: file of its model */
static s16 D_8011F8E0_gscode[8] = {
    0x462, 0x4E0, 0x6DC, 0x5BE, 0x13A, 0x2AA, 0x590, 0x238,
};

/* per character: file of its animation */
static s16 D_8011F8F0_gscode[8] = {
    0x7D8, 0x7DB, 0x7F9, 0x7F0, 0x7B3, 0x7C8, 0x7EA, 0x7C2,
};

/* the scene of the team select */
static objParams_t D_8011F900_gscode[] = {
    { 1.0f, 0.0f, { 2056.0f, 320.0f, 240.0f } },
    { 0.0f, 10.0f, { 568.0f, 2032.0f, -320.0f, -50.0f, 300.0f, 1.0f } },
    { 0.0f, 11.0f, { 568.0f, 2032.0f, -220.0f, -50.0f, 100.0f, 1.0f } },
    { 0.0f, 12.0f, { 568.0f, 2032.0f, 220.0f, -50.0f, 100.0f, 1.0f } },
    { 0.0f, 13.0f, { 568.0f, 2032.0f, 320.0f, -50.0f, 300.0f, 1.0f } },
    { -2.0f, 0.0f },
};

/* the team select: ids 0-7 the characters, 8-11 the players' names, 12-15 the titles, 16-19 the players' cursors;
   [5]: its title */
static menuItem_t D_8011FA80_gscode[28] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x8A6, 32, 12, 0xFF, SLIDE_LEFT },
    { -1, 0x8D2, 20, 48, 153, SLIDE_LEFT },
    { 12, 0x8A9, 196, 48, 153, SLIDE_RIGHT },
    { 13, 0x8D1, 40, 44, 153, SLIDE_LEFT },
    { 14, 0x8A8, 120, 112, 153, SLIDE_RIGHT },
    { 15, 0x8B7, 80, 164, 0xFF, SLIDE_BOTTOM },
    { 16, 0x8AD, 130, 52, 0xFF, SLIDE_BOTTOM },
    { 17, 0x8AC, 130, 68, 0xFF, SLIDE_BOTTOM },
    { 18, 0x8AB, 130, 122, 0xFF, SLIDE_BOTTOM },
    { 19, 0x8AA, 130, 138, 0xFF, SLIDE_BOTTOM },
    { 0, 0x8C2, 32, 188, 0xFF, SLIDE_BOTTOM },
    { 1, 0x8C5, 64, 188, 0xFF, SLIDE_BOTTOM },
    { 2, 0x8BE, 96, 188, 0xFF, SLIDE_BOTTOM },
    { 3, 0x8C3, 128, 188, 0xFF, SLIDE_BOTTOM },
    { 4, 0x8BF, 160, 188, 0xFF, SLIDE_BOTTOM },
    { 5, 0x8C4, 192, 188, 0xFF, SLIDE_BOTTOM },
    { 6, 0x8C1, 224, 188, 0xFF, SLIDE_BOTTOM },
    { 7, 0x8C0, 256, 188, 0xFF, SLIDE_BOTTOM },
    { 8, 0x8C9, 32, 180, 0xFF, SLIDE_BOTTOM },
    { 9, 0x8C8, 64, 180, 0xFF, SLIDE_BOTTOM },
    { 10, 0x8C7, 96, 188, 0xFF, SLIDE_BOTTOM },
    { 11, 0x8C6, 128, 188, 0xFF, SLIDE_BOTTOM },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* the players' character names: ids 24-27 and 28-31 */
static menuItem_t D_8011FB98_gscode[9] = {
    { 24, 0xA5D, 28, 132, 0xFF, SLIDE_NONE },
    { 28, 0xA68, 28, 140, 0xFF, SLIDE_NONE },
    { 25, 0xA5D, 28, 132, 0xFF, SLIDE_NONE },
    { 29, 0xA68, 28, 140, 0xFF, SLIDE_NONE },
    { 26, 0xA5D, 28, 132, 0xFF, SLIDE_NONE },
    { 30, 0xA68, 28, 140, 0xFF, SLIDE_NONE },
    { 27, 0xA5D, 28, 132, 0xFF, SLIDE_NONE },
    { 31, 0xA68, 28, 140, 0xFF, SLIDE_NONE },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per character: image of the second line of its name */
static s16 D_8011FBF4_gscode[8] = {
    0xA61, 0xA62, 0xA62, 0xA68, 0xA5F, 0xA5E, 0xA64, 0xA61,
};

/* per character: image of the first line of its name */
static s16 D_8011FC04_gscode[8] = {
    0xA5C, 0xA5D, 0xA5D, 0xA5C, 0xA5C, 0xA5C, 0xA5D, 0xA5C,
};

/* per character: its image once unlocked */
static s16 D_8011FC14_gscode[8] = {
    0x8C2, 0x8C5, 0x8BE, 0x8C3, 0x8BF, 0x8C4, 0x8C1, 0x8C0,
};

/* per character: x of a player's cursor on it */
static s16 D_8011FC24_gscode[8] = {
    146, 140, 149, 139, 139, 147, 140, 143,
};

/* per character: image of a player's cursor on it */
static s16 D_8011FC34_gscode[8] = {
    0x8B3, 0x8B6, 0x8AF, 0x8B4, 0x8B0, 0x8B5, 0x8B2, 0x8B1,
};

/* per bar of a character's figures: its colour */
static u8 D_8011FC44_gscode[3][3] = {
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
};

/* per character: height of its three bars, to 40 */
static u8 D_8011FC50_gscode[8][3] = {
    { 32, 28, 32 },
    { 28, 32, 32 },
    { 24, 40, 32 },
    { 20, 40, 16 },
    { 32, 28, 20 },
    { 40, 16, 40 },
    { 24, 24, 20 },
    { 28, 36, 28 },
};

/* per player: image of its name */
static s16 D_8011FC68_gscode[4] = {
    0x8C9, 0x8C8, 0x8C7, 0x8C6,
};

/* per player: image of its name, the computer playing */
static s16 D_8011FC70_gscode[4] = {
    0x8CD, 0x8CC, 0x8CB, 0x8CA,
};

/* per player: image of its name while it waits its turn */
static s16 D_8011FC78_gscode[4] = {
    0x8AD, 0x8AC, 0x8AB, 0x8AA,
};

static u8 D_8011FC80_gscode = 30; /* slide step of the team select */
static u8 D_8011FC81_gscode = 0; /* CHOICE_IN etc.; 4: the name entry */
static u8 D_8011FC82_gscode = 0; /* 0 choosing, 1 every team is done */

/* per team: its player choosing; bit 7: the team is done */
static u8 D_8011FC84_gscode[2] = {
    0, 0,
};

/*
================
GS_UnlockMask

Flag 4 adds 0x18, flag 8 adds 0x20 and flag 0x10 adds 0xC0; flag 4 is always set in the result.
================
*/
u8 GS_UnlockMask( u8 flags ) {
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
GS_TeamSelect

One frame of the team select: in turn, each team's players take a character with the D-pad and A or Start (B steps
back), while the characters' models and their three figure bars are drawn; the menu slides in and out around it.
Returns 1 once every team is done, 2 when backed out, else 0.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with the character cursor held in the loop counter j (marked R30
// below); with one local per meaning the s0/s1 order flips, and a reused counter is not counted as a match
s32 GS_TeamSelect( void ) {
    u8 slots[2][4];         /* per team: its players in turn order; 0xFF: none */
    u8 shown[4];            /* per player: has taken, or is taking, a character */
    u8 unlocked;            /* bit per character that can be taken */
    s32 i;
    s32 j;                  /* a team's player or bar; in the team loop, the character being chosen */
    u16 repeat;             /* D-pad presses, with auto-repeat */
    u16 pad;                /* buttons pressed this frame */
    s32 character;
    u8 player;

    unlocked = save_data.unlocks;
    memset( slots, 0xFF, sizeof( slots ) );

    if ( game_lastTeam == 0 ) {
        for ( i = 0; i < game_numPlayers; i++ ) {
            slots[0][i] = i;
        }
        Menu_SetItem( D_8011FA80_gscode, 12, 0x8CF, MENU_NONE, MENU_NONE, MENU_NONE );
        Menu_SetItem( D_8011FA80_gscode, 13, 0x8D0, 0x78, 0x30, MENU_NONE );
        Menu_SetItem( D_8011FA80_gscode, 14, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        Menu_SetItem( D_8011FA80_gscode, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        switch ( game_numPlayers ) {
        case 1:
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x5A, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 2:
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x46, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x6E, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 4:
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, 0x78, 0xFF );
            break;
        }
    } else {
        Menu_SetItem( D_8011FA80_gscode, 12, 0x8A9, MENU_NONE, MENU_NONE, MENU_NONE );
        Menu_SetItem( D_8011FA80_gscode, 13, 0x8D1, 0x28, 0x2C, MENU_NONE );
        Menu_SetItem( D_8011FA80_gscode, 14, 0x8A8, MENU_NONE, MENU_NONE, 0x99 );
        Menu_SetItem( D_8011FA80_gscode, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        switch ( game_numPlayers ) {
        case 2:
            if ( ( game_mode & 0xF ) != 1 ) {
                slots[0][0] = 0;
                if ( !( game_players[1] >> 7 ) ) {
                    slots[1][0] = 1;
                } else {
                    slots[0][1] = 1;
                }
            } else {
                slots[0][0] = 0;
                slots[0][1] = 1;
                if ( D_8011FC84_gscode[0] ) {
                    unlocked = GS_UnlockMask( save_data.unlocks );
                }
            }
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            slots[0][0] = 0;
            slots[0][1] = 1;
            if ( !( game_players[2] >> 7 ) ) {
                slots[1][0] = 2;
            } else {
                slots[0][2] = 2;
            }
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 4:
            slots[0][0] = 0;
            slots[0][1] = 1;
            if ( !( game_players[2] >> 7 ) || !( game_players[3] >> 7 ) ) {
                slots[1][0] = 2;
                slots[1][1] = 3;
            } else {
                slots[0][2] = 2;
                slots[0][3] = 3;
            }
            Menu_SetItem( D_8011FA80_gscode, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 18, MENU_NONE, MENU_NONE, 0x7A, 0xFF );
            Menu_SetItem( D_8011FA80_gscode, 19, MENU_NONE, MENU_NONE, 0x8A, 0xFF );
            break;
        }
    }

    switch ( game_numPlayers ) {
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
                    // RULE-EXCEPTION: R30, the character cursor shares j's register: retail allocates repeat and
                    // pad ahead of it, which a separate local reverses
                    j = game_players[player] & 0xF;
                    Menu_SetItem( D_8011FA80_gscode, player + 16, MENU_NONE, MENU_NONE, MENU_NONE, game_pulseAlpha );

                    if ( D_8011FC84_gscode[1] == 0x80 ) {
                        repeat = cont_allDpadRepeat;
                    } else {
                        repeat = cont_dpadRepeat[i];
                    }
                    if ( repeat & L_JPAD ) {
                        do {
                            j--;
                            if ( j < 0 ) {
                                j = 7;
                            }
                        } while ( !( ( unlocked >> j ) & 1 ) );
                        game_players[slots[i][D_8011FC84_gscode[i]]] =
                            ( game_players[slots[i][D_8011FC84_gscode[i]]] & 0xF0 ) | j;
                        Sound_PlaySe( SE_CURSOR, 0 );
                    }
                    if ( repeat & R_JPAD ) {
                        do {
                            j++;
                            if ( j == 8 ) {
                                j = 0;
                            }
                        } while ( !( ( unlocked >> j ) & 1 ) );
                        game_players[slots[i][D_8011FC84_gscode[i]]] =
                            ( game_players[slots[i][D_8011FC84_gscode[i]]] & 0xF0 ) | j;
                        Sound_PlaySe( SE_CURSOR, 0 );
                    }

                    if ( D_8011FC84_gscode[1] == 0x80 ) {
                        pad = cont_allPressed;
                    } else {
                        pad = cont_pressed[i];
                    }
                    if ( pad & ( A_BUTTON | Z_TRIG ) ) {
                        Sound_PlaySe( SE_DECIDE, 0 );
                        D_8011FC84_gscode[i]++;
                        if ( ( game_mode & 0xF ) == 1 && D_8011FC84_gscode[i] == 1 ) {
                            game_players[1] = ( game_players[1] & 0xF0 ) | 1;
                            unlocked = GS_UnlockMask( save_data.unlocks );
                            /* BUG: this reuses the team loop's counter, so the team loop goes on from the character
                               found (or ends) */
                            for ( i = 0; i < 8; i++ ) {
                                if ( ( unlocked ^ save_data.unlocks ) & ( 1 << i ) ) {
                                    game_players[1] = ( game_players[1] & 0xF0 ) | i;
                                    break;
                                }
                            }
                        }
                    }
                    if ( pad & B_BUTTON ) {
                        if ( game_replay ) {
                            if ( D_8011FC84_gscode[i] ) {
                                Sound_PlaySe( 2, 0 );
                                D_8011FC84_gscode[i]--;
                            }
                        } else {
                            Sound_PlaySe( 2, 0 );
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
                        pad = cont_allPressed;
                    } else {
                        pad = cont_pressed[i];
                    }
                    if ( ( pad & B_BUTTON ) && ( D_8011FC84_gscode[i] & 0x7F ) ) {
                        Sound_PlaySe( 2, 0 );
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
                if ( ( game_mode & 0xF ) == 1 ) {
                    if ( save_data.rumble ) {
                        GS_StartPakCheck();
                        D_8011FC81_gscode = 4;
                    } else {
                        game_leaveState = 1;
                        D_8011FC81_gscode = CHOICE_TAKEN;
                    }
                } else {
                    D_8011FC81_gscode = CHOICE_TAKEN;
                }
                for ( i = 0; i < game_numPlayers; i++ ) {
                    if ( ( game_players[i] >> 7 ) || !( ( game_players[i] >> 4 ) & 7 ) ) {
                        game_playerNames[i] = game_defaultNames[game_players[i] & 0xF];
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
        Menu_SetItem( D_8011FA80_gscode, i + 8, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < 8; i++ ) {
        Menu_SetItem( D_8011FB98_gscode, i + 24, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < game_numPlayers; i++ ) {
        if ( shown[i] ) {
            if ( !( game_players[i] >> 7 ) ) {
                Menu_SetItem( D_8011FA80_gscode, i + 8, D_8011FC68_gscode[i], ( game_players[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            } else {
                Menu_SetItem( D_8011FA80_gscode, i + 8, D_8011FC70_gscode[i], ( game_players[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            }
            Menu_SetItem( D_8011FA80_gscode, i + 16, D_8011FC34_gscode[game_players[i] & 0xF],
                           D_8011FC24_gscode[game_players[i] & 0xF], MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( D_8011FA80_gscode, i + 16, D_8011FC78_gscode[i], 0x82, MENU_NONE, MENU_NONE );
        }
    }
    for ( i = 0; i < 8; i++ ) {
        if ( ( unlocked >> i ) & 1 ) {
            Menu_SetItem( D_8011FA80_gscode, i, D_8011FC14_gscode[i], MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( D_8011FA80_gscode, i, 0x8BC, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_DrawItems( D_8011FA80_gscode, D_8011FC80_gscode );
    /* an empty loop: retail still counts it out */
    for ( i = 0; i < 4; i++ ) {
    }

    if ( D_8011FC81_gscode == CHOICE_RUN ) {
        for ( i = 0; i < 4; i++ ) {
            if ( i < game_numPlayers ) {
                character = game_players[i] & 0xF;
                if ( shown[i] ) {
                    ( (sceneObject_t *)obj_scene[10 + i] )->viewMask = 1;
                    if ( D_801207B4_gscode[i] != character ) {
                        D_801207B4_gscode[i] = character;
                        ( (sceneObject_t *)obj_scene[10 + i] )->frame = 0;
                        ( (sceneObject_t *)obj_scene[10 + i] )->model = D_801207C0_gscode[character];
                        ( (sceneObject_t *)obj_scene[10 + i] )->anim = D_801207E0_gscode[character];
                        if ( character == 6 ) {
                            D_80120800_gscode[i][1] += 40;
                        }
                        Mtx_SetRotScaleTrans( ( (sceneObject_t *)obj_scene[10 + i] )->mtx, 0.0f, 0.0f, 0.0f,
                                       D_8011F8C0_gscode[character], D_80120800_gscode[i][0], D_80120800_gscode[i][1],
                                       D_80120800_gscode[i][2] );
                    }
                } else {
                    ( (sceneObject_t *)obj_scene[10 + i] )->viewMask = 0;
                }
            } else {
                ( (sceneObject_t *)obj_scene[10 + i] )->viewMask = 0;
            }
        }
        Obj_Frame();
        for ( i = 0; i < game_numPlayers; i++ ) {
            if ( shown[i] ) {
                character = game_players[i] & 0xF;
                for ( j = 0; j < 3; j++ ) {
                    if ( D_80120818_gscode[i][j] < D_8011FC50_gscode[character][j] ) {
                        D_80120818_gscode[i][j]++;
                    }
                    if ( D_80120818_gscode[i][j] > D_8011FC50_gscode[character][j] ) {
                        D_80120818_gscode[i][j]--;
                    }
                    Draw_FillRect( D_80120828_gscode[i][j], D_80120840_gscode[i][j], 3, 40 - D_80120818_gscode[i][j],
                                   0xB0, 0xB0, 0xB0, 0xC0 );
                    Draw_FillRect( D_80120828_gscode[i][j], D_80120840_gscode[i][j] - ( D_80120818_gscode[i][j] - 40 ),
                                   3, D_80120818_gscode[i][j], D_8011FC44_gscode[j][0], D_8011FC44_gscode[j][1],
                                   D_8011FC44_gscode[j][2], 0xC0 );
                }
                Menu_SetItem( D_8011FB98_gscode, i + 24, D_8011FC04_gscode[character], D_80120828_gscode[i][0] - 16,
                               D_80120840_gscode[i][0] + 18, game_pulseAlpha );
                Menu_SetItem( D_8011FB98_gscode, i + 28, D_8011FBF4_gscode[character], D_80120828_gscode[i][0] - 16,
                               D_80120840_gscode[i][0] + 26, game_pulseAlpha );
            }
        }
    }

    Menu_DrawItems( D_8011FB98_gscode, D_8011FC80_gscode );

    switch ( D_8011FC81_gscode ) {
    case CHOICE_IN:
        D_8011FA80_gscode[5].slide = SLIDE_LEFT;
        D_8011FC80_gscode -= 2;
        if ( D_8011FC80_gscode == 0 ) {
            D_8011FC81_gscode = CHOICE_RUN;
            Game_StartPulse();
            Obj_CreateAll( gs_classes, D_8011F900_gscode );
            cam_cameras[0]->path->flags |= 4;
            for ( i = 0; i < 4; i++ ) {
                D_801207B4_gscode[i] = 0xFFFF;
                for ( j = 0; j < 3; j++ ) {
                    D_80120818_gscode[i][j] = 0;
                }
            }
        } else if ( D_8011FC80_gscode == 28 ) {
            for ( i = 0; i < 8; i++ ) {
                D_801207C0_gscode[i] = Cache_GetFile( D_8011F8E0_gscode[i], FILE_TYPE1 );
                D_801207E0_gscode[i] = Cache_GetFile( D_8011F8F0_gscode[i], 2 );
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011FA80_gscode[5].slide = SLIDE_RIGHT;
        D_8011FC80_gscode += 2;
        if ( D_8011FC80_gscode == 30 ) {
            if ( game_leaveState == 1 ) {
                if ( game_fade != 0xFF ) {
                    D_8011FC80_gscode = 28;
                    return 0;
                }
                game_leaveState = 2;
                game_fade = 0x107;
            }
            for ( i = 0; i < 4; i++ ) {
                shown[i] = 0;
            }
            Obj_FreeAll();
            D_8011FC82_gscode = 0;
            Cache_FreeBelow( 0x825 );
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
        if ( GS_PakCheck() ) {
            game_leaveState = 1;
            D_8011FC81_gscode = CHOICE_TAKEN;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/5EC00", GS_TeamSelect);
#endif
