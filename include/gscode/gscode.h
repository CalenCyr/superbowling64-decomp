#ifndef GSCODE_GSCODE_H
#define GSCODE_GSCODE_H

/* gscode.h -- the game-select overlay (file GSCODE  SYS, ROM 0x58F40, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
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

/* gs_modeState, gs_subModeState, gs_teamSizeState, gs_playersState, gs_twoChoiceState: step of the choice
 * menus of units 59490, 59B00, 5A020, 5A580 and 5A990 */
#define CHOICE_IN           0           /* sliding in */
#define CHOICE_RUN          1           /* in place, taking input */
#define CHOICE_TAKEN        2           /* a choice was taken, sliding out */
#define CHOICE_BACK         3           /* backed out, sliding out */

extern s32 gs_screenDepth;                    /* entries in gs_screenStack */
extern menuItem_t gs_modeMenu[12];            /* the menu of the five game modes, its background; [5]: its title */
extern menuItem_t gs_modeChoices[6];          /* its choices, ids 1 to 5 */
extern s16 gs_modeBlink[5][6];                /* per item of gs_modeChoices: images of its blink */
extern u8 gs_modeSlide;                       /* slide step of the menu of the five game modes */
extern u8 gs_modeState;                       /* CHOICE_IN etc. */
extern u8 gs_modeCursor;                      /* highlighted choice, 0 to 4 */
extern u8 gs_modeBlinkFrame;                  /* blink frame, 0 to 29 */
extern menuItem_t gs_subModeMenu[10];         /* the three-choice menu; [5]: its title */
extern s16 gs_subModeBlink[3][6];             /* per choice: images of its blink */
extern u8 gs_subModeSlide;                    /* slide step of the three-choice menu */
extern u8 gs_subModeState;                    /* CHOICE_IN etc. */
extern u8 gs_subModeCursor;                   /* highlighted choice: 0 left, 1 right, 2 below */
extern u8 gs_subModeBlinkFrame;               /* blink frame, 0 to 29 */
extern menuItem_t gs_teamSizeMenu[10];        /* the second three-choice menu; [5]: its title */
extern s16 gs_teamSizeBlink[3][6];            /* per choice: images of its blink */
extern u8 gs_teamSizeSlide;                   /* slide step of the second three-choice menu */
extern u8 gs_teamSizeState;                   /* CHOICE_IN etc. */
extern u8 gs_teamSizeCursor;                  /* highlighted choice: 0 left, 1 right, 2 below */
extern u8 gs_teamSizeBlinkFrame;              /* blink frame, 0 to 29 */
extern menuItem_t gs_playersMenu[9];          /* the menu of one or two players; [5]: its title */
extern s16 gs_playersBlink[2][6];             /* per choice: images of its blink */
extern u8 gs_playersSlide;                    /* slide step of the menu of one or two players */
extern u8 gs_playersState;                    /* CHOICE_IN etc. */
extern u8 gs_playersCursor;                   /* highlighted choice: 0 one player, 1 two */
extern u8 gs_playersBlinkFrame;               /* blink frame, 0 to 29 */
extern menuItem_t gs_twoChoiceMenu[9];        /* the two-choice menu; [5]: its title */
extern s16 gs_twoChoiceBlink[2][6];           /* per choice: images of its blink */
extern u8 gs_twoChoiceSlide;                  /* slide step of the two-choice menu */
extern u8 gs_twoChoiceState;                  /* CHOICE_IN etc. */
extern u8 gs_twoChoiceCursor;                 /* highlighted choice, 0 or 1 */
extern u8 gs_twoChoiceBlinkFrame;             /* blink frame, 0 to 29 */
extern menuItem_t gs_nameEntryMenu[7];        /* the name entry: id 0x17 its background, 0x18 the character grid, 0x19 the cursor */
extern s32 gs_nameEntryStep;                  /* step of the name entry: 1 next player, 2 backed out, 5 done */
extern u16 gs_nameEntryChars[3 * 2 * 5 * 12]; /* the characters of the name entry: per set, case, row (5) and column (12) */
extern u8 gs_gridExitColumn;                  /* column the cursor left the grid from */
extern menuItem_t gs_setupMenu[31];           /* the player setup: [5] its title, ids 9-0x10 the players' names */
extern s16 gs_rumbleBlink[4];                 /* images of the Rumble Pak marker's blink */
extern u16 gs_handicapDigits[10];             /* image of each digit of a handicap */
extern s32 gs_clearRecordAnswer;              /* answer of the prompt to clear a team's record */
extern s32 gs_setupFrames;                    /* frame counter of the player setup */
extern u8 gs_setupSlide;                      /* slide step of the player setup */
extern u8 gs_setupState;                      /* CHOICE_IN etc. */
extern u8 gs_cHeldFrames;                     /* frames a C button is held */
extern u8 gs_teamPromptChoice;                /* choice of the team prompt: 0 keep the name, 1 clear the record */
extern u8 gs_rumbleStatus[2];                 /* per port: controller status the Rumble Pak check compares with */
extern u8 gs_namePlayer;                      /* player whose name is entered */
extern s8 gs_nameCursorX;                     /* column of the cursor; on the row below the grid, 4 to 6 the sets, 7 the end */
extern s8 gs_nameCursorY;                     /* row of the cursor, 0 to 4; -1: the row below the grid */
extern u8 gs_nameCharSet;                     /* character set, 0 to 2 */
extern u8 gs_nameCharCase;                    /* case of the character set, 0 or 1 */
extern u8 gs_nameLength;                      /* characters entered of the name, 0 to 5 */
extern menuItem_t D_8011BD50_gscode[16];      /* the character select: [5] its title, ids 0-7 the portraits, id 9 the cursor */
extern menuItem_t D_8011BDF0_gscode[3];       /* the character menu's name (id 8) and marker (id 10) */
extern s16 D_8011BE10_gscode[8];              /* per character: image of its name */
extern s16 D_8011BE20_gscode[8];              /* per character: x of its name */
extern s16 D_8011BE30_gscode[4];              /* images of the marker */
extern u8 D_8011BE38_gscode[8];               /* per character: its image in D_8011BE30_gscode */
extern objParams_t *D_8011F880_gscode[8];     /* per character: the scene of its model */
extern u8 D_8011F8A0_gscode;                  /* frames before the camera turns */
extern f32 D_8011F8A4_gscode;                 /* angle of the camera about the model */
extern s16 D_8011F8A8_gscode[8];              /* per character: its portrait image */
extern u8 D_8011F8B8_gscode;                  /* slide step of the character menu */
extern u8 D_8011F8B9_gscode;                  /* CHOICE_IN etc.; 4: the name entry */
extern u8 D_8011F8BA_gscode;                  /* frame of the viewport's zoom, 0 to 15 */
extern objClass_t gs_classes[];               /* the classes of the character menu's scene */
extern menuItem_t D_80120430_gscode[17];      /* the character menu: [0-7] the characters, [9] the cursor, [10] the name */
extern s16 D_801204DC_gscode[8];              /* per character: its image once unlocked */
extern objParams_t D_801204EC_gscode[];       /* the scene of the character menu */
extern f32 D_801205AC_gscode[8];              /* per character: scale of its model */
extern s16 D_801205CC_gscode[8];              /* per character: file of its model */
extern u8 D_801205DC_gscode[8];               /* per character: animations in D_801205E4_gscode */
extern s16 D_801205E4_gscode[8][6];           /* per character: files of its animations; [0] the idle one */
extern s16 D_80120644_gscode[8];              /* per character: image of its name */
extern u8 D_80120654_gscode;                  /* slide step of the character menu */
extern u8 D_80120655_gscode;                  /* CHOICE_IN etc. */
extern u8 D_80120656_gscode;                  /* highlighted character, 0 to 7 */
extern u8 D_80120657_gscode;                  /* bit 0: the character menu takes input */
extern u8 D_80120658_gscode;                  /* animation of the highlighted character */
extern s16 D_80120660_gscode[4];              /* images of a blinking marker */
extern s32 gs_pakCheckFrames;                 /* frame counter of the pak check */
extern u8 gs_screen;                          /* current screen */
extern u8 gs_screenStack[];                   /* screens to go back to, a stack */
extern u16 D_80120A20_gscode;                 /* character whose model is shown; 0xFFFF: none */
extern fileType1_t *D_80120A28_gscode[8];     /* per character: its loaded model */
extern u16 D_80120A48_gscode[8];              /* per character: file of its loaded animation */
extern u32 gs_pakCheckStep;                   /* step of the pak check */
extern s32 gs_secondController;               /* 1: a second controller is in play */
extern u8 gs_lastContStatus[2];               /* per port: controller status of the last frame */

