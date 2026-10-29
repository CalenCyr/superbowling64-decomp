/* 3C570.c -- game overlay: the main loop states */

#include "common.h"
#include "libultra.h"
#include "input.h"
#include "gacode/gacode.h"

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112350_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_801125A0_gacode);

/*
================
func_80112A74_gacode
================
*/
void func_80112A74_gacode( void ) {
    s32 team;

    func_80120D7C_gacode( &D_8012CDE8_gacode );
    for ( team = 0; team <= D_8005C162; team++ ) {
        func_80114FFC_gacode( D_8012CE02_gacode[team][D_8012CDF8_gacode[team]] );
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112AF0_gacode);

/*
================
func_80112D94_gacode
================
*/
s32 func_80112D94_gacode( void ) {
    func_8002E4B0( 0x89E, 0x50, 0x40, 0x40, 1.0f, 1.0f );
    func_80112E04_gacode();
    if ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
        func_80037870( 11, 0 );
        D_8012CDF0_gacode = func_80112AF0_gacode;
    }
    return 0;
}

/*
================
func_80112E04_gacode
================
*/
void func_80112E04_gacode( void ) {
    if ( ( D_8005C160 & 0xF ) == 3 ) {
        func_8011A26C_gacode();
        return;
    }
    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        func_80039700( D_8005C160 >> 6, 0, 0x18, 0x30 );
        break;
    case 1:
        func_80039CBC( D_8005C160 >> 6, 0, 0x20, 0x30 );
        break;
    case 2:
        func_8003A3F0( 0, 0x18, 0x30 );
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80112EBC_gacode);

/*
================
func_80113210_gacode
================
*/
s32 func_80113210_gacode( void ) {
    switch ( func_8003B204() ) {
    case 1:
        D_8012CDF0_gacode = func_80113270_gacode;
        break;
    case 2:
        D_8012CDF0_gacode = func_80112AF0_gacode;
        break;
    }
    return 0;
}

/*
================
func_80113270_gacode
================
*/
s32 func_80113270_gacode( void ) {
    if ( func_80037410() ) {
        return 0;
    }
    D_8012CDF0_gacode = func_801132A8_gacode;
    return 1;
}

/*
================
func_801132A8_gacode
================
*/
s32 func_801132A8_gacode( void ) {
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_801132B0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113384_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113594_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_801136FC_gacode);

/*
================
func_801138EC_gacode

Advances *cur to the next player of the team, wrapping at the end of the list, and on past each player
whose D_800994BE flag is set, but stops at start.
================
*/
void func_801138EC_gacode( u8 *cur, s32 start, s32 team ) {
    if ( D_8012CE02_gacode[team][++( *cur )] == -1 ) {
        *cur = 0;
    }
    if ( *cur != start && D_800994BE[D_8012CE02_gacode[team][*cur]] ) {
        func_801138EC_gacode( cur, start, team );
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113968_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113AD4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113D48_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80113EE4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3C570", func_80114024_gacode);

/*
================
func_80114108_gacode
================
*/
s32 func_80114108_gacode( void ) {
    func_80114640_gacode( 0 );
    return 0;
}

INCLUDE_RODATA("asm/nonmatchings/gacode/3C570", D_8012CA00_gacode);
