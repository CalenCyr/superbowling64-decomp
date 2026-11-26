/* 5EC00.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* per character: scale of its model */
static f32 gs_teamModelScales[8] = {
    1.0f, 1.0f, 1.0f, 1.2f, 1.2f, 0.7f, 1.1f, 1.0f,
};

/* per character: file of its model */
static s16 gs_teamModelFiles[8] = {
    0x462, 0x4E0, 0x6DC, 0x5BE, 0x13A, 0x2AA, 0x590, 0x238,
};

/* per character: file of its animation */
static s16 gs_teamAnimFiles[8] = {
    0x7D8, 0x7DB, 0x7F9, 0x7F0, 0x7B3, 0x7C8, 0x7EA, 0x7C2,
};

/* the scene of the team select */
static objParams_t gs_teamScene[] = {
    { 1.0f, 0.0f, { 2056.0f, 320.0f, 240.0f } },
    { 0.0f, 10.0f, { 568.0f, 2032.0f, -320.0f, -50.0f, 300.0f, 1.0f } },
    { 0.0f, 11.0f, { 568.0f, 2032.0f, -220.0f, -50.0f, 100.0f, 1.0f } },
    { 0.0f, 12.0f, { 568.0f, 2032.0f, 220.0f, -50.0f, 100.0f, 1.0f } },
    { 0.0f, 13.0f, { 568.0f, 2032.0f, 320.0f, -50.0f, 300.0f, 1.0f } },
    { -2.0f, 0.0f },
};

/* the team select: ids 0-7 the characters, 8-11 the players' names, 12-15 the titles, 16-19 the players' cursors;
   [5]: its title */
