/* 13490.c -- menus of images that slide in and out, and the scoreboard */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "game.h"
#include "heap.h"
#include "save.h"
#include "sprite.h"
#include "menu.h"

/*
================
func_80038090

Draws maze, (2 * width + 1) rows of (2 * height + 1) bytes (0: path, 1: wall), into image, a 64 x 64 CI4
image (32 bytes a row), one pixel per byte.
================
*/
void func_80038090( u16 image, u32 width, u32 height, u8 *maze ) {
    u8 *pixels;
    s32 row;
    s32 col;

    func_80030C04( image, FILE_RAW );
    pixels = ( (fileImage_t *)D_800F80B0[image].data )->data + CI4_PALETTE_SIZE;
    memset( pixels, 0, 0x800 );
    for ( row = 0; row < width * 2 + 1; row++ ) {
        for ( col = 0; col < height * 2 + 1; col++ ) {
            if ( maze[row * ( height * 2 + 1 ) + col] == 0 ) {
                if ( col & 1 ) {
                    pixels[row * 32 + col / 2] &= 0xF0;
                } else {
                    pixels[row * 32 + col / 2] &= 0x0F;
                }
            } else if ( maze[row * ( height * 2 + 1 ) + col] == 1 ) {
                if ( col & 1 ) {
                    pixels[row * 32 + col / 2] |= 0x01;
                } else {
                    pixels[row * 32 + col / 2] |= 0x10;
                }
            }
        }
    }
}

