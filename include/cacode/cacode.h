#ifndef CACODE_CACODE_H
#define CACODE_CACODE_H

/* cacode.h -- the cacode overlay (file CACODE  SYS, ROM 0x67370, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "filecache.h"
#include "hostio.h"
#include "archive.h"
#include "heap.h"

/*
One slot of the cache of 320 x 240 BMP frames. The image is kept as 40 tiles of 64 x 32 RGBA16 texels, 5 across
and 8 down. The names of the type and its members are invented.
*/
typedef struct {
    /* 0x00 */ s16 frame;
    /* 0x02 */ s16 clip;
    /* 0x04 */ u32 stamp;               /* the slot with the smallest is reused; -1 is never smaller */
    /* 0x08 */ u16 ( *tiles )[32 * 64];
} frameSlot_t;                          /* size 0xC */

extern s32 D_80115374_cacode[3];        /* first frame number of each clip's BMP files */

extern frameSlot_t D_80115630_cacode[35];
extern u8 D_801157D4_cacode;            /* slots in use */

/*
Identity TODO: high addresses past the overlay's file, referenced without a segment suffix; their owner (perhaps
a heap buffer of main, or the overlay's own bss) is unknown.
*/
extern u8 *D_8013E880;                  /* buffer one BMP file is read into */

/* a 4 x 4 matrix copied whole; the name of the type is invented */
typedef struct {
    /* 0x00 */ f32 m[4][4];
} caMatrix_t;                           /* size 0x40 */

void func_80112CDC_cacode( caMatrix_t *src, f32 *trans, f32 *rot, f32 *scale );
void func_8011306C_cacode( float src[4][4], float dst[4][4] );

#endif
