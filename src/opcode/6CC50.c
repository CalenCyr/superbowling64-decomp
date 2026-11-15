/* 6CC50.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "opcode/opcode.h"

/*
================
func_80113130_opcode

Looks up a character code's entry in the table (codes 0x10-0x94); any other code gives 0x57.
================
*/
u16 func_80113130_opcode( u8 code ) {
    u16 entry;

    if ( (u8)( code - 0x10 ) < 0x85 ) {
        entry = D_801140C0_opcode[code - 0x10];
    } else {
        entry = 0x57;
    }
    return entry;
}

INCLUDE_ASM("asm/nonmatchings/opcode/6CC50", func_80113168_opcode);
