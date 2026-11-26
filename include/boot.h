#ifndef BOOT_H
#define BOOT_H

/* boot.h -- boot, threads and the cartridge PI handle (units 1060 and 2DF0) */

#include "common.h"
#include "libultra.h"
#include "structs.h"
#include "functions.h"

#define BOOT_STACK_SIZE     0x200       /* bytes of stack of each boot thread */
#define RAM_END             0x80400000  /* end of the 4 MB of RDRAM, in KSEG0 */
#define SCREEN_WIDTH        320         /* pixels of the framebuffers of sched_framebuffers */
#define SCREEN_HEIGHT       240

/* A player's record (RS_AddToRecord), kept four at a time in save_data and game_sessionRecords. Name invented. */
typedef struct {
    /* 0x00 */ u16 name[6];                 /* a player's name, characters ended by 0xFFFF; 0xFF bytes at boot */
    /* 0x0C */ u8 games;                    /* games played, to 255 */
    /* 0x0E */ u16 total;                   /* sum of their scores, to 0xFFFF */
    /* 0x10 */ u16 high;                    /* highest score */
    /* 0x12 */ u16 low;                     /* lowest score; 300 at boot */
    /* 0x14 */ u16 strikes;                 /* to 9999: of 12 a game */
    /* 0x16 */ u16 spares;                  /* of 10 a game */
    /* 0x18 */ u16 splits;                  /* of 10 a game */
    /* 0x1A */ u8 unk_1A[0x02];
} playerRecord_t;                           /* size 0x1C */

/* The records and settings the game saves (save_data; Save_Checksum). Name invented. */
typedef struct {
    /* 0x00 */ s32 checksum;                /* Save_Checksum, checked when the save is read (opcode) */
    /* 0x04 */ u32 unlocks;                 /* bit per character of gscode's character menu: unlocked; 0x200: a perfect game */
    /* 0x08 */ playerRecord_t records[4];
    /* 0x78 */ u8 difficulty;               /* option, 0 to 2: narrows the aim spread of players the game plays */
    /* 0x79 */ u8 music;                    /* option: BGM on */
    /* 0x7A */ u8 musicVolume;              /* option, 0 to 4: index of the BGM volume */
    /* 0x7B */ u8 rumble;                   /* option: use the Rumble Pak */
    /* 0x7C */ u8 unk_7C[0x100 - 0x7C];
} saveData_t;                               /* size 0x100; Save_Checksum sums bytes 0x04-0xFF */

/* What Main_StartAudioTask runs on the RSP; set through sched_audioProgram. Names invented. */
typedef struct {
    /* 0x00 */ u64 *dataPtr;
    /* 0x04 */ u32 dataSize;
    /* 0x08 */ u64 *ucode;
    /* 0x0C */ u64 *ucodeData;
} rspProgram_t;

/*
libultra's raw PI register access, which the SDK headers of 2.0K declare only in the library's internal
piint.h; prototypes as there.
*/
s32 __osEPiRawReadIo( OSPiHandle *pihandle, u32 devAddr, u32 *data );
s32 __osEPiRawWriteIo( OSPiHandle *pihandle, u32 devAddr, u32 data );

extern OSPiHandle *boot_romHandle;  /* cartridge ROM handle */
extern OSIoMesg boot_dmaRequest;    /* the request of each ROM DMA */
extern OSMesgQueue boot_dmaQueue;   /* where ROM DMAs report completion */

