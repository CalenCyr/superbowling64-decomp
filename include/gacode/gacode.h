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

extern f32 game_ballPos[2][3];        /* per lane: position of the ball */
extern u8 game_ballState[2];            /* per lane: state of the ball (GA_PublishBall) */

/* Main's game setup (game_setup): only the members the overlay uses. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[4];
    /* 0x04 */ u8 frameSetups[0x30];   /* indexed by frame */
} gameSetup_t;                       /* size 0x34 */

extern gameSetup_t game_setup;

/* Image of main's resource table: a header, then a 256-colour palette. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x07];
    /* 0x07 */ u8 alpha;
    /* 0x08 */ u8 unk_08[0x08];
    /* 0x10 */ u16 palette[256];
} unkImage_t;


/*
==============================================================================

OVERLAY

==============================================================================
*/

typedef f32 vec3_t[3];


/* Ball or pin of a lane. Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x40];
    /* 0x40 */ vec3_t pos;
    /* 0x4C */ vec3_t rot;
    /* 0x58 */ f32 scale;
    /* 0x5C */ u8 kind;                 /* 0: the ball, 1: a pin, 2: flat (GA_PlaceBody) */
    /* 0x5D */ u8 standState;               /* 1: still standing (pins) */
    /* 0x5E */ u8 unk_5E[2];
} gaBody_t;                             /* size 0x60 */

/* Where a body of a lane stands; GA_PlaceBody places the body there. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t pos;
    /* 0x0C */ vec3_t floorNormal;      /* of the slope the spot last slid on (GA_SlopeContact) */
    /* 0x18 */ f32 radius;              /* of the spot's sphere; 0: no such spot */
    /* 0x1C */ u32 contacts;            /* contact bits of GA_CollideLane: bit 0 hit this frame, bit n touching spots[n] */
} gaSpot_t;                             /* size 0x20 */

#define NUM_BODIES      11              /* the ball, then the ten pins */
#define NUM_PINS        10

/* Motion of a body of a lane, stepped each frame by GA_PhysicsStep. Name invented. */
typedef struct {
    /* 0x00 */ vec3_t vel;              /* added to the body's spot each frame; the ball's is turned into its spin
                                           by GA_SetBallHeading */
    /* 0x0C */ vec3_t spin;             /* added to the body's rot each frame (degrees, kept within -180..180) */
    /* 0x18 */ f32 unk_18[6];
    /* 0x30 */ f32 mass;                /* the ball's from ga_ballMasses, by gaPlayer_t.ballWeight */
    /* 0x34 */ u32 flags;
} gaMotion_t;                           /* size 0x38 */

/* Absolute value of a float, as 0 - x. */
#define FABS( x )       ( ( x ) < 0.0f ? 0.0f - ( x ) : ( x ) )

