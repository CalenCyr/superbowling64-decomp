/* 13490.c -- menus of images that slide in and out, and the scoreboard */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "save.h"
#include "sprite.h"
#include "menu.h"

INCLUDE_ASM("asm/nonmatchings/13490", func_80038090);

INCLUDE_ASM("asm/nonmatchings/13490", func_80038214);

/*
================
Menu_SlidePos

Returns pos, of an item of size, at the slide position scale (0x1000: in place) on its way out past edge.
================
*/
static inline s32 Menu_SlidePos( s16 pos, s16 size, s32 edge, u16 scale ) {
    return scale * ( pos + ( size / 2 - edge ) ) / 0x1000 + edge - size / 2;
}

/*
================
func_800386E0

Draws the menu items at step of their slide: 0 in place, MENU_SLIDE_STEPS - 1 off the screen.
================
*/
void func_800386E0( menuItem_t *items, u8 step ) {
    fileImage_t *image;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s32 i;

    for ( i = 0; items[i].image != MENU_END; i++ ) {
        if ( items[i].alpha == 0 ) {
            continue;
        }
        x = items[i].x;
        y = items[i].y;
        if ( D_800F80B0[items[i].image].refCount == 0 ) {
            func_80030C04( items[i].image, FILE_RAW );
        }
        image = D_800F80B0[items[i].image].data;
        width = image->width;
        height = image->height;
        switch ( items[i].slide ) {
        case SLIDE_TOP:
            y = Menu_SlidePos( y, height, -80, D_8005CA80[step] );
            break;
        case SLIDE_BOTTOM:
            y = Menu_SlidePos( y, height, SCREEN_WIDTH, D_8005CA80[step] );
            break;
        case SLIDE_LEFT:
            x = Menu_SlidePos( x, width, -140, D_8005CA80[step] );
            break;
        case SLIDE_RIGHT:
            x = Menu_SlidePos( x, width, 460, D_8005CA80[step] );
            break;
        }
        func_8002E4B0( items[i].image, x, y, items[i].alpha, 1.0f, 1.0f );
    }
}

/*
================
func_8003893C

Sets the image, position and alpha of the items id of the menu items; MENU_NONE keeps a member.
================
*/
void func_8003893C( menuItem_t *items, s16 id, s16 image, s16 x, s16 y, s16 alpha ) {
    s32 i;

    for ( i = 0; items[i].image != MENU_END; i++ ) {
        if ( items[i].id == id ) {
            if ( image != MENU_NONE ) {
                items[i].image = image;
            }
            if ( x != MENU_NONE ) {
                items[i].x = x;
            }
            if ( y != MENU_NONE ) {
                items[i].y = y;
            }
            if ( alpha != MENU_NONE ) {
                items[i].alpha = alpha;
            }
        }
    }
}

/*
================
func_800389D0

Draws the name plate of player at (x, y): its frame, its number, the icon of the player or of the computer,
and name.
================
*/
void func_800389D0( s32 player, u8 cpu, u16 *name, s16 x, s16 y ) {
    s16 *icons;

    icons = D_8005CAE4;
    func_8002DF3C( 0x90B, x, y );
    if ( D_800DF5D0[player] != 0 ) {
        func_800393D8( D_800DF5D0[player], x - 6, y + 12 );
    }
    func_8003B1CC( icons[cpu ? 4 : player], x + 2, y + 2 );
    func_80029088( &D_800F8094, x + 14, y + 2, 8, name );
}

/*
================
func_80038AE8

Draws the small name plate of player at (x, y): its frame and name, and the icon unless noIcon is set.
================
*/
void func_80038AE8( s32 player, u8 cpu, u16 *name, u8 noIcon, s16 x, s16 y ) {
    s16 *icons;

    icons = D_8005CAE4;
    func_8002DF3C( 0x9FE, x, y );
    if ( !noIcon ) {
        func_8003B1CC( icons[cpu ? 4 : player], x + 2, y + 1 );
        func_80029088( &D_800F8094, x + 3, y + 10, 8, name );
    } else {
        func_80029088( &D_800F8094, x + 3, y + 6, 8, name );
    }
}

/*
================
func_80038C1C

Draws the headers of count frames of the scoreboard from frame first, at (x, y): the frame number, or the
tenth frame's own header.
================
*/
void func_80038C1C( u8 count, u8 first, s16 x, s16 y ) {
    u16 *images;
    s32 i;
    s32 frame;

    images = D_8005CAC0;
    for ( i = 0; i < count; i++ ) {
        frame = first + i;
        if ( frame == 9 ) {
            func_8002DF3C( 0x87B, x, y );
        } else {
            func_8002DF3C( 0x87C, x, y );
            func_8002DF3C( images[frame + 1], x + 7, y + 1 );
        }
        x += 20;
    }
}

