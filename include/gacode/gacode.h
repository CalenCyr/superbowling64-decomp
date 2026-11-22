#ifndef GACODE_GACODE_H
#define GACODE_GACODE_H

/* gacode.h -- the game overlay (file GACODE  SYS, ROM 0x3C570, loaded at 0x80112350) */

#include "common.h"
#include "game.h"
#include "menu.h"
#include "libultra.h"
#include <PR/gu.h>
#include "input.h"
#include "gamesnd.h"
#include "filecache.h"
#include "boot.h"
#include "save.h"
#include "menu.h"
#include "sprite.h"
#include "object.h"

/*
==============================================================================

MAIN (functions and globals of main units without a header yet, under their current names)

==============================================================================
*/

extern f32 D_800E6560[2][3];        /* per lane: position of the ball */
extern u8 D_8006940D[NUM_PLAYERS];   /* per port (unit 2DF0) */
extern u8 D_800DF595[2];            /* per lane: state of the ball (func_80114DD4_gacode) */

/* Main's game setup (D_800DF59C): only the members the overlay uses. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[4];
    /* 0x04 */ u8 unk_04[0x30];        /* indexed by frame */
} unkGameSetup_t;                       /* size 0x34 */

extern unkGameSetup_t D_800DF59C;

/* Image of main's resource table: a header, then a 256-colour palette. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x07];
    /* 0x07 */ u8 unk_07;
    /* 0x08 */ u8 unk_08[0x08];
    /* 0x10 */ u16 palette[256];
} unkImage_t;


/*
==============================================================================

OVERLAY

==============================================================================
*/

typedef f32 vec3_t[3];

/* Joint of a scene object: its matrix, the translation in row 3. Name invented. */

/*
Object of main's scene engine (D_800EA120 holds pointers to them): only the members the overlay uses. Name
invented.
*/


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
    /* 0x0C */ vec3_t unk_0C;
    /* 0x18 */ f32 unk_18;
    /* 0x1C */ u32 unk_1C;              /* contact bits of func_8011E6F8_gacode: bit 0 hit this frame, bit n touching spots[n] */
} gaSpot_t;                             /* size 0x20 */

/* View of a lane, set each frame by the lane's unk_B0C handler. Name invented. */
typedef struct gaCamera_s {
    /* 0x00 */ vec3_t eye;
    /* 0x0C */ vec3_t at;
    /* 0x18 */ vec3_t unk_18;
    /* 0x24 */ s32 unk_24;
} gaCamera_t;

#define NUM_BODIES      11              /* the ball, then the ten pins */
#define NUM_PINS        10

/* Motion of a body of a lane, stepped each frame by func_80114898_gacode. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t vel;              /* added to the body's spot each frame; the ball's is turned into its spin
                                           by func_8011D7CC_gacode */
    /* 0x0C */ vec3_t spin;             /* added to the body's rot each frame (degrees, kept within -180..180) */
    /* 0x18 */ u8 unk_18[0x18];
    /* 0x30 */ f32 unk_30;              /* the ball's from D_801239E8_gacode, by gaPlayer_t.unk_93 */
    /* 0x34 */ u32 flags;
} gaMotion_t;                           /* size 0x38 */

/* Absolute value of a float, as 0 - x. */
#define FABS( x )       ( ( x ) < 0.0f ? 0.0f - ( x ) : ( x ) )

