#ifndef GAMESND_H
#define GAMESND_H

/* gamesnd.h -- the game's use of the sound driver: SE, BGM files and the audio task (unit 12C70) */

#include "common.h"
#include "libultra.h"
#include "structs.h"
#include "boot.h"

#define BGM_BUFFER_SIZE     0x4650      /* bytes of each BGM sequence buffer */
#define BGM_START_DELAY     10          /* frames from loading a BGM to starting it */
#define BGM_STOPPED         0x100       /* sound_bgm: the BGM is not playing */

/* Sound_PlaySe flags */
#define SE_PAN_MASK         0x03        /* 0: centre, 1: left, 2: right, 3: random pan and volume */
#define SE_NO_REUSE         0x80

/* SE indices (names invented) */
#define SE_CURSOR           0           /* a menu cursor moves */
#define SE_DECIDE           1           /* a menu choice is taken */

#define SCREENSHOT_WIDTH    320
#define SCREENSHOT_HEIGHT   240
#define SCREENSHOT_FILE_SIZE 0x38438   /* bytes written, as the header's bfSize: 2 more than the header and pixels */

/* The 54 bytes in front of a screenshot's pixels: a BMP file and info header, 24-bit, bottom-up. */
typedef struct {
    /* 0x00 */ u8 data[0x36];
} bmpHeader_t;

/* A screenshot as Dbg_ScreenShot writes it to the host. */
typedef struct {
    /* 0x00000 */ bmpHeader_t header;
    /* 0x00036 */ u8 pixels[SCREENSHOT_HEIGHT][SCREENSHOT_WIDTH][3];  /* blue, green, red */
} bmpFile_t;

extern s32 sound_bgmBuffer;         /* the sound_bgmBuffers buffer of the last BGM loaded */
extern u16 sound_bgm;               /* index of the current BGM, | BGM_STOPPED */
extern u16 sound_bgmVolumes[];      /* BGM volume of each save_data.musicVolume setting */
extern u8 sound_bgmIsMusic[];       /* nonzero: the BGM is skipped while save_data.music is 0 */
extern s16 sound_bgmFiles[];        /* str file of each BGM */
extern u8 sound_bgmBuffers[2][BGM_BUFFER_SIZE];  /* BGM sequence buffers, used in turn */
extern bmpHeader_t dbg_shotHeader;  /* the header in front of each screenshot's pixels */
extern s16 dbg_shotNumber;          /* number of the next screenshot */
extern OSMesgQueue sound_frameQueue;  /* Main_Thread sends to it every other retrace */
extern OSMesgQueue sched_queue;     /* Main_Thread's: the SP, DP, PRENMI and retrace events and the audio task requests */
extern OSMesgQueue sound_doneQueue;  /* where the audio task reports completion */
extern s32 sched_msgAudio;          /* the audio task request message: its code, SCHED_MSG_AUDIO */
extern u8 sound_heap[];             /* audio heap */
extern OSTime sound_taskStart;      /* when the last audio task started */
extern s32 sound_taskTime;          /* microseconds the last audio task took */

/* sound driver functions (unit 166B0) this unit calls that functions.h does not declare */
void Snd_Init( sndConfig_t *config );
u32 Snd_PlaySeEx( s32 index, s32 volume, s32 pan, s32 reuse, s32 priority );
s32 Snd_Stop( s32 id, s32 ticks );
void Snd_SetSeWaveBank( seTable_t *table, waveBank_t *bank );

u64 Sound_PlaySe( s32 index, s32 flags );
void Sound_Stop( u64 id );
void Sound_PlayBgm( s32 index );
void Sound_StopBgm( void );
void Sound_UpdateBgmVolume( void );
void func_80037BD8( void );
void Sound_WaitFrame( void );
void Sound_RunAudioTask( UnkRspProgram *program );
void func_80037CD0( void );
void Dbg_ScreenShot( void );

#endif
