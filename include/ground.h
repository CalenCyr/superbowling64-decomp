#ifndef GROUND_H
#define GROUND_H

/* ground.h -- unit C650: the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "object.h"

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

#define NUM_PUFFS           50
#define NUM_PUFF_SIZES      30          /* squares of D_8005C1F0: half-width 1 to 30 */

/* puff_t.flags */
#define PUFF_KIND           0xFF        /* 0 or 1: its texture (D_8005C1E4) and colour (D_8005C970) */
#define PUFF_CAMERA( n )    ( 0x100 << ( n ) )  /* drawn by camera n */

/* A puff: a square that faces the cameras, grows and fades, for life frames. */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u16 life;            /* frames left; 0: free */
    /* 0x04 */ s16 alpha;
    /* 0x06 */ s16 alphaStep;
    /* 0x08 */ s16 size;            /* its square in D_8005C1F0 */
    /* 0x0A */ s16 sizeStep;
} puff_t;                           /* size 0xC */

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
extern u8 D_8009CF89;               /* cameras that draw, a bit per index in D_8009CFA0 */
extern u8 D_8009CF9C;
extern u16 D_8009D064;
extern u8 D_800E6580;
extern u8 D_800F8090;
extern u16 D_800DF6BC;
extern u8 D_801000B0;
extern u8 D_80099448;
extern void *D_80099420;            /* drawEntry_t.unk_08 of the entries func_8003128C adds */
extern u16 D_80066E14;
extern u8 D_800693AC;
extern u8 D_800DF682;               /* set when the fog setting D_800E6588 changes */
extern u8 D_80066B30;               /* set when the fog setting D_800E6588 changes */
extern Vp D_8010A290;               /* the screen's viewport, in the KNSTATICSYS file */
extern u16 D_80000400[];            /* depth buffer, in the low RAM */


extern rgba8_t D_80068ACC;          /* the primitive colour set */
extern rgba8_t D_800EA0C8;          /* the environment colour set */
extern combine_t D_8009CF70;        /* the combiner of the last material */
extern u8 D_800EA0CC;               /* set: the combiner is set again */
extern u8 D_8006940C;               /* texturing on */
extern u8 D_800F7BE0;               /* texture generation on (MAT_TEXGEN) */
extern u8 D_800E6579;               /* MAT_SMOOTH of the last material */
extern u8 D_80066B32;               /* MAT_LIGHTING of the last material */
extern u8 D_80066B31;               /* 1: cull back faces, 0: front faces */
extern u32 D_800DF570;              /* the geometry mode set when D_800DF682 is */
extern u16 D_8009CF68;              /* tmem words of the first texture: where the second is loaded */
extern u16 D_800DF5D4;              /* the material (file index) last set up; 0xFFFF: none */
extern void *D_800A30B0;            /* the model matrix last loaded */

void func_80031250( void );
void func_8003128C( void );
void func_800316C0( sceneCamera_t *cam );
void func_80031E40( void );
void func_80031650( void );
void func_80031BB0( void );
void func_80031CD0( void );
s32 func_80031C9C( u32 value );
extern puff_t D_800E4D70[NUM_PUFFS];
extern u16 D_8005C1E4[2];           /* image files of the two kinds of puff */
extern Vtx D_8005C1F0[NUM_PUFF_SIZES * 4]; /* squares of four vertices, by size */
extern Mtx D_800E6770[2][NUM_PUFFS];    /* each puff turned to face camera 0, then camera 1 */
extern u8 D_8005C970[2][3];         /* rgb of the two kinds of puff */
extern u8 D_800A3018;               /* index in D_8009CFA0 of the camera func_800316C0 last set up; 2: none */
extern u8 D_80066C34;               /* microcode loaded: 0 gspF3DEX2_Rej_fifo, 1 gspF3DEX2_fifo, 2 gspL3DEX2_fifo */

void func_80035074( void );
void func_80035220( f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz );
void func_80035380( u16 flags, f32 x, f32 y, f32 z );
void func_80035748( void );

#endif