/* One of the two lanes (D_8012DCF0_gacode), shared by the players bowling on it. Name invented. */
typedef struct {
    /* 0x000 */ gaBody_t bodies[NUM_BODIES];
    /* 0x420 */ gaMotion_t motion[NUM_BODIES];
    /* 0x688 */ gaSpot_t spots[NUM_BODIES];
    /* 0x7E8 */ gaSpot_t unk_7E8[NUM_BODIES];
    /* 0x948 */ gaBody_t unk_948[3];
    /* 0xA68 */ gaSpot_t unk_A68[3];
    /* 0xAC8 */ sceneObject_t *bodyObjs[NUM_BODIES];
    /* 0xAF4 */ sceneObject_t *unk_AF4[3];
    /* 0xB00 */ gaCamera_t *camera;
    /* 0xB04 */ void (*unk_B04)( s32 player );
    /* 0xB08 */ void (*unk_B08)( s32 player );
    /* 0xB0C */ void (*unk_B0C)( s32 player, gaCamera_t *camera );
    /* 0xB10 */ s32 (*unk_B10)( s32 player, s32 input, s16 stickX, s16 stickY );
    /* 0xB14 */ f32 unk_B14;
    /* 0xB18 */ s64 unk_B18;            /* sound handle (func_80037870) */
    /* 0xB20 */ s16 unk_B20;
    /* 0xB22 */ u8 unk_B22;
    /* 0xB23 */ u8 unk_B23;
    /* 0xB24 */ u8 unk_B24;
    /* 0xB25 */ u8 unk_B25;
    /* 0xB26 */ u8 unk_B26;
    /* 0xB27 */ u8 unk_B27;
    /* 0xB28 */ f32 unk_B28;            /* camera height offset of func_8011B8AC_gacode */
    /* 0xB2C */ u16 unk_B2C;            /* from gaStage_t.unk_1E[0] */
    /* 0xB2E */ u8 unk_B2E[2];
} gaLane_t;                             /* size 0xB30 */

/* What gaCharacter_t.unk_00 points to. Name invented. */
typedef struct {
    /* 0x00 */ s16 *unk_00;                 /* file indices */
    /* 0x04 */ f32 unk_04;                  /* objParams_t.unk_08[4] of the player's scene object */
    /* 0x08 */ s8 unk_08;                   /* +-: a joint of the player's object */
    /* 0x09 */ u8 unk_09[0x03];
    /* 0x0C */ vec3_t unk_0C;               /* offset of the ball from that joint */
    /* 0x18 */ u8 unk_18;
    /* 0x19 */ u8 unk_19;
    /* 0x1A */ u8 unk_1A;
    /* 0x1B */ u8 unk_1B[5];            /* indexed by gaLane_t.unk_B26 */
} gaCharacterData_t;                        /* size 0x20 */

/* Per character (the last two empty): file indices, [0] and [1] the player's scene object's (func_80112350_gacode) */
extern s16 D_80123A14_gacode[10][7];
extern gaCharacterData_t D_80123AA0_gacode[10];   /* per character; unk_00 points to its row of D_80123A14_gacode */

/* Character record (D_80123BE0_gacode), copied into gaPlayer_t. Name invented. */
typedef struct {
    /* 0x00 */ gaCharacterData_t *unk_00;
    /* 0x04 */ u8 unk_04;
    /* 0x05 */ u8 unk_05;               /* nonzero: unk_30 runs from -unk_10 to 0, else from 0 to unk_10 */
    /* 0x06 */ u8 unk_06[0x02];
    /* 0x08 */ f32 unk_08;              /* upper limit of unk_2C */
    /* 0x0C */ f32 unk_0C;              /* lower limit of unk_2C */
    /* 0x10 */ f32 unk_10;              /* range of unk_30 */
    /* 0x14 */ vec3_t unk_14;
    /* 0x20 */ vec3_t unk_20;
    /* 0x2C */ f32 unk_2C;
    /* 0x30 */ f32 unk_30;
    /* 0x34 */ f32 unk_34;              /* an angle (radians; func_8011B1A0_gacode, func_8011840C_gacode) */
    /* 0x38 */ vec3_t unk_38;            /* func_8011D4E4_gacode of unk_20 and unk_14 */
    /* 0x44 */ f32 unk_44;
} gaCharacter_t;                        /* size 0x48 */

extern gaCharacter_t D_80123BE0_gacode[];

#define MAX_PLAYERS     4

