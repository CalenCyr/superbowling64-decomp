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
    if ( cont_allHeld & START_BUTTON ) {
        cache_hostMode = 1;
        Host_Sync();
    }
    if ( cache_hostMode ) {
        Host_ReadFileToObjBuf( "sce.dat" );
        cache_hostFiles = 1;
        Obj_CreateAll( pv_classes, (objParams_t *)obj_hostFile );
    } else {
        Obj_CreateAll( pv_classes, pv_sceneObjects );
    }
}

/*
================
PV_Frame

Frame of the overlay: runs the scene's objects; R starts GridEd_Frame, Start restarts Save_SaveSettings;
right plays the sound up and down pick and left stops it; then prints the frame's times and rate.
================
*/
void PV_Frame( void ) {
    u8 text[0x50];

    func_8002A2F0();
    if ( gridEd_active ) {
        GridEd_Frame();
    }
    if ( cont_allPressed & R_TRIG ) {
        gridEd_active = 1;
    }
    if ( cont_allPressed & START_BUTTON ) {
        pv_stepDone = 0;
    }
    if ( !pv_stepDone && !Save_SaveSettings() ) {
        pv_stepDone = 1;
    }
    if ( cont_allPressed & R_JPAD ) {
        pv_soundHandle = Sound_PlaySe( pv_sound, 0 );
    }
    if ( cont_allPressed & L_JPAD ) {
        Sound_Stop( pv_soundHandle );
    }
    if ( cont_allPressed & U_JPAD ) {
        pv_sound++;
    }
    if ( cont_allPressed & D_JPAD ) {
        pv_sound--;
    }
    if ( obj_paramsEnded ) {
        Txt_LoadFont( &game_gfx );
        Txt_SetupFont( &game_gfx, 0 );
        gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
        sprintf( text, "CPU=%6.6d", sched_cpuTime );
        Txt_DrawString( &game_gfx, 220, 180, text );
        sprintf( text, "SND=%6.6d", sound_taskTime );
        Txt_DrawString( &game_gfx, 220, 188, text );
        sprintf( text, "RSP=%6.6d", sched_rspTime );
        Txt_DrawString( &game_gfx, 220, 196, text );
        sprintf( text, "RDP=%6.6d", sched_rdpTime );
        Txt_DrawString( &game_gfx, 220, 204, text );
        sprintf( text, "FRATE=%3d", sched_frameRate );
        Txt_DrawString( &game_gfx, 220, 212, text );
    }
}
