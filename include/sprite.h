#ifndef SPRITE_H
#define SPRITE_H

/* sprite.h -- 2D drawing into the display list: images of the file cache, and rectangles (unit 7250) */

#include "common.h"
#include "libultra.h"

/* D_8005C190 (dbgtext.h): the RDP mode the last 2D drawing set up */
#define DRAW2D_COPY         0           /* copy mode, Draw_Image */
#define DRAW2D_1CYCLE       1           /* one-cycle mode, Draw_ImageScaled */
#define DRAW2D_FILL         0xFF        /* rectangles, Draw_FillRect */

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

extern Vtx D_80063A70[2][2];        /* the two ends of the line Draw_Line3D draws, by camera */
extern Mtx D_80063AB0;
extern u16 D_80062A70[2][4][256];  /* palette copies of Draw_ImageCopyPalette: four per frame buffer (sched_bufferIndex) */
extern s32 D_8005C194;              /* the next of those four */              /* identity, the model matrix of that line */

void Draw_CiImage( s16 type, s16 x, s16 y, s16 width, s16 height, void *palette, void *pixels );
void Draw_Image( u16 id, s16 x, s16 y );
void Draw_ImageCopyPalette( u16 id, s16 x, s16 y );
void Draw_ImageScaled( u16 id, s16 x, s16 y, u8 alpha, f32 scaleX, f32 scaleY );
void Draw_FillRect( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a );
void Draw_RectOutline( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a );
void Draw_RoundedBox( s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a );
void Draw_ClearScreen( u32 r, u32 g, u32 b );
void Draw_Line3D( s32 index, f32 *start, f32 *end, u8 *color );

#endif
