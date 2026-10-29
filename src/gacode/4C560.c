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
        ( (unkImage_t *)D_800F80B0[1673].data )->unk_07 = D_8009CFB0;
        ( (unkImage_t *)D_800F80B0[1674].data )->unk_07 = D_8009CFB0;
    }
}

/*
================
func_8012239C_gacode

With set nonzero, gives each player's scene object (140 on) the flag bits of the low nibble of its
D_8005C0E4 entry.
================
*/
void func_8012239C_gacode( s32 set ) {
    s32 i;

    if ( !set ) {
        return;
    }
    for ( i = 0; i < D_8005C161; i++ ) {
        switch ( D_8005C0E4[i] & 0xF ) {
        case 1:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x1400;
            break;
        case 7:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x1C00;
            break;
        case 0:
        case 2:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x1000;
            break;
        case 4:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x1E00;
            break;
        case 3:
        case 6:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x3000;
            break;
        case 5:
            ( (sceneObj_t *)D_800EA120[140 + i] )->unk_00 |= 0x2800;
            break;
        }
    }
}

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
