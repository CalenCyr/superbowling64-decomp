#ifndef GACODE_GACODE_H
#define GACODE_GACODE_H

/* gacode.h -- the game overlay (file GACODE  SYS, ROM 0x3C570, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
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
extern u8 D_800DF595[2];            /* per lane: state of the ball (GA_PublishBall) */

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
    /* 0x5D */ u8 standState;               /* 1: still standing (pins) */
    /* 0x5E */ u8 unk_5E[2];
} gaBody_t;                             /* size 0x60 */

/* Where a body of a lane stands; GA_PlaceBody places the body there. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t pos;
    /* 0x0C */ vec3_t unk_0C;
    /* 0x18 */ f32 unk_18;
    /* 0x1C */ u32 unk_1C;              /* contact bits of GA_CollideLane: bit 0 hit this frame, bit n touching spots[n] */
} gaSpot_t;                             /* size 0x20 */

/* View of a lane, set each frame by the lane's cameraFunc handler. Name invented. */
typedef struct gaCamera_s {
    /* 0x00 */ vec3_t eye;
    /* 0x0C */ vec3_t at;
    /* 0x18 */ vec3_t unk_18;
    /* 0x24 */ s32 unk_24;
} gaCamera_t;

#define NUM_BODIES      11              /* the ball, then the ten pins */
#define NUM_PINS        10

/* Motion of a body of a lane, stepped each frame by GA_PhysicsStep. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t vel;              /* added to the body's spot each frame; the ball's is turned into its spin
                                           by GA_SetBallHeading */
    /* 0x0C */ vec3_t spin;             /* added to the body's rot each frame (degrees, kept within -180..180) */
    /* 0x18 */ u8 unk_18[0x18];
    /* 0x30 */ f32 unk_30;              /* the ball's from D_801239E8_gacode, by gaPlayer_t.unk_93 */
    /* 0x34 */ u32 flags;
} gaMotion_t;                           /* size 0x38 */

/* Absolute value of a float, as 0 - x. */
#define FABS( x )       ( ( x ) < 0.0f ? 0.0f - ( x ) : ( x ) )

/* One of the two lanes (ga_lanes), shared by the players bowling on it. Name invented. */
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
    /* 0xB04 */ void (*turnFunc)( s32 player );
    /* 0xB08 */ void (*panelFunc)( s32 player );
    /* 0xB0C */ void (*cameraFunc)( s32 player, gaCamera_t *camera );
    /* 0xB10 */ s32 (*inputFunc)( s32 player, s32 input, s16 stickX, s16 stickY );
    /* 0xB14 */ f32 unk_B14;
    /* 0xB18 */ s64 soundHandle;            /* sound handle (Sound_PlaySe) */
    /* 0xB20 */ s16 unk_B20;
    /* 0xB22 */ u8 unk_B22;
    /* 0xB23 */ u8 unk_B23;
    /* 0xB24 */ u8 unk_B24;
    /* 0xB25 */ u8 unk_B25;
    /* 0xB26 */ u8 unk_B26;
    /* 0xB27 */ u8 unk_B27;
    /* 0xB28 */ f32 cameraRaise;            /* camera height offset of func_8011B8AC_gacode */
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

/* Per character (the last two empty): file indices, [0] and [1] the player's scene object's (GA_Init) */
extern s16 D_80123A14_gacode[10][7];
extern gaCharacterData_t ga_characterData[10];   /* per character; unk_00 points to its row of D_80123A14_gacode */

/* Character record (ga_characters), copied into gaPlayer_t. Name invented. */
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
    /* 0x38 */ vec3_t unk_38;            /* GA_VecDirection of unk_20 and unk_14 */
    /* 0x44 */ f32 unk_44;
} gaCharacter_t;                        /* size 0x48 */

extern gaCharacter_t ga_characters[];

#define MAX_PLAYERS     4