extern u8 boot_staticFile[];        /* start of the memory main clears at boot (the KNSTATICSYS file) */
extern u8 D_800A2FE4;
extern u16 *sched_framebuffers[3];  /* RGBA5551 320x240 framebuffers: 0x8038F800, 0x803B5000, 0x803DA800 at boot */
/* index in sched_framebuffers of the framebuffer being drawn */
// volatile: the scheduler (1060) reloads it after its own increment, before the compare with 3
extern volatile s32 sched_drawFb;
extern s32 sched_rspTime;           /* RSP time of the last frame (mgcode's meter shows it as RSP) */
extern s32 sched_rdpTime;           /* RDP time of the last frame (shown as RDP) */
extern s32 sched_cpuTime;           /* CPU time of the last frame (shown as CPU) */
extern s32 sched_frameRate;         /* frame rate (shown as FRATE) */
/* 0 or 1: which of the double-buffered display lists and matrices is built */
// volatile: units 1060, 7250 and C650 read it with lbu and a separate sign extension, once per use (53B0 alone uses lb)
extern volatile s8 sched_bufferIndex;
extern Gfx *game_gfx;               /* next command of the display list being built */
extern saveData_t save_data;
extern saveData_t *save_dataPtr;    /* &save_data */
extern playerRecord_t game_sessionRecords[4];  /* per character: records since power-on, not saved */

extern OSThread boot_bootThread;    /* boot thread: starts the managers, then idles */
extern u64 boot_bootStack[BOOT_STACK_SIZE / sizeof( u64 )];
extern OSThread boot_mainThread;    /* main thread, Main_Thread */
extern u64 boot_mainStack[BOOT_STACK_SIZE / sizeof( u64 )];
extern OSMesgQueue boot_piQueue;    /* PI manager commands */
extern OSMesg boot_piMsgs[8];

extern OSTask sched_audioTask;
extern rspProgram_t *sched_audioProgram;
extern s32 sched_rspBusy;           /* a task runs on the RSP */
extern s32 sched_audioState;        /* 2: the audio task runs; 1: it waits for the graphics task to yield */
extern u64 sound_bgmId;             /* sound id of the playing BGM, zero-extended; 0: none */
extern s32 sound_bgmDelay;          /* frames until the BGM sound_nextBgm starts */
extern s32 sound_bgmStartVolume;    /* volume it starts at; nonzero while it waits */
extern seqHeader_t *sound_nextBgm;  /* the BGM Sound_PlayBgm loaded */

/* The main thread (Main_Thread) schedules the RSP and RDP; these are its messages, queues and state. */
#define SCHED_MSG_RETRACE   0           /* VI retrace (sched_msgRetrace) */
#define SCHED_MSG_SP_DONE   1           /* RSP task done (sched_msgSpDone) */
#define SCHED_MSG_DP_DONE   2           /* RDP done (sched_msgDpDone) */
#define SCHED_MSG_PRENMI    3           /* reset button pressed (sched_msgPreNmi) */
#define SCHED_MSG_GFX       4           /* the game thread finished a display list (sched_msgGfx) */
#define SCHED_MSG_AUDIO     5           /* the audio thread asks for its task (sched_msgAudio) */

/* each message is a pointer to its code */
extern s32 sched_msgRetrace;
extern s32 sched_msgSpDone;
extern s32 sched_msgDpDone;
extern s32 sched_msgPreNmi;
extern s32 sched_msgGfx;

extern OSMesg boot_dmaMsgs[1];      /* buffer of boot_dmaQueue */
extern OSMesg sched_msgs[8];        /* buffer of sched_queue */
extern OSMesgQueue sched_retraceQueue;  /* gets the retrace message each VI */
extern OSMesg sched_retraceMsgs[1];
extern OSMesg sound_frameMsgs[1];   /* buffer of sound_frameQueue */
extern OSMesgQueue D_800E8078;
extern OSMesg D_8009CF98[1];
extern OSMesg sound_doneMsgs[1];    /* buffer of sound_doneQueue */
extern OSMesg cont_requestMsgs[1];  /* buffer of cont_requestQueue */
extern OSMesg cont_replyMsgs[1];    /* buffer of cont_replyQueue */
extern OSMesgQueue game_drawQueue;  /* tells the game thread which framebuffer to draw next */
extern OSMesg game_drawMsgs[1];

