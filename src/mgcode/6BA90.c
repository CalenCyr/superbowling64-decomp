/* 6BA90.c -- mgcode overlay */

#include "common.h"
#include "libultra.h"
#include "mgcode/mgcode.h"

/*
================
func_80112350_mgcode

Entry of the overlay: creates the scene's objects, then builds a new maze and its image.
================
*/
void func_80112350_mgcode( void ) {
    func_8002A1EC( D_80112550_mgcode, D_801125B0_mgcode );
    func_80038214( MG_MAZE_SIZE, MG_MAZE_SIZE, D_80112730_mgcode );
    func_80038090( 0x90F, MG_MAZE_SIZE, MG_MAZE_SIZE, D_80112730_mgcode );
}

/*
================
func_801123AC_mgcode

Frame of the overlay: A builds a new maze; runs the scene's objects, then prints the frame's times and rate.
================
*/
void func_801123AC_mgcode( void ) {
    u8 text[0x50];

    if ( D_800E4D62 & A_BUTTON ) {
        func_80038214( MG_MAZE_SIZE, MG_MAZE_SIZE, D_80112730_mgcode );
        func_80038090( 0x90F, MG_MAZE_SIZE, MG_MAZE_SIZE, D_80112730_mgcode );
    }
    func_8002A2F0();
    if ( D_800A2FDC ) {
        func_80028A0C( &D_800F8094 );
        func_80028950( &D_800F8094, 0 );
        gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
        sprintf( text, "CPU=%6.6d", D_8005B0EC );
        func_80028DA4( &D_800F8094, 220, 180, text );
        sprintf( text, "SND=%6.6d", D_8005B0F0 );
        func_80028DA4( &D_800F8094, 220, 188, text );
        sprintf( text, "RSP=%6.6d", D_8005B0E4 );
        func_80028DA4( &D_800F8094, 220, 196, text );
        sprintf( text, "RDP=%6.6d", D_8005B0E8 );
        func_80028DA4( &D_800F8094, 220, 204, text );
        sprintf( text, "FRATE=%3d", D_8005B0FC );
        func_80028DA4( &D_800F8094, 220, 212, text );
    }
}

/*
================
func_80112538_mgcode

An empty handler of the scene's classes.
================
*/
void func_80112538_mgcode( void ) {
}

/*
================
func_80112540_mgcode

An empty handler of the scene's classes.
================
*/
void func_80112540_mgcode( void ) {
}