/* A player (ga_players). Name invented. */
typedef struct {
    /* 0x00 */ gaCharacter_t character;
    /* 0x48 */ f32 unk_48[4][4];        /* copy of obj->mtx */
    /* 0x88 */ sceneObject_t *obj;
    /* 0x8C */ gaLane_t *lane;
    /* 0x90 */ s8 port;                 /* controller port; -1: played by the game */
    /* 0x91 */ u8 laneIndex;
    /* 0x92 */ s8 ballView;               /* 0 to 4, stepped by func_8011BB44_gacode */
    /* 0x93 */ u8 unk_93;               /* 0 to 10, chosen with C-left and C-right (func_801194D0_gacode) */
    /* 0x94 */ u16 unk_94;
    /* 0x96 */ u8 unk_96[0x02];
    /* 0x98 */ f32 unk_98;
    /* 0x9C */ f32 unk_9C;
} gaPlayer_t;                           /* size 0xA0 */

/* Configuration of a stage (ga_stages); ga_stage points to the current one. Name invented. */
typedef struct {
    /* 0x00 */ objParams_t *unk_00;      /* the scene (Obj_CreateAll); one object per player first */
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
extern u16 ga_lanePaletteImages[][4];      /* image files of GA_LoadLanePalettes, by its index */
/* image files of GA_AnimateTextures's animated palettes, by animation frame */
extern u16 D_8012C928_gacode[11];
extern u16 D_8012C940_gacode[11];
extern u16 D_8012C958_gacode[2];
extern u16 D_8012C95C_gacode[2];
extern u16 D_8012C960_gacode[2];
extern u16 D_8012C964_gacode[9];
extern void *ga_savedTexels[2];      /* texels of resources 1523 and 1524, saved by GA_AnimateTextures */
/* image files of GA_FlickerLight's four palette slots, by brightness step */
extern u16 D_8012C978_gacode[4];
extern u16 D_8012C980_gacode[4];
extern u16 D_8012C988_gacode[4];
extern u16 D_8012C990_gacode[4];
extern u16 ga_flickerTimer;           /* frames to the next flicker of GA_FlickerLight */
extern u16 ga_flickerSoundTimer;           /* frames to GA_FlickerLight's flicker sound */
extern gaSprite_t D_8012D8E0_gacode[2];   /* per lane */
extern gaSprite_t D_8012D240_gacode[2];   /* per lane (GA_SetupLanes) */
extern gaSprite_t D_8012CE10_gacode[2];   /* per lane (GA_SetupLanes) */

/* A pin setup of a game mode, by frame (D_801237F0_gacode, D_80123950_gacode). Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ u16 pins;                /* pins standing, pin 1 in bit 9 (GA_UnpackPinMask) */
    /* 0x04 */ u16 unk_04;
} gaPinSetup_t;                         /* size 0x6 */

extern gaPinSetup_t D_801237F0_gacode[];   /* for func_801156D8_gacode, by D_800DF59C.unk_04[] */
extern gaPinSetup_t D_80123950_gacode[];   /* for func_801155BC_gacode, by D_800DF59C.unk_04[] */
extern u16 ga_editorPins;              /* pins of func_801157F8_gacode */

/* Handler of a player: the game mode (ga_gameMode) or what starts a turn (D_8012D23C_gacode). */
typedef s32 (*gaPlayerFunc_t)( s32 player );

extern s16 ga_blinkLevel;          /* blink level, 0 to 0x80 by 8 (GA_Frame) */
extern s8 ga_blinkFalling;           /* 1: ga_blinkLevel falls, else it rises */
extern u8 ga_gameDone;           /* what the main loop state returned: nonzero, the game is done */
extern u8 ga_leaveGame;           /* what the pause menu's state returned: nonzero, leave the game */
extern u8 ga_pinRows[2][NUM_PINS];  /* per side: the pins (1 to 10) from the back row forward (GA_PickAimPin) */
/* By 2 to 6 standing pins (head pin down): a count, then that many sorted masks of the standing pins
   (bit 10 - n for pin n), the leaves GA_IsSplit accepts */
