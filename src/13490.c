/* 13490.c -- menus of images that slide in and out, and the scoreboard */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "game.h"
#include "heap.h"
#include "save.h"
#include "sprite.h"
#include "menu.h"

/* ===== Data ===== */

static u16 menu_slidePositions[MENU_SLIDE_STEPS] = {  /* position of a sliding item at each step, 0x1000: in place */
    0x1000, 0x0FF6, 0x0FEC, 0x0FD7, 0x0FB5, 0x0F89, 0x0F53, 0x0F14,
    0x0ECB, 0x0E78, 0x0E1B, 0x0DB5, 0x0D45, 0x0CCB, 0x0C48, 0x0BBB,
    0x0B24, 0x0A83, 0x09D8, 0x0924, 0x0866, 0x079F, 0x06CD, 0x05F2,
    0x050D, 0x041F, 0x0326, 0x0224, 0x0118, 0x0003, 0x0001, 0x0000,
};
static u16 score_images[16] = {             /* image of each digit, then of the scoreboard's symbols */
    0x0A56, 0x0A55, 0x0A54, 0x0A53, 0x0A52, 0x0A51, 0x0A50, 0x0A4F,
    0x0A4E, 0x0A4D, 0x0861, 0x085D, 0x0A4A, 0x0A4C, 0x090C, 0x085F,
};
static s32 D_8005CAE0 = 0;             /* unreferenced */
static s16 score_playerIcons[16] = {        /* icon of each player, then of the computer */
    0x08DA, 0x08D8, 0x08D6, 0x08D4, 0x08CE, 0x0000, -1, 0x09DC,
    0x09DB, 0x09DA, 0x09D9, 0x09D8, 0x09D7, 0x09D6, 0x09D5, 0x09D4,
};
static u16 score_smallDigitImages[10] = {   /* image of each small digit */
    0x09FA, 0x09F9, 0x09F8, 0x09F7, 0x09F6, 0x09F5, 0x09F4, 0x09F3, 0x09F2, 0x09F1,
};
static u16 score_digitImages[10] = {        /* image of each digit */
    0x09FA, 0x09F9, 0x09F8, 0x09F7, 0x09F6, 0x09F5, 0x09F4, 0x09F3, 0x09F2, 0x09F1,
};
static u16 score_parFrameImages[9] = {      /* Score_DrawParCard: image of each frame's number */
    0x0A44, 0x0A43, 0x0A42, 0x0A41, 0x0A40, 0x0A3F, 0x0A3E, 0x0A3D, 0x0A3C,
};
static u16 score_parFrameBoxes[6] = {       /* Score_DrawParCard: per row, the image of a frame's box */
    0x0A39, 0x0A39, 0x0A37, 0x0A37, 0x0A37, 0x0A37,
};
static u16 score_parTotalBoxes[6] = {       /* Score_DrawParCard: per row, the image of the total's box */
    0x0A38, 0x0A38, 0x0A36, 0x0A36, 0x0A36, 0x0A36,
};
static u16 score_signImages[4] = {          /* Score_DrawCardFrame: sign of a score: negative, zero, positive */
    0x09EF, 0x09EE, 0x09F0, 0x0000,
};
u8 game_replay = 0;

