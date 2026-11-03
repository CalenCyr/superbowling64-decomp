/* 5EC00.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80118010_gscode

Flag 4 adds 0x18, flag 8 adds 0x20 and flag 0x10 adds 0xC0; flag 4 is always set in the result.
================
*/
u8 func_80118010_gscode( u8 flags ) {
    u8 result;

    result = flags | 4;
    if ( flags & 4 ) {
        result = flags | 0x1C;
    }
    if ( flags & 8 ) {
        result |= 0x20;
    }
    if ( flags & 0x10 ) {
        result |= 0xC0;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/gscode/5EC00", func_80118040_gscode);
