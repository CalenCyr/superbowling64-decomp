#ifndef MENU_H
#define MENU_H

/* menu.h -- menus of images that slide in and out, and the scoreboard's digits (unit 13490) */

#include "common.h"
#include "libultra.h"

#define MENU_END            -1          /* menuItem_t.image of the record that ends a menu */
#define MENU_NONE           -1          /* func_8003893C: keep the member */
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

/* func_80039340: pins of a roll that are not a number */
#define ROLL_STRIKE         -1
#define ROLL_SPARE          -2

extern u16 D_8005CA80[MENU_SLIDE_STEPS];    /* position of a sliding item at each step, 0x1000: in place */
extern u16 D_8005CAC0[16];
extern u16 D_8005CB18[10];          /* image of each digit */
extern u16 D_8005CB04[10];          /* image of each small digit */
extern s16 D_8005CAE4[];           /* icon of each player, then of the computer */
extern s8 D_800DF5D0[];            /* number shown on each player's name plate; 0: none */

void func_800386E0( menuItem_t *items, u8 step );
void func_800389D0( s32 player, u8 cpu, u16 *name, s16 x, s16 y );
void func_80038AE8( s32 player, u8 cpu, u16 *name, u8 noIcon, s16 x, s16 y );
void func_80038C1C( u8 count, u8 first, s16 x, s16 y );
void func_80038D0C( u8 tenth, s16 x, s16 y );
void func_8003893C( menuItem_t *items, s16 id, s16 image, s16 x, s16 y, s16 alpha );
void func_800393D8( s16 value, s16 x, s16 y );
u16 func_80039340( s16 pins, u8 foul );
void func_80039554( s16 *digits, s16 value, u8 small, u8 tens );
void func_8003AE9C( s16 a, s16 b, u8 show, s16 x, s16 y );
void func_8003A8B4( s32 arg0, u8 count, u8 small, s16 x, s16 y );
void func_8003B1CC( s16 image, s16 x, s16 y );
s32 func_8003B204( void );

#endif