/*
================
Maze_Draw

Draws maze, (2 * width + 1) rows of (2 * height + 1) bytes (0: path, 1: wall), into image, a 64 x 64 CI4
image (32 bytes a row), one pixel per byte.
================
*/
void Maze_Draw( u16 image, u32 width, u32 height, u8 *maze ) {
    u8 *pixels;
    s32 row;
    s32 col;

    Cache_GetFile( image, FILE_RAW );
    pixels = ( (fileImage_t *)cache_files[image].data )->data + CI4_PALETTE_SIZE;
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
Maze_Build

Builds a random width x height maze into maze, (2 * width + 1) rows of (2 * height + 1) bytes (1: wall), by
opening the walls between cells in a random order whenever the two cells are not yet connected; the entrance is
on the left of the first row, the exit on the right of the last.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with locals doing two jobs (marked R30 below): retail keeps the
// inner counter j, the first cell of a wall and the last loop's i - (width - 1) * height in one register (a1), and
// the last loop's row in next's; that long life is what drops j below the inner loops' pointers and bounds in
// allocation order. With separate cell/row locals the inner loops swap a0/a1 (36 lines); j alone as the last
// loop's counter, or a fresh local for its offset, fixes only some loops (row in wall instead of next also works).
// Also checked: declaration order, a stride local, wall as the best-wall counter, bestWall = 0 first,
// weight[j] > best, from == set[j] (no effect or worse).
void Maze_Build( u32 width, u32 height, u8 *maze ) {
    s32 i;
    // RULE-EXCEPTION: R30, j is also the first cell of a wall and the last loop's offset of a wall within the
    // walls between columns, and next is also the last loop's row; separate locals change the register allocation.
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
    u32 next;
    u16 from;
    u16 to;
    u32 col;

    work = Mem_Alloc( sizeof( mazeWork_t ) );
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
            j = wall % ( width - 1 ) * height + wall / ( width - 1 );
            next = j + height;
        } else {
            wall -= ( width - 1 ) * height;
            j = wall % width * height + wall / width;
            next = j + 1;
        }
        if ( set[j] != set[next] ) {
            closed[order[i]] = 0;
            if ( set[j] < set[next] ) {
                from = set[next];
                to = set[j];
            } else {
                from = set[j];
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
                next = i % ( width - 1 ) * 2 + 2;
            } else {
                j = i - ( width - 1 ) * height;
                col = j / width * 2 + 2;
                next = j % width * 2 + 1;
            }
            maze[next * ( height * 2 + 1 ) + col] = 0;
        }
    }
    maze[height * 2 + 1] = 0;
    maze[( width * 2 - 1 ) * ( height * 2 + 1 ) + height * 2] = 0;
    Mem_Free( work );
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", Maze_Build);
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
Menu_DrawItems

Draws the menu items at step of their slide: 0 in place, MENU_SLIDE_STEPS - 1 off the screen.
================
*/
void Menu_DrawItems( menuItem_t *items, u8 step ) {
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
        if ( cache_files[items[i].image].refCount == 0 ) {
            Cache_GetFile( items[i].image, FILE_RAW );
        }
        image = cache_files[items[i].image].data;
        width = image->width;
        height = image->height;
        switch ( items[i].slide ) {
        case SLIDE_TOP:
            y = Menu_SlidePos( y, height, -80, menu_slidePositions[step] );
            break;
        case SLIDE_BOTTOM:
            y = Menu_SlidePos( y, height, SCREEN_WIDTH, menu_slidePositions[step] );
            break;
        case SLIDE_LEFT:
            x = Menu_SlidePos( x, width, -140, menu_slidePositions[step] );
            break;
        case SLIDE_RIGHT:
            x = Menu_SlidePos( x, width, 460, menu_slidePositions[step] );
            break;
        }
        Draw_ImageScaled( items[i].image, x, y, items[i].alpha, 1.0f, 1.0f );
    }
}

