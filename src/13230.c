/* 13230.c -- debug screenshots: the screen written to the development host as a BMP file */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "hostio.h"
#include "input.h"
#include "dbgshot.h"

/*
================
Dbg_ScreenShot

On an L press: writes the screen to the host as the next "BWnnn.BMP".
================
*/
void Dbg_ScreenShot( void ) {
    bmpFile_t *file;
    u8 *dst;
    s32 x;
    s32 y;
    u16 pixel;
    s32 fb;
    u8 name[0x10];

    if ( cont_allPressed & L_TRIG ) {
        file = (bmpFile_t *)sched_framebuffers[0];
        dst = (u8 *)file;
        file->header = dbg_shotHeader;
        dst += sizeof( bmpHeader_t );
        fb = 1;
        for ( y = 0; y < SCREENSHOT_HEIGHT / 2; y++ ) {
            for ( x = 0; x < SCREENSHOT_WIDTH; x++ ) {
                pixel = sched_framebuffers[fb][y * SCREENSHOT_WIDTH + x];
                sched_framebuffers[fb][y * SCREENSHOT_WIDTH + x] =
                    sched_framebuffers[fb][( SCREENSHOT_HEIGHT - 1 - y ) * SCREENSHOT_WIDTH + x];
                sched_framebuffers[fb][( SCREENSHOT_HEIGHT - 1 - y ) * SCREENSHOT_WIDTH + x] = pixel;
            }
        }
        for ( y = 0; y < SCREENSHOT_HEIGHT; y++ ) {
            for ( x = 0; x < SCREENSHOT_WIDTH; x++ ) {
                *dst++ = ( sched_framebuffers[fb][y * SCREENSHOT_WIDTH + x] & 0x3E ) << 2;
                *dst++ = ( sched_framebuffers[fb][y * SCREENSHOT_WIDTH + x] & 0x7C0 ) >> 3;
                *dst++ = ( sched_framebuffers[fb][y * SCREENSHOT_WIDTH + x] & 0xF800 ) >> 8;
            }
        }
        sprintf( name, "BW%3.3d.BMP", dbg_shotNumber++ );
        Host_WriteFile( name, (u8 *)file, SCREENSHOT_FILE_SIZE );
        cont_allPressed = 0;
    }
}