/* One of the two lanes (ga_lanes), shared by the players bowling on it. Name invented. */
typedef struct {
    /* 0x000 */ gaBody_t bodies[NUM_BODIES];
    /* 0x420 */ gaMotion_t motion[NUM_BODIES];
    /* 0x688 */ gaSpot_t spots[NUM_BODIES];
    /* 0x7E8 */ gaSpot_t secondSpots[NUM_BODIES]; /* each body's second sphere; radius 0: none */
    /* 0x948 */ gaBody_t markers[3];        /* the throw's markers; [2] the aim marker */
    /* 0xA68 */ gaSpot_t markerSpots[3];    /* [0] the aim, [1] the stance, [2] the aim marker */
    /* 0xAC8 */ sceneObject_t *bodyObjs[NUM_BODIES];
    /* 0xAF4 */ sceneObject_t *markerObjs[3];
    /* 0xB00 */ sceneCamera_t *camera;  /* main's scene camera obj_scene[lane] (Cam_Create) */
    /* 0xB04 */ void (*turnFunc)( s32 player );
    /* 0xB08 */ void (*panelFunc)( s32 player );
    /* 0xB0C */ void (*cameraFunc)( s32 player, sceneCamera_t *camera );
    /* 0xB10 */ s32 (*inputFunc)( s32 player, s32 input, s16 stickX, s16 stickY );
    /* 0xB14 */ f32 offsetX;                /* x of the lane (lane 1: GA_SetupLanes' offset) */
    /* 0xB18 */ s64 soundHandle;            /* sound handle (Sound_PlaySe) */
    /* 0xB20 */ s16 timer;                  /* frames waited by the input handlers */
    /* 0xB22 */ u8 pinsHit;                 /* pins knocked down by this throw */
    /* 0xB23 */ u8 pinsDown;                /* pins down after it; 10 ends the frame */
    /* 0xB24 */ u8 frameOver;               /* the throw ended the frame (GA_EndTurn) */
    /* 0xB25 */ u8 callout;                 /* image shown over the lane, index into ga_calloutImages; 0: none */
    /* 0xB26 */ u8 reaction;                /* the character's reaction animation (files[reaction + 2]) */
    /* 0xB27 */ u8 unk_B27;
    /* 0xB28 */ f32 cameraRaise;            /* camera height offset of GA_CameraRaise */
    /* 0xB2C */ s16 bounceSound;            /* of the ball's floor hits: gaStage_t.sounds[0], [1] for a curving throw */
    /* 0xB2E */ u8 unk_B2E[2];
} gaLane_t;                             /* size 0xB30 */

/* What gaCharacter_t.data points to. Name invented. */
typedef struct {
    /* 0x00 */ s16 *fileIndices;            /* file indices */
    /* 0x04 */ f32 unk_04;                  /* objParams_t.args[4] of the player's scene object */
    /* 0x08 */ s8 ballJoint;                /* +-: a joint of the player's object */
    /* 0x09 */ u8 unk_09[0x03];
    /* 0x0C */ vec3_t ballOffset;           /* offset of the ball from that joint */
    /* 0x18 */ u8 timingStartFrame;
    /* 0x19 */ u8 timingEndFrame;
    /* 0x1A */ u8 releaseFrame;
    /* 0x1B */ u8 reactionFrames[5];    /* indexed by gaLane_t.reaction */
} gaCharacterData_t;                        /* size 0x20 */

/* Per character (the last two empty): file indices, [0] and [1] the player's scene object's (GA_Init) */
extern s16 ga_characterFiles[10][7];
extern gaCharacterData_t ga_characterData[10];   /* per character; fileIndices points to its row of ga_characterFiles */

/* Character record (ga_characters), copied into gaPlayer_t. Name invented. */
typedef struct {
    /* 0x00 */ gaCharacterData_t *data;
    /* 0x04 */ u8 ballWeight;           /* copied into gaPlayer_t.ballWeight */
    /* 0x05 */ u8 reverseSpin;          /* nonzero: spin runs from -maxSpin to 0, else from 0 to maxSpin */
    /* 0x06 */ u8 unk_06[0x02];
    /* 0x08 */ f32 maxSpeed;            /* upper limit of speed */
    /* 0x0C */ f32 minSpeed;            /* lower limit of speed */
    /* 0x10 */ f32 maxSpin;             /* range of spin */
    /* 0x14 */ vec3_t aim;              /* moved with the stick (GA_InputAim) */
    /* 0x20 */ vec3_t stance;           /* where the player stands (GA_InputStance) */
    /* 0x2C */ f32 speed;               /* of the ball at release, between the limits */
    /* 0x30 */ f32 spin;                /* of the ball at release (motion[0].spin[1]) */
    /* 0x34 */ f32 timingAngle;         /* the throw's timing, -pi/2 to pi/2 (GA_PanelTiming); 0 is straight */
    /* 0x38 */ vec3_t direction;        /* GA_VecDirection of stance and aim */
    /* 0x44 */ f32 hook;                /* added to the rolling ball's motion[0].vel[0] each frame */
} gaCharacter_t;                        /* size 0x48 */

