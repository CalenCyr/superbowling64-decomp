#ifndef GSCODE_GSCODE_H
#define GSCODE_GSCODE_H

/* gscode.h -- the game-select overlay (file GSCODE  SYS, ROM 0x58F40, loaded at 0x80112350) */

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

extern u8 D_8005B191;               /* set while the overlay checks the controller paks */
extern u8 D_8005C0E4[];             /* per player: bit 7 set for a player of the game (no controller) */
extern u8 D_8005C161;               /* number of players */
extern u8 D_8005C162;               /* index of the last team */
extern u8 D_8005C163;
extern u8 D_8005CB60;

/*
==============================================================================

OVERLAY

==============================================================================
*/

extern s32 D_8011B5A0_gscode;       /* entries in D_80120781_gscode */
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

/* unit 5EC00 */
u8 func_80118010_gscode( u8 flags );

/* unit 61CA0 */
void func_8011B0B0_gscode( void );
s32 func_8011B214_gscode( void );

#endif
