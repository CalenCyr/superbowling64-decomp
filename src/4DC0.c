/* 4DC0.c -- reading files of the str segment, with their LZSS decompression */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "archive.h"
#include "libc.h"

/* ===== State ===== */

static u8 lz_window[LZSS_N + LZSS_F - 1];  /* LZSS window, and its look-ahead copy at the end */

#define ARCHIVE_DMA_CHUNK   0x1000      /* bytes per ROM DMA */

/*
================
LZ_Decompress

Decompresses the len LZSS bytes at src into dst.
================
*/
void LZ_Decompress( u8 *dst, u8 *src, u16 len ) {
    s32 i;
    s32 j;
    s32 k;
    s32 r;
    s32 pos;
    u16 flags;
    s32 c;

    memset( lz_window, 0, LZSS_N - LZSS_F );
    r = LZSS_N - LZSS_F;
    flags = 0;
    for ( pos = 0; pos < len; ) {
        if ( ( ( flags >>= 1 ) & 0x100 ) == 0 ) {
            flags = src[pos++] | 0xFF00;
        }
        if ( flags & 1 ) {
            c = src[pos++];
            *dst++ = c;
            lz_window[r++] = c;
            r &= LZSS_N - 1;
        } else {
            i = src[pos++];
            j = src[pos++];
            i |= ( j & 0xF0 ) << 4;
            j = ( j & 0x0F ) + LZSS_THRESHOLD;
            for ( k = 0; k <= j; k++ ) {
                c = lz_window[( i + k ) & ( LZSS_N - 1 )];
                *dst++ = c;
                lz_window[r++] = c;
                r &= LZSS_N - 1;
            }
        }
    }
}

/*
================
Str_ReadFile

Reads file index of the str segment into dst, decompressing it if it is stored compressed; returns its size.
================
*/
u32 Str_ReadFile( s32 index, u8 *dst ) {
    u8 *out;
    s32 compressed;
    u32 done;
    u32 remaining;

    if ( STR_FILE( index, STR_FILE_RAWSIZE ) == STR_FILE( index, STR_FILE_SIZE ) ) {
        compressed = FALSE;
    } else {
        compressed = TRUE;
        out = dst;
        dst = lz_readBuffer;
    }
    done = 0;
    osWritebackDCacheAll();
    osInvalDCache( dst, STR_FILE( index, STR_FILE_SIZE ) );
    for ( remaining = STR_FILE( index, STR_FILE_SIZE ); remaining > ARCHIVE_DMA_CHUNK;
          remaining -= ARCHIVE_DMA_CHUNK ) {
        boot_dmaRequest.dramAddr = dst + done;
        boot_dmaRequest.devAddr = (u32)str_ROM_START + STR_FILE( index, STR_FILE_ROMOFFSET ) + done;
        boot_dmaRequest.size = ARCHIVE_DMA_CHUNK;
        osEPiStartDma( boot_romHandle, &boot_dmaRequest, OS_READ );
        osRecvMesg( &boot_dmaQueue, NULL, OS_MESG_BLOCK );
        done += ARCHIVE_DMA_CHUNK;
    }
    boot_dmaRequest.dramAddr = dst + done;
    boot_dmaRequest.devAddr = (u32)str_ROM_START + STR_FILE( index, STR_FILE_ROMOFFSET ) + done;
    boot_dmaRequest.size = ( remaining + 1 ) & ~1;
    osEPiStartDma( boot_romHandle, &boot_dmaRequest, OS_READ );
    osRecvMesg( &boot_dmaQueue, NULL, OS_MESG_BLOCK );
    if ( compressed ) {
        LZ_Decompress( out, dst, STR_FILE( index, STR_FILE_SIZE ) );
    }
    return STR_FILE( index, STR_FILE_RAWSIZE );
}