extern gaCharacter_t ga_characters[10];   /* per character (the last two empty) */

#define MAX_PLAYERS     4

/* A player (ga_players). Name invented. */
typedef struct {
    /* 0x00 */ gaCharacter_t character;
    /* 0x48 */ f32 homeMtx[4][4];       /* copy of obj->mtx at set-up, restored for each throw */
    /* 0x88 */ sceneObject_t *obj;
    /* 0x8C */ gaLane_t *lane;
    /* 0x90 */ s8 port;                 /* controller port; -1: played by the game */
    /* 0x91 */ u8 laneIndex;
    /* 0x92 */ s8 ballView;               /* 0 to 4, stepped by GA_CameraStepView */
    /* 0x93 */ u8 ballWeight;           /* 0 to 10, shown as 6 to 16; C-left and C-right (GA_DrawWeightSelector) */
    /* 0x94 */ u16 releaseFrame;        /* animation frame when the throw was committed */
    /* 0x96 */ u8 unk_96[0x02];
    /* 0x98 */ f32 cpuAim;              /* aim[0] that a player the game plays heads for */
    /* 0x9C */ f32 cpuStance;           /* stance[0] that a player the game plays heads for */
} gaPlayer_t;                           /* size 0xA0 */

/* Configuration of a stage (ga_stages); ga_stage points to the current one. Name invented. */
typedef struct {
    /* 0x00 */ objParams_t *scene;       /* the scene (Obj_CreateAll); one object per player first */
    /* 0x04 */ f32 laneOffset;
    /* 0x08 */ f32 hookStartZ;
    /* 0x0C */ f32 hook;
    /* 0x10 */ f32 slide;
    /* 0x14 */ f32 floorHeight;
    /* 0x18 */ u16 floorGridFile;       /* archive index of the floor grid (game_floorGrids) */
    /* 0x1A */ u16 bgm;
    /* 0x1C */ s16 ambientSound;        /* sound id; -1: none */
    /* 0x1E */ s16 sounds[9];
    /* 0x30 */ void (*update)( void );  /* each frame */
} gaStage_t;                            /* size 0x34 */

/* Main resource image with a range of its palette that the overlay fades; keeps a copy of the palette. Name
invented. */
typedef struct {
    /* 0x000 */ u16 id;
    /* 0x002 */ u16 first;                  /* palette range */
    /* 0x004 */ u16 last;
    /* 0x006 */ u16 palette[256];
    /* 0x206 */ u8 unk_206;
} gaSprite_t;                           /* size 0x208 */

/* A list of image ids with a cursor; one per lane (ga_spinImages). Name invented. */
typedef struct {
    /* 0x00 */ u8 unk_00[2];
    /* 0x02 */ u8 cur;
    /* 0x03 */ u8 count;
    /* 0x04 */ s16 *ids;
} gaImageList_t;                        /* size 0x8 */

/* image ids of the gauge's spin images: [0..7] for a forward spin, [8..15] for a reversed one */
extern s16 ga_spinImageIds[16];
extern gaImageList_t ga_spinImages[2];   /* per lane; both list ga_spinImageIds */
extern gaSprite_t ga_timingGauge[2];   /* per lane */
extern gaSprite_t ga_speedGauge[2];   /* per lane (GA_SetupLanes) */
extern gaSprite_t ga_spinGauge[2];   /* per lane (GA_SetupLanes) */

/* A pin setup of a game mode, by frame (ga_setupPinSets, ga_parPinSets). Name invented. */
typedef struct {
    /* 0x00 */ u8 par;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ u16 pins;                /* pins standing, pin 1 in bit 9 (GA_UnpackPinMask) */
    /* 0x04 */ u16 image;
} gaPinSetup_t;                         /* size 0x6 */

