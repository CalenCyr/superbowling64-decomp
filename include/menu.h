#ifndef MENU_H
#define MENU_H

/* menu.h -- menus of images that slide in and out, and the scoreboard's digits (unit 13490) */

#include "common.h"
#include "libultra.h"
#include "boot.h"

#define MENU_END            -1          /* menuItem_t.image of the record that ends a menu */
#define MENU_NONE           -1          /* Menu_SetItem: keep the member */
#define MENU_SLIDE_STEPS    32          /* entries of D_8005CA80 */

/* menuItem_t.slide: the edge an item slides in from */
#define SLIDE_NONE          0
#define SLIDE_TOP           1
#define SLIDE_BOTTOM        2
#define SLIDE_LEFT          3
#define SLIDE_RIGHT         4

/* Image of a menu; a menu is an array of them, ended by one whose image is MENU_END. */
typedef struct {
    /* 0x00 */ s16 id;
    /* 0x02 */ s16 image;           /* index in the file cache */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ u8 alpha;            /* 0: not drawn */
    /* 0x09 */ s8 slide;            /* SLIDE_NONE etc. */
} menuItem_t;                       /* size 0xA */

/* D_8005CAC0: image of each digit, then of the scoreboard's symbols */
#define SCORE_IMAGE_STRIKE  10
#define SCORE_IMAGE_SPARE   11
#define SCORE_IMAGE_GUTTER  12
#define SCORE_IMAGE_FOUL    14

/* Score_RollImage: pins of a roll that are not a number */
#define ROLL_STRIKE         -1
#define ROLL_SPARE          -2

/* Maze_Build: work space of a maze's construction, on the heap */
typedef struct {
    /* 0x0000 */ u16 set[0x400];        /* per cell, the lowest cell it is connected to */
    /* 0x0800 */ u8 closed[0x800];      /* per wall, 0: opened */
    /* 0x1000 */ u16 weight[0x800];     /* per wall, its random weight; 0 once ordered */
    /* 0x2000 */ u16 order[0x800];      /* the walls by decreasing weight */
} mazeWork_t;                           /* size 0x3000 */

extern u16 D_8005CA80[MENU_SLIDE_STEPS];    /* position of a sliding item at each step, 0x1000: in place */
extern u16 D_8005CAC0[16];
extern u16 D_8005CB18[10];          /* image of each digit */
extern u16 D_8005CB04[10];          /* image of each small digit */
extern s16 D_8005CAE4[];           /* icon of each player, then of the computer */
extern s8 D_800DF5D0[];
extern u16 D_8005CB58[4];          /* Score_DrawCardFrame: sign of a score: negative, zero, positive */
extern u16 D_8005CB2C[9];          /* Score_DrawParCard: image of each frame's number */
extern u16 D_8005CB40[6];          /* Score_DrawParCard: per row, the image of a frame's box */
extern u16 D_8005CB4C[6];          /* Score_DrawParCard: per row, the image of the total's box */
/* Score_SetRecordFigures: per kind of record (0: D_800E44E8, 1: D_8009D098), per character, the record's figures */
extern f32 D_800E66B0[3][4];        /* average score */
extern f32 D_800DF5D8[3][4];        /* unk_14, percent of 12 a game */
extern f32 D_800A2FE8[3][4];        /* unk_16, percent of 10 a game */
extern f32 D_80099488[3][4];        /* unk_18, percent of 10 a game */            /* number shown on each player's name plate; 0: none */

void Maze_Draw( u16 image, u32 width, u32 height, u8 *maze );
void Maze_Build( u32 width, u32 height, u8 *maze );
void Menu_DrawItems( menuItem_t *items, u8 step );
void Menu_DrawNamePlate( s32 player, u8 cpu, u16 *name, s16 x, s16 y );
void Score_DrawTeamBoard( u8 mode, u8 all, s16 x, s16 y );
void Score_DrawParCard( u8 kind, u8 all, s16 x, s16 y );
void Score_DrawAllBoards( s32 arg0, s16 x, s16 y );
void Score_DrawPlayerBoard( s32 player, s16 x, s16 y );
void Score_DrawRecordTable( u16 count, playerRecord_t *records, u8 kind, s16 x, s32 y );
void Menu_DrawSmallNamePlate( s32 player, u8 cpu, u16 *name, u8 noIcon, s16 x, s16 y );
void Score_DrawFrameHeaders( u8 count, u8 first, s16 x, s16 y );
void Score_DrawHeaders( u8 tenth, s16 x, s16 y );
void Menu_SetItem( menuItem_t *items, s16 id, s16 image, s16 x, s16 y, s16 alpha );
void Score_DrawNumber( s16 value, s16 x, s16 y );
void Score_DrawFrame( s32 player, u8 frame, u8 last, u8 throw, u8 arg4, s16 x, s16 y );
u16 Score_RollImage( s16 pins, u8 foul );
void Score_GetDigits( s16 *digits, s16 value, u8 small, u8 tens );
void Score_DrawCardFrame( s32 player, u8 frame, u8 kind, u8 all, s16 x, s16 y );
void Score_DrawTotal( s32 player, u8 duel, s16 x, s16 y );
void Score_SetRecordFigures( playerRecord_t *record, s32 index, u8 kind );
void Score_DrawNumberBox( s32 a, s32 b, u8 show, s16 x, s16 y );
void Menu_DrawBox( s32 arg0, u8 count, u8 small, s16 x, s16 y );
void Menu_DrawImage( s16 image, s16 x, s16 y );
s32 Menu_TwoChoicePrompt( void );

#endif
