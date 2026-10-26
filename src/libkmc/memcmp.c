/* memcmp.c -- memory compare, a word at a time when both blocks share an alignment */

#include "libc.h"

/*
================
memcmp

Returns the difference of the first differing units. Where s1 and s2 are equally aligned it compares a byte and
a halfword up to word alignment, then whole words, then the tail; a word or halfword difference is returned as
the difference of the two units; for words it can wrap and give the wrong sign (KMC's bug, kept). The wider
steps advance through GNU C cast lvalues, *( (u16 *)p )++.
================
*/
int memcmp( const void *s1, const void *s2, size_t n ) {
    const u8 *p1;
    const u8 *p2;
    int       d;
    size_t    rest;

    p1 = s1;
    p2 = s2;
    if ( n == 0 ) {
        return 0;
    }
    if ( ( p1 - p2 ) & 3 ) {
        while ( n-- != 0 ) {
            d = *p1++ - *p2++;
            if ( d != 0 ) {
                return d;
            }
        }
        return 0;
    }
    if ( (u32)p1 & 1 ) {
        d = *p1++ - *p2++;
        if ( d != 0 ) {
            return d;
        }
        n--;
    }
    if ( n >= 2 && ( (u32)p1 & 2 ) ) {
        d = *((u16 *)p1)++ - *((u16 *)p2)++;
        if ( d != 0 ) {
            return d;
        }
        n -= 2;
    }
    rest = n;
    n >>= 2; /* n now counts words */
    while ( n-- != 0 ) {
        d = *((u32 *)p1)++ - *((u32 *)p2)++;
        if ( d != 0 ) {
            return d;
        }
    }
    if ( rest & 2 ) {
        d = *((u16 *)p1)++ - *((u16 *)p2)++;
        if ( d != 0 ) {
            return d;
        }
    }
    if ( rest & 1 ) {
        d = *p1 - *p2;
        if ( d != 0 ) {
            return d;
        }
    }
    return 0;
}
