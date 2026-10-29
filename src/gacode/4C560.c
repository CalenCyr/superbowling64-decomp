/* 4C560.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "gacode/gacode.h"

/*
================
func_80122340_gacode

Counts a frame; in mode 3, gives resources 1673 and 1674 the D_8009CFB0 value in their byte 7.
Declared inline: retail repeats its body in each caller below, and still has this out-of-line copy, which
nothing calls (gcc 2.7 emits an extern inline function).
================
*/
inline void func_80122340_gacode( void ) {
    D_8012C924_gacode++;
    if ( ( D_8005C160 & 0xF ) == 3 ) {
        D_800F80B0[1673].image->unk_07 = D_8009CFB0;
        D_800F80B0[1674].image->unk_07 = D_8009CFB0;
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_8012239C_gacode);

/*
================
func_80122470_gacode
================
*/
void func_80122470_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 0 );
    }
    func_80122340_gacode();
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_801224F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80122788_gacode);

/*
================
func_80122BD8_gacode
================
*/
void func_80122BD8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 1 );
    }
    func_80122340_gacode();
}

/*
================
func_80122C60_gacode
================
*/
void func_80122C60_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 0 );
    }
    func_80122340_gacode();
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80122CE8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80123004_gacode);

/*
================
func_801233A8_gacode
================
*/
void func_801233A8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 1 );
    }
    func_80122340_gacode();
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80123430_gacode);
