#ifndef FAT_H
#define FAT_H

/* fat.h -- the BW64FAT file table and the ROM file loader (unit 3AD0) */

#include "common.h"

#define FAT_NAME_LEN    11          /* 8.3 name without the dot */
#define FAT_DMA_CHUNK   0x2000      /* bytes per ROM DMA */

/* One file of the table. */
typedef struct {
    /* 0x00 */ u8 *name;            /* FAT_NAME_LEN characters; NULL ends the table */
    /* 0x04 */ u32 romStart;
    /* 0x08 */ u32 romEnd;
    /* 0x0C */ u32 unk_0C;
} FatEntry;

/* The BW64FAT file (ROM 0x3C2A0), which boot loads to D_8010A080. */
typedef struct {
    /* 0x00 */ u8 magic[8];         /* "BW64FAT" */
    /* 0x08 */ FatEntry entries[1]; /* as many as the file holds */
} FatTable;

extern FatTable D_8010A080;

s16 func_800286D0( u8 *name );
s32 func_80028744( u8 *name, void *dst, s32 size, u32 offset );
s32 func_8002889C( u8 *name );

#endif
