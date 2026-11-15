/* 6BE70.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "opcode/opcode.h"

/*
================
func_80112350_opcode

Entry of the overlay: in mode 0xA, Start held selects mode 0xFE, and no controller in port 1 mode 0xFD.
================
*/
void func_80112350_opcode( void ) {
    D_80114350_opcode = 0;
    if ( D_8005C0E8 == 10 ) {
        if ( D_800994BC & START_BUTTON ) {
            D_8005C0E8 = 0xFE;
        }
        if ( D_8005B194 == 1 ) {
            D_8005C0E8 = 0xFD;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/opcode/6BE70", func_801123A8_opcode);