extern gaPinSetup_t ga_setupPinSets[];   /* for GA_ScoreSetupGame, by game_setup.unk_04[] */
extern gaPinSetup_t ga_parPinSets[];   /* for GA_ScorePar, by game_setup.unk_04[] */
extern u16 ga_editorPins;              /* pins of GA_ScorePinEditor */

/* Handler of a player: the game mode (ga_gameMode) or what starts a turn (ga_endThrowFunc). */
typedef s32 (*gaPlayerFunc_t)( s32 player );

extern s16 ga_blinkLevel;          /* blink level, 0 to 0x80 by 8 (GA_Frame) */
/* By 2 to 6 standing pins (head pin down): a count, then that many sorted masks of the standing pins
   (bit 10 - n for pin n), the leaves GA_IsSplit accepts */
extern u16 ga_splits2[1 + 23];
extern u16 ga_splits3[1 + 42];
extern u16 ga_splits4[1 + 57];
extern u16 ga_splits5[1 + 23];
extern u16 ga_splits6[1 + 1];
extern f32 ga_cpuThrows[16][4];     /* unk_98, unk_9C, unk_2C, unk_30 of a player, by game_setup.unk_04[] */
extern s16 ga_calloutImages[];
extern vec3_t ga_spotOffset;      /* (0, 24, 0) (GA_PhysicsStep) */
/* sound ids: by game_strikeRun[] up to 10 (GA_ScoreBattle), and from PAR_SOUNDS on by score against par
   (GA_ScorePar) */
extern s16 ga_runSounds[20];
#define PAR_SOUNDS      13              /* ga_runSounds[PAR_SOUNDS + score against par] */
extern f32 ga_ballMasses[11];      /* by gaPlayer_t.ballWeight */

/* Image ids of the player panel's numbers. Name invented. */
typedef struct {
    /* 0x00 */ u16 weights[11];         /* by gaPlayer_t.ballWeight */
    /* 0x16 */ u16 numbers[11];         /* the numbers 1 to 9, by number ([0] and [10] none) */
} gaPanelImages_t;                      /* size 0x2C */

extern gaPanelImages_t ga_panelImages;
extern u16 ga_parChartPins[NUM_PINS][2];  /* x and y of each pin in the pin chart (GA_DrawParFramePanel) */
extern u16 ga_parStageImages[8];       /* image ids, by game_stage */
extern u16 ga_pinPanelPins[NUM_PINS][2];  /* x and y of each pin in the pin panel (GA_DrawPinPanel) */
extern u16 ga_editorPinPos[NUM_PINS][2];  /* x and y of each pin in the pin editor (GA_PinEditor) */
extern u8 ga_editorCursor;           /* pin editor: the pin under the cursor (0 to 9) */
extern u8 ga_editorOnPins;           /* pin editor: nonzero while the cursor is on the pins */
extern u8 ga_parFrame;           /* current frame of the par game (0 to 9; par in game_frameFlags[0][]) */
extern f32 ga_spinCameraAngle[2];       /* per lane: angle of GA_CameraSpin's camera */
extern vec3_t ga_freeCameraPos[2];    /* per lane: position of GA_FreeCamera's camera (x and z) */
extern u8 ga_laneCameraStarted;           /* GA_CameraLane's camera has started (GA_UpdateLaneCamera) */
extern u16 ga_throwCamPathsLate[5];       /* path files of GA_CameraThrow's camera, late in the throw */
extern u16 ga_throwCamPaths[5];       /* path files of GA_CameraThrow's camera */
extern s16 ga_followCamPaths[10];      /* path files of GA_CameraFollowBall's camera */
extern s16 ga_laneCamPaths[5];       /* path files of GA_CameraLane's camera */
extern u8 ga_aimChannelOrders[3][3];     /* channel orders of GA_DrawAimLine's aim line colour */
extern u16 ga_aimColorAngle;          /* angle of GA_DrawAimLine's colour cycle */
extern u8 ga_aimChannelOrder;           /* channel order of GA_DrawAimLine's colour, mod 3 */
extern gaStage_t ga_stages[8];
/* the stages' scenes (gaStage_t.scene) */
extern objParams_t ga_stageScene0[63];
extern objParams_t ga_stageScene1[98];
extern objParams_t ga_stageScene2[46];
extern objParams_t ga_stageScene3[64];
extern objParams_t ga_stageScene4[45];
extern objParams_t ga_stageScene5[56];
extern objParams_t ga_stageScene6[66];
extern objParams_t ga_stageScene7[59];
extern gaStage_t *ga_stage;
extern objClass_t ga_classes[12];   /* the classes of the game's scene */
extern u8 ga_steerBall;             /* nonzero: Z steers the rolling ball by the stick (GA_InputRoll) */
extern u8 ga_earlyCommit;            /* nonzero: B commits the throw early (GA_InputSwing) */

