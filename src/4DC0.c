/* 4DC0.c -- reading files of the str segment, with their LZSS decompression */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "archive.h"
#include "libc.h"

#define ARCHIVE_DMA_CHUNK   0x1000      /* bytes per ROM DMA */

/*
================
func_800299C0

Decompresses the len LZSS bytes at src into dst.
================
*/
void func_800299C0( u8 *dst, u8 *src, u16 len ) {
    s32 i;
    s32 j;
    s32 k;
    s32 r;
    s32 pos;
    u16 flags;
    s32 c;

    memset( D_80061A00, 0, LZSS_N - LZSS_F );
    r = LZSS_N - LZSS_F;
    flags = 0;
    for ( pos = 0; pos < len; ) {
        if ( ( ( flags >>= 1 ) & 0x100 ) == 0 ) {
            flags = src[pos++] | 0xFF00;
        }
        if ( flags & 1 ) {
            c = src[pos++];
            *dst++ = c;
            D_80061A00[r++] = c;
            r &= LZSS_N - 1;
        } else {
            i = src[pos++];
            j = src[pos++];
            i |= ( j & 0xF0 ) << 4;
            j = ( j & 0x0F ) + LZSS_THRESHOLD;
            for ( k = 0; k <= j; k++ ) {
                c = D_80061A00[( i + k ) & ( LZSS_N - 1 )];
                *dst++ = c;
                D_80061A00[r++] = c;
                r &= LZSS_N - 1;
            }
        }
    }
}

/*
================
func_80029AF4

Reads file index of the str segment into dst, decompressing it if it is stored compressed; returns its size.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register/schedule; retail keeps the addresses of D_800A30C0's fields in registers (hoisting
// &D_800A30C0[index].romOffset out of the loop) where this draft addresses them with %hi/%lo each time
u32 func_80029AF4( s32 index, u8 *dst ) {
    u8 *out;
    s32 compressed;
    u32 done;
    u32 remaining;

    if ( D_800A30C0[index].rawSize == D_800A30C0[index].size ) {
        compressed = FALSE;
    } else {
        compressed = TRUE;
        out = dst;
        dst = D_80102080;
    }
    done = 0;
    osWritebackDCacheAll();
    osInvalDCache( dst, D_800A30C0[index].size );
    for ( remaining = D_800A30C0[index].size; remaining > ARCHIVE_DMA_CHUNK; remaining -= ARCHIVE_DMA_CHUNK ) {
        D_80066B70.dramAddr = dst + done;
        D_80066B70.devAddr = D_800A30C0[index].romOffset + ( done + (u32)str_ROM_START );
        D_80066B70.size = ARCHIVE_DMA_CHUNK;
        done += ARCHIVE_DMA_CHUNK;
        osEPiStartDma( D_800ABD60, &D_80066B70, OS_READ );
        osRecvMesg( &D_800DF578, NULL, OS_MESG_BLOCK );
    }
    D_80066B70.dramAddr = dst + done;
    D_80066B70.size = ( remaining + 1 ) & ~1;
    D_80066B70.devAddr = D_800A30C0[index].romOffset + ( done + (u32)str_ROM_START );
    osEPiStartDma( D_800ABD60, &D_80066B70, OS_READ );
    osRecvMesg( &D_800DF578, NULL, OS_MESG_BLOCK );
    if ( compressed ) {
        func_800299C0( out, dst, D_800A30C0[index].size );
    }
    return D_800A30C0[index].rawSize;
}
#else
INCLUDE_ASM("asm/nonmatchings/4DC0", func_80029AF4);
#endif