/*
================
func_80038214

Builds a random width x height maze into maze, (2 * width + 1) rows of (2 * height + 1) bytes (1: wall), by
opening the walls between cells in a random order whenever the two cells are not yet connected; the entrance is
on the left of the first row, the exit on the right of the last.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: calls, CFG, constants and the frame match; registers differ only in the inner loops: retail keeps
// the inner counter j (and the first cell of a wall) in a1 and the walking pointer or bound in a0, this draft swaps
// them (declaration order of the locals tried, no effect; a stride = height * 2 + 1 local is much worse).
// Permuter clue (target-only scoring, best 325 from 415): reusing a u32 local for height * 2 in the two final
// stores (maze[row + 1], ... * (row + 1)) moves allocation toward retail; the inner-loop a0/a1 swap remains.
// Also checked: wall as the best-wall search counter (worse), bestWall = 0 before best = 0, weight[j] > best and
// from == set[j] (no effect).
void func_80038214( u32 width, u32 height, u8 *maze ) {
    s32 i;
    s32 j;
    mazeWork_t *work;
    u16 *set;
    u8 *closed;
    u16 *weight;
    u16 *order;
    u16 walls;          /* (width - 1) * height between rows, then width * (height - 1) within them */
    u16 best;
    u16 bestWall;
    u32 wall;
    u32 cell;
    u32 next;
    u16 from;
    u16 to;
    u32 row;
    u32 col;

    work = func_80029E28( sizeof( mazeWork_t ) );
    set = work->set;
    closed = work->closed;
    weight = work->weight;
    order = work->order;
    walls = ( width - 1 ) * height + width * ( height - 1 );
    memset( work->closed, 1, sizeof( work->closed ) );
    for ( i = 0; i < walls; i++ ) {
        weight[i] = guRandom() % 0xFFFE + 1;
    }
    for ( i = 0; i < walls; i++ ) {
        best = 0;
        bestWall = 0;
        for ( j = 0; j < walls; j++ ) {
            if ( best < weight[j] ) {
                best = weight[j];
                bestWall = j;
            }
        }
        order[i] = bestWall;
        weight[bestWall] = 0;
    }
    for ( i = 0; i < width * height; i++ ) {
        set[i] = i;
    }
    for ( i = 0; i < walls; i++ ) {
        wall = order[i];
        if ( wall < ( width - 1 ) * height ) {
            cell = wall % ( width - 1 ) * height + wall / ( width - 1 );
            next = cell + height;
        } else {
            wall -= ( width - 1 ) * height;
            cell = wall % width * height + wall / width;
            next = cell + 1;
        }
        if ( set[cell] != set[next] ) {
            closed[order[i]] = 0;
            if ( set[cell] < set[next] ) {
                from = set[next];
                to = set[cell];
            } else {
                from = set[cell];
                to = set[next];
            }
            for ( j = 0; j < width * height; j++ ) {
                if ( set[j] == from ) {
                    set[j] = to;
                }
            }
        }
    }
    memset( maze, 0, width * height );
    for ( i = 0; i < width * 2 + 1; i++ ) {
        for ( j = 0; j < height * 2 + 1; j++ ) {
            maze[i * ( height * 2 + 1 ) + j] = 1;
        }
    }
    for ( i = 0; i < width; i++ ) {
        for ( j = 0; j < height; j++ ) {
            maze[( i * 2 + 1 ) * ( height * 2 + 1 ) + j * 2 + 1] = 0;
        }
    }
    for ( i = 0; i < walls; i++ ) {
        if ( closed[i] == 0 ) {
            if ( i < ( width - 1 ) * height ) {
                col = i / ( width - 1 ) * 2 + 1;
                row = i % ( width - 1 ) * 2 + 2;
            } else {
                col = ( i - ( width - 1 ) * height ) / width * 2 + 2;
                row = ( i - ( width - 1 ) * height ) % width * 2 + 1;
            }
            maze[row * ( height * 2 + 1 ) + col] = 0;
        }
    }
    maze[height * 2 + 1] = 0;
    maze[( width * 2 - 1 ) * ( height * 2 + 1 ) + height * 2] = 0;
    func_80029F18( work );
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", func_80038214);
#endif

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
inline void func_800389D0( s32 player, u8 cpu, u16 *name, s16 x, s16 y ) {
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
inline void func_80038C1C( u8 count, u8 first, s16 x, s16 y ) {
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

/*
================
func_80038DD4

Draws the player's frame of the scoreboard at (x, y), last being the frame in play and throw its throw: the
frame's box, the image of each roll played, and the frame's score.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: calls, CFG, constants and stack match; registers differ: retail gives rowY s4 (frame s5, arg4 s6)
// and roll v0 (so its bne delay slots are not likely-branches). Tried: rowY as s32 (worse), y itself as the cursor
// (same), roll by ternary/if (block 1 needs the if), foul as an s32 local (needed for retail's andi),
// rowY = y + 2 once before the frame test (worse).
// Permuter (target-only scoring, 7 min): best 615 from 865 only through noise (a u16 temporary for the third
// roll, a duplicated if/else), no source clue.
void func_80038DD4( s32 player, u8 frame, u8 last, u8 throw, u8 arg4, s16 x, s16 y ) {
    s16 rowY;
    s32 pins;
    s16 roll;
    s32 foul;

    if ( frame == 9 ) {
        func_8002DF3C( 0x87D, x, y );
        rowY = y + 2;
        if ( last == 9 && throw != 0 ) {
            if ( ( D_80099458[player][9] & 1 ) && ( D_80099458[player][9] & 0x10 ) ) {
                func_8002DF3C( 0x85F, x - 1, y );
            }
            pins = D_80066E18[player][frame];
            if ( pins == -1 ) {
                roll = -2;
            } else {
                roll = pins;
            }
            func_8003B1CC( func_80039340( roll, FALSE ), x + 1, rowY );
        }
        if ( last == 9 && ( throw >= 2 || arg4 ) ) {
            if ( ( D_80099458[player][frame] & 1 ) && ( D_80099458[player][frame] & 0x20 ) ) {
                func_8002DF3C( 0x85F, x + 9, rowY - 2 );
            }
            pins = D_800A3020[player][frame];
            roll = pins == -1 && D_80066E18[player][frame] == pins ? -2 : pins;
            foul = TRUE;
            if ( roll == 0 ) {
                foul = D_80066E18[player][frame] != -2;
            }
            func_8003B1CC( func_80039340( roll, foul ), x + 11, rowY );
        }
        if ( arg4 ) {
            if ( D_80066E18[player][frame] == -1 || D_800A3020[player][frame] == -1 ) {
                pins = D_800994B8[player];
                roll = pins == -1 && D_800A3020[player][frame] == pins ? -2 : pins;
                foul = TRUE;
                if ( roll == 0 ) {
                    foul = D_800A3020[player][frame] >= 0;
                }
                func_8003B1CC( func_80039340( roll, foul ), x + 21, rowY );
            }
            rowY += 9;
            pins = D_800DF630[player][frame];
            if ( pins >= 0 ) {
                pins += D_800DF5D0[player];
            }
            func_800393D8( pins, x + 5, rowY );
        }
    } else {
        func_8002DF3C( 0x87E, x, y );
        rowY = y + 2;
        if ( last > frame || ( last == frame && throw != 0 ) ) {
            if ( D_80099458[player][frame] != 0 ) {
                func_8002DF3C( 0x85F, x - 1, y );
            }
            func_8003B1CC( func_80039340( D_80066E18[player][frame], FALSE ), x + 1, rowY );
        }
        if ( frame < last ) {
            func_8003B1CC( func_80039340( D_800A3020[player][frame], TRUE ), x + 11, rowY );
            rowY += 9;
            pins = D_800DF630[player][frame];
            if ( pins >= 0 ) {
                pins += D_800DF5D0[player];
            }
            func_800393D8( pins, x, rowY );
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", func_80038DD4);
#endif

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

/*
================
func_80039554

Sets digits to the images of the four digits of value, up to 9999, from D_8005CB04 when small is set: -1, no
image, for leading zeros, but the tens are shown when tens is set.
================
*/
void func_80039554( s16 *digits, s16 value, u8 small, u8 tens ) {
    u16 *images;
    s16 thousands;
    s16 hundreds;
    s16 ten;
    s16 ones;

    thousands = value / 1000;
    ones = value - thousands * 1000;
    hundreds = ones / 100;
    ones -= hundreds * 100;
    ten = ones / 10;
    ones -= ten * 10;
    images = D_8005CB18;
    if ( small ) {
        images = D_8005CB04;
    }
    if ( thousands != 0 ) {
        digits[0] = images[thousands];
    } else {
        digits[0] = -1;
    }
    if ( thousands != 0 || hundreds != 0 ) {
        digits[1] = images[hundreds];
    } else {
        digits[1] = -1;
    }
    if ( tens ) {
        digits[2] = images[ten];
    } else if ( thousands != 0 || hundreds != 0 || ten != 0 ) {
        digits[2] = images[ten];
    } else {
        digits[2] = -1;
    }
    digits[3] = images[ones];
}

/*
================
func_80039700

Draws the scoreboard of the game at (x, y), mode 1 by teams of two players and mode 2 in two teams with their
totals: the numbered heads of the ten frames, then per player (or team) its name plates and its ten frames, all
of them when all is set.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: calls, CFG, constants and stack match; only the team plates' registers differ: retail gives the
// computer flag s0, player * 2 s5 and the icons s6 (this draft s6, s0, s5). Declaration order and spellings of
// player (ternary, if/else, u8) don't change it. Permuter (target-only scoring, 8 min): nothing below the base
// score 100.
// Also checked: the single-plate case first (mode != 1, worse), player = ( i == 1 ) * 2, player in the block's
// scope, player++ for the second plate (worse), frameX = x after the frame-3 header (worse), the helper indexing
// D_8005CAE4 without its icons local (worse here and in the other callers).
void func_80039700( u8 mode, u8 all, s16 x, s16 y ) {
    s32 i;
    s32 frame;
    s32 count;
    s32 player;
    s16 frameX;

    if ( mode == 2 ) {
        func_8002DF3C( 0xA12, x, y - 3 );
    }
    func_80038C1C( 10, 0, x + 0x38, y );
    y += 9;
    count = mode == 1 ? D_8005C161 / 2 : D_8005C161;
    for ( i = 0; i < count; i++ ) {
        frameX = x;
        if ( mode == 2 && i == 2 ) {
            func_8002DF3C( 0x9FB, frameX, y + 4 );
            y += 16;
        }
        if ( mode == 1 ) {
            player = i == 1 ? 2 : 0;
            func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, frameX, y );
            func_800389D0( player + 1, D_8005C0E4[player + 1] >> 7, D_80068218[player + 1].chars, frameX, y + 20 );
        } else {
            func_800389D0( i, D_8005C0E4[i] >> 7, D_80068218[i].chars, frameX, y );
        }
        frameX += 0x38;
        if ( mode == 1 ) {
            y += 10;
        }
        for ( frame = 0; frame < 9; frame++ ) {
            func_80038DD4( i, frame, all ? 9 : D_8009D19C[i], D_80068A50[i], D_800994BE[i], frameX, y );
            frameX += 20;
        }
        func_80038DD4( i, 9, all ? 9 : D_8009D19C[i], D_80068A50[i], D_800994BE[i], frameX, y );
        y += mode == 1 ? 40 : 20;
        if ( mode == 2 && i % 2 == 1 ) {
            func_8002DF3C( i == 1 ? 0xA13 : 0x9FC, 0xCC, y );
            func_800393D8( D_800EA090[i - 1] + D_800EA090[i], 0x10E, y + 4 );
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", func_80039700);
#endif

/*
================
func_80039CBC

Draws the score card at (x, y): its title, then a row of frame numbers, a row of each frame's par and the par
total, and a row per player with its name plate, its frames (func_8003A074, by kind and all) and its total
(func_8003A21C, kind as duel).
================
*/
void func_80039CBC( u8 kind, u8 all, s16 x, s16 y ) {
    s32 row;
    s32 player;
    s32 i;
    s16 rowX;
    s16 rowY;

    func_8002DF3C( !kind ? 0xA1C : 0xA2D, x, y );
    rowY = y + 16;
    for ( row = 0; row < D_8005C161 + 2; row++ ) {
        player = row - 2;
        rowX = x;
        switch ( row ) {
        case 0:
            func_8002DF3C( 0xA3A, rowX, rowY );
            break;
        case 1:
            func_8002DF3C( 0xA2C, rowX, rowY );
            break;
        default:
            func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, rowX, rowY );
            break;
        }
        rowX += 0x38;
        for ( i = 0; i < 9; i++ ) {
            func_8002DF3C( D_8005CB40[row], rowX, rowY );
            switch ( row ) {
            case 0:
                func_8002DF3C( D_8005CB2C[i], rowX + 6, rowY + 3 );
                break;
            case 1:
                func_800393D8( D_80099458[0][i], rowX - 6, rowY + 4 );
                break;
            default:
                func_8003A074( player, i, kind, all, rowX, rowY );
                break;
            }
            rowX += 18;
        }
        func_8002DF3C( D_8005CB4C[row], rowX, rowY );
        switch ( row ) {
        case 0:
            func_8002DF3C( 0x9E8, rowX, rowY );
            break;
        case 1:
            func_800393D8( D_8009CF80[0], rowX + 2, rowY + 4 );
            break;
        default:
            func_8003A21C( player, kind, rowX, rowY );
            break;
        }
        rowY += row < 2 ? 15 : 20;
    }
}

/*
================
func_8003A074

Draws the player's frame at (x, y), once it is played or when all is set: kind 0 its score, signed; otherwise
the image of its D_800A3020 entry.
================
*/
void func_8003A074( s32 player, u8 frame, u8 kind, u8 all, s16 x, s16 y ) {
    s16 images[3] = { -1, 0x9EA, 0x9EB };
    s16 score;
    s32 sign;

    if ( kind == 0 ) {
        score = D_800DF630[player][frame];
        if ( all || frame < D_8009D19C[player] ) {
            sign = score < 0 ? 0 : score > 0 ? 2 : 1;
            func_8002DF3C( D_8005CB58[sign], x + 3, y + 4 );
            func_800393D8( score < 0 ? -score : score, x - 2, y + 7 );
        }
    } else if ( all || frame < D_8009D19C[player] ) {
        func_8003B1CC( images[D_800A3020[player][frame]], x + 4, y + 3 );
    }
}

/*
================
func_8003A21C

Draws the player's total at (x, y): its score, signed, or in a duel whether the player leads, ties or trails
on frames won, and by how many.
================
*/
void func_8003A21C( s32 player, u8 duel, s16 x, s16 y ) {
    s16 images[3] = { 0x9ED, 0x9E9, 0x9EC };
    s16 digits[4];
    s16 score;
    s16 lead;
    s32 other;
    s32 sign;

    if ( !duel ) {
        score = D_800EA090[player];
        sign = score < 0 ? 0 : score > 0 ? 2 : 1;
        func_8002DF3C( D_8005CB58[sign], x + 4, y + 4 );
        func_800393D8( score < 0 ? -score : score, x + 8, y + 7 );
    } else {
        other = player != 1;
        sign = D_800994B8[player] > D_800994B8[other] ? 0 : D_800994B8[player] == D_800994B8[other] ? 1 : 2;
        func_8002DF3C( images[sign], x, y + 2 );
        lead = D_800994B8[player] - D_800994B8[other];
        if ( lead != 0 ) {
            func_80039554( digits, lead < 0 ? -lead : lead, FALSE, FALSE );
            func_8003B1CC( digits[3], x + 6, y + 4 );
        }
    }
}

/*
================
func_8003A3F0

Draws the scoreboard of every player at (x, y): the numbered heads of the ten frames, then per player its name
plate and its ten frames, marking the frames already played.
================
*/
void func_8003A3F0( s32 arg0, s16 x, s16 y ) {
    s32 i;
    s32 player;
    s16 frameX;

    func_80038C1C( 10, 0, x + 0x38, y );
    y += 9;
    for ( player = 0; player < D_8005C161; player++ ) {
        frameX = x;
        func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, frameX, y );
        frameX += 0x38;
        for ( i = 0; i < 10; i++ ) {
            func_8002DF3C( i == 9 ? 0xA36 : 0xAD3, frameX, y );
            if ( i < D_8009D19C[player] ) {
                func_8002DF3C( 0x9EB, frameX + ( i == 9 ? 10 : 5 ), y + 3 );
            }
            frameX += 20;
        }
        y += 20;
    }
}

