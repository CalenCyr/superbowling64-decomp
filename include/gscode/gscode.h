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

/* defined in unit 58F40 by its position, first in the overlay's .data; used by units 5E2B0, 5EC00 and 61640 */
extern objClass_t gs_classes[];               /* the classes of the character menu's scene */

/* The bss of units 5AD50 and 5EC00 (0x80120790-0x80120860) and of unit 61640 (0x80120A20-0x80120A60) stays in asm
 * (asm/data/gscode): KMC GCC's assembler put objects of 16 bytes or more at a multiple of 8 (D_801207C0_gscode,
 * D_80120828_gscode, D_80120A28_gscode), which this assembler does not, and nothing references 0x80120799-0x801207B1,
 * between 5AD50's last object and 5EC00's first. */

/* the player setup and the name entry (unit 5AD50) */
extern u8 gs_teamPromptChoice;                /* choice of the team prompt: 0 keep the name, 1 clear the record */
extern u8 gs_rumbleStatus[2];                 /* per port: controller status the Rumble Pak check compares with */
extern u8 gs_namePlayer;                      /* player whose name is entered */
extern s8 gs_nameCursorX;                     /* column of the cursor; on the row below the grid, 4 to 6 the sets, 7 the end */
extern s8 gs_nameCursorY;                     /* row of the cursor, 0 to 4; -1: the row below the grid */
extern u8 gs_nameCharSet;                     /* character set, 0 to 2 */
extern u8 gs_nameCharCase;                    /* case of the character set, 0 or 1 */
extern u8 gs_nameLength;                      /* characters entered of the name, 0 to 5 */

/* the team select (unit 5EC00): each team's players take characters in turn */
extern u8 D_801207B2_gscode;              /* frames before the team select leaves, once every team is done */
extern u16 D_801207B4_gscode[4];          /* per player: character whose model is shown; 0xFFFF: none */
extern fileType1_t *D_801207C0_gscode[8]; /* per character: its loaded model */
extern fileType2_t *D_801207E0_gscode[8];         /* per character: its loaded animation */
extern s16 D_80120800_gscode[4][3];       /* per player: position of its model */
extern u8 D_80120818_gscode[4][3];        /* per player: heights of its bars, growing to the character's */
extern s16 D_80120828_gscode[4][3];       /* per player: x of its bars */
extern s16 D_80120840_gscode[4][3];       /* per player: y of the bottom of its bars */

/* the character menu (unit 61640) */
extern u16 D_80120A20_gscode;                 /* character whose model is shown; 0xFFFF: none */
extern fileType1_t *D_80120A28_gscode[8];     /* per character: its loaded model */
extern u16 D_80120A48_gscode[8];              /* per character: file of its loaded animation */

/* gs_optionsCursor: line of the options screen */
#define OPTION_DIFFICULTY   0           /* save_data.difficulty, 0 to 2 */
#define OPTION_MUSIC        1           /* save_data.music, 0 or 1 */
#define OPTION_SOUND_TEST   2           /* gs_soundTestSound */
#define OPTION_BGM_VOLUME   3           /* save_data.musicVolume, 0 to 4 */
#define OPTION_RUMBLE       4           /* save_data.rumble, 0 or 1 */
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