/* Per lane: state of GA_MoveAimMarker's aiming. Name invented. */
typedef struct {
    /* 0x00 */ f32 savedMarkerZ;
    /* 0x04 */ s16 push;                   /* within 999 either way */
    /* 0x06 */ u8 step;                    /* step reached (GA_StepAim), 6 at start */
    /* 0x07 */ u8 pushRumbled;
} gaLaneAim_t;                              /* size 0x08 */

extern gaLaneAim_t ga_laneAim[2];
extern gaPlayer_t ga_players[MAX_PLAYERS];
extern u8 ga_lastScorer;
extern u8 ga_paused;
/* A page of the pause menu's page browser (ga_menuPages, GA_PageBrowser). Name invented. */
typedef struct {
    /* 0x00 */ u16 backY;                  /* y of the back item, from the page's top */
    /* 0x02 */ s8 cursor;                  /* item under the cursor; -1: the page has no items */
    /* 0x03 */ s8 backItem;                /* index of the back item (the items before it open pages) */
    /* 0x04 */ u16 image;                  /* image id of the page */
    /* 0x06 */ u8 firstPage;               /* page that item 0 opens; item n opens firstPage + n */
    /* 0x07 */ s8 parent;                  /* page the back item returns to; -1: the pause menu */
} gaMenuPage_t;                              /* size 0x08 */

extern u8 ga_pauseCursor;           /* pause menu: the item under the cursor (0 to 3) */
extern u8 ga_bonusOwed[4];        /* per player (GA_ScoreThirdBall) */
extern u16 ga_throwCount;
extern u8 ga_pinsDown;
extern gaLane_t ga_lanes[2];
extern gaBody_t ga_initBodies[NUM_BODIES];     /* initial bodies of a lane (GA_ResetLane) */
extern gaMotion_t ga_initMotion[NUM_BODIES];   /* initial gaLane_t.motion */
extern gaSpot_t ga_initSpots[2][NUM_BODIES];  /* initial gaLane_t.spots and secondSpots */
extern gaBody_t ga_initialMarkers[3];              /* initial gaLane_t.markers */

/* A state of the overlay's main loop; returns nonzero when the state is done. */
typedef s32 (*gaState_t)( void );

extern gaState_t ga_state;     /* current state */
extern gaPlayerFunc_t ga_gameMode;  /* game mode */
extern gaPlayerFunc_t ga_endThrowFunc;
extern gaState_t ga_modeLoop;     /* main loop state of the game mode (GA_StartMode) */
extern s16 ga_bgm;          /* BGM of the game (Sound_PlayBgm) */
extern s64 ga_sound;           /* sound handle (Sound_PlaySe) */
extern u8 ga_teamCurrent[2];         /* per team: index in ga_teamPlayers of the current player */
extern s8 ga_teamPlayers[2][5];      /* per team: player numbers, -1 ends */

