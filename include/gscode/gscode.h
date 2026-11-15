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
#include "object.h"
#include "filecache.h"

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

/* D_8011B6A1_gscode, D_8011B739_gscode, D_8011B7C9_gscode, D_8011B845_gscode, D_8011B8C5_gscode: step of the choice menus of units
 * 59490, 59B00, 5A020, 5A580 and 5A990 */
#define CHOICE_IN           0           /* sliding in */
#define CHOICE_RUN          1           /* in place, taking input */
#define CHOICE_TAKEN        2           /* a choice was taken, sliding out */
#define CHOICE_BACK         3           /* backed out, sliding out */

extern s32 D_8011B5A0_gscode;       /* entries in D_80120781_gscode */
extern menuItem_t D_8011B5B0_gscode[12]; /* the menu of the five game modes, its background; [5]: its title */
extern menuItem_t D_8011B628_gscode[6];  /* its choices, ids 1 to 5 */
extern s16 D_8011B664_gscode[5][6];  /* per item of D_8011B628_gscode: images of its blink */
extern u8 D_8011B6A0_gscode;        /* slide step of the menu of the five game modes */
extern u8 D_8011B6A1_gscode;        /* CHOICE_IN etc. */
extern u8 D_8011B6A2_gscode;        /* highlighted choice, 0 to 4 */
extern u8 D_8011B6A3_gscode;        /* blink frame, 0 to 29 */
extern menuItem_t D_8011B6B0_gscode[10]; /* the three-choice menu; [5]: its title */
extern s16 D_8011B714_gscode[3][6];  /* per choice: images of its blink */
extern u8 D_8011B738_gscode;        /* slide step of the three-choice menu */
extern u8 D_8011B739_gscode;        /* CHOICE_IN etc. */
extern u8 D_8011B73A_gscode;        /* highlighted choice: 0 left, 1 right, 2 below */
extern u8 D_8011B73B_gscode;        /* blink frame, 0 to 29 */
extern menuItem_t D_8011B740_gscode[10]; /* the second three-choice menu; [5]: its title */
extern s16 D_8011B7A4_gscode[3][6];  /* per choice: images of its blink */
extern u8 D_8011B7C8_gscode;        /* slide step of the second three-choice menu */
extern u8 D_8011B7C9_gscode;        /* CHOICE_IN etc. */
extern u8 D_8011B7CA_gscode;        /* highlighted choice: 0 left, 1 right, 2 below */
extern u8 D_8011B7CB_gscode;        /* blink frame, 0 to 29 */
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
extern menuItem_t D_8011BA08_gscode[7];  /* the name entry: id 0x17 its background, 0x18 the character grid, 0x19 the cursor */
extern s32 D_8011BA6C_gscode;       /* step of the name entry: 1 next player, 2 backed out, 5 done */
extern u16 D_8011BA70_gscode[3 * 2 * 5 * 12]; /* the characters of the name entry: per set, case, row (5) and column (12) */
extern u8 D_8011BD40_gscode;        /* column the cursor left the grid from */
extern u8 D_80120793_gscode;        /* player whose name is entered */
extern s8 D_80120794_gscode;        /* column of the cursor; on the row below the grid, 4 to 6 the sets, 7 the end */
extern s8 D_80120795_gscode;        /* row of the cursor, 0 to 4; -1: the row below the grid */
extern u8 D_80120796_gscode;        /* character set, 0 to 2 */
extern u8 D_80120797_gscode;        /* case of the character set, 0 or 1 */
extern u8 D_80120798_gscode;        /* characters entered of the name, 0 to 5 */
extern menuItem_t D_8011BD50_gscode[16]; /* the character select: [5] its title, ids 0-7 the portraits, id 9 the cursor */
extern menuItem_t D_8011BDF0_gscode[3];  /* the character menu's name (id 8) and marker (id 10) */
extern s16 D_8011BE10_gscode[8];    /* per character: image of its name */
extern s16 D_8011BE20_gscode[8];    /* per character: x of its name */
extern s16 D_8011BE30_gscode[4];    /* images of the marker */
extern u8 D_8011BE38_gscode[8];     /* per character: its image in D_8011BE30_gscode */
extern objParams_t *D_8011F880_gscode[8]; /* per character: the scene of its model */
extern u8 D_8011F8A0_gscode;        /* frames before the camera turns */
extern f32 D_8011F8A4_gscode;       /* angle of the camera about the model */
extern s16 D_8011F8A8_gscode[8];    /* per character: its portrait image */
extern u8 D_8011F8B8_gscode;        /* slide step of the character menu */
extern u8 D_8011F8B9_gscode;        /* CHOICE_IN etc.; 4: the name entry */
extern u8 D_8011F8BA_gscode;        /* frame of the viewport's zoom, 0 to 15 */
extern objClass_t D_8011B540_gscode[];   /* the classes of the character menu's scene */
extern menuItem_t D_80120430_gscode[17]; /* the character menu: [0-7] the characters, [9] the cursor, [10] the name */
extern s16 D_801204DC_gscode[8];    /* per character: its image once unlocked */
extern objParams_t D_801204EC_gscode[]; /* the scene of the character menu */
extern f32 D_801205AC_gscode[8];    /* per character: scale of its model */
extern s16 D_801205CC_gscode[8];    /* per character: file of its model */
extern u8 D_801205DC_gscode[8];     /* per character: animations in D_801205E4_gscode */
extern s16 D_801205E4_gscode[8][6]; /* per character: files of its animations; [0] the idle one */
extern s16 D_80120644_gscode[8];    /* per character: image of its name */
extern u8 D_80120654_gscode;        /* slide step of the character menu */
extern u8 D_80120655_gscode;        /* CHOICE_IN etc. */
extern u8 D_80120656_gscode;        /* highlighted character, 0 to 7 */
extern u8 D_80120657_gscode;        /* bit 0: the character menu takes input */
extern u8 D_80120658_gscode;        /* animation of the highlighted character */
extern s16 D_80120660_gscode[4];    /* images of a blinking marker */
extern s32 D_80120668_gscode;       /* frame counter of the pak check */
extern u8 D_80120780_gscode;        /* current screen */
extern u8 D_80120781_gscode[];      /* screens to go back to, a stack */
extern u16 D_80120A20_gscode;       /* character whose model is shown; 0xFFFF: none */
extern fileType1_t *D_80120A28_gscode[8]; /* per character: its loaded model */
extern u16 D_80120A48_gscode[8];    /* per character: file of its loaded animation */
extern u32 D_80120A60_gscode;       /* step of the pak check */
extern s32 D_80120A64_gscode;       /* 1: a second controller is in play */
extern u8 D_80120A68_gscode[2];     /* per port: controller status of the last frame */