extern u16 ga_splits2[1 + 23];
extern u16 ga_splits3[1 + 42];
extern u16 ga_splits4[1 + 57];
extern u16 ga_splits5[1 + 23];
extern u16 ga_splits6[1 + 1];
extern s32 D_8012C920_gacode;
extern s32 ga_frameCount;          /* frame counter */
extern f32 D_8012C158_gacode[];
extern f32 ga_cpuThrows[][4];     /* unk_98, unk_9C, unk_2C, unk_30 of a player, by D_800DF59C.unk_04[] */
extern s16 D_8012C784_gacode[];
extern vec3_t D_8012C758_gacode;      /* (0, 24, 0) (GA_PhysicsStep) */
extern s16 ga_runSounds[];      /* sound ids, by D_800A304C[] up to 10 (func_8011582C_gacode) */
/* by score against par; TODO identity: scores below par read before the label, so the table likely starts
   earlier */
extern s16 D_8012C7E6_gacode[];
extern f32 D_801239E8_gacode[11];      /* by gaPlayer_t.unk_93 */
extern u16 D_8012C7F4_gacode[11];      /* image ids, by gaPlayer_t.unk_93 */
extern u16 ga_numberImages[11];      /* image ids of the numbers 1 to 9, by number ([0] and [10] none) */
extern u16 ga_parChartPins[NUM_PINS][2];  /* x and y of each pin in the pin chart (GA_DrawParFramePanel) */
extern u16 D_8012C848_gacode[8];       /* image ids, by D_8005C163 */
extern u16 ga_pinPanelPins[NUM_PINS][2];  /* x and y of each pin in the pin panel (func_80119DD8_gacode) */
extern u16 ga_editorPinPos[NUM_PINS][2];  /* x and y of each pin in the pin editor (GA_PinEditor) */
extern u8 ga_editorCursor;           /* pin editor: the pin under the cursor (0 to 9) */
extern u8 ga_editorOnPins;           /* pin editor: nonzero while the cursor is on the pins */
extern u8 ga_parFrame;           /* current frame of the par game (0 to 9; par in D_80099458[0][]) */
extern f32 ga_spinCameraAngle[2];       /* per lane: angle of func_8011C158_gacode's camera */
extern vec3_t ga_freeCameraPos[2];    /* per lane: position of GA_FreeCamera's camera (x and z) */
extern u8 D_8012C7C8_gacode;           /* func_8011C57C_gacode's camera has started (GA_UpdateLaneCamera) */
extern u16 ga_throwCamPathsLate[5];       /* path files of func_8011C254_gacode's camera, late in the throw */
extern u16 ga_throwCamPaths[5];       /* path files of func_8011C254_gacode's camera */
extern s16 ga_followCamPaths[10];      /* path files of func_8011C468_gacode's camera */
extern s16 ga_laneCamPaths[5];       /* path files of func_8011C57C_gacode's camera */
extern u8 ga_aimChannelOrders[3][3];     /* channel orders of GA_DrawAimLine's aim line colour */
extern u16 ga_aimColorAngle;          /* angle of GA_DrawAimLine's colour cycle */
extern u8 ga_aimChannelOrder;           /* channel order of GA_DrawAimLine's colour, mod 3 */
extern gaStage_t ga_stages[];
extern gaStage_t *ga_stage;
extern objClass_t ga_classes[12];   /* the classes of the game's scene */
extern u8 ga_steerBall;             /* nonzero: Z steers the rolling ball by the stick (func_801187D0_gacode) */
extern u8 ga_earlyCommit;            /* nonzero: B commits the throw early (func_8011840C_gacode) */

/* Per lane: state of func_80117124_gacode's aiming. Name invented. */
typedef struct {
    /* 0x00 */ f32 unk_00;
    /* 0x04 */ s16 unk_04;                 /* within 999 either way */
    /* 0x06 */ u8 unk_06;                  /* step reached (func_80116FF0_gacode), 6 at start */
    /* 0x07 */ u8 unk_07;
} gaLaneAim_t;                              /* size 0x08 */

