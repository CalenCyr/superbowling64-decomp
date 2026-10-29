#ifndef FILECACHE_H
#define FILECACHE_H

/* filecache.h -- reference-counted cache of the files of the str segment, by file index (unit BEF0) */

#include "common.h"

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
    /* 0x00 */ u8 unk_00[0x2E];
    /* 0x2E */ s16 numRefs;
    /* 0x30 */ fileType1Ref_t refs[1];   /* numRefs entries */
} fileType1Part_t;

typedef struct {
    /* 0x00 */ u32 numParts;
    /* 0x04 */ void *unk_04;
    /* 0x08 */ fileType1Part_t *parts[1];    /* numParts entries */
} fileType1_t;

typedef struct {
    /* 0x00 */ u8 unk_00[0x04];
    /* 0x04 */ void *unk_04;
    /* 0x08 */ void *unk_08;        /* passed to func_8002B380 */
} fileRefA_t;

typedef struct {
    /* 0x00 */ u8 unk_00[0x2C];
    /* 0x2C */ void *unk_2C;
    /* 0x30 */ void *unk_30;
    /* 0x34 */ void *unk_34;
    /* 0x38 */ void *unk_38;
} fileRefB_t;

typedef struct {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ void *unk_10;
    /* 0x14 */ void *unk_14;
} fileType2_t;

typedef struct {
    /* 0x00 */ u8 unk_00[0x48];
    /* 0x48 */ void *unk_48;
    /* 0x4C */ void *unk_4C;
    /* 0x50 */ void *unk_50;
} fileType3_t;

extern fileCacheEntry_t D_800F80B0[FILECACHE_SIZE];
/* both set: files are read from the development host (as "data.NNNN") instead of the str segment */
extern u8 D_8005C0BC;
extern u8 D_8005C0BD;

void func_80030AF0( void );
void func_80030B14( s16 index );
void *func_80030C04( s16 index, s16 type );
void func_80031104( void *data );
void func_80031184( s16 index );
void func_800311D0( s16 count );

#endif