static menuItem_t gs_teamItems[28] = {
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
static menuItem_t gs_teamNameItems[9] = {
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
static s16 gs_nameLine2Images[8] = {
    0xA61, 0xA62, 0xA62, 0xA68, 0xA5F, 0xA5E, 0xA64, 0xA61,
};

/* per character: image of the first line of its name */
static s16 gs_nameLine1Images[8] = {
    0xA5C, 0xA5D, 0xA5D, 0xA5C, 0xA5C, 0xA5C, 0xA5D, 0xA5C,
};

/* per character: its image once unlocked */
static s16 gs_teamPortraits[8] = {
    0x8C2, 0x8C5, 0x8BE, 0x8C3, 0x8BF, 0x8C4, 0x8C1, 0x8C0,
};

/* per character: x of a player's cursor on it */
static s16 gs_cursorX[8] = {
    146, 140, 149, 139, 139, 147, 140, 143,
};

/* per character: image of a player's cursor on it */
static s16 gs_cursorImages[8] = {
    0x8B3, 0x8B6, 0x8AF, 0x8B4, 0x8B0, 0x8B5, 0x8B2, 0x8B1,
};

/* per bar of a character's figures: its colour */
static u8 gs_barColors[3][3] = {
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
};

/* per character: height of its three bars, to 40 */
static u8 gs_barHeights[8][3] = {
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
static s16 gs_playerNameImages[4] = {
    0x8C9, 0x8C8, 0x8C7, 0x8C6,
};

/* per player: image of its name, the computer playing */
static s16 gs_cpuNameImages[4] = {
    0x8CD, 0x8CC, 0x8CB, 0x8CA,
};

/* per player: image of its name while it waits its turn */
static s16 gs_waitNameImages[4] = {
    0x8AD, 0x8AC, 0x8AB, 0x8AA,
};

static u8 gs_teamSlideStep = 30; /* slide step of the team select */
static u8 gs_teamChoice = 0; /* CHOICE_IN etc.; 4: the name entry */
static u8 gs_teamPhase = 0; /* 0 choosing, 1 every team is done */

/* per team: its player choosing; bit 7: the team is done */
static u8 gs_teamPlayer[2] = {
    0, 0,
};

/* ===== State ===== */

static u8 D_801207A0_gscode[18];          /* unreferenced; it may instead end 5AD50's or 5E2B0's bss: nothing tells */
static u8 gs_leaveDelay;              /* frames before the team select leaves, once every team is done */
static u16 gs_shownModels[4];          /* per player: character whose model is shown; 0xFFFF: none */
static fileType1_t *gs_teamModels[8]; /* per character: its loaded model */
static fileType2_t *gs_teamAnims[8]; /* per character: its loaded animation */
static s16 gs_modelPos[4][3];       /* per player: position of its model */
static u8 gs_barLevels[4][3];        /* per player: heights of its bars, growing to the character's */
static s16 gs_barX[4][3];       /* per player: x of its bars */
static s16 gs_barY[4][3];       /* per player: y of the bottom of its bars */

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
        Menu_SetItem( gs_teamItems, 12, 0x8CF, MENU_NONE, MENU_NONE, MENU_NONE );
        Menu_SetItem( gs_teamItems, 13, 0x8D0, 0x78, 0x30, MENU_NONE );
        Menu_SetItem( gs_teamItems, 14, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        Menu_SetItem( gs_teamItems, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
        switch ( game_numPlayers ) {
        case 1:
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x5A, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 2:
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x46, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x6E, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 4:
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x50, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, 0x64, 0xFF );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, 0x78, 0xFF );
            break;
        }
    } else {
        Menu_SetItem( gs_teamItems, 12, 0x8A9, MENU_NONE, MENU_NONE, MENU_NONE );
        Menu_SetItem( gs_teamItems, 13, 0x8D1, 0x28, 0x2C, MENU_NONE );
        Menu_SetItem( gs_teamItems, 14, 0x8A8, MENU_NONE, MENU_NONE, 0x99 );
        Menu_SetItem( gs_teamItems, 15, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
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
                if ( gs_teamPlayer[0] ) {
                    unlocked = GS_UnlockMask( save_data.unlocks );
                }
            }
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x3C, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
            break;
        case 3:
            slots[0][0] = 0;
            slots[0][1] = 1;
            if ( !( game_players[2] >> 7 ) ) {
                slots[1][0] = 2;
            } else {
                slots[0][2] = 2;
            }
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, 0x82, 0xFF );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
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
            Menu_SetItem( gs_teamItems, 16, MENU_NONE, MENU_NONE, 0x34, 0xFF );
            Menu_SetItem( gs_teamItems, 17, MENU_NONE, MENU_NONE, 0x44, 0xFF );
            Menu_SetItem( gs_teamItems, 18, MENU_NONE, MENU_NONE, 0x7A, 0xFF );
            Menu_SetItem( gs_teamItems, 19, MENU_NONE, MENU_NONE, 0x8A, 0xFF );
            break;
        }
    }

    switch ( game_numPlayers ) {
    case 1:
        gs_barX[0][0] = 0x38;
        gs_barY[0][0] = 0x7A;
        gs_barX[0][1] = 0x40;
        gs_barY[0][1] = 0x77;
        gs_barX[0][2] = 0x48;
        gs_barY[0][2] = 0x74;
        gs_modelPos[0][0] = -290;
        gs_modelPos[0][1] = -50;
        gs_modelPos[0][2] = 200;
        break;
    case 2:
        gs_barX[0][0] = 0x38;
        gs_barY[0][0] = 0x7A;
        gs_barX[0][1] = 0x40;
        gs_barY[0][1] = 0x77;
        gs_barX[0][2] = 0x48;
        gs_barY[0][2] = 0x74;
        gs_barX[1][0] = 0xF0;
        gs_barY[1][0] = 0x74;
        gs_barX[1][1] = 0xF8;
        gs_barY[1][1] = 0x77;
        gs_barX[1][2] = 0x100;
        gs_barY[1][2] = 0x7A;
        gs_modelPos[0][0] = -290;
        gs_modelPos[0][1] = -50;
        gs_modelPos[0][2] = 200;
        gs_modelPos[1][0] = 290;
        gs_modelPos[1][1] = -50;
        gs_modelPos[1][2] = 200;
        break;
    case 3:
        gs_barX[0][0] = 0x20;
        gs_barY[0][0] = 0x82;
        gs_barX[0][1] = 0x28;
        gs_barY[0][1] = 0x7F;
        gs_barX[0][2] = 0x30;
        gs_barY[0][2] = 0x7C;
        gs_barX[1][0] = 0x50;
        gs_barY[1][0] = 0x72;
        gs_barX[1][1] = 0x58;
        gs_barY[1][1] = 0x6F;
        gs_barX[1][2] = 0x60;
        gs_barY[1][2] = 0x6C;
        gs_barX[2][0] = 0xF0;
        gs_barY[2][0] = 0x74;
        gs_barX[2][1] = 0xF8;
        gs_barY[2][1] = 0x77;
        gs_barX[2][2] = 0x100;
        gs_barY[2][2] = 0x7A;
        gs_modelPos[0][0] = -320;
        gs_modelPos[0][1] = -50;
        gs_modelPos[0][2] = 300;
        gs_modelPos[1][0] = -220;
        gs_modelPos[1][1] = -50;
        gs_modelPos[1][2] = 100;
        gs_modelPos[2][0] = 290;
        gs_modelPos[2][1] = -50;
        gs_modelPos[2][2] = 200;
        break;
    case 4:
        gs_barX[0][0] = 0x20;
        gs_barY[0][0] = 0x82;
        gs_barX[0][1] = 0x28;
        gs_barY[0][1] = 0x7F;
        gs_barX[0][2] = 0x30;
        gs_barY[0][2] = 0x7C;
        gs_barX[1][0] = 0x50;
        gs_barY[1][0] = 0x72;
        gs_barX[1][1] = 0x58;
        gs_barY[1][1] = 0x6F;
        gs_barX[1][2] = 0x60;
        gs_barY[1][2] = 0x6C;
        gs_barX[2][0] = 0xD8;
        gs_barY[2][0] = 0x6C;
        gs_barX[2][1] = 0xE0;
        gs_barY[2][1] = 0x6F;
        gs_barX[2][2] = 0xE8;
        gs_barY[2][2] = 0x72;
        gs_barX[3][0] = 0x108;
        gs_barY[3][0] = 0x7C;
        gs_barX[3][1] = 0x110;
        gs_barY[3][1] = 0x7F;
        gs_barX[3][2] = 0x118;
        gs_barY[3][2] = 0x82;
        gs_modelPos[0][0] = -320;
        gs_modelPos[0][1] = -50;
        gs_modelPos[0][2] = 300;
        gs_modelPos[1][0] = -220;
        gs_modelPos[1][1] = -50;
        gs_modelPos[1][2] = 100;
        gs_modelPos[2][0] = 220;
        gs_modelPos[2][1] = -50;
        gs_modelPos[2][2] = 100;
        gs_modelPos[3][0] = 320;
        gs_modelPos[3][1] = -50;
        gs_modelPos[3][2] = 300;
        break;
    }

    if ( gs_teamChoice == CHOICE_RUN ) {
        switch ( gs_teamPhase ) {
        case 0:
            for ( i = 0; i < 2; i++ ) {
                if ( !( gs_teamPlayer[i] & 0x80 ) ) {
                    player = slots[i][gs_teamPlayer[i]];
                    if ( player != 0xFF ) {
                    // RULE-EXCEPTION: R30, the character cursor shares j's register: retail allocates repeat and
                    // pad ahead of it, which a separate local reverses
                    j = game_players[player] & 0xF;
                    Menu_SetItem( gs_teamItems, player + 16, MENU_NONE, MENU_NONE, MENU_NONE, game_pulseAlpha );

                    if ( gs_teamPlayer[1] == 0x80 ) {
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
                        game_players[slots[i][gs_teamPlayer[i]]] =
                            ( game_players[slots[i][gs_teamPlayer[i]]] & 0xF0 ) | j;
                        Sound_PlaySe( SE_CURSOR, 0 );
                    }
                    if ( repeat & R_JPAD ) {
                        do {
                            j++;
                            if ( j == 8 ) {
                                j = 0;
                            }
                        } while ( !( ( unlocked >> j ) & 1 ) );
                        game_players[slots[i][gs_teamPlayer[i]]] =
                            ( game_players[slots[i][gs_teamPlayer[i]]] & 0xF0 ) | j;
                        Sound_PlaySe( SE_CURSOR, 0 );
                    }

                    if ( gs_teamPlayer[1] == 0x80 ) {
                        pad = cont_allPressed;
                    } else {
                        pad = cont_pressed[i];
                    }
                    if ( pad & ( A_BUTTON | Z_TRIG ) ) {
                        Sound_PlaySe( SE_DECIDE, 0 );
                        gs_teamPlayer[i]++;
                        if ( ( game_mode & 0xF ) == 1 && gs_teamPlayer[i] == 1 ) {
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
                            if ( gs_teamPlayer[i] ) {
                                Sound_PlaySe( 2, 0 );
                                gs_teamPlayer[i]--;
                            }
                        } else {
                            Sound_PlaySe( 2, 0 );
                            if ( gs_teamPlayer[i] == 0 ) {
                                gs_teamChoice = CHOICE_BACK;
                                break;
                            }
                            gs_teamPlayer[i]--;
                        }
                    }
                    } else {
                        gs_teamPlayer[i] |= 0x80;
                    }
                } else {
                    if ( gs_teamPlayer[1] == 0x80 ) {
                        pad = cont_allPressed;
                    } else {
                        pad = cont_pressed[i];
                    }
                    if ( ( pad & B_BUTTON ) && ( gs_teamPlayer[i] & 0x7F ) ) {
                        Sound_PlaySe( 2, 0 );
                        gs_teamPlayer[i] = ( gs_teamPlayer[i] & 0x7F ) - 1;
                    }
                }
            }
            if ( ( gs_teamPlayer[0] & 0x80 ) && ( gs_teamPlayer[1] & 0x80 ) ) {
                gs_teamPhase = 1;
                gs_leaveDelay = 5;
            }
            break;
        case 1:
            if ( --gs_leaveDelay == 0 ) {
                if ( ( game_mode & 0xF ) == 1 ) {
                    if ( save_data.rumble ) {
                        GS_StartPakCheck();
                        gs_teamChoice = 4;
                    } else {
                        game_leaveState = 1;
                        gs_teamChoice = CHOICE_TAKEN;
                    }
                } else {
                    gs_teamChoice = CHOICE_TAKEN;
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
        for ( j = 0; j <= ( gs_teamPlayer[i] & 0x7F ); j++ ) {
            if ( slots[i][j] != 0xFF ) {
                shown[slots[i][j]] = 1;
            }
        }
    }

    for ( i = 0; i < 4; i++ ) {
        Menu_SetItem( gs_teamItems, i + 8, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < 8; i++ ) {
        Menu_SetItem( gs_teamNameItems, i + 24, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }
    for ( i = 0; i < game_numPlayers; i++ ) {
        if ( shown[i] ) {
            if ( !( game_players[i] >> 7 ) ) {
                Menu_SetItem( gs_teamItems, i + 8, gs_playerNameImages[i], ( game_players[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            } else {
                Menu_SetItem( gs_teamItems, i + 8, gs_cpuNameImages[i], ( game_players[i] & 0xF ) * 32 + 32,
                               MENU_NONE, 0xFF );
            }
            Menu_SetItem( gs_teamItems, i + 16, gs_cursorImages[game_players[i] & 0xF],
                           gs_cursorX[game_players[i] & 0xF], MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( gs_teamItems, i + 16, gs_waitNameImages[i], 0x82, MENU_NONE, MENU_NONE );
        }
    }
    for ( i = 0; i < 8; i++ ) {
        if ( ( unlocked >> i ) & 1 ) {
            Menu_SetItem( gs_teamItems, i, gs_teamPortraits[i], MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( gs_teamItems, i, 0x8BC, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_DrawItems( gs_teamItems, gs_teamSlideStep );
    /* an empty loop: retail still counts it out */
    for ( i = 0; i < 4; i++ ) {
    }

    if ( gs_teamChoice == CHOICE_RUN ) {
        for ( i = 0; i < 4; i++ ) {
            if ( i < game_numPlayers ) {
                character = game_players[i] & 0xF;
                if ( shown[i] ) {
                    ( (sceneObject_t *)obj_scene[10 + i] )->viewMask = 1;
                    if ( gs_shownModels[i] != character ) {
                        gs_shownModels[i] = character;
                        ( (sceneObject_t *)obj_scene[10 + i] )->frame = 0;
                        ( (sceneObject_t *)obj_scene[10 + i] )->model = gs_teamModels[character];
                        ( (sceneObject_t *)obj_scene[10 + i] )->anim = gs_teamAnims[character];
                        if ( character == 6 ) {
                            gs_modelPos[i][1] += 40;
                        }
                        Mtx_SetRotScaleTrans( ( (sceneObject_t *)obj_scene[10 + i] )->mtx, 0.0f, 0.0f, 0.0f,
                                       gs_teamModelScales[character], gs_modelPos[i][0], gs_modelPos[i][1],
                                       gs_modelPos[i][2] );
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
                    if ( gs_barLevels[i][j] < gs_barHeights[character][j] ) {
                        gs_barLevels[i][j]++;
                    }
                    if ( gs_barLevels[i][j] > gs_barHeights[character][j] ) {
                        gs_barLevels[i][j]--;
                    }
                    Draw_FillRect( gs_barX[i][j], gs_barY[i][j], 3, 40 - gs_barLevels[i][j],
                                   0xB0, 0xB0, 0xB0, 0xC0 );
                    Draw_FillRect( gs_barX[i][j], gs_barY[i][j] - ( gs_barLevels[i][j] - 40 ),
                                   3, gs_barLevels[i][j], gs_barColors[j][0], gs_barColors[j][1],
                                   gs_barColors[j][2], 0xC0 );
                }
                Menu_SetItem( gs_teamNameItems, i + 24, gs_nameLine1Images[character], gs_barX[i][0] - 16,
                               gs_barY[i][0] + 18, game_pulseAlpha );
                Menu_SetItem( gs_teamNameItems, i + 28, gs_nameLine2Images[character], gs_barX[i][0] - 16,
                               gs_barY[i][0] + 26, game_pulseAlpha );
            }
        }
    }

    Menu_DrawItems( gs_teamNameItems, gs_teamSlideStep );

    switch ( gs_teamChoice ) {
    case CHOICE_IN:
        gs_teamItems[5].slide = SLIDE_LEFT;
        gs_teamSlideStep -= 2;
        if ( gs_teamSlideStep == 0 ) {
            gs_teamChoice = CHOICE_RUN;
            Game_StartPulse();
            Obj_CreateAll( gs_classes, gs_teamScene );
            cam_cameras[0]->path->flags |= 4;
            for ( i = 0; i < 4; i++ ) {
                gs_shownModels[i] = 0xFFFF;
                for ( j = 0; j < 3; j++ ) {
                    gs_barLevels[i][j] = 0;
                }
            }
        } else if ( gs_teamSlideStep == 28 ) {
            for ( i = 0; i < 8; i++ ) {
                gs_teamModels[i] = Cache_GetFile( gs_teamModelFiles[i], FILE_TYPE1 );
                gs_teamAnims[i] = Cache_GetFile( gs_teamAnimFiles[i], 2 );
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_teamItems[5].slide = SLIDE_RIGHT;
        gs_teamSlideStep += 2;
        if ( gs_teamSlideStep == 30 ) {
            if ( game_leaveState == 1 ) {
                if ( game_fade != 0xFF ) {
                    gs_teamSlideStep = 28;
                    return 0;
                }
                game_leaveState = 2;
                game_fade = 0x107;
            }
            for ( i = 0; i < 4; i++ ) {
                shown[i] = 0;
            }
            Obj_FreeAll();
            gs_teamPhase = 0;
            Cache_FreeBelow( 0x825 );
            gs_teamPlayer[0] = gs_teamPlayer[1] = 0;
            if ( gs_teamChoice == CHOICE_TAKEN ) {
                gs_teamChoice = CHOICE_IN;
                return 1;
            }
            gs_teamChoice = CHOICE_IN;
            return 2;
        }
        break;
    case 4:
        if ( GS_PakCheck() ) {
            game_leaveState = 1;
            gs_teamChoice = CHOICE_TAKEN;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/5EC00", GS_TeamSelect);
#endif