/*
================
Menu_SetItem

Sets the image, position and alpha of the items id of the menu items; MENU_NONE keeps a member.
================
*/
void Menu_SetItem( menuItem_t *items, s16 id, s16 image, s16 x, s16 y, s16 alpha ) {
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
Menu_DrawNamePlate

Draws the name plate of player at (x, y): its frame, its number, the icon of the player or of the computer,
and name.
================
*/
inline void Menu_DrawNamePlate( s32 player, u8 cpu, u16 *name, s16 x, s16 y ) {
    s16 *icons;

    icons = score_playerIcons;
    Draw_Image( 0x90B, x, y );
    if ( score_plateNumbers[player] != 0 ) {
        Score_DrawNumber( score_plateNumbers[player], x - 6, y + 12 );
    }
    Menu_DrawImage( icons[cpu ? 4 : player], x + 2, y + 2 );
    Txt_DrawGlyphs( &game_gfx, x + 14, y + 2, 8, name );
}

/*
================
Menu_DrawSmallNamePlate

Draws the small name plate of player at (x, y): its frame and name, and the icon unless noIcon is set.
================
*/
void Menu_DrawSmallNamePlate( s32 player, u8 cpu, u16 *name, u8 noIcon, s16 x, s16 y ) {
    s16 *icons;

    icons = score_playerIcons;
    Draw_Image( 0x9FE, x, y );
    if ( !noIcon ) {
        Menu_DrawImage( icons[cpu ? 4 : player], x + 2, y + 1 );
        Txt_DrawGlyphs( &game_gfx, x + 3, y + 10, 8, name );
    } else {
        Txt_DrawGlyphs( &game_gfx, x + 3, y + 6, 8, name );
    }
}

/*
================
Score_DrawFrameHeaders

Draws the headers of count frames of the scoreboard from frame first, at (x, y): the frame number, or the
tenth frame's own header.
================
*/
inline void Score_DrawFrameHeaders( u8 count, u8 first, s16 x, s16 y ) {
    u16 *images;
    s32 i;
    s32 frame;

    images = score_images;
    for ( i = 0; i < count; i++ ) {
        frame = first + i;
        if ( frame == 9 ) {
            Draw_Image( 0x87B, x, y );
        } else {
            Draw_Image( 0x87C, x, y );
            Draw_Image( images[frame + 1], x + 7, y + 1 );
        }
        x += 20;
    }
}

/*
================
Score_DrawHeaders

Draws the headers of the scoreboard's frames at (x, y): of the tenth frame alone when tenth is set, else of
frames 1 to 9.
================
*/
void Score_DrawHeaders( u8 tenth, s16 x, s16 y ) {
    u16 *images;
    s32 i;

    images = score_images;
    if ( tenth ) {
        Draw_Image( 0x87B, x + 16, y );
        return;
    }
    for ( i = 0; i < 9; i++ ) {
        Draw_Image( 0x87C, x, y );
        Draw_Image( images[i + 1], x + 7, y + 1 );
        x += 24;
    }
}

/*
================
Score_DrawFrame

Draws the player's frame of the scoreboard at (x, y), last being the frame in play and throw its throw: the
frame's box, the image of each roll played, and the frame's score; done is game_playerDone, set once the
player has finished the game.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: calls, CFG, constants and stack match; registers differ: retail gives rowY s4 (frame s5, done s6)
// and roll v0 (so its bne delay slots are not likely-branches). Tried: rowY as s32 (worse), y itself as the cursor
// (same), roll by ternary/if (block 1 needs the if), foul as an s32 local (needed for retail's andi),
// rowY = y + 2 once before the frame test (worse).
// Permuter (target-only scoring, 7 min): best 615 from 865 only through noise (a u16 temporary for the third
// roll, a duplicated if/else), no source clue.
// A further 20-minute run reached 275 only by holding -2 in a local shared with the foul test (left unset on one
// path). No effect: roll = pins then the -1 test, pins != -1 first, rowY = y before the first Draw_Image.
// Dumps (-dl/-dg): rowY (9 refs over 246 insns) loses to frame (10 over 238) and to the rowY + 9 temporary, which cse
// substitutes for rowY at its one later use, so the temporary takes a2; retail's in-place s4 += 9 follows if rowY is
// allocated first. No effect or worse: rowY declared in each branch (worse), rowY += 9 just before the call (worse).
// Also worse: rowY + 9 written at the Score_DrawNumber calls, y itself stepped with y += 2, rowY = y with + 2 at
// each use; rowY = y then rowY += 2 is the same. A 35-minute permuter run: 385 only through throw++/throw-- (noise).
// Dumps (-ds): cse rewrites the call argument's sign extension of rowY to the rowY + 9 temporary, so flow deletes
// rowY's set; with the set kept, combine would add in place and rowY's extra refs would rank it above frame. No effect
// or worse: rowY as u16, y as an s32 parameter (with rowY s16 or s32).
// Restricted permuter search (perm macros only, 25 min): declaration order, pins s32/s16, foul s32/u8/s16, rowY += 9
// or rowY = rowY + 9, the -2 tests' operand orders, foul set by its if or by ||: nothing at or below the base 865.
void Score_DrawFrame( s32 player, u8 frame, u8 last, u8 throw, u8 done, s16 x, s16 y ) {
    s16 rowY;
    s32 pins;
    s16 roll;
    s32 foul;

    if ( frame == 9 ) {
        Draw_Image( 0x87D, x, y );
        rowY = y + 2;
        if ( last == 9 && throw != 0 ) {
            if ( ( game_frameFlags[player][9] & 1 ) && ( game_frameFlags[player][9] & 0x10 ) ) {
                Draw_Image( 0x85F, x - 1, y );
            }
            pins = game_firstRolls[player][frame];
            if ( pins == -1 ) {
                roll = -2;
            } else {
                roll = pins;
            }
            Menu_DrawImage( Score_RollImage( roll, FALSE ), x + 1, rowY );
        }
        if ( last == 9 && ( throw >= 2 || done ) ) {
            if ( ( game_frameFlags[player][frame] & 1 ) && ( game_frameFlags[player][frame] & 0x20 ) ) {
                Draw_Image( 0x85F, x + 9, rowY - 2 );
            }
            pins = game_secondRolls[player][frame];
            roll = pins == -1 && game_firstRolls[player][frame] == pins ? -2 : pins;
            foul = TRUE;
            if ( roll == 0 ) {
                foul = game_firstRolls[player][frame] != -2;
            }
            Menu_DrawImage( Score_RollImage( roll, foul ), x + 11, rowY );
        }
        if ( done ) {
            if ( game_firstRolls[player][frame] == -1 || game_secondRolls[player][frame] == -1 ) {
                pins = D_800994B8[player];
                roll = pins == -1 && game_secondRolls[player][frame] == pins ? -2 : pins;
                foul = TRUE;
                if ( roll == 0 ) {
                    foul = game_secondRolls[player][frame] >= 0;
                }
                Menu_DrawImage( Score_RollImage( roll, foul ), x + 21, rowY );
            }
            rowY += 9;
            pins = game_frameScores[player][frame];
            if ( pins >= 0 ) {
                pins += score_plateNumbers[player];
            }
            Score_DrawNumber( pins, x + 5, rowY );
        }
    } else {
        Draw_Image( 0x87E, x, y );
        rowY = y + 2;
        if ( last > frame || ( last == frame && throw != 0 ) ) {
            if ( game_frameFlags[player][frame] != 0 ) {
                Draw_Image( 0x85F, x - 1, y );
            }
            Menu_DrawImage( Score_RollImage( game_firstRolls[player][frame], FALSE ), x + 1, rowY );
        }
        if ( frame < last ) {
            Menu_DrawImage( Score_RollImage( game_secondRolls[player][frame], TRUE ), x + 11, rowY );
            rowY += 9;
            pins = game_frameScores[player][frame];
            if ( pins >= 0 ) {
                pins += score_plateNumbers[player];
            }
            Score_DrawNumber( pins, x, rowY );
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", Score_DrawFrame);
#endif

/*
================
Score_RollImage

Returns the scoreboard image of a roll of pins: its number, a strike, a spare, or a gutter (a foul with foul
set); 0xFFFF when there is none.
================
*/
u16 Score_RollImage( s16 pins, u8 foul ) {
    u16 *images;

    images = score_images;
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
Score_DrawNumber

Draws value, up to 999, in three digits at (x, y), without leading zeros; nothing when value is -1.
================
*/
void Score_DrawNumber( s16 value, s16 x, s16 y ) {
    u16 *images;
    s16 digits[3];
    s16 hundreds;
    s16 tens;
    s16 ones;

    images = score_images;
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
    Menu_DrawImage( digits[0], x, y );
    Menu_DrawImage( digits[1], x + 6, y );
    Menu_DrawImage( digits[2], x + 12, y );
}

/*
================
Score_GetDigits

Sets digits to the images of the four digits of value, up to 9999, from score_smallDigitImages when small is set: -1, no
image, for leading zeros, but the tens are shown when tens is set.
================
*/
void Score_GetDigits( s16 *digits, s16 value, u8 small, u8 tens ) {
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
    images = score_digitImages;
    if ( small ) {
        images = score_smallDigitImages;
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
Score_DrawTeamBoard

Draws the scoreboard of the game at (x, y), mode 1 by teams of two players and mode 2 in two teams with their
totals: the numbered heads of the ten frames, then per player (or team) its name plates and its ten frames, all
of them when all is set.
================
*/
void Score_DrawTeamBoard( u8 mode, u8 all, s16 x, s16 y ) {
    s32 i;
    s32 frame;
    s32 count;
    s32 player;
    s16 frameX;
    u8 cpu;             /* the plate's player is played by the game */

    if ( mode == 2 ) {
        Draw_Image( 0xA12, x, y - 3 );
    }
    Score_DrawFrameHeaders( 10, 0, x + 0x38, y );
    y += 9;
    count = mode == 1 ? game_numPlayers / 2 : game_numPlayers;
    for ( i = 0; i < count; i++ ) {
        frameX = x;
        if ( mode == 2 && i == 2 ) {
            Draw_Image( 0x9FB, frameX, y + 4 );
            y += 16;
        }
        if ( mode == 1 ) {
            player = i == 1 ? 2 : 0;
            cpu = game_players[player] >> 7;
            Menu_DrawNamePlate( player, cpu, game_playerNames[player].chars, frameX, y );
            cpu = game_players[player + 1] >> 7;
            Menu_DrawNamePlate( player + 1, cpu, game_playerNames[player + 1].chars, frameX, y + 20 );
        } else {
            cpu = game_players[i] >> 7;
            Menu_DrawNamePlate( i, cpu, game_playerNames[i].chars, frameX, y );
        }
        frameX += 0x38;
        if ( mode == 1 ) {
            y += 10;
        }
        for ( frame = 0; frame < 9; frame++ ) {
            Score_DrawFrame( i, frame, all ? 9 : game_frame[i], game_throw[i], game_playerDone[i], frameX, y );
            frameX += 20;
        }
        Score_DrawFrame( i, 9, all ? 9 : game_frame[i], game_throw[i], game_playerDone[i], frameX, y );
        y += mode == 1 ? 40 : 20;
        if ( mode == 2 && i % 2 == 1 ) {
            Draw_Image( i == 1 ? 0xA13 : 0x9FC, 0xCC, y );
            Score_DrawNumber( game_totals[i - 1] + game_totals[i], 0x10E, y + 4 );
        }
    }
}

/*
================
Score_DrawParCard

Draws the score card at (x, y): its title, then a row of frame numbers, a row of each frame's par and the par
total, and a row per player with its name plate, its frames (Score_DrawCardFrame, by kind and all) and its total
(Score_DrawTotal, kind as duel).
================
*/
void Score_DrawParCard( u8 kind, u8 all, s16 x, s16 y ) {
    s32 row;
    s32 player;
    s32 i;
    s16 rowX;
    s16 rowY;

    Draw_Image( !kind ? 0xA1C : 0xA2D, x, y );
    rowY = y + 16;
    for ( row = 0; row < game_numPlayers + 2; row++ ) {
        player = row - 2;
        rowX = x;
        switch ( row ) {
        case 0:
            Draw_Image( 0xA3A, rowX, rowY );
            break;
        case 1:
            Draw_Image( 0xA2C, rowX, rowY );
            break;
        default:
            Menu_DrawNamePlate( player, game_players[player] >> 7, game_playerNames[player].chars, rowX, rowY );
            break;
        }
        rowX += 0x38;
        for ( i = 0; i < 9; i++ ) {
            Draw_Image( score_parFrameBoxes[row], rowX, rowY );
            switch ( row ) {
            case 0:
                Draw_Image( score_parFrameImages[i], rowX + 6, rowY + 3 );
                break;
            case 1:
                Score_DrawNumber( game_frameFlags[0][i], rowX - 6, rowY + 4 );
                break;
            default:
                Score_DrawCardFrame( player, i, kind, all, rowX, rowY );
                break;
            }
            rowX += 18;
        }
        Draw_Image( score_parTotalBoxes[row], rowX, rowY );
        switch ( row ) {
        case 0:
            Draw_Image( 0x9E8, rowX, rowY );
            break;
        case 1:
            Score_DrawNumber( game_splits[0], rowX + 2, rowY + 4 );
            break;
        default:
            Score_DrawTotal( player, kind, rowX, rowY );
            break;
        }
        rowY += row < 2 ? 15 : 20;
    }
}

/*
================
Score_DrawCardFrame

Draws the player's frame at (x, y), once it is played or when all is set: kind 0 its score, signed; otherwise
the image of its game_secondRolls entry.
================
*/
void Score_DrawCardFrame( s32 player, u8 frame, u8 kind, u8 all, s16 x, s16 y ) {
    s16 images[3] = { -1, 0x9EA, 0x9EB };
    s16 score;
    s32 sign;

    if ( kind == 0 ) {
        score = game_frameScores[player][frame];
        if ( all || frame < game_frame[player] ) {
            sign = score < 0 ? 0 : score > 0 ? 2 : 1;
            Draw_Image( score_signImages[sign], x + 3, y + 4 );
            Score_DrawNumber( score < 0 ? -score : score, x - 2, y + 7 );
        }
    } else if ( all || frame < game_frame[player] ) {
        Menu_DrawImage( images[game_secondRolls[player][frame]], x + 4, y + 3 );
    }
}

/*
================
Score_DrawTotal

Draws the player's total at (x, y): its score, signed, or in a duel whether the player leads, ties or trails
on frames won, and by how many.
================
*/
void Score_DrawTotal( s32 player, u8 duel, s16 x, s16 y ) {
    s16 images[3] = { 0x9ED, 0x9E9, 0x9EC };
    s16 digits[4];
    s16 score;
    s16 lead;
    s32 other;
    s32 sign;

    if ( !duel ) {
        score = game_totals[player];
        sign = score < 0 ? 0 : score > 0 ? 2 : 1;
        Draw_Image( score_signImages[sign], x + 4, y + 4 );
        Score_DrawNumber( score < 0 ? -score : score, x + 8, y + 7 );
    } else {
        other = player != 1;
        sign = D_800994B8[player] > D_800994B8[other] ? 0 : D_800994B8[player] == D_800994B8[other] ? 1 : 2;
        Draw_Image( images[sign], x, y + 2 );
        lead = D_800994B8[player] - D_800994B8[other];
        if ( lead != 0 ) {
            Score_GetDigits( digits, lead < 0 ? -lead : lead, FALSE, FALSE );
            Menu_DrawImage( digits[3], x + 6, y + 4 );
        }
    }
}

/*
================
Score_DrawAllBoards

Draws the scoreboard of every player at (x, y): the numbered heads of the ten frames, then per player its name
plate and its ten frames, marking the frames already played. unused is not read (0 from the game, 1 from the results).
================
*/
void Score_DrawAllBoards( s32 unused, s16 x, s16 y ) {
    s32 i;
    s32 player;
    s16 frameX;

    Score_DrawFrameHeaders( 10, 0, x + 0x38, y );
    y += 9;
    for ( player = 0; player < game_numPlayers; player++ ) {
        frameX = x;
        Menu_DrawNamePlate( player, game_players[player] >> 7, game_playerNames[player].chars, frameX, y );
        frameX += 0x38;
        for ( i = 0; i < 10; i++ ) {
            Draw_Image( i == 9 ? 0xA36 : 0xAD3, frameX, y );
            if ( i < game_frame[player] ) {
                Draw_Image( 0x9EB, frameX + ( i == 9 ? 10 : 5 ), y + 3 );
            }
            frameX += 20;
        }
        y += 20;
    }
}

/*
================
Score_DrawPlayerBoard

Draws the scoreboard of player at (x, y): the numbered heads of the ten frames, the player's name plate, then the
ten frames.
================
*/
void Score_DrawPlayerBoard( s32 player, s16 x, s16 y ) {
    s32 i;
    s16 frameX;

    Score_DrawFrameHeaders( 10, 0, x + 0x38, y );
    y += 9;
    frameX = x;
    Menu_DrawNamePlate( player, game_players[player] >> 7, game_playerNames[player].chars, frameX, y );
    frameX += 0x38;
    for ( i = 0; i < 9; i++ ) {
        Score_DrawFrame( player, i, 9, game_throw[player], game_playerDone[player], frameX, y );
        frameX += 20;
    }
    Score_DrawFrame( player, 9, 9, game_throw[player], game_playerDone[player], frameX, y );
}

/*
================
Menu_DrawBox

Draws count pieces of a box at (x, y): its left end, then its body; small ones when small is set. The
callers pass the player, which is unused.
================
*/
void Menu_DrawBox( s32 player, u8 count, u8 small, s16 x, s16 y ) {
    s32 i;
    s32 offset;

    offset = small ? 16 : 20;
    for ( i = 0; i < count; i++ ) {
        if ( i == 0 ) {
            Draw_Image( small ? 0x865 : 0x867, x, y );
        } else {
            Draw_Image( small ? 0x864 : 0x866, x + offset, y );
        }
    }
}

/*
================
Score_DrawRecordTable

Draws the table of count records of the given kind at (x, y): its column heads, then a row per record with its
name plate and, once it holds a game, its games, average, high and low scores and its three percentages.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: prologue, frame (0xA0), calls, CFG and constants match (Score_DrawNumberBox takes s32 numbers:
// retail passes the lhu field and the truncated float as they are); rowX as s32 gives retail's spill of x (sh to
// sp+0x2E, lhu each row). Left: retail draws the name plate from the reloaded x itself and forms rowX = x + 0x2C
// without sign-extending x first, where this draft sign-extends x into rowX (sll/sra) and adds 0x2C to it.
// Tried with s32 rowX: the plate at x (worse), rowX = x + 0x2C after it (loop hoists rowX, worse), the inline
// Menu_DrawSmallNamePlate (no change). Earlier, with s16 rowX: rowY/played as s16/s32/u8, x and rowX as u16,
// the plate at x with rowX = x + 0x2C (each keeps x in s5, frame 0x98).
// Restricted permuter search (perm macros only, 20 min): declaration order, i s32/s16/u16, played s32/u8/s16, rowY,
// rowX and topY s16/s32, the name plate from rowX or x (rowX = x + 0x2C), played set before or after the plate,
// rowY += 0x1C or rowY = rowY + 0x1C: nothing at or below the base 275.
void Score_DrawRecordTable( u16 count, playerRecord_t *records, u8 kind, s16 x, s32 y ) {
    s16 digits[4];
    s32 i;
    s32 played;
    s16 rowY;
    s32 rowX;
    s16 topY;      /* of the upper number */

    Draw_Image( 0x9E1, x + 0x2C, y );
    Draw_Image( 0xA15, x + 0x52, y );
    Draw_Image( 0xA11, x + 0x78, y );
    Draw_Image( 0x9E2, x + 0x9E, y );
    Draw_Image( 0x9E4, x + 0xC4, y );
    Draw_Image( 0x9E3, x + 0xEA, y );
    rowY = y + 16;
    for ( i = 0; i < count; i++ ) {
        played = records[i].games != 0;
        rowX = x;
        Draw_Image( 0x9FE, rowX, rowY );
        Txt_DrawGlyphs( &game_gfx, rowX + 3, rowY + 6, 8, records[i].name );
        rowX += 0x2C;
        topY = rowY + 2;
        Draw_Image( 0x9E7, rowX, rowY );
        if ( played ) {
            Score_GetDigits( digits, records[i].games, TRUE, FALSE );
            Menu_DrawImage( digits[0], rowX + 2, rowY + 7 );
            Menu_DrawImage( digits[1], rowX + 10, rowY + 7 );
            Menu_DrawImage( digits[2], rowX + 18, rowY + 7 );
            Menu_DrawImage( digits[3], rowX + 26, rowY + 7 );
        }
        Draw_Image( played ? 0x9E6 : 0x9E7, rowX + 0x26, rowY );
        if ( played ) {
            Score_GetDigits( digits, score_averages[kind][i] * 10.0f, TRUE, TRUE );
            Menu_DrawImage( digits[0], rowX + 0x2A, rowY + 7 );
            Menu_DrawImage( digits[1], rowX + 0x31, rowY + 7 );
            Menu_DrawImage( digits[2], rowX + 0x39, rowY + 7 );
            Menu_DrawImage( digits[3], rowX + 0x43, rowY + 7 );
        }
        Draw_Image( 0x9E7, rowX + 0x4C, rowY );
        if ( played ) {
            Score_GetDigits( digits, records[i].high, FALSE, FALSE );
            Menu_DrawImage( digits[0], rowX + 0x4E, topY );
            Menu_DrawImage( digits[1], rowX + 0x56, topY );
            Menu_DrawImage( digits[2], rowX + 0x5E, topY );
            Menu_DrawImage( digits[3], rowX + 0x66, topY );
            Score_GetDigits( digits, records[i].low, FALSE, FALSE );
            Menu_DrawImage( digits[0], rowX + 0x4E, rowY + 14 );
            Menu_DrawImage( digits[1], rowX + 0x56, rowY + 14 );
            Menu_DrawImage( digits[2], rowX + 0x5E, rowY + 14 );
            Menu_DrawImage( digits[3], rowX + 0x66, rowY + 14 );
        }
        Score_DrawNumberBox( records[i].strikes, score_strikeRates[kind][i] * 10.0f, played, rowX + 0x72, rowY );
        Score_DrawNumberBox( records[i].spares, score_spareRates[kind][i] * 10.0f, played, rowX + 0x98, rowY );
        Score_DrawNumberBox( records[i].splits, score_splitRates[kind][i] * 10.0f, played, rowX + 0xBE, rowY );
        rowY += 0x1C;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/13490", Score_DrawRecordTable);
#endif

/*
================
Score_DrawNumberBox

Draws the box of two numbers at (x, y), a above b, or the empty box when show is clear.
================
*/
void Score_DrawNumberBox( s32 a, s32 b, u8 show, s16 x, s16 y ) {
    s16 digits[4];
    s16 topY;      /* of the upper number */

    topY = y + 2;
    if ( show ) {
        Draw_Image( 0x9E5, x, y );
        Score_GetDigits( digits, a, FALSE, FALSE );
        Menu_DrawImage( digits[0], x + 4, topY );
        Menu_DrawImage( digits[1], x + 10, topY );
        Menu_DrawImage( digits[2], x + 16, topY );
        Menu_DrawImage( digits[3], x + 22, topY );
        Score_GetDigits( digits, b, FALSE, TRUE );
        Menu_DrawImage( digits[0], x + 4, y + 14 );
        Menu_DrawImage( digits[1], x + 10, y + 14 );
        Menu_DrawImage( digits[2], x + 16, y + 14 );
        Menu_DrawImage( digits[3], x + 24, y + 14 );
    } else {
        Draw_Image( 0x9E7, x, y );
    }
}

/*
================
Score_SetRecordFigures

Sets the figures of record, of character index, in the kind's row of score_averages,
score_strikeRates, score_spareRates and score_splitRates; all 0 when it holds no game.
================
*/
void Score_SetRecordFigures( playerRecord_t *record, s32 index, u8 kind ) {
    if ( record->games == 0 ) {
        score_averages[kind][index] = score_strikeRates[kind][index] = score_spareRates[kind][index] =
            score_splitRates[kind][index] = 0.0f;
    } else {
        score_averages[kind][index] = (f32)record->total / record->games;
        score_strikeRates[kind][index] = record->strikes * 100.0f / ( record->games * 12 );
        score_spareRates[kind][index] = record->spares * 100.0f / ( record->games * 10 );
        score_splitRates[kind][index] = record->splits * 100.0f / ( record->games * 10 );
    }
}

/*
================
Menu_DrawImage

Draws image at (x, y), unless it is negative.
================
*/
void Menu_DrawImage( s16 image, s16 x, s16 y ) {
    if ( image >= 0 ) {
        Draw_Image( image, x, y );
    }
}

/*
================
Menu_TwoChoicePrompt

Runs the two-choice prompt 0x957 for a frame; returns 1 or 2 for the choice taken, 0 until then.
================
*/
s32 Menu_TwoChoicePrompt( void ) {
    save_choiceWidth[0] = save_choiceWidth[1] = 0x24;
    save_choiceHeight[0] = save_choiceHeight[1] = 0x10;
    save_choiceX[0] = 0x36;
    save_choiceY[0] = 0x1E;
    save_choiceX[1] = 0x5E;
    save_choiceY[1] = 0x1E;
    if ( Save_ShowPrompt( 0x957, PROMPT_CHOICE2_H ) ) {
        if ( save_promptChoice == 0 ) {
            return 1;
        }
        if ( save_promptChoice == 1 ) {
            save_promptChoice = 0;
            return 2;
        }
    }
    return 0;
}
