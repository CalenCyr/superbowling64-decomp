#ifndef ARCHIVE_H
#define ARCHIVE_H

/* archive.h -- files of the str ROM segment, stored raw or LZSS-compressed (unit 4DC0) */

#include "common.h"

/* LZSS (the classic 4 KB window variant): window size, longest match, and the shortest match coded */
#define LZSS_N          4096
#define LZSS_F          18
#define LZSS_THRESHOLD  2

/* One file of the str segment. Names of the type and its members are invented. */
typedef struct {
    /* 0x00 */ u32 romOffset;       /* from the start of the str segment */
    /* 0x04 */ u32 size;            /* bytes stored in ROM */
    /* 0x08 */ u32 rawSize;         /* bytes once decompressed; equal to size when stored raw */
} ArchiveEntry;

extern u8 str_ROM_START[];          /* linker: ROM address of the str segment */

extern ArchiveEntry D_800A30C0[];
extern u8 D_80061A00[LZSS_N];       /* LZSS window */
extern u8 D_80102080[];             /* where compressed files are read before decompression */

void func_800299C0( u8 *dst, u8 *src, u16 len );
u32 func_80029AF4( s32 index, u8 *dst );

#endif