/* A player (D_8012D658_gacode). Name invented. */
typedef struct {
    /* 0x00 */ gaCharacter_t character;
    /* 0x48 */ f32 unk_48[4][4];        /* copy of obj->mtx */
    /* 0x88 */ sceneObject_t *obj;
    /* 0x8C */ gaLane_t *lane;
    /* 0x90 */ s8 port;                 /* controller port; -1: played by the game */
    /* 0x91 */ u8 laneIndex;
    /* 0x92 */ s8 unk_92;               /* 0 to 4, stepped by func_8011BB44_gacode */
    /* 0x93 */ u8 unk_93;               /* 0 to 10, chosen with C-left and C-right (func_801194D0_gacode) */
    /* 0x94 */ u16 unk_94;
    /* 0x96 */ u8 unk_96[0x02];
    /* 0x98 */ f32 unk_98;
    /* 0x9C */ f32 unk_9C;
} gaPlayer_t;                           /* size 0xA0 */

/* Configuration of a stage (D_80123EB0_gacode); D_8012D220_gacode points to the current one. Name invented. */
typedef struct {
    /* 0x00 */ objParams_t *unk_00;      /* the scene (func_8002A1EC); one object per player first */
    /* 0x04 */ f32 unk_04;
    /* 0x08 */ f32 unk_08;
    /* 0x0C */ f32 unk_0C;
    /* 0x10 */ f32 unk_10;
    /* 0x14 */ f32 unk_14;
    /* 0x18 */ u16 unk_18;              /* archive index of the floor grid (D_80100100) */
    /* 0x1A */ u16 unk_1A;
    /* 0x1C */ s16 unk_1C;              /* sound id; -1: none */
    /* 0x1E */ s16 unk_1E[9];
    /* 0x30 */ void (*update)( void );  /* each frame */
} gaStage_t;                            /* size 0x34 */

/* Main resource image with a range of its palette that the overlay fades; keeps a copy of the palette. Name
invented. */
typedef struct {
    /* 0x000 */ u16 id;
    /* 0x002 */ u16 first;              /* palette range */
    /* 0x004 */ u16 last;
    /* 0x006 */ u16 palette[256];
    /* 0x206 */ u8 unk_206;
} gaSprite_t;                           /* size 0x208 */

/* A list of image ids with a cursor; one per lane (D_801236B0_gacode). Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[2];
    /* 0x02 */ u8 cur;
    /* 0x03 */ u8 count;
    /* 0x04 */ s16 *ids;
} gaImageList_t;                        /* size 0x8 */

extern gaImageList_t D_801236B0_gacode[2];
extern u16 D_8012C9A0_gacode[][4];      /* image files of func_80123430_gacode, by its index */
/* image files of func_80122788_gacode's animated palettes, by animation frame */
extern u16 D_8012C928_gacode[11];
extern u16 D_8012C940_gacode[11];
extern u16 D_8012C958_gacode[2];
extern u16 D_8012C95C_gacode[2];
extern u16 D_8012C960_gacode[2];
extern u16 D_8012C964_gacode[9];
extern void *D_8012CDD0_gacode[2];      /* texels of resources 1523 and 1524, saved by func_80122788_gacode */
/* image files of func_80122CE8_gacode's four palette slots, by brightness step */
extern u16 D_8012C978_gacode[4];
extern u16 D_8012C980_gacode[4];
extern u16 D_8012C988_gacode[4];
extern u16 D_8012C990_gacode[4];
extern u16 D_8012C99C_gacode;           /* frames to the next flicker of func_80122CE8_gacode */
extern u16 D_8012CDD8_gacode;           /* frames to func_80122CE8_gacode's flicker sound */
extern gaSprite_t D_8012D8E0_gacode[2];   /* per lane */
extern gaSprite_t D_8012D240_gacode[2];   /* per lane (func_80114140_gacode) */
extern gaSprite_t D_8012CE10_gacode[2];   /* per lane (func_80114140_gacode) */

