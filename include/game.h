#ifndef GAME_H
#define GAME_H

/*
game.h -- main's game-state globals that the code overlays (gacode, rscode, gscode) read. Their owning main units
are not identified yet; each is declared once here until they are.
*/

#include "common.h"

/* A player's name: characters ended by 0xFFFF; copied whole by assignment */
typedef struct {
    /* 0x00 */ u16 chars[6];
} playerName_t;                              /* size 0x0C */


extern u8 D_8005C0E4[];             /* per player: bit 7 set for a player of the game (no controller); low nibble a character */
extern u8 D_8005C160;               /* low nibble the game mode; bits 4-5 a sub-mode, bits 6-7 its argument */
extern u8 D_8005C161;               /* number of players */
extern u8 D_8005C162;               /* index of the last team */
extern u8 D_8005C163;
extern u8 D_8005CB60;
extern u8 D_800693DC[2];            /* per lane: games won */
extern u8 D_8009D19C[4];            /* per player: current frame (10: the game is over) */
extern s16 D_800DF630[4][10];       /* per player and frame: score */
extern s8 D_800A3020[4][10];        /* per player and frame */
extern s8 D_80066E18[4][10];        /* per player and frame */
extern s8 D_800994B8[2];            /* per player of a duel: frames won */
extern s16 D_800EA090[];
extern u16 D_8009D030[2][3];        /* per player of a duel and game: score */
extern u8 D_8005C0B6;               /* restored by the results overlay on leaving */
extern u8 D_8005C0BF;               /* 0, then 1 once an overlay asks to leave, 2 when it goes */
extern s16 D_8005C0C0;
extern u32 D_8005C0F0;               /* demo the opening overlay plays next, 0 to 3 */
extern u8 D_8005C0F4;                /* counts the demos' characters */
extern playerName_t D_8005C0F8[8];   /* per character: its default name */
extern playerName_t D_80068218[];    /* per player: name */
extern u8 D_80068A50[];             /* per player (func_8003A658) */
extern u8 D_800994BE[];             /* per player (func_8003A658) */
extern u8 D_80099458[4][10];        /* per player and frame: flags; in the par game, [0] holds each frame's par (func_80122140_gacode) */
extern u16 D_8009CF80[4];           /* per player of a duel: games won */
extern u16 D_800DF6B4[4];           /* per player: counted up with D_800A304C (func_801151A0_gacode) */
extern u16 D_8009D088[4];           /* per player: counted when the frame ends after a non-run turn (func_801151A0_gacode) */
extern u8 D_800A304C[4];            /* per player: a run counter (to 255) the game modes read and reset (func_8011582C_gacode) */
extern u16 D_800A3050[4][12];       /* per player and frame: a pin mask, bit 9 - n for pin n */
extern u16 D_800F7C18[4][12];       /* per player and frame: a second pin mask, the same way */

#endif
