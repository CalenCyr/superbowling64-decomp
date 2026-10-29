#ifndef SPRITE_H
#define SPRITE_H

/* sprite.h -- 2D drawing into the display list: images of the file cache, and rectangles (unit 7250) */

#include "common.h"
#include "libultra.h"

/* Image file of the file cache, as func_8002E4B0 draws it. */
typedef struct {
    /* 0x00 */ u32 unk_00;
    /* 0x04 */ u32 width;
    /* 0x08 */ u32 height;
} fileImage_t;

void func_8002E4B0( u16 id, s16 x, s16 y, u8 arg3, f32 scaleX, f32 scaleY );
void func_8002FDE4( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a );

#endif
