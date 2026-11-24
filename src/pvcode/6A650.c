/* 6A650.c -- pvcode overlay */

#include "common.h"
#include "libultra.h"
#include "pvcode/pvcode.h"

/*
================
PV_Init

Entry of the overlay: Start switches to files from the development host; from the host, the scene's
objects come from its "sce.dat", else from the overlay's own records.
================
*/
void PV_Init( void ) {
    if ( D_800994BC & START_BUTTON ) {
        D_8005C0BC = 1;
        Host_Sync();
    }
    if ( D_8005C0BC ) {
        func_8002A7EC( "sce.dat" );
        D_8005C0BD = 1;
        func_8002A1EC( pv_classes, (objParams_t *)D_800F30E0 );
    } else {
        func_8002A1EC( pv_classes, pv_sceneObjects );
    }
}

/*
================
PV_Frame

Frame of the overlay: runs the scene's objects; R starts func_80035DA0, Start restarts Save_SaveSettings;
right plays the sound up and down pick and left stops it; then prints the frame's times and rate.
================
*/
void PV_Frame( void ) {
    u8 text[0x50];

    func_8002A2F0();
    if ( D_8005C980 ) {
        func_80035DA0();
    }
    if ( D_800E4D62 & R_TRIG ) {
        D_8005C980 = 1;
    }
    if ( D_800E4D62 & START_BUTTON ) {
        pv_stepDone = 0;
    }
    if ( !pv_stepDone && !Save_SaveSettings() ) {
        pv_stepDone = 1;
    }
    if ( D_800E4D62 & R_JPAD ) {
        pv_soundHandle = func_80037870( pv_sound, 0 );
    }
    if ( D_800E4D62 & L_JPAD ) {
        func_800379A8( pv_soundHandle );
    }
    if ( D_800E4D62 & U_JPAD ) {
        pv_sound++;
    }
    if ( D_800E4D62 & D_JPAD ) {
        pv_sound--;
    }
    if ( D_800A2FDC ) {
        Txt_LoadFont( &D_800F8094 );
        Txt_SetupFont( &D_800F8094, 0 );
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