/* A pin setup of a game mode, by frame (D_801237F0_gacode, D_80123950_gacode). Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ u16 pins;                /* pins standing, pin 1 in bit 9 (func_80120980_gacode) */
    /* 0x04 */ u16 unk_04;
} gaPinSetup_t;                         /* size 0x6 */

extern gaPinSetup_t D_801237F0_gacode[];   /* for func_801156D8_gacode, by D_800DF59C.unk_04[] */
extern gaPinSetup_t D_80123950_gacode[];   /* for func_801155BC_gacode, by D_800DF59C.unk_04[] */
extern u16 D_8012C7C6_gacode;              /* pins of func_801157F8_gacode */

/* Handler of a player: the game mode (D_8012CDF4_gacode) or what starts a turn (D_8012D23C_gacode). */
typedef s32 (*gaPlayerFunc_t)( s32 player );

extern s16 D_80123600_gacode;          /* blink level, 0 to 0x80 by 8 (func_801125A0_gacode) */
extern u8 D_80123601_gacode;
extern s8 D_80123602_gacode;           /* 1: D_80123600_gacode falls, else it rises */
extern u8 D_80123603_gacode;           /* what the main loop state returned: nonzero, the game is done */
extern u8 D_80123604_gacode;           /* what the pause menu's state returned: nonzero, leave the game */
extern u8 D_8012C900_gacode[2][NUM_PINS];  /* per side: the pins (1 to 10) from the back row forward (func_801203AC_gacode) */
/* By 2 to 6 standing pins (head pin down): a count, then that many sorted masks of the standing pins
   (bit 10 - n for pin n), the leaves func_801206E0_gacode accepts */
extern u16 D_801236C0_gacode[1 + 23];
extern u16 D_801236F0_gacode[1 + 42];
extern u16 D_80123748_gacode[1 + 57];
extern u16 D_801237BC_gacode[1 + 23];
extern u16 D_801237EC_gacode[1 + 1];
extern s32 D_8012C920_gacode;
extern s32 D_8012C924_gacode;          /* frame counter */
extern f32 D_8012C158_gacode[];
extern f32 D_80123850_gacode[][4];     /* unk_98, unk_9C, unk_2C, unk_30 of a player, by D_800DF59C.unk_04[] */
extern s16 D_8012C784_gacode[];
extern vec3_t D_8012C758_gacode;      /* (0, 24, 0) (func_80114898_gacode) */
extern s16 D_8012C7CC_gacode[];      /* sound ids, by D_800A304C[] up to 10 (func_8011582C_gacode) */
/* by score against par; TODO identity: scores below par read before the label, so the table likely starts
   earlier */