#define CONT_STACK_SIZE     0x800       /* bytes of stack of the controller thread */
#define GAME_STACK_SIZE     0x2000      /* bytes of stack of the game thread */
extern OSThread boot_contThread;    /* controller thread, Cont_Thread */
extern u64 boot_contStack[CONT_STACK_SIZE / sizeof( u64 )];
extern OSThread boot_gameThread;    /* game thread, Game_Thread */
extern u64 boot_gameStack[GAME_STACK_SIZE / sizeof( u64 )];

/* A display list the game thread builds, two of them used in turn. */
#define GFX_LIST_LEN        0x3000
typedef struct {
    /* 0x00000 */ Gfx list[GFX_LIST_LEN];
    /* 0x18000 */ u32 size;             /* bytes of list, the task's data_size */
    /* 0x18004 */ s32 fbIndex;          /* index in sched_framebuffers of the framebuffer it draws */
} GfxFrame;                             /* size 0x18008 */

extern GfxFrame sched_gfxFrames[2];
extern GfxFrame *sched_buildFrame;  /* the one of sched_gfxFrames the game thread builds */
extern OSTask sched_gfxTask;        /* the graphics task */
extern s32 sched_runList;           /* index in sched_gfxFrames of the list the RSP runs */
extern s32 sched_listState[2];      /* state of each list of sched_gfxFrames: 0 building, 1 built, 2 running, 3 done */
extern s32 sched_fbState[3];        /* state of each framebuffer of sched_framebuffers: 1 drawn, 4 shown */
/* framebuffer to show next */
// volatile: the scheduler (1060) reads it with lbu and a separate sign extension, and reloads it right after each store
extern volatile s8 sched_nextFb;
/* framebuffer shown */
// volatile: the scheduler (1060) reads it with lbu and a separate sign extension, and reloads it right after each store
extern volatile s8 sched_shownFb;
extern u8 sched_reset[2];           /* [0]: reset pressed; [1]: tasks stopped after it */
extern s32 sched_yieldAsked;        /* the graphics task was asked to yield */
extern s32 sched_building;          /* the game thread is building a list */
extern s32 sched_gfxState;          /* graphics task state: 0 idle, 1 running, 2 yielded */
extern s32 sched_retraces;          /* retraces since the last swap */
extern s32 sched_swaps;             /* swaps this second */
extern OSTime sched_gfxStartTime;   /* when the graphics task started */
extern OSTime sched_drawStartTime;  /* when the game thread was told to draw */

void Game_Thread( void *arg );

/* The game thread (Game_Thread): its state. */
#define CURSOR_MIN_X        16          /* bounds of the cursor game_cursorX */
#define CURSOR_MAX_X        303
#define CURSOR_MIN_Y        16
#define CURSOR_MAX_Y        227
extern u8 game_pulseAlpha;          /* pulses between 75 and 255 by game_pulseStep a frame */
extern s8 game_pulseStep;           /* +12 or -12 */
extern s16 game_cursorX;            /* a cursor's x, moved by the stick; 160 at boot */
extern s16 game_cursorY;            /* its y; 120 at boot */
extern u8 game_cursorSpeed;         /* row of game_cursorSpeeds the cursor moves by */
extern u8 game_cursorSpeeds[3][4];  /* pixels a frame the cursor moves by, from the least to the most tilt */
extern u8 game_cursorLocked;        /* when set, the cursor stays */
extern u8 game_blankFrames;         /* blank frames drawn before the next scene loads */
extern u8 game_loadedScene;         /* the scene loaded (game_scene); 0xFF at boot */
extern u16 game_demoTitles[4];      /* per demo (game_demo): the image the demo shows its title by */
extern Gfx D_8010A2C8[];            /* display lists of the KNSTATICSYS file each frame starts with */
extern Gfx D_8010A2A0[];

void Boot_Thread( void *arg );
void Main_StartAudioTask( void );
void Main_Thread( void *arg );
void Rumble_StartBoth( void );
void Game_StartPulse( void );


#endif
