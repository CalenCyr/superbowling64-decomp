#ifndef RSCODE_RSCODE_H
#define RSCODE_RSCODE_H

/* rscode.h -- the results overlay (file RSCODE  SYS, ROM 0x56FF0, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "sprite.h"
#include "gamesnd.h"
#include "menu.h"
#include "save.h"

/*
==============================================================================

MAIN (functions and globals of main units without a header yet, under their current names)

==============================================================================
*/

extern u8 D_8005C0B6;
extern u8 D_8005C0BF;               /* 0, then 1 once the overlay asks to leave, 2 when it goes */
extern s16 D_8005C0C0;
extern u8 D_8005C0E4[];             /* per player: bit 7 set for a player of the game (no controller) */
extern u8 D_8005C160;               /* low nibble the game mode; bits 4-5 a sub-mode, bits 6-7 its argument */
extern u8 D_8005C161;               /* number of players */
extern u8 D_8005C163;
extern u8 D_8005CB60;
extern s16 D_800EA090[];
extern u8 D_8009D19C[4];            /* per player: current frame (10: the game is over) */
extern s16 D_800DF630[4][10];       /* per player and frame: score */

void func_8002C950( s32 x, s32 y, s32 w, s32 h, u8 r, u8 g, u8 b, u8 a );    /* unit 7250 */
void func_8003068C( s32 r, s32 g, s32 b );                                   /* unit 7250 */
void func_80039700( s32 arg0, s32 arg1, s32 arg2, s32 arg3 );                /* unit 13490 */
void func_80039CBC( s32 arg0, s32 arg1, s32 arg2, s32 arg3 );                /* unit 13490 */
void func_8003A3F0( s32 arg0, s32 arg1, s32 arg2 );                          /* unit 13490 */

/*
==============================================================================

OVERLAY

==============================================================================
*/

/* A state of the overlay's main loop (D_801142AC_rscode). Name invented. */
typedef void (*rsState_t)( void );

extern u8 D_80114250_rscode;        /* set: leave the overlay */
extern u8 D_80114251_rscode;
extern u8 D_80114252_rscode;        /* alpha of the boxes, 0 to 0x80 and back */
extern u8 D_80114253_rscode;        /* 1: D_80114252_rscode falls */
extern s16 D_80114254_rscode;       /* frames the current message has been shown */
extern u16 D_801142A0_rscode;       /* buttons pressed this frame (0 once leaving) */
extern u16 D_801142A6_rscode;       /* D-pad presses this frame (0 once leaving) */
extern u8 D_801142A8_rscode;        /* copied back to D_8005C0B6 on leaving */
extern rsState_t D_801142AC_rscode; /* current state */
extern rsState_t D_801142B0_rscode; /* state to return to */

void func_80112350_rscode( void );
void func_801123A8_rscode( void );
void func_80112574_rscode( void );
void func_80112674_rscode( void );
void func_8011286C_rscode( void );
void func_8011291C_rscode( void );
void func_801129CC_rscode( void );
void func_80112B60_rscode( void );
void func_80113030_rscode( void );
void func_801130AC_rscode( void );
void func_8011311C_rscode( void );
void func_80113504_rscode( void );
void func_80113564_rscode( void );
void func_8011372C_rscode( void );
void func_8011377C_rscode( void );
void func_801137CC_rscode( void );

#endif