extern s16 D_8012C7E6_gacode[];
extern f32 D_801239E8_gacode[11];      /* by gaPlayer_t.unk_93 */
extern u16 D_8012C7F4_gacode[11];      /* image ids, by gaPlayer_t.unk_93 */
extern u16 D_8012C80A_gacode[11];      /* image ids of the numbers 1 to 9, by number ([0] and [10] none) */
extern u16 D_8012C820_gacode[NUM_PINS][2];  /* x and y of each pin in the pin chart (func_80119ACC_gacode) */
extern u16 D_8012C848_gacode[8];       /* image ids, by D_8005C163 */
extern u16 D_8012C858_gacode[NUM_PINS][2];  /* x and y of each pin in the pin panel (func_80119DD8_gacode) */
extern u16 D_8012C880_gacode[NUM_PINS][2];  /* x and y of each pin in the pin editor (func_8011A404_gacode) */
extern u8 D_8012C8A8_gacode;           /* pin editor: the pin under the cursor (0 to 9) */
extern u8 D_8012C8A9_gacode;           /* pin editor: nonzero while the cursor is on the pins */
extern u8 D_8012D238_gacode;           /* current frame of the par game (0 to 9; par in D_80099458[0][]) */
extern f32 D_8012C764_gacode[2];       /* per lane: angle of func_8011C158_gacode's camera */
extern vec3_t D_8012C76C_gacode[2];    /* per lane: position of func_8011BD34_gacode's camera (x and z) */
extern u8 D_8012C7C8_gacode;           /* func_8011C57C_gacode's camera has started (func_801150B4_gacode) */
extern u16 D_8012C8BC_gacode[5];       /* path files of func_8011C254_gacode's camera, late in the throw */
extern u16 D_8012C8C8_gacode[5];       /* path files of func_8011C254_gacode's camera */
extern s16 D_8012C8D4_gacode[10];      /* path files of func_8011C468_gacode's camera */
extern s16 D_8012C8E8_gacode[5];       /* path files of func_8011C57C_gacode's camera */
extern u8 D_8012C8AC_gacode[3][3];     /* channel orders of func_8011A8AC_gacode's aim line colour */
extern u16 D_8012C8B6_gacode;          /* angle of func_8011A8AC_gacode's colour cycle */
extern u8 D_8012C8B8_gacode;           /* channel order of func_8011A8AC_gacode's colour, mod 3 */
extern gaStage_t D_80123EB0_gacode[];
extern gaStage_t *D_8012D220_gacode;
extern objClass_t D_80124050_gacode[12];   /* the classes of the game's scene */
extern u8 D_8012D224_gacode;             /* nonzero: Z steers the rolling ball by the stick (func_801187D0_gacode) */
extern u8 D_8012D225_gacode;            /* nonzero: B commits the throw early (func_8011840C_gacode) */

/* Per lane: state of func_80117124_gacode's aiming. Name invented. */
typedef struct {
    /* 0x00 */ f32 unk_00;
    /* 0x04 */ s16 unk_04;                 /* within 999 either way */
    /* 0x06 */ u8 unk_06;                  /* step reached (func_80116FF0_gacode), 6 at start */
    /* 0x07 */ u8 unk_07;
} gaLaneAim_t;                              /* size 0x08 */

extern gaLaneAim_t D_8012D228_gacode[2];
extern gaPlayer_t D_8012D658_gacode[MAX_PLAYERS];
extern u8 D_8012D651_gacode;
extern u8 D_8012D650_gacode;
/* A page of the pause menu's page browser (D_80123608_gacode, func_80112EBC_gacode). Name invented. */
typedef struct {
    /* 0x00 */ u16 backY;                  /* y of the back item, from the page's top */
    /* 0x02 */ s8 cursor;                  /* item under the cursor; -1: the page has no items */
    /* 0x03 */ s8 backItem;                /* index of the back item (the items before it open pages) */
    /* 0x04 */ u16 image;                  /* image id of the page */
    /* 0x06 */ u8 firstPage;               /* page that item 0 opens; item n opens firstPage + n */
    /* 0x07 */ s8 parent;                  /* page the back item returns to; -1: the pause menu */
} gaMenuPage_t;                              /* size 0x08 */

extern gaMenuPage_t D_80123608_gacode[15];
extern u8 D_80123680_gacode;           /* current page of D_80123608_gacode */
extern u8 D_8012D239_gacode;           /* pause menu: the item under the cursor (0 to 3) */
extern u8 D_8012CDE0_gacode[4];        /* per player (func_801219CC_gacode) */
extern u16 D_8012D8D8_gacode;
extern u8 D_8012D8DA_gacode;
extern gaLane_t D_8012DCF0_gacode[2];
extern gaBody_t D_8012BCF0_gacode[NUM_BODIES];     /* initial bodies of a lane (func_8011641C_gacode) */
extern gaMotion_t D_8012C230_gacode[NUM_BODIES];   /* initial gaLane_t.motion */
extern gaSpot_t D_8012C498_gacode[2][NUM_BODIES];  /* initial gaLane_t.spots and unk_7E8 */
extern gaBody_t D_8012C110_gacode[3];              /* initial gaLane_t.unk_948 */