/* the team select (unit 5EC00): each team's players take characters in turn */
extern f32 D_8011F8C0_gscode[8];    /* per character: scale of its model */
extern s16 D_8011F8E0_gscode[8];    /* per character: file of its model */
extern s16 D_8011F8F0_gscode[8];    /* per character: file of its animation */
extern objParams_t D_8011F900_gscode[]; /* the scene of the team select */
extern menuItem_t D_8011FA80_gscode[28]; /* the team select: ids 0-7 the characters, 8-11 the players' names, 12-15
                                            the titles, 16-19 the players' cursors; [5]: its title */
extern menuItem_t D_8011FB98_gscode[9];   /* the players' character names: ids 24-27 and 28-31 */
extern s16 D_8011FBF4_gscode[8];          /* per character: image of the second line of its name */
extern s16 D_8011FC04_gscode[8];          /* per character: image of the first line of its name */
extern s16 D_8011FC14_gscode[8];          /* per character: its image once unlocked */
extern s16 D_8011FC24_gscode[8];          /* per character: x of a player's cursor on it */
extern s16 D_8011FC34_gscode[8];          /* per character: image of a player's cursor on it */
extern u8 D_8011FC44_gscode[3][3];        /* per bar of a character's figures: its colour */
extern u8 D_8011FC50_gscode[8][3];        /* per character: height of its three bars, to 40 */
extern s16 D_8011FC68_gscode[4];          /* per player: image of its name */
extern s16 D_8011FC70_gscode[4];          /* per player: image of its name, the computer playing */
extern s16 D_8011FC78_gscode[4];          /* per player: image of its name while it waits its turn */
extern u8 D_8011FC80_gscode;              /* slide step of the team select */
extern u8 D_8011FC81_gscode;              /* CHOICE_IN etc.; 4: the name entry */
extern u8 D_8011FC82_gscode;              /* 0 choosing, 1 every team is done */
extern u8 D_8011FC84_gscode[2];           /* per team: its player choosing; bit 7: the team is done */
extern u8 D_801207B2_gscode;              /* frames before the team select leaves, once every team is done */
extern u16 D_801207B4_gscode[4];          /* per player: character whose model is shown; 0xFFFF: none */
extern fileType1_t *D_801207C0_gscode[8]; /* per character: its loaded model */
extern s32 *D_801207E0_gscode[8];         /* per character: its loaded animation */
extern s16 D_80120800_gscode[4][3];       /* per player: position of its model */
extern u8 D_80120818_gscode[4][3];        /* per player: heights of its bars, growing to the character's */
extern s16 D_80120828_gscode[4][3];       /* per player: x of its bars */
extern s16 D_80120840_gscode[4][3];       /* per player: y of the bottom of its bars */

