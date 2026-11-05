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

#endif