/* A state of the overlay's main loop; returns nonzero when the state is done. */
typedef s32 (*gaState_t)( void );

extern gaState_t D_8012CDF0_gacode;     /* current state */
extern gaPlayerFunc_t D_8012CDF4_gacode;  /* game mode */
extern gaPlayerFunc_t D_8012D23C_gacode;
extern gaState_t D_8012CDFC_gacode;     /* main loop state of the game mode (func_80113594_gacode) */
extern s16 D_8012CE00_gacode;          /* BGM of the game (func_800379C8) */
extern s64 D_8012CDE8_gacode;           /* sound handle (func_80037870) */
extern u8 D_8012CDF8_gacode[2];         /* per team: index in D_8012CE02_gacode of the current player */
extern s8 D_8012CE02_gacode[2][5];      /* per team: player numbers, -1 ends */
extern u8 D_80123684_gacode[2];         /* per pair of players: the index the turn passes to next (func_80113AD4_gacode) */

/* unit 3C570 */
s32 func_80112AF0_gacode( void );
s32 func_80112D94_gacode( void );
void func_80112E04_gacode( void );
s32 func_80112EBC_gacode( void );
s32 func_80113210_gacode( void );
s32 func_80113270_gacode( void );
s32 func_801132A8_gacode( void );
s32 func_80114108_gacode( void );
void func_80112A74_gacode( void );
void func_801138EC_gacode( u8 *cur, s32 start, s32 team );
void func_801132B0_gacode( void );
void func_80113384_gacode( void );
void func_80113594_gacode( gaState_t state, gaPlayerFunc_t mode );
s32 func_80113968_gacode( void );
s32 func_80113D48_gacode( void );
s32 func_80113AD4_gacode( void );
void func_801136FC_gacode( void );
s32 func_80113EE4_gacode( void );
s32 func_80114024_gacode( void );