/*
================
func_80038D0C

Draws the headers of the scoreboard's frames at (x, y): of the tenth frame alone when tenth is set, else of
frames 1 to 9.
================
*/
void func_80038D0C( u8 tenth, s16 x, s16 y ) {
    u16 *images;
    s32 i;

    images = D_8005CAC0;
    if ( tenth ) {
        func_8002DF3C( 0x87B, x + 16, y );
        return;
    }
    for ( i = 0; i < 9; i++ ) {
        func_8002DF3C( 0x87C, x, y );
        func_8002DF3C( images[i + 1], x + 7, y + 1 );
        x += 24;
    }
}

INCLUDE_ASM("asm/nonmatchings/13490", func_80038DD4);

/*
================
func_80039340

Returns the scoreboard image of a roll of pins: its number, a strike, a spare, or a gutter (a foul with foul
set); 0xFFFF when there is none.
================
*/
u16 func_80039340( s16 pins, u8 foul ) {
    u16 *images;

    images = D_8005CAC0;
    if ( pins >= 1 && pins <= 9 ) {
        return images[pins];
    }
    if ( pins == ROLL_STRIKE && foul ) {
        return images[SCORE_IMAGE_STRIKE];
    }
    if ( pins == ROLL_SPARE ) {
        return images[SCORE_IMAGE_SPARE];
    }
    if ( pins == 0 ) {
        if ( foul ) {
            return images[SCORE_IMAGE_FOUL];
        }
        return images[SCORE_IMAGE_GUTTER];
    }
    return 0xFFFF;
}

/*
================
func_800393D8

Draws value, up to 999, in three digits at (x, y), without leading zeros; nothing when value is -1.
================
*/
void func_800393D8( s16 value, s16 x, s16 y ) {
    u16 *images;
    s16 digits[3];
    s16 hundreds;
    s16 tens;
    s16 ones;

    images = D_8005CAC0;
    if ( value == -1 ) {
        digits[0] = digits[1] = digits[2] = -1;
    } else {
        hundreds = value / 100;
        ones = value - hundreds * 100;
        tens = ones / 10;
        ones -= tens * 10;
        if ( hundreds != 0 ) {
            digits[0] = images[hundreds];
        } else {
            digits[0] = -1;
        }
        if ( ( hundreds | tens ) != 0 ) {
            digits[1] = images[tens];
        } else {
            digits[1] = -1;
        }
        digits[2] = images[ones];
    }
    func_8003B1CC( digits[0], x, y );
    func_8003B1CC( digits[1], x + 6, y );
    func_8003B1CC( digits[2], x + 12, y );
}

INCLUDE_ASM("asm/nonmatchings/13490", func_80039554);

INCLUDE_ASM("asm/nonmatchings/13490", func_80039700);

INCLUDE_ASM("asm/nonmatchings/13490", func_80039CBC);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003A074);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003A21C);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003A3F0);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003A658);

/*
================
func_8003A8B4

Draws count pieces of a box at (x, y): its left end, then its body; small ones when small is set.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: the selects of the images and of the offset; retail computes all three from one mask before the loop, this selects the offset with a branch
void func_8003A8B4( s32 arg0, u8 count, u8 small, s16 x, s16 y ) {
    s32 i;
    s32 offset;

    offset = small ? 16 : 36;
    for ( i = 0; i < count; i++ ) {
        if ( i == 0 ) {
            func_8002DF3C( small ? 0x865 : 0x867, x, y );
        } else {
            func_8002DF3C( small ? 0x864 : 0x866, x + offset, y );
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", func_8003A8B4);
#endif

INCLUDE_ASM("asm/nonmatchings/13490", func_8003A970);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003AE9C);

INCLUDE_ASM("asm/nonmatchings/13490", func_8003B034);

/*
================
func_8003B1CC

Draws image at (x, y), unless it is negative.
================
*/
void func_8003B1CC( s16 image, s16 x, s16 y ) {
    if ( image >= 0 ) {
        func_8002DF3C( image, x, y );
    }
}

/*
================
func_8003B204

Runs the two-choice prompt 0x957 for a frame; returns 1 or 2 for the choice taken, 0 until then.
================
*/
s32 func_8003B204( void ) {
    D_800DF590[0] = D_800DF590[1] = 0x24;
    D_800E6538[0] = D_800E6538[1] = 0x10;
    D_800E57E0[0] = 0x36;
    D_800E4D68[0] = 0x1E;
    D_800E57E0[1] = 0x5E;
    D_800E4D68[1] = 0x1E;
    if ( func_80036E60( 0x957, PROMPT_CHOICE2_H ) ) {
        if ( D_80066C44 == 0 ) {
            return 1;
        }
        if ( D_80066C44 == 1 ) {
            D_80066C44 = 0;
            return 2;
        }
    }
    return 0;
}
