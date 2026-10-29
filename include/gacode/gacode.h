#ifndef GACODE_GACODE_H
#define GACODE_GACODE_H

/* gacode.h -- the game overlay (file GACODE  SYS, ROM 0x3C570, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "input.h"

/*
==============================================================================

MAIN (functions and globals of main units without a header yet, under their current names)

==============================================================================
*/

extern u8 D_8005C160;               /* low nibble 3: func_8011A26C; else bits 4-5 pick a mode, bits 6-7 its arg */
extern u8 D_8005C162;               /* index of the last team */
extern u8 D_800994BE[];

s32 func_80037410( void );          /* unit 12260 */
void func_800379A8( s64 handle );   /* unit 12C70: stops the sound of a func_80037870 handle */
s32 func_8003B204( void );          /* unit 13490 */
void func_8002E4B0( s32 arg0, s32 arg1, s32 arg2, s32 arg3, f32 arg4, f32 arg5 );   /* unit 7250 */
void func_80039700( s32 arg0, s32 arg1, s32 arg2, s32 arg3 );   /* unit 13490 */
void func_80039CBC( s32 arg0, s32 arg1, s32 arg2, s32 arg3 );   /* unit 13490 */
void func_8003A3F0( s32 arg0, s32 arg1, s32 arg2 );             /* unit 13490 */

/*
==============================================================================

OVERLAY

==============================================================================
*/

/* A state of the overlay's main loop; returns nonzero when the state is done. */
typedef s32 (*gaState_t)( void );

extern gaState_t D_8012CDF0_gacode;     /* current state */
extern s64 D_8012CDE8_gacode;           /* sound handle (func_80037870) */
extern u8 D_8012CDF8_gacode[2];         /* per team: index in D_8012CE02_gacode of the current player */
extern s8 D_8012CE02_gacode[2][5];      /* per team: player numbers, -1 ends */

/* unit 3C570 */
s32 func_80112AF0_gacode( void );
s32 func_80112D94_gacode( void );
void func_80112E04_gacode( void );
s32 func_80113210_gacode( void );
s32 func_80113270_gacode( void );
s32 func_801132A8_gacode( void );
s32 func_80114108_gacode( void );
void func_80112A74_gacode( void );
void func_801138EC_gacode( u8 *cur, s32 start, s32 team );

/* unit 3E350 */
void func_80114640_gacode( s32 arg0 );
void func_8011A26C_gacode( void );
void func_80114FFC_gacode( s32 player );

/* unit 474E0 */
void func_80120D7C_gacode( s64 *handle );

#endif
