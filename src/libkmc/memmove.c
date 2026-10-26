/* memmove.c -- overlapping copy and fill, a word at a time where alignment allows */

#include "libc.h"

/*
================
func_80055FE0

Copies n bytes from src to dst backwards, from the last byte down, for an overlapping move to a higher address.
Where dst and src share an alignment it moves a byte and a halfword down to word alignment, then whole words,
then the head.
================
*/
void *func_80055FE0( void *dst, const void *src, size_t n ) {
    u8       *d;
    const u8 *s;
    size_t    words;

    d = (u8 *)dst + n;
    s = (const u8 *)src + n;
    if ( ( s - d ) & 3 ) {
        while ( n-- != 0 ) {
            *--d = *--s;
        }
    } else if ( n != 0 ) {
        if ( (u32)d & 1 ) {
            *--d = *--s;
            n--;
        }
        if ( n >= 2 && ( (u32)d & 2 ) ) {
            *--( (u16 *)d ) = *--( (const u16 *)s );
            n -= 2;
        }
        words = n >> 2;
        while ( words-- != 0 ) {
            *--( (u32 *)d ) = *--( (const u32 *)s );
        }
        if ( n & 2 ) {
            *--( (u16 *)d ) = *--( (const u16 *)s );
        }
        if ( n & 1 ) {
            d[-1] = s[-1];
        }
    }
    return dst;
}

/*
================
memmove

A move to a lower address (or in place) copies forwards with memcpy; one to a higher address copies backwards.
================
*/
void *memmove( void *dst, const void *src, size_t n ) {
    if ( (const u8 *)src < (u8 *)dst ) {
        func_80055FE0( dst, src, n );
        return dst;
    }
    memcpy( dst, src, n );
    return dst;
}

/*
================
func_8005612C

memmove with bcopy's argument order. libultra's bcopy is linked, so this one is not bcopy; its name is unknown.
================
*/
void func_8005612C( const void *src, void *dst, size_t n ) {
    memmove( dst, src, n );
}

/*
================
memset

Fills a byte and a halfword up to word alignment, then whole words of c repeated, then the tail.
================
*/
void *memset( void *dst, int c, size_t n ) {
    u8    *p;
    size_t words;

    if ( n != 0 ) {
        c &= 0xFF;
        c |= c << 8;
        c |= c << 16;
        p = dst;
        if ( (u32)dst & 1 ) {
            *p++ = c;
            n--;
        }
        if ( n >= 2 && ( (u32)p & 2 ) ) {
            *( (u16 *)p )++ = c;
            n -= 2;
        }
        words = n >> 2;
        while ( words-- != 0 ) {
            *( (u32 *)p )++ = c;
        }
        if ( n & 2 ) {
            *( (u16 *)p )++ = c;
        }
        if ( n & 1 ) {
            *p = c;
        }
    }
    return dst;
}

/*
================
func_800561F0

memset with the count before the fill byte. The name is unknown.
================
*/
void *func_800561F0( void *dst, size_t n, int c ) {
    return memset( dst, c, n );
}
