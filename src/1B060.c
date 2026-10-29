/*
 * 1B060.c -- the audio heap: the libaudio ALHeap every sound allocation comes from, and its byte helpers.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Snd_InitHeap

Clears len bytes at base and makes them the audio heap.
================
*/
void Snd_InitHeap( u8 *base, s32 len ) {
    Snd_MemSet( base, 0, len );
    alHeapInit( &snd_heap, base, len );
}

/*
================
Snd_Alloc

Allocates size bytes from the audio heap.
================
*/
void *Snd_Alloc( s32 size ) {
    return alHeapAlloc( &snd_heap, 1, size );
}

/*
================
Snd_HeapUsed

Returns the bytes allocated from the audio heap so far.
================
*/
s32 Snd_HeapUsed( void ) {
    return snd_heap.cur - snd_heap.base;
}

/*
================
Snd_GetHeap
================
*/
ALHeap *Snd_GetHeap( void ) {
    return &snd_heap;
}

/*
================
Snd_MemSet

Fills len bytes at dst with value.
================
*/
void Snd_MemSet( void *dst, s32 value, u32 len ) {
    u8 *p;

    p = dst;
    while ( len-- ) {
        *p++ = value;
    }
}

/*
================
Snd_MemMove

Copies len bytes from src to dst; the areas may overlap.
================
*/
void Snd_MemMove( void *dst, void *src, u32 len ) {
    char *d;
    char *s;

    d = dst;
    s = src;
    if ( s < d ) {
        d += len;
        s += len;
        while ( len-- ) {
            *--d = *--s;
        }
    } else {
        while ( len-- ) {
            *d++ = *s++;
        }
    }
}