/* unit 3C570 */
s32 GA_PauseMenu( void );
s32 GA_PauseScoreboard( void );
void GA_DrawScoreboard( void );
s32 GA_PageBrowser( void );
s32 GA_PauseQuitPrompt( void );
s32 GA_PauseSaveSettings( void );
s32 GA_PauseQuitDone( void );
s32 GA_RunFirstPlayer( void );
void GA_NextTeamPlayer( u8 *cur, s32 start, s32 team );
void GA_SetupMode( void );
void GA_SetupStandardGame( void );
void GA_StartMode( gaState_t state, gaPlayerFunc_t mode );
s32 GA_TeamGameLoop( void );
s32 GA_ParGameLoop( void );
s32 GA_PairGameLoop( void );
void GA_FillTeams( void );
s32 GA_DuelLoop( void );
s32 GA_DistanceLoop( void );

/* unit 3E350 */
void GA_NullCreate( void );
void GA_SetupLanes( u8 twoTeams, f32 offset, u8 distanceMode );
void GA_SetupPlayer( s32 player, s32 port, u8 character, u8 lane );
void GA_NullUpdate( void );
s32 GA_RunPlayer( s32 player );
void GA_HoldBall( s32 player );
void GA_UpdateLaneCamera( s32 player );
void GA_PhysicsStep( s32 player, s32 port );
void GA_DrawDistancePanels( void );
void GA_StopLanes( u8 lastLane );
void GA_PublishBall( gaPlayer_t *pl );
s32 GA_StopPanelHandlers( u8 twoLanes );
void GA_PlaceBody( gaBody_t *body, gaSpot_t *spot, sceneObject_t *obj, f32 offset );
void GA_RunPanelFunc( s32 player );
void GA_FreezePlayerObject( s32 player );
s32 GA_ScorePinEditor( s32 player );
s32 GA_EndThrowFrame( s32 player );
s32 GA_EndThrowNextTurn( s32 player );
void GA_ParkLane( s32 player, u8 restart );
s32 GA_EndDuelGame( void );
void GA_EndGameOverTurns( s32 skip );
s32 GA_IsParkedRestart( s32 player );
s32 GA_StopViewHandlers( u8 twoLanes );
s32 GA_ScorePar( s32 player );
s32 GA_ScoreStandard( s32 player );
s32 GA_ScoreBattle( s32 player );
s32 GA_ScoreSetupGame( s32 player );
void GA_ResetViews( void );
s32 GA_EndTurn( s32 player );
void GA_StartTurn( s32 player );
void GA_TurnFirstBall( s32 player );
void GA_TurnResetLane( s32 player );
void GA_TurnResetBall( s32 player );
void GA_ResetLane( s32 player, u8 *pins );
void GA_SoundThrowResult( s32 player, gaPlayer_t *pl, u8 strong );
void GA_ShowStandingPins( gaLane_t *lane );
void GA_HidePins( gaLane_t *lane );
void GA_SetTurnHandlers( gaPlayer_t *pl );
s32 GA_CheckGutter( gaPlayer_t *pl );
s32 GA_CheckBallStopped( gaPlayer_t *pl );
void GA_StepClamped( s32 player, f32 *value, s16 delta );
void GA_SteerCharacter( gaPlayer_t *pl, s16 stickX, s16 stickY );
s32 GA_InputWaitCamera( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputWaitStart( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputIdle( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_MoveAimMarker( s32 player, gaPlayer_t *pl );
s32 GA_InputAim( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputStance( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputTakeBall( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputSetThrow( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputApproach( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputSwing( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputRelease( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputBeginTurn( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputCountPins( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputRoll( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputReaction( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputEndThrow( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputNextTurn( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputGameOver( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputParkedRestart( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputParked( s32 player, s32 input, s16 stickX, s16 stickY );
s32 GA_InputGutter( s32 player, s32 input, s16 stickX, s16 stickY );
void GA_DrawScorePanel( s32 player, u8 lane );
void GA_DrawPinDiagram( gaPlayer_t *pl );
void GA_DrawParPanel( s32 player, u8 lane );
void GA_DrawParFramePanel( s32 player, u8 lane );
void GA_DrawFramePanel( s32 player, u8 lane );
void GA_DrawPinPanel( s32 player, u8 lane );
void GA_DrawAimPanel( s32 player, u8 lane );
void GA_PinEditor( gaPlayer_t *pl );
void GA_DrawAimLine( gaPlayer_t *pl );
void GA_DrawModePanel( s32 player );
void GA_DrawWeightSelector( gaPlayer_t *pl );
void GA_SetGaugeSprites( gaPlayer_t *pl );
void GA_DrawPlayerPanel( s32 player, u8 full );
void GA_PanelWeight( s32 player );
void GA_PanelFrame( s32 player );
void GA_PanelFade( s32 player );
void GA_PanelTiming( s32 player );
void GA_PanelRelease( s32 player );
void GA_PanelNone( s32 player );
void GA_DrawCallout( s32 player );
void GA_DrawParkedPrompt( s32 player );
void GA_DrawLaneShade( s32 lane );
void GA_SetBallView( gaPlayer_t *pl, s32 view );
void GA_CameraRaise( s32 player, sceneCamera_t *camera );
void GA_CameraBehindAim( s32 player, sceneCamera_t *camera );
void GA_CameraBehindStance( s32 player, sceneCamera_t *camera );
void GA_CameraStepView( s32 player, sceneCamera_t *camera );
void GA_CameraTakeBall( s32 player, sceneCamera_t *camera );
void GA_CameraPinDeck( s32 player, sceneCamera_t *camera );
void GA_CameraReaction( s32 player, sceneCamera_t *camera );
void GA_FreeCamera( s32 player, sceneCamera_t *camera );
void GA_CameraBehindLane( s32 player, sceneCamera_t *camera );
void GA_CameraSpin( s32 player, sceneCamera_t *camera );
void GA_CameraThrow( s32 player, sceneCamera_t *camera );
void GA_CameraFollowBall( s32 player, sceneCamera_t *camera );
void GA_CameraNone( s32 player, sceneCamera_t *camera );
s32 GA_CpuInput( gaPlayer_t *pl, s32 player );
void GA_DrawLeadPanel( s32 lane );
void GA_CameraLane( s32 player, sceneCamera_t *camera );
void GA_SetCpuThrow( gaPlayer_t *pl, s32 player );
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
s32 GA_KeepOnFloor( gaMotion_t *motion, gaSpot_t *spot, gaSpot_t *spot2, f32 ceiling, f32 floorHeight );
f32 GA_FloorHeight( f32 x, f32 z, vec3_t normal );
s32 GA_SlopeContact( gaMotion_t *motion, gaSpot_t *spot, vec3_t normal );
f32 GA_AimSpin( vec3_t pos, vec3_t vel, f32 x, f32 scale );
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
void GA_ScoreFirstBall( u8 player );
void GA_ScoreSecondBall( u8 player );
void GA_ScoreThirdBall( u8 player );
void GA_ClearScores( void );
void GA_ScoreThrow( u8 player, u8 pinsHit );
s32 GA_EndFrame( u8 player, s32 done );
s32 GA_CountParThrow( u8 player, s32 force );
void GA_DecideFrame( u8 frame );

/* unit 4C560 */
void GA_CountFrame( void );
void GA_SetPlayerObjectFlags( s32 set );
void GA_StageFrame0( void );
void GA_StageFrame1( void );
void GA_AnimateTextures( void );
void GA_StageFrame3( void );
void GA_StageFrame4( void );
void GA_FlickerLight( void );
void GA_StageFrame6( void );
void GA_StageFrame7( void );
void GA_LoadLanePalettes( s32 lane, s32 alt, s32 index );

#endif
