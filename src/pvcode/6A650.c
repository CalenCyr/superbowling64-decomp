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

    Obj_Frame();
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

/* ===== Data ===== */

s32 pv_stepDone = 1;
u8 pv_sound = 0;
u64 pv_soundHandle = 0;

/* The classes of the scene, by objParams_t.type: 0 and 10 scene objects, 1 and 11 cameras; the rest unused. */
objClass_t pv_classes[] = {
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_Draw },
    { Cam_Create, (objUpdateFunc_t)Cam_UpdateAndAdvance },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { (objCreateFunc_t)PV_NullCreate, (objUpdateFunc_t)PV_NullHandler },
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_AddToDrawList },
    { Cam_Create, (objUpdateFunc_t)Cam_Update },
};

/* The scene's objects: a camera (class 11), then the scene objects. */
objParams_t pv_sceneObjects[] = {
    { 11.0f, 1.0f, { 2083.0f, 320.0f, 240.0f } },
    { 0.0f, 5.0f, { 1318.0f, 2018.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 11.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, -151.0f, 0.0f, -520.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 11.0f, 0.0f, -1020.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, -151.0f, 0.0f, -1560.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, -23.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 151.0f, 0.0f, -510.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, -11.0f, 0.0f, -1030.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 151.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 99.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 271.0f, 0.0f, -520.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 99.0f, 0.0f, -1020.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 271.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 406.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 244.0f, 0.0f, -510.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 406.0f, 0.0f, -1030.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 244.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1070.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 250.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -600.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, -950.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, -130.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, 710.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, 1550.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, -1000.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, -180.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, 660.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, 1480.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1100.0f, 0.0f, -1000.0f, 3.0f, 0.0f, 1.0f, 0.0f, -92.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1200.0f, 0.0f, -740.0f, 3.0f, 0.0f, 1.0f, 0.0f, -83.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1450.0f, 0.0f, 1200.0f, 3.0f, 0.0f, 1.0f, 0.0f, -75.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1400.0f, 0.0f, 1100.0f, 3.0f, 0.0f, 1.0f, 0.0f, -93.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 150.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1200.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1350.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 0.0f, -800.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 1500.0f, -400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 1500.0f, 400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, -850.0f, 0.0f, 1800.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -300.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -220.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -850.0f, 1200.0f, -1300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 570.0f, 1100.0f, -1300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 0.0f, 100.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 1200.0f, 600.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 1260.0f, 720.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -850.0f, 0.0f, 1700.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -400.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -470.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -600.0f, 0.0f, -800.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -930.0f, 0.0f, -200.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -900.0f, 0.0f, -130.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -820.0f, 0.0f, -240.0f, 3.0f, 0.0f, 1.0f, 0.0f, 80.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -820.0f, 0.0f, 1400.0f, 3.0f, 0.0f, 1.0f, 0.0f, 80.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -770.0f, 0.0f, 1470.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 800.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 20.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 890.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -100.0f, 0.0f, 2070.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -170.0f, 0.0f, 2100.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -60.0f, 0.0f, -1380.0f, 3.0f, 0.0f, 1.0f, 0.0f, 120.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1300.0f, 0.0f, 1500.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1500.0f, 0.0f, 980.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1470.0f, 0.0f, 910.0f, 3.0f, 0.0f, 1.0f, 0.0f, 26.0f } },
    { OBJ_PARAMS_END },
};