/* unit 3E350 */
void func_80114130_gacode( void );
void func_80114140_gacode( u8 arg0, f32 offset, u8 arg2 );
void func_80114550_gacode( s32 player, s32 port, u8 character, u8 lane );
void func_80114138_gacode( void );
s32 func_80114640_gacode( s32 player );
void func_80114EC4_gacode( s32 player );
void func_801150B4_gacode( s32 player );
void func_80114898_gacode( s32 player, s32 port );
void func_8011A26C_gacode( void );
void func_801144C4_gacode( u8 lastLane );
void func_80114DD4_gacode( gaPlayer_t *pl );
s32 func_80115ED4_gacode( u8 twoLanes );
void func_80114CB4_gacode( gaBody_t *body, gaSpot_t *spot, sceneObject_t *obj, f32 offset );
void func_80114FC4_gacode( s32 player );
void func_80114FFC_gacode( s32 player );
s32 func_801157F8_gacode( s32 player );
s32 func_80115CDC_gacode( s32 player );
s32 func_80115D38_gacode( s32 player );
void func_80115D70_gacode( s32 player, u8 arg1 );
s32 func_80115FCC_gacode( void );
void func_80116170_gacode( s32 skip );
s32 func_80115DF4_gacode( s32 player );
s32 func_80115E24_gacode( u8 twoLanes );
s32 func_801155BC_gacode( s32 player );
s32 func_801151A0_gacode( s32 player );
s32 func_8011582C_gacode( s32 player );
s32 func_801156D8_gacode( s32 player );
void func_80115054_gacode( void );
s32 func_80115B9C_gacode( s32 player );
void func_80116248_gacode( s32 player );
void func_80116398_gacode( s32 player );
void func_801163A0_gacode( s32 player );
void func_80116414_gacode( s32 player );
void func_8011641C_gacode( s32 player, u8 *pins );
void func_801167C0_gacode( s32 player, gaPlayer_t *pl, u8 strong );
void func_80116750_gacode( gaLane_t *lane );
void func_80116798_gacode( gaLane_t *lane );
void func_8011669C_gacode( gaPlayer_t *pl );
s32 func_801168D0_gacode( gaPlayer_t *pl );
s32 func_80116998_gacode( gaPlayer_t *pl );
void func_801169F8_gacode( s32 player, f32 *value, s16 delta );
void func_80116A64_gacode( gaPlayer_t *pl, s16 stickX, s16 stickY );
s32 func_801178D8_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_801179FC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80117ABC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80117124_gacode( s32 player, gaPlayer_t *pl );
s32 func_80117BDC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80117D20_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80117EFC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118014_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118270_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_8011840C_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118524_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80117AE0_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_801189C4_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_801187D0_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118CCC_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118D9C_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118E04_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118EA0_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118F08_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118F38_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
s32 func_80118F68_gacode( s32 player, s32 input, s16 stickX, s16 stickY );
void func_8011917C_gacode( s32 player, u8 lane );
void func_80119328_gacode( gaPlayer_t *pl );
void func_801198E4_gacode( s32 player, u8 lane );
void func_80119ACC_gacode( s32 player, u8 lane );
void func_80119C50_gacode( s32 player, u8 lane );
void func_80119DD8_gacode( s32 player, u8 lane );
void func_80119F8C_gacode( s32 player, u8 lane );
void func_8011A404_gacode( gaPlayer_t *pl );
void func_8011A8AC_gacode( gaPlayer_t *pl );
void func_8011AA78_gacode( s32 player );
void func_801194D0_gacode( gaPlayer_t *pl );
void func_80119638_gacode( gaPlayer_t *pl );
void func_80119800_gacode( s32 player, u8 full );
void func_8011AB0C_gacode( s32 player );
void func_8011AD5C_gacode( s32 player );
void func_8011B07C_gacode( s32 player );
void func_8011B1A0_gacode( s32 player );
void func_8011B2E8_gacode( s32 player );
void func_8011B400_gacode( s32 player );
void func_8011B408_gacode( s32 player );
void func_8011B564_gacode( s32 player );
void func_8011B5C8_gacode( s32 lane );
void func_8011B634_gacode( gaPlayer_t *pl, s32 view );
void func_8011B8AC_gacode( s32 player, gaCamera_t *camera );
void func_8011B974_gacode( s32 player, gaCamera_t *camera );
void func_8011BA00_gacode( s32 player, gaCamera_t *camera );
void func_8011BB44_gacode( s32 player, gaCamera_t *camera );
void func_8011BA8C_gacode( s32 player, gaCamera_t *camera );
void func_8011BC10_gacode( s32 player, gaCamera_t *camera );
void func_8011BCA8_gacode( s32 player, gaCamera_t *camera );
void func_8011BD34_gacode( s32 player, gaCamera_t *camera );
void func_8011C0DC_gacode( s32 player, gaCamera_t *camera );
void func_8011C158_gacode( s32 player, gaCamera_t *camera );
void func_8011C254_gacode( s32 player, gaCamera_t *camera );
void func_8011C468_gacode( s32 player, gaCamera_t *camera );
void func_8011C650_gacode( s32 player, gaCamera_t *camera );
s32 func_8011C658_gacode( gaPlayer_t *pl, s32 player );
void func_8011B490_gacode( s32 lane );
void func_8011C57C_gacode( s32 player, gaCamera_t *camera );
void func_8011CEE4_gacode( gaPlayer_t *pl, s32 player );
void func_8011C93C_gacode( gaPlayer_t *pl, s32 player );
void func_8011CF30_gacode( s32 player );

