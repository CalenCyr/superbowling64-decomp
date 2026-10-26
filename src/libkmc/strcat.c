/* strcat.c -- string concatenation, finding the end a word at a time */

#include "libc.h"

/*
================
strcat

Finds the terminator of dst, bytewise up to word alignment and then a word at a time (big-endian: the first
byte is the top one), and copies src there with strcpy.
================
*/
char *strcat( char *dst, const char *src ) {
    char *end;
    u32   word;

    end = dst;
    while ( (u32)end & 3 ) {
        if ( *end == '\0' ) {
            goto found;
        }
        end++;
    }
    for ( ;; ) {
        word = *(u32 *)end;
        if ( !( word & 0xFF000000 ) ) {
            break;
        }
        if ( !( word & 0x00FF0000 ) ) {
            end += 1;
            break;
        }
        if ( !( word & 0x0000FF00 ) ) {
            end += 2;
            break;
        }
        if ( !( word & 0x000000FF ) ) {
            end += 3;
            break;
        }
        end += 4;
    }
found:
    strcpy( end, src );
    return dst;
}