/* D_80120426_gscode: line of the options screen */
#define OPTION_SOUND        0           /* D_8009D098.unk_78, 0 to 2 */
#define OPTION_79           1           /* D_8009D098.unk_79, 0 or 1 */
#define OPTION_SOUND_TEST   2           /* D_80120427_gscode */
#define OPTION_BGM_VOLUME   3           /* D_8009D098.unk_7A, 0 to 4 */
#define OPTION_7B           4           /* D_8009D098.unk_7B, 0 or 1 */
#define OPTION_RECORDS      5           /* toggles the records page */
#define OPTION_CREDITS      6           /* plays the credits */
#define OPTION_EXIT         7

#define NUM_SOUND_TESTS     20
#define NUM_CREDIT_SPRITES  20

/* D_8011FD58_gscode: commands of the credits script */
#define CREDIT_SHOW         0           /* show image a at (b, c) with alpha */
#define CREDIT_MOVE         1           /* move to (a, b) in c frames */
#define CREDIT_FADE         2           /* fade to alpha a in b frames */
#define CREDIT_WAIT         3           /* wait sprite frames */
#define CREDIT_END          4

typedef struct {
    /* 0x00 */ s16 op;              /* CREDIT_SHOW etc. */
    /* 0x02 */ s16 sprite;          /* index in D_80120860_gscode; CREDIT_WAIT: frames */
    /* 0x04 */ u16 a;
    /* 0x06 */ u16 b;
    /* 0x08 */ u16 c;
    /* 0x0A */ u8 unk_0A;
    /* 0x0B */ u8 alpha;            /* CREDIT_SHOW */
} creditCmd_t;                      /* size 0xC */

/* An image of the credits, moving and fading to its targets. */
typedef struct {
    /* 0x00 */ s16 image;           /* -1: unused */
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ u8 alpha;
    /* 0x08 */ s16 targetX;
    /* 0x0A */ s16 targetY;
    /* 0x0C */ s16 moveFrames;      /* 0: not moving */
    /* 0x0E */ s16 moveFrame;
    /* 0x10 */ u8 targetAlpha;
    /* 0x12 */ s16 fadeFrames;      /* 0: not fading */
    /* 0x14 */ s16 fadeFrame;
} creditSprite_t;                   /* size 0x16 */

extern menuItem_t D_8011FC90_gscode[20]; /* the options screen; [5] its title, id 0 the cursor */
extern creditCmd_t D_8011FD58_gscode[142]; /* the credits script */
extern u32 D_80120400_gscode;       /* frames since the last sound test */
extern s16 D_80120404_gscode[4];    /* per D_8009D098.unk_78: its image */
extern s16 D_8012040C_gscode[2];    /* per D_8009D098.unk_79 and unk_7B: their image */
extern s16 D_80120410_gscode[10];   /* image of each digit */
extern u8 D_80120424_gscode;        /* slide step of the options screen */
extern u8 D_80120425_gscode;        /* 0 sliding in, 1 in place, 2 and 3 sliding out (3: backed out) */
extern u8 D_80120426_gscode;        /* OPTION_SOUND etc. */
extern u8 D_80120427_gscode;        /* sound of the sound test, 0 to NUM_SOUND_TESTS - 1 */
extern u8 D_80120428_gscode;        /* 0 the options, 1 the records page, 2 the credits */
extern creditSprite_t D_80120860_gscode[NUM_CREDIT_SPRITES];
extern u16 D_80120A18_gscode;        /* frames before the next credits command; 0xFFFF: the script ended */
extern u16 D_80120A1A_gscode;        /* next command of the credits script */

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
s32 func_80114680_gscode( void );
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