extern gaLaneAim_t ga_laneAim[2];
extern gaPlayer_t ga_players[MAX_PLAYERS];
extern u8 D_8012D651_gacode;
extern u8 D_8012D650_gacode;
/* A page of the pause menu's page browser (ga_menuPages, GA_PageBrowser). Name invented. */
typedef struct {
    /* 0x00 */ u16 backY;                  /* y of the back item, from the page's top */
    /* 0x02 */ s8 cursor;                  /* item under the cursor; -1: the page has no items */
    /* 0x03 */ s8 backItem;                /* index of the back item (the items before it open pages) */
    /* 0x04 */ u16 image;                  /* image id of the page */
    /* 0x06 */ u8 firstPage;               /* page that item 0 opens; item n opens firstPage + n */
    /* 0x07 */ s8 parent;                  /* page the back item returns to; -1: the pause menu */
} gaMenuPage_t;                              /* size 0x08 */

extern gaMenuPage_t ga_menuPages[15];
extern u8 ga_menuPage;           /* current page of ga_menuPages */
extern u8 ga_pauseCursor;           /* pause menu: the item under the cursor (0 to 3) */
extern u8 D_8012CDE0_gacode[4];        /* per player (GA_ScoreThirdBall) */
extern u16 D_8012D8D8_gacode;
extern u8 D_8012D8DA_gacode;
extern gaLane_t ga_lanes[2];
extern gaBody_t ga_initBodies[NUM_BODIES];     /* initial bodies of a lane (GA_ResetLane) */
extern gaMotion_t ga_initMotion[NUM_BODIES];   /* initial gaLane_t.motion */
extern gaSpot_t ga_initSpots[2][NUM_BODIES];  /* initial gaLane_t.spots and unk_7E8 */
extern gaBody_t D_8012C110_gacode[3];              /* initial gaLane_t.unk_948 */

/* A state of the overlay's main loop; returns nonzero when the state is done. */
typedef s32 (*gaState_t)( void );

extern gaState_t ga_state;     /* current state */
extern gaPlayerFunc_t ga_gameMode;  /* game mode */
extern gaPlayerFunc_t D_8012D23C_gacode;
extern gaState_t ga_modeLoop;     /* main loop state of the game mode (GA_StartMode) */
extern s16 ga_bgm;          /* BGM of the game (Sound_PlayBgm) */
extern s64 ga_sound;           /* sound handle (Sound_PlaySe) */
extern u8 ga_teamCurrent[2];         /* per team: index in ga_teamPlayers of the current player */
extern s8 ga_teamPlayers[2][5];      /* per team: player numbers, -1 ends */
extern u8 ga_pairNext[2];         /* per pair of players: the index the turn passes to next (GA_PairGameLoop) */

/* unit 3C570 */
s32 GA_PauseMenu( void );
s32 func_80112D94_gacode( void );
void func_80112E04_gacode( void );
s32 GA_PageBrowser( void );
s32 func_80113210_gacode( void );
s32 func_80113270_gacode( void );
s32 func_801132A8_gacode( void );
s32 func_80114108_gacode( void );
void GA_NextTeamPlayer( u8 *cur, s32 start, s32 team );
void GA_SetupMode( void );
void func_80113384_gacode( void );
void GA_StartMode( gaState_t state, gaPlayerFunc_t mode );
s32 GA_TeamGameLoop( void );
s32 GA_ParGameLoop( void );
s32 GA_PairGameLoop( void );
void GA_FillTeams( void );
s32 GA_DuelLoop( void );
s32 func_80114024_gacode( void );

