/* 6BA90.c -- mgcode overlay */

#include "common.h"
#include "libultra.h"
#include "mgcode/mgcode.h"

/*
================
MG_Init

Entry of the overlay: creates the scene's objects, then builds a new maze and its image.
================
*/
void MG_Init( void ) {
    Obj_CreateAll( mg_classes, mg_sceneObjects );
    Maze_Build( MG_MAZE_SIZE, MG_MAZE_SIZE, mg_maze );
    Maze_Draw( 0x90F, MG_MAZE_SIZE, MG_MAZE_SIZE, mg_maze );
}

/*
================
MG_Frame

Frame of the overlay: A builds a new maze; runs the scene's objects, then prints the frame's times and rate.
================
*/
void MG_Frame( void ) {
    u8 text[0x50];

    if ( cont_allPressed & A_BUTTON ) {
        Maze_Build( MG_MAZE_SIZE, MG_MAZE_SIZE, mg_maze );
        Maze_Draw( 0x90F, MG_MAZE_SIZE, MG_MAZE_SIZE, mg_maze );
    }
    Obj_Frame();
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

/*
================
MG_NullCreate

An empty handler of the scene's classes.
================
*/
void MG_NullCreate( void ) {
}

/*
================
MG_NullHandler

An empty handler of the scene's classes.
================
*/
void MG_NullHandler( void ) {
}

/* ===== Data ===== */

/* The classes of the scene, by objParams_t.type: 0 and 10 scene objects, 1 and 11 cameras; the rest unused. */
objClass_t mg_classes[] = {
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_Draw },
    { Cam_Create, (objUpdateFunc_t)Cam_UpdateAndAdvance },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { (objCreateFunc_t)MG_NullCreate, (objUpdateFunc_t)MG_NullHandler },
    { Obj_CreateSceneObject, (objUpdateFunc_t)Obj_AddToDrawList },
    { Cam_Create, (objUpdateFunc_t)Cam_Update },
};

/*
The scene: two cameras on path 2083, each 160 x 240, side by side; a scene object of model 1318 and animation
2018; the maze's image 0x90F (2319, drawn by Maze_Draw) at 20, 20.
*/
objParams_t mg_sceneObjects[] = {
    { 1.0f, 0.0f, { 2083.0f, 160.0f, 240.0f, 0.0f } },
    { 1.0f, 0.0f, { 2083.0f, 160.0f, 240.0f, 160.0f } },
    { 0.0f, 5.0f, { 1318.0f, 2018.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { OBJ_PARAMS_SKIP, 2319.0f, { 20.0f, 20.0f, 255.0f, 3.0f, 3.0f } },
    { OBJ_PARAMS_END },
};

/* ===== BSS ===== */

/* The maze buffer; the 10 x 10 maze uses its first 21 x 21 bytes. */
u8 mg_maze[MG_MAZE_BUF_SIZE];