/*
================
func_8003A658

Draws the scoreboard of player at (x, y): the numbered heads of the ten frames, the player's name plate, then the
ten frames.
================
*/
void func_8003A658( s32 player, s16 x, s16 y ) {
    s32 i;
    s16 frameX;

    func_80038C1C( 10, 0, x + 0x38, y );
    y += 9;
    frameX = x;
    func_800389D0( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, frameX, y );
    frameX += 0x38;
    for ( i = 0; i < 9; i++ ) {
        func_80038DD4( player, i, 9, D_80068A50[player], D_800994BE[player], frameX, y );
        frameX += 20;
    }
    func_80038DD4( player, 9, 9, D_80068A50[player], D_800994BE[player], frameX, y );
}

/*
================
func_8003A8B4

Draws count pieces of a box at (x, y): its left end, then its body; small ones when small is set.
================
*/
void func_8003A8B4( s32 arg0, u8 count, u8 small, s16 x, s16 y ) {
    s32 i;
    s32 offset;

    offset = small ? 16 : 20;
    for ( i = 0; i < count; i++ ) {
        if ( i == 0 ) {
            func_8002DF3C( small ? 0x865 : 0x867, x, y );
        } else {
            func_8002DF3C( small ? 0x864 : 0x866, x + offset, y );
        }
    }
}

