#ifndef GAME_H
#define GAME_H

/*
game.h -- main's game-state globals that the code overlays (gacode, rscode, gscode) read. Their owning main units
are not identified yet; each is declared once here until they are.
*/

#include "common.h"

extern u8 D_8005C0E4[];             /* per player: bit 7 set for a player of the game (no controller); low nibble a character */
extern u8 D_8005C160;               /* low nibble the game mode; bits 4-5 a sub-mode, bits 6-7 its argument */
extern u8 D_8005C161;               /* number of players */
extern u8 D_8005C162;               /* index of the last team */
extern u8 D_8005C163;
extern u8 D_8005CB60;
extern u8 D_800693DC[2];            /* per lane: games won */
extern u8 D_8009D19C[4];            /* per player: current frame (10: the game is over) */
extern s16 D_800DF630[4][10];       /* per player and frame: score */
extern s16 D_800EA090[];
extern u8 D_8005C0B6;               /* restored by the results overlay on leaving */
extern u8 D_8005C0BF;               /* 0, then 1 once an overlay asks to leave, 2 when it goes */
extern s16 D_8005C0C0;

#endif
