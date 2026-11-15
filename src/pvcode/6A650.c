/* 6A650.c -- pvcode overlay */

#include "common.h"
#include "libultra.h"
#include "pvcode/pvcode.h"

/*
================
func_80112350_pvcode

Entry of the overlay: Start switches to files from the development host; from the host, the scene's
objects come from its "sce.dat", else from the overlay's own records.
================
*/
void func_80112350_pvcode( void ) {
    if ( D_800994BC & START_BUTTON ) {
        D_8005C0BC = 1;
        func_80029524();
    }
    if ( D_8005C0BC ) {
        func_8002A7EC( "sce.dat" );
        D_8005C0BD = 1;
        func_8002A1EC( D_80112660_pvcode, (objParams_t *)D_800F30E0 );
    } else {
        func_8002A1EC( D_80112660_pvcode, D_801126C0_pvcode );
    }
}

/*
================
func_801123D8_pvcode

Frame of the overlay: runs the scene's objects; R starts func_80035DA0, Start restarts func_80037410;
right plays the sound up and down pick and left stops it; then prints the frame's times and rate.
================
*/
void func_801123D8_pvcode( void ) {
    u8 text[0x50];

    func_8002A2F0();
    if ( D_8005C980 ) {
        func_80035DA0();
    }
    if ( D_800E4D62 & R_TRIG ) {
        D_8005C980 = 1;
    }
    if ( D_800E4D62 & START_BUTTON ) {
        D_80112650_pvcode = 0;
    }
    if ( !D_80112650_pvcode && !func_80037410() ) {
        D_80112650_pvcode = 1;
    }
    if ( D_800E4D62 & R_JPAD ) {
        D_80112658_pvcode = func_80037870( D_80112654_pvcode, 0 );
    }
    if ( D_800E4D62 & L_JPAD ) {
        func_800379A8( D_80112658_pvcode );
    }
    if ( D_800E4D62 & U_JPAD ) {
        D_80112654_pvcode++;
    }
    if ( D_800E4D62 & D_JPAD ) {
        D_80112654_pvcode--;
    }
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
