#ifndef GSCODE_GSCODE_H
#define GSCODE_GSCODE_H

/* gscode.h -- the game-select overlay (file GSCODE  SYS, ROM 0x58F40, loaded at 0x80112350) */

#include "common.h"
#include "game.h"
#include "menu.h"
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

extern u8 D_8005B191;               /* set while the overlay checks the controller paks */

void func_8002C950( s32 x, s32 y, s32 w, s32 h, u8 r, u8 g, u8 b, u8 a );    /* unit 7250 */
void func_8003068C( s32 r, s32 g, s32 b );                                   /* unit 7250 */

/*
==============================================================================

OVERLAY

==============================================================================
*/

/* D_8011B8C5_gscode: step of the two-choice menu of unit 5A990 */
#define CHOICE_IN           0           /* sliding in */
#define CHOICE_RUN          1           /* in place, taking input */
#define CHOICE_TAKEN        2           /* a choice was taken, sliding out */
#define CHOICE_BACK         3           /* backed out, sliding out */

extern s32 D_8011B5A0_gscode;       /* entries in D_80120781_gscode */
extern menuItem_t D_8011B850_gscode[9];  /* the two-choice menu; [5]: its title */
extern s16 D_8011B8AC_gscode[2][6];  /* per choice: images of its blink */
extern u8 D_8011B8C4_gscode;        /* slide step of the two-choice menu */
extern u8 D_8011B8C5_gscode;        /* CHOICE_IN etc. */
extern u8 D_8011B8C6_gscode;        /* highlighted choice, 0 or 1 */
extern u8 D_8011B8C7_gscode;        /* blink frame, 0 to 29 */
extern s16 D_80120660_gscode[4];    /* images of a blinking marker */
extern s32 D_80120668_gscode;       /* frame counter of the pak check */
extern u8 D_80120780_gscode;        /* current screen */
extern u8 D_80120781_gscode[];      /* screens to go back to, a stack */
extern u32 D_80120A60_gscode;       /* step of the pak check */
extern s32 D_80120A64_gscode;       /* 1: a second controller is in play */
extern u8 D_80120A68_gscode[2];     /* per port: controller status of the last frame */

/* unit 58F40 */
void func_80112350_gscode( void );
void func_801123C4_gscode( u8 screen );
void func_801123E8_gscode( void );
void func_8011288C_gscode( void );
void func_80112894_gscode( void );

/* unit 5A990 */
s32 func_80113DA0_gscode( void );

/* unit 5EC00 */
u8 func_80118010_gscode( u8 flags );

/* unit 61CA0 */
void func_8011B0B0_gscode( void );
s32 func_8011B214_gscode( void );

#endif
