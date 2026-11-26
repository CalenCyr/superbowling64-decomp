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
Sound_PlaySe

Plays SE index, panned by flags; returns its sound id.
================
*/
u64 Sound_PlaySe( s32 index, s32 flags ) {
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
Sound_Stop

Stops sound id.
================
*/
void Sound_Stop( u64 id ) {
    Snd_Stop( id, 4 );
}

/*
================
Sound_PlayBgm

Stops the BGM and loads BGM index into the other buffer; it starts BGM_START_DELAY frames later, unless the
settings skip it.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: stack frame; retail's is 0x30 bytes, 0x10 more than this needs, as if from an unused 16-byte local; the code is exact.
// An unused 16-byte local gives the exact bytes (not adopted, R17); a dead call with 6 to 8 argument words would
// too (gcc 2.7.2 keeps outgoing-argument space for calls it deletes), but this unit has none to point to.
void Sound_PlayBgm( s32 index ) {
    if ( sound_bgmId != 0 ) {
        Snd_Stop( sound_bgmId, 5 );
    }
    sound_bgmBuffer ^= 1;
    Str_ReadFile( sound_bgmFiles[index], sound_bgmBuffers[sound_bgmBuffer] );
    if ( save_data.music != 0 || sound_bgmIsMusic[index] == 0 ) {
        sound_bgm = index;
        sound_bgmDelay = BGM_START_DELAY;
        sound_nextBgm = (seqHeader_t *)sound_bgmBuffers[sound_bgmBuffer];
        sound_bgmStartVolume = sound_bgmVolumes[save_data.musicVolume];
    } else {
        sound_bgmId = 0;
        sound_bgm = index | BGM_STOPPED;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/12C70", Sound_PlayBgm);
#endif

/*
================
Sound_StopBgm

Stops the BGM.
================
*/
void Sound_StopBgm( void ) {
    if ( sound_bgmId != 0 ) {
        Snd_Stop( sound_bgmId, 5 );
    }
    sound_bgmId = 0;
    sound_bgm |= BGM_STOPPED;
}

/*
================
Sound_UpdateBgmVolume

Sets the BGM to the volume of the current setting.
================
*/
void Sound_UpdateBgmVolume( void ) {
    if ( sound_bgmId != 0 ) {
        Snd_SetVolume( sound_bgmId, sound_bgmVolumes[save_data.musicVolume] );
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
Sound_WaitFrame

Waits for the next message on sound_frameQueue.
================
*/
void Sound_WaitFrame( void ) {
    OSMesg msg;

    osRecvMesg( &sound_frameQueue, &msg, OS_MESG_BLOCK );
}

/*
================
Sound_RunAudioTask

Runs program as the audio task and waits for it; sound_taskTime gets the time it took.
================
*/
void Sound_RunAudioTask( rspProgram_t *program ) {
    sound_taskStart = osGetTime();
    sched_audioProgram = program;
    osWritebackDCache( sound_heap, 0x33800 );
    osSendMesg( &sched_queue, &sched_msgAudio, OS_MESG_NOBLOCK );
    osRecvMesg( &sound_doneQueue, NULL, OS_MESG_BLOCK );
    sound_taskTime = OS_CYCLES_TO_USEC( osGetTime() - sound_taskStart );
}

INCLUDE_ASM("asm/nonmatchings/12C70", func_80037CD0);

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
