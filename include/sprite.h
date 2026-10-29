#ifndef SPRITE_H
#define SPRITE_H

/* sprite.h -- 2D drawing into the display list: images of the file cache, and rectangles (unit 7250) */

#include "common.h"
#include "libultra.h"

/* D_8005C190 (dbgtext.h): the RDP mode the last 2D drawing set up */
#define DRAW2D_COPY         0           /* copy mode, func_8002DF3C */
#define DRAW2D_1CYCLE       1           /* one-cycle mode, func_8002E4B0 */
#define DRAW2D_FILL         0xFF        /* rectangles, func_8002FDE4 */

/* fileImage_t.type */
#define IMAGE_CI4           0           /* 16-colour palette, then 4-bit pixels */
#define IMAGE_CI8           1           /* 256-colour palette, then 8-bit pixels */

#define CI4_PALETTE_SIZE    ( 16 * sizeof( u16 ) )
#define CI8_PALETTE_SIZE    ( 256 * sizeof( u16 ) )

/* Image file of the file cache. */
typedef struct {
    /* 0x00 */ u32 type;            /* IMAGE_CI4, IMAGE_CI8, or a direct-colour format */
    /* 0x04 */ u32 width;
    /* 0x08 */ u32 height;
    /* 0x0C */ u32 unk_0C;
    /* 0x10 */ u8 data[1];          /* palette of a colour-indexed image, then the pixels */
} fileImage_t;


void func_8002CE24( s16 type, s16 x, s16 y, s16 width, s16 height, void *palette, void *pixels );
void func_8002DF3C( u16 id, s16 x, s16 y );
void func_8002E4B0( u16 id, s16 x, s16 y, u8 alpha, f32 scaleX, f32 scaleY );
void func_8002FDE4( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a );

#endif
