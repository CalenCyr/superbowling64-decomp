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

typedef f32 vec3_t[3];

/*
Object of main's scene engine (D_800EA120 holds pointers to them): only the members the overlay uses. Name
invented.
*/
typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ f32 mtx[4][4];           /* placement, set by func_8011D2C0_gacode */
    /* 0x44 */ f32 unk_44;
    /* 0x48 */ u16 unk_48;
    /* 0x4A */ u8 unk_4A;               /* 0: not drawn */
} sceneObj_t;

/* Ball or pin of a lane. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x40];
    /* 0x40 */ vec3_t pos;
    /* 0x4C */ vec3_t rot;
    /* 0x58 */ f32 scale;
    /* 0x5C */ u8 unk_5C;
    /* 0x5D */ u8 unk_5D;               /* 1: still standing (pins) */
    /* 0x5E */ u8 unk_5E[2];
} gaBody_t;                             /* size 0x60 */

/* Where a body of a lane stands; func_80114CB4_gacode places the body there. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t pos;
    /* 0x0C */ u8 unk_0C[0x14];
} gaSpot_t;                             /* size 0x20 */

/* View of a lane, set each frame by the lane's unk_B0C handler. Name invented. */
typedef struct gaCamera_s {
    /* 0x00 */ vec3_t eye;
    /* 0x0C */ vec3_t at;
    /* 0x18 */ vec3_t unk_18;
} gaCamera_t;

#define NUM_BODIES      11              /* the ball, then the ten pins */
#define NUM_PINS        10

/* One of the two lanes (D_8012DCF0_gacode), shared by the players bowling on it. Name invented. */
typedef struct {
    /* 0x000 */ gaBody_t bodies[NUM_BODIES];
    /* 0x420 */ u8 unk_420[0x8];
    /* 0x428 */ f32 unk_428;
    /* 0x42C */ u8 unk_42C[0x688 - 0x42C];
    /* 0x688 */ gaSpot_t spots[NUM_BODIES];
    /* 0x7E8 */ u8 unk_7E8[0x948 - 0x7E8];
    /* 0x948 */ gaBody_t unk_948[3];
    /* 0xA68 */ gaSpot_t unk_A68[3];
    /* 0xAC8 */ sceneObj_t *bodyObjs[NUM_BODIES];
    /* 0xAF4 */ sceneObj_t *unk_AF4[3];
    /* 0xB00 */ gaCamera_t *camera;
    /* 0xB04 */ void (*unk_B04)( s32 player );
    /* 0xB08 */ void (*unk_B08)( s32 player );
    /* 0xB0C */ void (*unk_B0C)( s32 player, gaCamera_t *camera );
    /* 0xB10 */ s32 (*unk_B10)( s32 player, s32 input, s16 stickX, s16 stickY );
    /* 0xB14 */ f32 unk_B14;
    /* 0xB18 */ s64 unk_B18;            /* sound handle (func_80037870) */
    /* 0xB20 */ s16 unk_B20;
    /* 0xB22 */ u8 unk_B22[2];
    /* 0xB24 */ u8 unk_B24;
    /* 0xB25 */ u8 unk_B25;
    /* 0xB26 */ u8 unk_B26;
    /* 0xB27 */ u8 unk_B27[9];
} gaLane_t;                             /* size 0xB30 */

/* Character record (D_80123BE0_gacode), copied into gaPlayer_t. Name invented. */
typedef struct {
    /* 0x00 */ u8 *unk_00;
    /* 0x04 */ u8 unk_04;
    /* 0x05 */ u8 unk_05[0x0F];
    /* 0x14 */ f32 unk_14;
    /* 0x18 */ u8 unk_18[0x30];
} gaCharacter_t;                        /* size 0x48 */

#define MAX_PLAYERS     4

/* A player (D_8012D658_gacode). Name invented. */
typedef struct {
    /* 0x00 */ gaCharacter_t character;
    /* 0x48 */ f32 unk_48[4][4];        /* copy of obj->mtx */
    /* 0x88 */ sceneObj_t *obj;
    /* 0x8C */ gaLane_t *lane;
    /* 0x90 */ s8 port;                 /* controller port; -1: played by the game */
    /* 0x91 */ u8 laneIndex;
    /* 0x92 */ u8 unk_92;
    /* 0x93 */ u8 unk_93;
    /* 0x94 */ u8 unk_94[0x0C];
} gaPlayer_t;                           /* size 0xA0 */

/* Handler of a player: the game mode (D_8012CDF4_gacode) or what starts a turn (D_8012D23C_gacode). */
typedef s32 (*gaPlayerFunc_t)( s32 player );

extern gaPlayer_t D_8012D658_gacode[MAX_PLAYERS];
extern u16 D_8012D8D8_gacode;
extern gaLane_t D_8012DCF0_gacode[2];

/* A state of the overlay's main loop; returns nonzero when the state is done. */
typedef s32 (*gaState_t)( void );

extern gaState_t D_8012CDF0_gacode;     /* current state */
extern gaPlayerFunc_t D_8012CDF4_gacode;  /* game mode */
extern gaPlayerFunc_t D_8012D23C_gacode;
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
void func_80114130_gacode( void );
void func_80114138_gacode( void );
void func_80114640_gacode( s32 arg0 );
void func_8011A26C_gacode( void );
void func_80114FC4_gacode( s32 player );
void func_80114FFC_gacode( s32 player );
s32 func_801157F8_gacode( s32 player );
s32 func_80115CDC_gacode( s32 player );
s32 func_80115D38_gacode( s32 player );
s32 func_80115DF4_gacode( s32 player );
void func_80116248_gacode( s32 player );
void func_80116398_gacode( s32 player );
void func_801163A0_gacode( s32 player );
void func_80116414_gacode( s32 player );
void func_80116750_gacode( gaLane_t *lane );
void func_80116798_gacode( gaLane_t *lane );
s32 func_80116998_gacode( gaPlayer_t *pl );
s32 func_80117ABC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_801189C4_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118D9C_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118F08_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118F38_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
void func_8011B400_gacode( s32 player );
void func_8011C0DC_gacode( s32 player, gaCamera_t *camera );
void func_8011C650_gacode( s32 player, gaCamera_t *camera );

/* unit 474E0 */
void func_80120D7C_gacode( s64 *handle );
void func_80120DD0_gacode( void );

#endif
