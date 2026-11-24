#ifndef FILECACHE_H
#define FILECACHE_H

/* filecache.h -- reference-counted cache of the files of the str segment, by file index (unit BEF0) */

#include "common.h"
#include "libultra.h"

#define FILECACHE_SIZE      0x1000      /* file indices the cache can hold */

/* how func_80030C04 relocates a file's offsets into pointers once it is loaded */
#define FILE_RAW            0
#define FILE_TYPE1          1           /* a fileType1_t, and the files its parts name */
#define FILE_TYPE2          2           /* a fileType2_t */
#define FILE_TYPE3          3           /* a fileType3_t */

/* Offset stored in a file, from the file's start, as the pointer it becomes once the file is in memory. */
#define FILE_RELOC( file, offset )  ( (void *)( (u8 *)( file ) + (u32)( offset ) ) )

typedef struct {
    /* 0x00 */ s32 refCount;        /* 0: not loaded */
    /* 0x04 */ void *data;
} fileCacheEntry_t;

/* Pairs of file indices in a fileType1_t part, each loaded and relocated with it. */
typedef struct {
    /* 0x00 */ u16 unk_00;          /* a fileRefA_t, which func_8002B380 sets up */
    /* 0x02 */ u16 unk_02;          /* a fileRefB_t */
} fileType1Ref_t;

typedef struct {
    /* 0x00 */ u8 unk_00[0x04];
    /* 0x04 */ s32 unk_04;          /* the parent part, whose joint cacode divides out */
    /* 0x08 */ f32 unk_08[3];       /* rotation (radians); cacode's viewer copies it per frame */
    /* 0x14 */ u8 unk_14[0x1A];
    /* 0x2E */ s16 numRefs;
    /* 0x30 */ fileType1Ref_t refs[1];   /* numRefs entries */
} fileType1Part_t;

typedef struct {
    /* 0x00 */ u32 numParts;
    /* 0x04 */ void *unk_04;
    /* 0x08 */ fileType1Part_t *parts[1];    /* numParts entries */
} fileType1_t;

/* A mesh: its vertices, loaded at 0, and the commands that draw them (func_80031E40). */
typedef struct {
    /* 0x00 */ u16 numVerts;
    /* 0x02 */ u8 unk_02[0x02];
    /* 0x04 */ void *unk_04;        /* numVerts Vtx */
    /* 0x08 */ Gfx *unk_08;         /* model commands, converted by func_8002B380 */
} fileRefA_t;

/* An 8-bit colour, compared and copied whole (memcmp, assignment). */
typedef struct {
    u8 r, g, b, a;
} rgba8_t;

/* The arguments of gDPSetCombineLERP, cycle 0 then cycle 1. */
typedef struct {
    u8 a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0;
    u8 a1, b1, c1, d1, Aa1, Ab1, Ac1, Ad1;
} combine_t;

/* fileRefB_t.flags */
#define MAT_TEXTURE         1           /* loads the texture at unk_30 into tile 0 */
#define MAT_TEXTURE2        2           /* else, loads the texture at unk_38 into tile 1, after the first */
#define MAT_LIGHTING        4
#define MAT_SMOOTH          8
#define MAT_TEXGEN          0x10

/* fileRefB_t.fmt: how a texture is loaded */
#define MAT_I4              0
#define MAT_CI4             1
#define MAT_CI8             2
#define MAT_RGBA16          3
#define MAT_I8              4

/* A material (func_80031E40): colours, combiner, and up to two textures. */
typedef struct {
    /* 0x00 */ u32 flags;           /* MAT_* */
    /* 0x04 */ rgba8_t prim;
    /* 0x08 */ rgba8_t env;
    /* 0x0C */ combine_t combine;
    /* 0x1C */ u8 width;            /* of the texture at unk_30 */
    /* 0x1D */ u8 height;
    /* 0x1E */ u8 fmt;              /* MAT_I4 ... */
    /* 0x1F */ u8 wrap;             /* s in bits 0-1, t in bits 2-3: 0 wrap, 1 clamp, 2 mirror */
    /* 0x20 */ u8 width2;           /* of the texture at unk_38 */
    /* 0x21 */ u8 height2;
    /* 0x22 */ u8 fmt2;
    /* 0x23 */ u8 wrap2;
    /* 0x24 */ u16 scroll[2];       /* s, t of tile 0 in 10.2 fixed point */
    /* 0x28 */ u8 unk_28[0x04];
    /* 0x2C */ void *unk_2C;         /* palette of unk_30 */
    /* 0x30 */ void *unk_30;         /* texels */
    /* 0x34 */ void *unk_34;         /* palette of unk_38 */
    /* 0x38 */ void *unk_38;         /* texels */
} fileRefB_t;

/* Animation of a model (FILE_TYPE2): per frame, the floats its joints' masks select. */
#define FILE3_FOG           1           /* fileType3_t.unk_38: fog from fogNear to fogFar in fogColor */

typedef struct {
    /* 0x00 */ u32 numFrames;
    /* 0x04 */ u8 unk_04[0x04];
    /* 0x08 */ u32 unk_08;
    /* 0x0C */ u32 frameSize;       /* floats per frame */
    /* 0x10 */ u16 *unk_10;
    /* 0x14 */ f32 *unk_14;
} fileType2_t;

/* Camera path (FILE_TYPE3): per frame, the floats its masks select (bit 4 >> i for component i). */
typedef struct {
    /* 0x00 */ u32 numFrames;
    /* 0x04 */ u8 unk_04[0x04];
    /* 0x08 */ f32 fovy;
    /* 0x0C */ u16 frameSize;       /* floats per frame */
    /* 0x0E */ u8 unk_0E[0x02];
    /* 0x10 */ u16 eyeMask;
    /* 0x12 */ u16 unk_12;          /* the mask of unk_2C */
    /* 0x14 */ f32 at[3];
    /* 0x20 */ f32 eye[3];          /* where the frames leave a component out */
    /* 0x2C */ f32 unk_2C[3];
    /* 0x38 */ u32 unk_38;           /* flags; FILE3_FOG; 4 set by gscode's character menu */
    /* 0x3C */ u32 fogColor;        /* rgba */
    /* 0x40 */ u16 fogFar;
    /* 0x42 */ u16 fogNear;
    /* 0x44 */ u8 unk_44[0x01];
    /* 0x45 */ u8 clearColor[3];  /* rgb gscode's character menu fills its viewport with */
    /* 0x48 */ f32 *frames;
    /* 0x4C */ Lights1 *light;
    /* 0x50 */ void *unk_50;
} fileType3_t;

extern fileCacheEntry_t D_800F80B0[FILECACHE_SIZE];
/* both set: files are read from the development host (as "data.NNNN") instead of the str segment */
extern u8 D_8005C0BC;
extern u8 D_8005C0BD;

void Cache_Clear( void );
void Cache_LoadFile( s16 index );
void *func_80030C04( s16 index, s16 type );
void Cache_ReleaseData( void *data );
void Cache_Release( s16 index );
void Cache_FreeBelow( s16 count );

#endif
