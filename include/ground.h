#ifndef GROUND_H
#define GROUND_H

/* ground.h -- unit C650: the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"

#define DRAW_BUCKETS        0x400       /* keys of the draw list */
#define DRAW_NONE           -1          /* end of a chain of draw entries; an empty bucket */

/* One entry of the draw list (D_8009D1A0), chained by index. */
typedef struct {
    /* 0x00 */ s16 next;            /* index of the next entry, or DRAW_NONE */
    /* 0x02 */ u16 key;             /* entries of a bucket are kept in key order */
    /* 0x04 */ u16 unk_04;
    /* 0x06 */ u16 unk_06;
    /* 0x08 */ void *unk_08;
} drawEntry_t;                      /* size 0xC */

extern s16 D_800DF6A8;              /* first entry of the draw list, once func_80031650 has chained the buckets */
extern s16 D_800E8070;              /* entries in D_8009D1A0 */
extern s16 D_800E4FE0[DRAW_BUCKETS];    /* first entry of each bucket, or DRAW_NONE */
extern s16 D_800E4560[DRAW_BUCKETS];    /* last entry of each bucket */
extern drawEntry_t D_8009D1A0[];
extern u8 D_800A30B4;               /* surface type, 0 to 6 */
extern u8 D_800E4D6C;
extern u8 D_800E4D60;
extern u8 D_800E6588;               /* fog on */
extern u32 D_801000B4;              /* render mode of the first cycle */
extern u32 D_800E4D64;              /* render mode of the second cycle */
extern u16 D_8005C1D0[];             /* 2, 4, ... 512 */
extern Vp D_8010A290;               /* the screen's viewport, in the KNSTATICSYS file */
extern u16 D_80000400[];            /* depth buffer, in the low RAM */

void func_80031250( void );
void func_80031650( void );
void func_80031BB0( void );
void func_80031CD0( void );
s32 func_80031C9C( u32 value );
void func_80035074( void );

#endif
