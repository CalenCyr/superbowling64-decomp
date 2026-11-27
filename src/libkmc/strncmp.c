/* strncmp.c -- bounded string compare, a word at a time when both strings share an alignment */

#include "libc.h"

/*
================
strncmp

Compares at most n characters, stopping after a terminator. Where s1 and s2 are equally aligned it compares
bytes up to word alignment, then whole words while neither differs nor holds a terminator, then the remaining
bytes. A differing word is compared down to its first terminator, if it has one, as the difference of the
masked words; like memcmp's, that difference can wrap and give the wrong sign.
================
*/
int strncmp( const char *s1, const char *s2, size_t n ) {
    const u8 *p1;
    const u8 *p2;
    int       c;
    int       d;
    size_t    words;
    u32       word1;
    u32       word2;

    p1 = (const u8 *)s1;
    p2 = (const u8 *)s2;
    if ( ( p1 - p2 ) & 3 ) {
    bytes:
        while ( n-- != 0 ) {
            c = *p1++;
            d = c - *p2++;
            if ( d != 0 || c == '\0' ) {
                return d;
            }
        }
        return 0;
    }
    while ( (u32)p1 & 3 ) {
        if ( n-- == 0 ) {
            return 0;
        }
        c = *p1++;
        d = c - *p2++;
        if ( d != 0 || c == '\0' ) {
            return d;
        }
    }
    words = n >> 2;
    while ( words-- != 0 ) {
        word1 = *( (u32 *)p1 )++;
        word2 = *( (u32 *)p2 )++;
        d = word1 - word2;
        if ( d != 0 ) {
            /* RULE-EXCEPTION: R30, d becomes the mask of the bytes up to the terminator: retail keeps both in one
               register, and a separate mask local compiles one instruction shorter */
            if ( !( word1 & 0xFF000000 ) ) {
                d = 0xFF000000;
            } else if ( !( word1 & 0x00FF0000 ) ) {
                d = 0xFFFF0000;
            } else if ( !( word1 & 0x0000FF00 ) ) {
                d = 0xFFFFFF00;
            } else {
                return d;
            }
            return ( word1 & d ) - ( word2 & d );
        }
        if ( !( word1 << 24 ) || !( word1 & 0x0000FF00 ) || !( word1 & 0x00FF0000 ) || !( word1 & 0xFF000000 ) ) {
            return d; /* 0: equal through the terminator */
        }
    }
    n &= 3;
    goto bytes;
}
