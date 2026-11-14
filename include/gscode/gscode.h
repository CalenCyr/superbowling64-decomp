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


/*
==============================================================================

OVERLAY

==============================================================================
*/

/* D_8011B845_gscode, D_8011B8C5_gscode: step of the two-choice menus of units 5A580 and 5A990 */
#define CHOICE_IN           0           /* sliding in */
#define CHOICE_RUN          1           /* in place, taking input */
#define CHOICE_TAKEN        2           /* a choice was taken, sliding out */
#define CHOICE_BACK         3           /* backed out, sliding out */

extern s32 D_8011B5A0_gscode;       /* entries in D_80120781_gscode */
extern menuItem_t D_8011B7D0_gscode[9];  /* the menu of one or two players; [5]: its title */
extern s16 D_8011B82C_gscode[2][6];  /* per choice: images of its blink */
extern u8 D_8011B844_gscode;        /* slide step of the menu of one or two players */
extern u8 D_8011B845_gscode;        /* CHOICE_IN etc. */
extern u8 D_8011B846_gscode;        /* highlighted choice: 0 one player, 1 two */
extern u8 D_8011B847_gscode;        /* blink frame, 0 to 29 */
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
s32 func_80112414_gscode( void );
void func_8011288C_gscode( void );
void func_80112894_gscode( void );

/* units 59490, 59B00, 5A020: the first screens */
s32 func_801128A0_gscode( void );
s32 func_80112F10_gscode( void );
s32 func_80113430_gscode( void );

/* unit 5A580 */
s32 func_80113990_gscode( void );

/* unit 5A990 */
s32 func_80113DA0_gscode( void );

/* unit 5AD50 */
s32 func_80114D1C_gscode( void );

/* unit 5E2B0 */
s32 func_801176C0_gscode( void );

/* unit 5EC00 */
u8 func_80118010_gscode( u8 flags );
s32 func_80118040_gscode( void );

/* units 605D0, 61640 */
s32 func_801199E0_gscode( void );
s32 func_8011AA50_gscode( void );

/* unit 61CA0 */
void func_8011B0B0_gscode( void );
s32 func_8011B214_gscode( void );

#endif
