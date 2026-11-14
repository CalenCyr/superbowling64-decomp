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

/*
================
func_80112414_gscode

Frame of the game-select overlay: runs the current screen and, on its result, pushes it and opens the
next screen, or goes back to the last pushed one. The screens after the first depend on the game mode.
Declared s32 but returns nothing (the caller ignores it): the original's return register is live at the
exit, which keeps the delay slot of the game-mode switch's range check empty.
================
*/
s32 func_80112414_gscode( void ) {
    switch ( D_80120780_gscode ) {
    case 0:
        if ( func_801128A0_gscode() == 1 ) {
            func_801123C4_gscode( 0 );
            switch ( D_8005C160 & 0xF ) {
            case 0:
                D_80120780_gscode = 1;
                break;
            case 1:
                D_8005C162 = 1;
                D_8005C161 = 2;
                D_8005C0E4[1] = 0x81;
                D_8005C0E4[0] = 0;
                D_80120780_gscode = 4;
                break;
            case 2:
                D_8005C161 = 1;
                D_8005C162 = 0;
                D_8005C0E4[0] = 0;
                D_80120780_gscode = 6;
                break;
            case 3:
                D_8005C162 = 1;
                D_8005C161 = 2;
                D_80120780_gscode = 5;
                break;
            case 4:
                D_80120780_gscode = 8;
                break;
            }
        }
        break;
    case 1:
        switch ( func_80112F10_gscode() ) {
        case 1:
            func_801123C4_gscode( 1 );
            switch ( ( D_8005C160 >> 4 ) & 3 ) {
            case 0:
                D_80120780_gscode = 2;
                break;
            case 1:
                D_80120780_gscode = 3;
                break;
            case 2:
                D_80120780_gscode = 5;
                break;
            }
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 2:
        switch ( func_80113430_gscode() ) {
        case 1:
            func_801123C4_gscode( 2 );
            D_80120780_gscode = 5;
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 3:
        switch ( func_80113990_gscode() ) {
        case 1:
            func_801123C4_gscode( 3 );
            D_80120780_gscode = 5;
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 4:
        switch ( func_80113DA0_gscode() ) {
        case 1:
            func_801123C4_gscode( 4 );
            D_80120780_gscode = 6;
            break;
        case 2:
            func_801123E8_gscode();
            break;
        case 3:
            func_801123C4_gscode( 4 );
            D_80120780_gscode = 9;
            func_800379C8( 13 );
            break;
        }
        break;
    case 5:
        switch ( func_80114D1C_gscode() ) {
        case 1:
            func_801123C4_gscode( 5 );
            D_80120780_gscode = 6;
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 6:
        switch ( func_80118040_gscode() ) {
        case 1:
            func_801123C4_gscode( 6 );
            switch ( D_8005C160 & 0xF ) {
            case 0:
            case 2:
            case 3:
                D_80120780_gscode = 7;
                break;
            case 1:
                D_8005C0BF = 2;
                D_8005C0B6 = 0;
                D_8005C163 = D_8005C0E4[1] & 0xF;
                break;
            }
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 7:
        switch ( func_801176C0_gscode() ) {
        case 1:
            D_8005C0BF = 2;
            D_8005C0B6 = 0;
            break;
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 8:
        switch ( func_801199E0_gscode() ) {
        case 1:
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    case 9:
        switch ( func_8011AA50_gscode() ) {
        case 1:
        case 2:
            func_801123E8_gscode();
            break;
        }
        break;
    }
}

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
