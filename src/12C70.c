/* 12C70.c -- the game's use of the sound driver: SE, BGM files and the audio task; screenshots */

#include "common.h"
#include "libultra.h"
#include <PR/gu.h>
#include "boot.h"
#include "archive.h"
#include "fat.h"
#include "gamesnd.h"
#include "hostio.h"
#include "input.h"

/*
================
func_80037870

Plays SE index, panned by flags; returns its sound id.
================
*/
u64 func_80037870( s32 index, s32 flags ) {
    s32 reuse;
    s32 volume;
    s32 pan;

    volume = 0x80;
    if ( flags & SE_NO_REUSE ) {
        reuse = FALSE;
    } else {
        reuse = TRUE;
    }
    switch ( flags & SE_PAN_MASK ) {
    case 0:
        pan = 0x80;
        break;
    case 1:
        pan = 0x40;
        break;
    case 2:
        pan = 0xC0;
        break;
    case 3:
        pan = guRandom() % 255;
        volume = guRandom() % 127 + 0x60;
        break;
    }
    if ( index == 50 ) {
        return Snd_PlaySeEx( index, volume, pan, reuse, 100 );
    }
    return Snd_PlaySeEx( index, volume, pan, reuse, 101 );
}

/*
================
func_800379A8

Stops sound id.
================
*/
void func_800379A8( u64 id ) {
    Snd_Stop( id, 4 );
}

/*
================
func_800379C8

Stops the BGM and loads BGM index into the other buffer; it starts BGM_START_DELAY frames later, unless the
settings skip it.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: stack frame; retail's is 0x30 bytes, 0x10 more than this needs, as if from an unused 16-byte local; the code is exact
void func_800379C8( s32 index ) {
    if ( D_8005C9D8 != 0 ) {
        Snd_Stop( D_8005C9D8, 5 );
    }
    D_8005C9D0 ^= 1;
    func_80029AF4( D_8005CA0C[index], D_800EA440[D_8005C9D0] );
    if ( D_8009D098.unk_79 != 0 || D_8005C9F8[index] == 0 ) {
        D_8005C9E8 = index;
        D_8005C9E0 = BGM_START_DELAY;
        D_800A3048 = (seqHeader_t *)D_800EA440[D_8005C9D0];
        D_8005C9E4 = D_8005C9EC[D_8009D098.unk_7A];
    } else {
        D_8005C9D8 = 0;
        D_8005C9E8 = index | BGM_STOPPED;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/12C70", func_800379C8);
#endif

/*
================
func_80037B18

Stops the BGM.
================
*/
void func_80037B18( void ) {
    if ( D_8005C9D8 != 0 ) {
        Snd_Stop( D_8005C9D8, 5 );
    }
    D_8005C9D8 = 0;
    D_8005C9E8 |= BGM_STOPPED;
}

/*
================
func_80037B84

Sets the BGM to the volume of the current setting.
================
*/
void func_80037B84( void ) {
    if ( D_8005C9D8 != 0 ) {
        Snd_SetVolume( D_8005C9D8, D_8005C9EC[D_8009D098.unk_7A] );
    }
}

/*
================
func_80037BD8
================
*/
void func_80037BD8( void ) {
}

/*
================
func_80037BE0

Waits for the next message on D_800E6520.
================
*/
void func_80037BE0( void ) {
    OSMesg msg;

    osRecvMesg( &D_800E6520, &msg, OS_MESG_BLOCK );
}

/*
================
func_80037C08

Runs program as the audio task and waits for it; D_8005B0F0 gets the time it took.
================
*/
void func_80037C08( UnkRspProgram *program ) {
    D_80099450 = osGetTime();
    D_80066B50 = program;
    osWritebackDCache( D_800ABD70, 0x33800 );
    osSendMesg( &D_80099428, D_800DF6A4, OS_MESG_NOBLOCK );
    osRecvMesg( &D_8009D040, NULL, OS_MESG_BLOCK );
    D_8005B0F0 = OS_CYCLES_TO_USEC( osGetTime() - D_80099450 );
}

INCLUDE_ASM("asm/nonmatchings/12C70", func_80037CD0);

/*
================
func_80037E30

On an L press: writes the screen to the host as the next "BWnnn.BMP".
================
*/
void func_80037E30( void ) {
    bmpFile_t *file;
    u8 *dst;
    s32 x;
    s32 y;
    u16 pixel;
    s32 fb;
    u8 name[0x10];

    if ( D_800E4D62 & L_TRIG ) {
        file = (bmpFile_t *)D_80066C38[0];
        dst = (u8 *)file;
        file->header = D_8005CA40;
        dst += sizeof( bmpHeader_t );
        fb = 1;
        for ( y = 0; y < SCREENSHOT_HEIGHT / 2; y++ ) {
            for ( x = 0; x < SCREENSHOT_WIDTH; x++ ) {
                pixel = D_80066C38[fb][y * SCREENSHOT_WIDTH + x];
                D_80066C38[fb][y * SCREENSHOT_WIDTH + x] = D_80066C38[fb][( SCREENSHOT_HEIGHT - 1 - y ) * SCREENSHOT_WIDTH + x];
                D_80066C38[fb][( SCREENSHOT_HEIGHT - 1 - y ) * SCREENSHOT_WIDTH + x] = pixel;
            }
        }
        for ( y = 0; y < SCREENSHOT_HEIGHT; y++ ) {
            for ( x = 0; x < SCREENSHOT_WIDTH; x++ ) {
                *dst++ = ( D_80066C38[fb][y * SCREENSHOT_WIDTH + x] & 0x3E ) << 2;
                *dst++ = ( D_80066C38[fb][y * SCREENSHOT_WIDTH + x] & 0x7C0 ) >> 3;
                *dst++ = ( D_80066C38[fb][y * SCREENSHOT_WIDTH + x] & 0xF800 ) >> 8;
            }
        }
        sprintf( name, "BW%3.3d.BMP", D_8005CA76++ );
        func_80029814( name, (u8 *)file, SCREENSHOT_FILE_SIZE );
        D_800E4D62 = 0;
    }
}
