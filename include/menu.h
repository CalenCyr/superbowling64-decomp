#ifndef MENU_H
#define MENU_H

/* menu.h -- menus of images that slide in and out, and the scoreboard's digits (unit 13490) */

#include "common.h"
#include "libultra.h"
#include "boot.h"

#define MENU_END            -1          /* menuItem_t.image of the record that ends a menu */
#define MENU_NONE           -1          /* Menu_SetItem: keep the member */
#define MENU_SLIDE_STEPS    32          /* entries of menu_slidePositions */

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

/* score_images: image of each digit, then of the scoreboard's symbols */
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

extern s8 score_plateNumbers[];     /* number shown on each player's name plate; 0: none */
/* Score_SetRecordFigures: per kind of record (0: game_sessionRecords, 1: save_data), per character, the record's figures */
extern f32 score_averages[3][4];    /* average score */
extern f32 score_strikeRates[3][4];  /* strikes, percent of 12 a game */
extern f32 score_spareRates[3][4];  /* spares, percent of 10 a game */
extern f32 score_splitRates[3][4];  /* splits, percent of 10 a game */

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
void Score_DrawFrame( s32 player, u8 frame, u8 last, u8 throw, u8 done, s16 x, s16 y );
u16 Score_RollImage( s16 pins, u8 foul );
void Score_GetDigits( s16 *digits, s16 value, u8 small, u8 tens );
void Score_DrawCardFrame( s32 player, u8 frame, u8 kind, u8 all, s16 x, s16 y );
void Score_DrawTotal( s32 player, u8 duel, s16 x, s16 y );
void Score_SetRecordFigures( playerRecord_t *record, s32 index, u8 kind );
void Score_DrawNumberBox( s32 a, s32 b, u8 show, s16 x, s16 y );
void Menu_DrawBox( s32 player, u8 count, u8 small, s16 x, s16 y );
void Menu_DrawImage( s16 image, s16 x, s16 y );
s32 Menu_TwoChoicePrompt( void );

#endif