/* unit 3E350 */
void func_80114130_gacode( void );
void GA_SetupLanes( u8 arg0, f32 offset, u8 arg2 );
void GA_SetupPlayer( s32 player, s32 port, u8 character, u8 lane );
void func_80114138_gacode( void );
s32 GA_RunPlayer( s32 player );
void GA_HoldBall( s32 player );
void GA_UpdateLaneCamera( s32 player );
void GA_PhysicsStep( s32 player, s32 port );
void GA_DrawDistancePanels( void );
void GA_StopLanes( u8 lastLane );
void GA_PublishBall( gaPlayer_t *pl );
s32 func_80115ED4_gacode( u8 twoLanes );
void GA_PlaceBody( gaBody_t *body, gaSpot_t *spot, sceneObject_t *obj, f32 offset );
void func_80114FC4_gacode( s32 player );
void func_80114FFC_gacode( s32 player );
s32 func_801157F8_gacode( s32 player );
s32 func_80115CDC_gacode( s32 player );
s32 func_80115D38_gacode( s32 player );
void func_80115D70_gacode( s32 player, u8 arg1 );
s32 GA_EndDuelGame( void );
void func_80116170_gacode( s32 skip );
s32 func_80115DF4_gacode( s32 player );
s32 func_80115E24_gacode( u8 twoLanes );
s32 func_801155BC_gacode( s32 player );
s32 func_801151A0_gacode( s32 player );
s32 func_8011582C_gacode( s32 player );
s32 func_801156D8_gacode( s32 player );
void func_80115054_gacode( void );
s32 GA_EndTurn( s32 player );
void GA_StartTurn( s32 player );
void func_80116398_gacode( s32 player );
void func_801163A0_gacode( s32 player );
void func_80116414_gacode( s32 player );
void GA_ResetLane( s32 player, u8 *pins );
void GA_SoundThrowResult( s32 player, gaPlayer_t *pl, u8 strong );
void func_80116750_gacode( gaLane_t *lane );
void GA_HidePins( gaLane_t *lane );
void GA_SetTurnHandlers( gaPlayer_t *pl );
s32 func_801168D0_gacode( gaPlayer_t *pl );
s32 func_80116998_gacode( gaPlayer_t *pl );
void GA_StepClamped( s32 player, f32 *value, s16 delta );
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
void GA_DrawScorePanel( s32 player, u8 lane );
void GA_DrawPinDiagram( gaPlayer_t *pl );
void GA_DrawParPanel( s32 player, u8 lane );
void GA_DrawParFramePanel( s32 player, u8 lane );
void func_80119C50_gacode( s32 player, u8 lane );
void func_80119DD8_gacode( s32 player, u8 lane );
void func_80119F8C_gacode( s32 player, u8 lane );
void GA_PinEditor( gaPlayer_t *pl );
void GA_DrawAimLine( gaPlayer_t *pl );
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
void GA_SetBallView( gaPlayer_t *pl, s32 view );
void func_8011B8AC_gacode( s32 player, gaCamera_t *camera );
void func_8011B974_gacode( s32 player, gaCamera_t *camera );
void func_8011BA00_gacode( s32 player, gaCamera_t *camera );
void func_8011BB44_gacode( s32 player, gaCamera_t *camera );
void func_8011BA8C_gacode( s32 player, gaCamera_t *camera );
void func_8011BC10_gacode( s32 player, gaCamera_t *camera );
void func_8011BCA8_gacode( s32 player, gaCamera_t *camera );
void GA_FreeCamera( s32 player, gaCamera_t *camera );
void func_8011C0DC_gacode( s32 player, gaCamera_t *camera );
void func_8011C158_gacode( s32 player, gaCamera_t *camera );
void func_8011C254_gacode( s32 player, gaCamera_t *camera );
void func_8011C468_gacode( s32 player, gaCamera_t *camera );
void func_8011C650_gacode( s32 player, gaCamera_t *camera );
s32 GA_CpuInput( gaPlayer_t *pl, s32 player );
void func_8011B490_gacode( s32 lane );
void func_8011C57C_gacode( s32 player, gaCamera_t *camera );
void func_8011CEE4_gacode( gaPlayer_t *pl, s32 player );
void GA_CpuSetup( gaPlayer_t *pl, s32 player );
void GA_DebugText( s32 player );

