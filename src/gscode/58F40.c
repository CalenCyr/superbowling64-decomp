/* 58F40.c -- game-select overlay */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_80112350_gscode

Entry of the overlay: starts BGM 1; coming back from a game (D_8005CB60 set) opens screen 6, else clears
the players and opens screen 0.
================
*/
void func_80112350_gscode( void ) {
    s32 i;

    func_800379C8( 1 );
    if ( !D_8005CB60 ) {
        for ( i = 0; i < 4; i++ ) {
            D_8005C0E4[i] = 0;
            D_800DF5D0[i] = 0;
        }
        D_8005C163 = 0;
        D_80120780_gscode = 0;
    } else {
        D_80120780_gscode = 6;
    }
}

/*
================
func_801123C4_gscode

Pushes a screen to go back to.
================
*/
void func_801123C4_gscode( u8 screen ) {
    D_80120781_gscode[D_8011B5A0_gscode++] = screen;
}

/*
================
func_801123E8_gscode

Goes back to the last pushed screen.
================
*/
void func_801123E8_gscode( void ) {
    D_80120780_gscode = D_80120781_gscode[--D_8011B5A0_gscode];
}

INCLUDE_ASM("asm/nonmatchings/gscode/58F40", func_80112414_gscode);

/*
================
func_8011288C_gscode
================
*/
void func_8011288C_gscode( void ) {
}

/*
================
func_80112894_gscode
================
*/
void func_80112894_gscode( void ) {
}
