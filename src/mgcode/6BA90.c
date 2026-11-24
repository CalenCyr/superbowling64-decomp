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
    func_8002A2F0();
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
