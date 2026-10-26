/* 3AD0.c -- files of the BW64FAT table: lookup by name, size, and loading from ROM */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "fat.h"
#include "libc_boot.h"

/*
================
func_800286D0

Returns the table index of the file name, or -1.
================
*/
s16 func_800286D0( u8 *name ) {
    s32 i;

    for ( i = 0; ; i++ ) {
        if ( D_8010A080.entries[i].name == NULL ) {
            return -1;
        }
        if ( func_80056530( name, D_8010A080.entries[i].name, FAT_NAME_LEN ) == 0 ) {
            return i;
        }
    }
}

/*
================
func_80028744

Reads size bytes of the file name, from offset on, into dst, one FAT_DMA_CHUNK at a time; returns 0, or -1
if there is no such file.
================
*/
s32 func_80028744( u8 *name, void *dst, s32 size, u32 offset ) {
    s16 index;
    s32 i;
    s32 len;

    index = func_800286D0( name );
    if ( index == -1 ) {
        return -1;
    }
    osInvalDCache( dst, size );
    for ( i = 0; size - i * FAT_DMA_CHUNK > 0; i++ ) {
        D_80066B70.dramAddr = (u8 *)dst + i * FAT_DMA_CHUNK;
        D_80066B70.devAddr = D_8010A080.entries[index].romStart + offset + i * FAT_DMA_CHUNK;
        len = size - i * FAT_DMA_CHUNK;
        if ( len > FAT_DMA_CHUNK ) {
            len = FAT_DMA_CHUNK;
        }
        D_80066B70.size = len;
        osEPiStartDma( D_800ABD60, &D_80066B70, OS_READ );
        osRecvMesg( &D_800DF578, NULL, OS_MESG_BLOCK );
    }
    return 0;
}

/*
================
func_8002889C

Returns the size of the file name, or 0 if there is no such file.
================
*/
s32 func_8002889C( u8 *name ) {
    s16 index;

    index = func_800286D0( name );
    if ( index == -1 ) {
        return 0;
    }
    return D_8010A080.entries[index].romEnd - D_8010A080.entries[index].romStart;
}
