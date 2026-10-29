/* 474E0.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "gacode/gacode.h"

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D2C0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D304_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D3A0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D3D4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D424_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D4E4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D5B8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D6C4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D7CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D9CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DB54_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DCB8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E014_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E12C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E504_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E63C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E6F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F334_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F4CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F87C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011FD08_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8012016C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801203AC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801205A0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120680_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801206E0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120980_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801209C0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120A7C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120B10_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120B50_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120C0C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120D30_gacode);

/*
================
func_80120D7C_gacode

Stops the sound of *handle, if any, and clears it.
================
*/
void func_80120D7C_gacode( s64 *handle ) {
    if ( *handle != 0 ) {
        func_800379A8( *handle );
        *handle = 0;
    }
}

/*
================
func_80120DD0_gacode
================
*/
void func_80120DD0_gacode( void ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120DD8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801213B4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801219CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80121E34_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80121FD0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80122060_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80122140_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8012224C_gacode);