/*
================
func_8003A970

Draws the table of count records of the given kind at (x, y): its column heads, then a row per record with its
name plate and, once it holds a game, its games, average, high and low scores and its three percentages.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: prologue, calls, CFG and constants match (func_8003AE9C takes s32 numbers: retail passes the
// lhu field and the truncated float as they are); the frame is 8 bytes short (0x98, retail 0xA0): retail spills x
// to sp+0x2E, reloads it each row and adds 0x2C there (s5 = x + 0x2C, then s5 + 2 ...), while this draft keeps x
// in s5 and folds the offsets (x + 0x2E); rowY + 2 as topY reproduces retail's early s2. Tried: rowX/rowY/played
// as s16/s32/u8, the inline small plate func_80038AE8 (no change), x and rowX as u16 (no change;
// retail reloads x with lhu), the name plate drawn at x with rowX = x + 0x2C (worse).
void func_8003A970( u16 count, UnkRecord *records, u8 kind, s16 x, s32 y ) {
    s16 digits[4];
    s32 i;
    s32 played;
    s16 rowY;
    s16 rowX;
    s16 topY;      /* of the upper number */

    func_8002DF3C( 0x9E1, x + 0x2C, y );
    func_8002DF3C( 0xA15, x + 0x52, y );
    func_8002DF3C( 0xA11, x + 0x78, y );
    func_8002DF3C( 0x9E2, x + 0x9E, y );
    func_8002DF3C( 0x9E4, x + 0xC4, y );
    func_8002DF3C( 0x9E3, x + 0xEA, y );
    rowY = y + 16;
    for ( i = 0; i < count; i++ ) {
        played = records[i].games != 0;
        rowX = x;
        func_8002DF3C( 0x9FE, rowX, rowY );
        func_80029088( &D_800F8094, rowX + 3, rowY + 6, 8, records[i].name );
        rowX += 0x2C;
        topY = rowY + 2;
        func_8002DF3C( 0x9E7, rowX, rowY );
        if ( played ) {
            func_80039554( digits, records[i].games, TRUE, FALSE );
            func_8003B1CC( digits[0], rowX + 2, rowY + 7 );
            func_8003B1CC( digits[1], rowX + 10, rowY + 7 );
            func_8003B1CC( digits[2], rowX + 18, rowY + 7 );
            func_8003B1CC( digits[3], rowX + 26, rowY + 7 );
        }
        func_8002DF3C( played ? 0x9E6 : 0x9E7, rowX + 0x26, rowY );
        if ( played ) {
            func_80039554( digits, D_800E66B0[kind][i] * 10.0f, TRUE, TRUE );
            func_8003B1CC( digits[0], rowX + 0x2A, rowY + 7 );
            func_8003B1CC( digits[1], rowX + 0x31, rowY + 7 );
            func_8003B1CC( digits[2], rowX + 0x39, rowY + 7 );
            func_8003B1CC( digits[3], rowX + 0x43, rowY + 7 );
        }
        func_8002DF3C( 0x9E7, rowX + 0x4C, rowY );
        if ( played ) {
            func_80039554( digits, records[i].high, FALSE, FALSE );
            func_8003B1CC( digits[0], rowX + 0x4E, topY );
            func_8003B1CC( digits[1], rowX + 0x56, topY );
            func_8003B1CC( digits[2], rowX + 0x5E, topY );
            func_8003B1CC( digits[3], rowX + 0x66, topY );
            func_80039554( digits, records[i].low, FALSE, FALSE );
            func_8003B1CC( digits[0], rowX + 0x4E, rowY + 14 );
            func_8003B1CC( digits[1], rowX + 0x56, rowY + 14 );
            func_8003B1CC( digits[2], rowX + 0x5E, rowY + 14 );
            func_8003B1CC( digits[3], rowX + 0x66, rowY + 14 );
        }
        func_8003AE9C( records[i].unk_14, D_800DF5D8[kind][i] * 10.0f, played, rowX + 0x72, rowY );
        func_8003AE9C( records[i].unk_16, D_800A2FE8[kind][i] * 10.0f, played, rowX + 0x98, rowY );
        func_8003AE9C( records[i].unk_18, D_80099488[kind][i] * 10.0f, played, rowX + 0xBE, rowY );
        rowY += 0x1C;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", func_8003A970);
#endif

/*
================
func_8003AE9C

Draws the box of two numbers at (x, y), a above b, or the empty box when show is clear.
================
*/
void func_8003AE9C( s32 a, s32 b, u8 show, s16 x, s16 y ) {
    s16 digits[4];
    s16 topY;      /* of the upper number */

    topY = y + 2;
    if ( show ) {
        func_8002DF3C( 0x9E5, x, y );
        func_80039554( digits, a, FALSE, FALSE );
        func_8003B1CC( digits[0], x + 4, topY );
        func_8003B1CC( digits[1], x + 10, topY );
        func_8003B1CC( digits[2], x + 16, topY );
        func_8003B1CC( digits[3], x + 22, topY );
        func_80039554( digits, b, FALSE, TRUE );
        func_8003B1CC( digits[0], x + 4, y + 14 );
        func_8003B1CC( digits[1], x + 10, y + 14 );
        func_8003B1CC( digits[2], x + 16, y + 14 );
        func_8003B1CC( digits[3], x + 24, y + 14 );
    } else {
        func_8002DF3C( 0x9E7, x, y );
    }
}

/*
================
func_8003B034

Sets the figures of record, of character index, in the kind's row of D_800E66B0, D_800DF5D8, D_800A2FE8 and
D_80099488; all 0 when it holds no game.
================
*/
void func_8003B034( UnkRecord *record, s32 index, u8 kind ) {
    if ( record->games == 0 ) {
        D_800E66B0[kind][index] = D_800DF5D8[kind][index] = D_800A2FE8[kind][index] = D_80099488[kind][index] = 0.0f;
    } else {
        D_800E66B0[kind][index] = (f32)record->total / record->games;
        D_800DF5D8[kind][index] = record->unk_14 * 100.0f / ( record->games * 12 );
        D_800A2FE8[kind][index] = record->unk_16 * 100.0f / ( record->games * 10 );
        D_80099488[kind][index] = record->unk_18 * 100.0f / ( record->games * 10 );
    }
}

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
