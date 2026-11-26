/* 12C70.c -- the game's use of the sound driver: SE, BGM files and the audio task */

#include "common.h"
#include "libultra.h"
#include <PR/gu.h>
#include "boot.h"
#include "archive.h"
#include "fat.h"
#include "gamesnd.h"

/* ===== Data ===== */

static s32 sound_bgmBuffer = 0;             /* the sound_bgmBuffers buffer of the last BGM loaded */
u64 sound_bgmId = 0;
s32 sound_bgmDelay = 0;
s32 sound_bgmStartVolume = 0;
u16 sound_bgm = 0xFFFF;
static u16 sound_bgmVolumes[5] = { 0x60, 0x70, 0x80, 0xA0, 0xC0 };  /* BGM volume of each save_data.musicVolume setting */
static u8 sound_bgmIsMusic[20] = {          /* nonzero: the BGM is skipped while save_data.music is 0 */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0,
};
static s16 sound_bgmFiles[20] = {           /* str file of each BGM */
    0x0B72, 0x0B71, 0x0B70, 0x0B6F, 0x0B6E, 0x0B6D, 0x0B6C, 0x0B6B, 0x0B6A, 0x0B69,
    0x0B68, 0x0B67, 0x0B66, 0x0B65, 0x0B64, 0x0B63, 0x0B62, 0x0B61, 0x0B60, 0x0B5F,
};
static audioTaskFuncs_t sound_taskFuncs = { Sound_AudioInit, Sound_WaitFrame, Sound_RunAudioTask };

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
void Sound_PlayBgm( s32 index ) {
    // R36: unused in the original: retail's frame reserves 16 bytes no instruction touches
    char fileName[16];

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
Sound_AudioInit
================
*/
void Sound_AudioInit( void ) {
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

/*
================
Sound_Init

Loads the BGM wave bank (BGMPTR), the SE wave bank (SEPTR) and the SE table (BFX), gives the audio thread this
unit's task routines and starts the sound driver.
================
*/
void Sound_Init( void ) {
    sndConfig_t config;

    FAT_ReadFile( "BGMPTR  SYS", &sound_bgmWaveBank, FAT_GetFileSize( "BGMPTR  SYS" ), 0 );
    FAT_ReadFile( "SEPTR   SYS", &sound_seWaveBank, FAT_GetFileSize( "SEPTR   SYS" ), 0 );
    FAT_ReadFile( "BFX     SYS", &sound_seTable, FAT_GetFileSize( "BFX     SYS" ), 0 );
    config.unk_00 = 0;
    config.numVoices = 32;
    config.sched = NULL;
    config.voicePriority = 18;
    config.heapBase = sound_heap;
    config.heapSize = 0x33800;
    config.waveBank = &sound_bgmWaveBank;
    config.seTable = &sound_seTable;
    config.numMsgs = 64;
    config.waveRomBase = (s32)bgmwbk_ROM_START;
    config.outputRate = 22050;
    config.maxUpdates = 0x200;
    config.maxCmds = 0x2000;
    config.ticksPerFrame = 2;
    config.numDmaBuffers = 128;
    config.dmaBufferSize = 0x180;
    Audio_SetTaskFuncs( &sound_taskFuncs );
    Snd_Init( &config );
    Snd_LoadWaveBank( &sound_seWaveBank, (s32)sewbk_ROM_START );
    Snd_SetSeWaveBank( &sound_seTable, &sound_seWaveBank );
}