/* gs_optionsCursor: line of the options screen */
#define OPTION_DIFFICULTY   0           /* D_8009D098.difficulty, 0 to 2 */
#define OPTION_MUSIC        1           /* D_8009D098.music, 0 or 1 */
#define OPTION_SOUND_TEST   2           /* gs_soundTestSound */
#define OPTION_BGM_VOLUME   3           /* D_8009D098.musicVolume, 0 to 4 */
#define OPTION_RUMBLE       4           /* D_8009D098.rumble, 0 or 1 */
#define OPTION_RECORDS      5           /* toggles the records page */
#define OPTION_CREDITS      6           /* plays the credits */
#define OPTION_EXIT         7

#define NUM_SOUND_TESTS     20
#define NUM_CREDIT_SPRITES  20

/* gs_creditsScript: commands of the credits script */
#define CREDIT_SHOW         0           /* show image a at (b, c) with alpha */
#define CREDIT_MOVE         1           /* move to (a, b) in c frames */
#define CREDIT_FADE         2           /* fade to alpha a in b frames */
#define CREDIT_WAIT         3           /* wait sprite frames */
#define CREDIT_END          4

typedef struct {
    /* 0x00 */ s16 op;              /* CREDIT_SHOW etc. */
    /* 0x02 */ s16 sprite;          /* index in gs_creditSprites; CREDIT_WAIT: frames */
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

extern menuItem_t gs_optionsMenu[20];     /* the options screen; [5] its title, id 0 the cursor */
extern creditCmd_t gs_creditsScript[142]; /* the credits script */
extern u32 gs_soundTestFrames;            /* frames since the last sound test */
extern s16 D_80120404_gscode[4];          /* per D_8009D098.difficulty: its image */
extern s16 D_8012040C_gscode[2];          /* per D_8009D098.music and unk_7B: their image */
extern s16 D_80120410_gscode[10];         /* image of each digit */
extern u8 gs_optionsSlide;                /* slide step of the options screen */
extern u8 gs_optionsState;                /* 0 sliding in, 1 in place, 2 and 3 sliding out (3: backed out) */
extern u8 gs_optionsCursor;               /* OPTION_DIFFICULTY etc. */
extern u8 gs_soundTestSound;              /* sound of the sound test, 0 to NUM_SOUND_TESTS - 1 */
extern u8 gs_optionsPage;                 /* 0 the options, 1 the records page, 2 the credits */
extern creditSprite_t gs_creditSprites[NUM_CREDIT_SPRITES];
extern u16 gs_creditsWait;                /* frames before the next credits command; 0xFFFF: the script ended */
extern u16 gs_creditsPos;                 /* next command of the credits script */

/* unit 58F40 */
void GS_PushScreen( u8 screen );
void GS_PopScreen( void );
void GS_NullCreate( void );
void GS_NullHandler( void );

/* units 59490, 59B00, 5A020: the first screens */
s32 GS_ModeMenu( void );
s32 GS_SubModeMenu( void );
s32 GS_TeamSizeMenu( void );

/* unit 5A580 */
s32 GS_PlayersMenu( void );

/* unit 5A990 */
s32 GS_TwoChoiceMenu( void );

/* unit 5AD50 */
s32 GS_NameEntry( void );
s32 GS_PlayerSetup( void );

/* unit 5E2B0 */
s32 GS_CharacterSelect( void );

/* unit 5EC00 */
u8 GS_UnlockMask( u8 flags );
s32 GS_TeamSelect( void );

/* units 605D0, 61640 */
s32 GS_Options( void );
s32 GS_CharacterMenu( void );

/* unit 61CA0 */
void GS_StartPakCheck( void );
s32 GS_PakCheck( void );

#endif
