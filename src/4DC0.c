/* 4DC0.c -- reading files of the str segment, with their LZSS decompression */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "archive.h"
#include "libc.h"

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
#ifdef NON_MATCHING
// NON_MATCHING: register/schedule; retail keeps the addresses of str_files's fields in registers (hoisting
// &str_files[index].romOffset out of the loop) where this draft addresses them with %hi/%lo each time; an
// ArchiveEntry *entry local is worse. Permuter (target-only scoring, 5 min): best 2560 from 3030 only through noise
// (a pointer temporary for str_files after the last DMA), no source clue.
u32 Str_ReadFile( s32 index, u8 *dst ) {
    u8 *out;
    s32 compressed;
    u32 done;
    u32 remaining;

    if ( str_files[index].rawSize == str_files[index].size ) {
        compressed = FALSE;
    } else {
        compressed = TRUE;
        out = dst;
        dst = lz_readBuffer;
    }
    done = 0;
    osWritebackDCacheAll();
    osInvalDCache( dst, str_files[index].size );
    for ( remaining = str_files[index].size; remaining > ARCHIVE_DMA_CHUNK; remaining -= ARCHIVE_DMA_CHUNK ) {
        boot_dmaRequest.dramAddr = dst + done;
        boot_dmaRequest.devAddr = str_files[index].romOffset + ( done + (u32)str_ROM_START );
        boot_dmaRequest.size = ARCHIVE_DMA_CHUNK;
        done += ARCHIVE_DMA_CHUNK;
        osEPiStartDma( boot_romHandle, &boot_dmaRequest, OS_READ );
        osRecvMesg( &boot_dmaQueue, NULL, OS_MESG_BLOCK );
    }
    boot_dmaRequest.dramAddr = dst + done;
    boot_dmaRequest.size = ( remaining + 1 ) & ~1;
    boot_dmaRequest.devAddr = str_files[index].romOffset + ( done + (u32)str_ROM_START );
    osEPiStartDma( boot_romHandle, &boot_dmaRequest, OS_READ );
    osRecvMesg( &boot_dmaQueue, NULL, OS_MESG_BLOCK );
    if ( compressed ) {
        LZ_Decompress( out, dst, str_files[index].size );
    }
    return str_files[index].rawSize;
}
#else
INCLUDE_ASM("asm/nonmatchings/4DC0", Str_ReadFile);
#endif