/* unit 474E0 */
s32 GA_PickAimPin( vec3_t out, gaBody_t *bodies, gaSpot_t *spots, s32 side );
void GA_BodyMatrix( f32 mf[4][4], vec3_t pos, vec3_t rot, f32 scale );
void GA_VecSub( vec3_t out, vec3_t from, vec3_t to );
void GA_TransformPoint( vec3_t out, vec3_t in, vec3_t rot, vec3_t pos );
f32 GA_VecLength( vec3_t v );
f32 GA_VecNormalize( vec3_t out, vec3_t in );
f32 GA_VecDirection( vec3_t out, vec3_t from, vec3_t to );
void GA_SetBallHeading( vec3_t v, gaBody_t *ball );
s32 GA_SpotsOverlap( gaSpot_t *a, gaSpot_t *b );
s32 GA_SegmentHitsSpot( gaSpot_t *spot, vec3_t from, vec3_t to, f32 radius );
f32 GA_SpotContact( vec3_t vel, gaSpot_t *a, gaSpot_t *b, vec3_t normal );
void GA_CollideBodies( gaMotion_t *a, gaMotion_t *b, gaSpot_t *sa, gaSpot_t *sb );
f32 GA_CollideMomentum( gaMotion_t **out, gaMotion_t **in, gaSpot_t **spots );
s32 GA_TipBody( u8 kind, gaMotion_t *motion, gaBody_t *body, u8 flags );
f32 GA_VecCosAngle( vec3_t a, vec3_t b );
void GA_VecCrossNormal( vec3_t out, vec3_t a, vec3_t b );
s32 GA_ApproachLimit( f32 *value, f32 *speed, f32 limit );
s32 GA_CollideLane( s32 player, gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots, u8 lane );
void GA_SettleBodies( gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots );
void GA_KeepInGrid( gaSpot_t *spot, gaSpot_t *spot2, gaMotion_t *motion, f32 ceiling );
s32 GA_KeepOnFloor( gaMotion_t *motion, gaSpot_t *spot, gaSpot_t *spot2, f32 ceiling, f32 arg4 );
f32 GA_FloorHeight( f32 x, f32 z, vec3_t normal );
s32 GA_SlopeContact( gaMotion_t *motion, gaSpot_t *spot, vec3_t normal );
f32 func_801205A0_gacode( vec3_t pos, vec3_t vel, f32 x, f32 scale );
s32 GA_BinarySearch( u16 key, s32 count, u16 *table );
u8 GA_IsSplit( gaLane_t *lane );
void GA_UnpackPinMask( u8 *pins, u16 mask );
void GA_ShuffleOrder( u8 *order, u8 count, u8 first );
void GA_SetupSprite( gaSprite_t *sprite, u16 id, u16 first, u16 last );
void GA_RestorePalette( gaSprite_t *sprite );
void GA_FillPalette( gaSprite_t *sprite, u16 color, u16 index );
void GA_FillPaletteFraction( gaSprite_t *sprite, u16 color, f32 t );
u64 GA_PlayLaneSound( u8 lane, s32 id );
void GA_StopLaneSound( s64 *handle );
void func_80120DD0_gacode( void );
void GA_ScoreFirstBall( u8 arg0 );
void GA_ScoreSecondBall( u8 arg0 );
void GA_ScoreThirdBall( u8 arg0 );
void GA_ClearScores( void );
void func_80121FD0_gacode( u8 arg0, u8 arg1 );
s32 GA_EndFrame( u8 player, s32 done );
s32 GA_CountParThrow( u8 player, s32 force );
void GA_DecideFrame( u8 frame );

/* unit 4C560 */
void GA_CountFrame( void );
void GA_SetPlayerObjectFlags( s32 set );
void func_80122470_gacode( void );
void func_80122BD8_gacode( void );
void func_80122C60_gacode( void );
void func_801233A8_gacode( void );
void GA_LoadLanePalettes( s32 lane, s32 alt, s32 index );

#endif