/* unit 474E0 */
s32 func_801203AC_gacode( vec3_t out, gaBody_t *bodies, gaSpot_t *spots, s32 side );
void func_8011D2C0_gacode( f32 mf[4][4], vec3_t pos, vec3_t rot, f32 scale );
void func_8011D3A0_gacode( vec3_t out, vec3_t from, vec3_t to );
void func_8011D304_gacode( vec3_t out, vec3_t in, vec3_t rot, vec3_t pos );
f32 func_8011D3D4_gacode( vec3_t v );
f32 func_8011D424_gacode( vec3_t out, vec3_t in );
f32 func_8011D4E4_gacode( vec3_t out, vec3_t from, vec3_t to );
void func_8011D7CC_gacode( vec3_t v, gaBody_t *ball );
s32 func_8011D9CC_gacode( gaSpot_t *a, gaSpot_t *b );
s32 func_8011DB54_gacode( gaSpot_t *spot, vec3_t from, vec3_t to, f32 radius );
f32 func_8011DCB8_gacode( vec3_t vel, gaSpot_t *a, gaSpot_t *b, vec3_t normal );
void func_8011E014_gacode( gaMotion_t *a, gaMotion_t *b, gaSpot_t *sa, gaSpot_t *sb );
f32 func_8011E12C_gacode( gaMotion_t **out, gaMotion_t **in, gaSpot_t **spots );
s32 func_8011E504_gacode( u8 kind, gaMotion_t *motion, gaBody_t *body, u8 flags );
f32 func_8011D5B8_gacode( vec3_t a, vec3_t b );
void func_8011D6C4_gacode( vec3_t out, vec3_t a, vec3_t b );
s32 func_8011E63C_gacode( f32 *value, f32 *speed, f32 limit );
s32 func_8011E6F8_gacode( s32 player, gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots, u8 lane );
void func_8011F334_gacode( gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots );
void func_8012016C_gacode( gaSpot_t *spot, gaSpot_t *spot2, gaMotion_t *motion, f32 ceiling );
s32 func_8011F4CC_gacode( gaMotion_t *motion, gaSpot_t *spot, gaSpot_t *spot2, f32 ceiling, f32 arg4 );
f32 func_8011F87C_gacode( f32 x, f32 z, vec3_t normal );
s32 func_8011FD08_gacode( gaMotion_t *motion, gaSpot_t *spot, vec3_t normal );
f32 func_801205A0_gacode( vec3_t pos, vec3_t vel, f32 x, f32 scale );
s32 func_80120680_gacode( u16 key, s32 count, u16 *table );
u8 func_801206E0_gacode( gaLane_t *lane );
void func_80120980_gacode( u8 *pins, u16 mask );
void func_801209C0_gacode( u8 *order, u8 count, u8 first );
void func_80120A7C_gacode( gaSprite_t *sprite, u16 id, u16 first, u16 last );
void func_80120B10_gacode( gaSprite_t *sprite );
void func_80120B50_gacode( gaSprite_t *sprite, u16 color, u16 index );
void func_80120C0C_gacode( gaSprite_t *sprite, u16 color, f32 t );
u64 func_80120D30_gacode( u8 lane, s32 id );
void func_80120D7C_gacode( s64 *handle );
void func_80120DD0_gacode( void );
void func_80120DD8_gacode( u8 arg0 );
void func_801213B4_gacode( u8 arg0 );
void func_801219CC_gacode( u8 arg0 );
void func_80121E34_gacode( void );
void func_80121FD0_gacode( u8 arg0, u8 arg1 );
s32 func_80122060_gacode( u8 player, s32 done );
s32 func_80122140_gacode( u8 player, s32 force );
void func_8012224C_gacode( u8 frame );

/* unit 4C560 */
void func_80122340_gacode( void );
void func_8012239C_gacode( s32 set );
void func_80122470_gacode( void );
void func_80122BD8_gacode( void );
void func_80122C60_gacode( void );
void func_801233A8_gacode( void );
void func_80123430_gacode( s32 lane, s32 alt, s32 index );

#endif
