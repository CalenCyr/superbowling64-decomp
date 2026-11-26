/* 1060.c -- boot: main, the boot threads, the RSP start-up task, and the main thread */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "heap.h"
#include "input.h"
#include "libc.h"
#include "fat.h"
#include "archive.h"
#include "filecache.h"
#include "game.h"
#include "gamesnd.h"
#include "dbgshot.h"
#include "ground.h"
#include "sprite.h"
#include "dbgtext.h"
#include "overlay.h"

#define HEAP_START  0x80150000
#define HEAP_END    0x8038F7FF

/* ===== Data ===== */

static u8 game_loadedScene = 0xFF;      /* the scene loaded (game_scene); 0xFF at boot */
s32 sched_rspTime = 0;
s32 sched_rdpTime = 0;
s32 sched_cpuTime = 0;
s32 sound_taskTime = 0;
// volatile: as declared in input.h (Main_Thread reloads it right after its own store)
volatile s32 sched_frameCount = 0;
// volatile: as declared in boot.h (the scheduler reloads it after its own increment)
volatile s32 sched_drawFb = 0;
s32 sched_frameRate = 0;
static s32 sched_rspBusy = 0;           /* a task runs on the RSP */
static s32 sched_yieldAsked = 0;        /* the graphics task was asked to yield */
static s32 sched_building = 0;          /* the game thread is building a list */
static s32 sched_gfxState = 0;          /* graphics task state: 0 idle, 1 running, 2 yielded */
static s32 sched_audioState = 0;        /* 2: the audio task runs; 1: it waits for the graphics task to yield */
static s32 sched_fbState[3] = { 4, 4, 4 };  /* state of each framebuffer of sched_framebuffers: 1 drawn, 4 shown */
static s32 sched_runList = 0;           /* index in sched_gfxFrames of the list the RSP runs */
static s32 sched_listState[2] = { 3, 3 };  /* state of each list of sched_gfxFrames: 0 building, 1 built, 2 running, 3 done */
static s32 sched_retraces = 0;          /* retraces since the last swap */
static s32 sched_swaps = 0;             /* swaps this second */
/* pixels a frame the cursor moves by, from the least to the most tilt */
static u8 game_cursorSpeeds[3][4] = {
    { 2, 4, 6, 8 },
    { 3, 6, 9, 12 },
    { 3, 8, 12, 16 },
};
/* the graphics task */
static OSTask sched_gfxTask = { {
    M_GFXTASK, OS_TASK_LOADABLE,
    NULL, 0,                            /* ucode_boot, set per task */
    NULL, SP_UCODE_SIZE,
    NULL, SP_UCODE_DATA_SIZE,
    D_800F7C90, SP_DRAM_STACK_SIZE8,
    SCHED_FIFO_START, SCHED_FIFO_END,   /* the RDP's FIFO: output_buff_size is the buffer's end */
    NULL, 0,                            /* data_ptr, set per task */
    D_800E5800, OS_YIELD_DATA_SIZE,
} };
static u16 game_demoTitles[4] = { 0x0A72, 0x0A73, 0x0A74, 0x0A70 };  /* per demo (game_demo): the image the demo shows its title by */
static u8 game_blankFrames = 0;         /* blank frames drawn before the next scene loads */

/* ===== State ===== */

static OSTask sched_audioTask;      /* the audio task Main_StartAudioTask runs */
static OSThread boot_bootThread;    /* boot thread: starts the managers, then idles */
static u64 boot_bootStack[BOOT_STACK_SIZE / sizeof( u64 )];
static OSThread boot_mainThread;    /* main thread, Main_Thread */
static u64 boot_mainStack[BOOT_STACK_SIZE / sizeof( u64 )];
static OSThread boot_gameThread;    /* game thread, Game_Thread */
static OSThread boot_contThread;    /* controller thread, Cont_Thread */
static s8 game_pulseStep;           /* +12 or -12 */

/*
================
main

Initialises libultra, clears memory from boot_staticFile to the end of RDRAM, sets up the heap and the
records, and starts the boot thread.
================
*/
void main( void ) {
    s32 i;

    __osInitialize_common();
    __osInitialize_autodetect();
    D_800A2FE4 = 0;
    memset( boot_staticFile, 0, RAM_END - (u32)boot_staticFile );
    sched_framebuffers[0] = (u16 *)0x8038F800;
    sched_framebuffers[1] = (u16 *)0x803B5000;
    sched_framebuffers[2] = (u16 *)0x803DA800;
    Mem_Reset( (HeapBlock *)HEAP_START, (u8 *)HEAP_END );
    save_dataPtr = &save_data;
    save_data.checksum = 0;
    save_data.unlocks = 3;
    save_data.difficulty = 1;
    save_data.music = 1;
    save_data.musicVolume = 2;
    save_data.rumble = 1;
    for ( i = 0; i < 4; i++ ) {
        memset( &save_data.records[i], 0, sizeof( playerRecord_t ) );
        save_data.records[i].low = 300;
        memset( save_data.records[i].name, 0xFF, sizeof( save_data.records[i].name ) );
        memset( &game_sessionRecords[i], 0, sizeof( playerRecord_t ) );
        game_sessionRecords[i].low = 300;
        memset( game_sessionRecords[i].name, 0xFF, sizeof( game_sessionRecords[i].name ) );
    }
    osCreateThread( &boot_bootThread, 1, Boot_Thread, NULL, &boot_bootStack[BOOT_STACK_SIZE / sizeof( u64 )], 10 );
    osStartThread( &boot_bootThread );
}

/*
================
Boot_Thread

Boot thread: starts the VI and PI managers and the main thread, then idles.
================
*/
void Boot_Thread( void *arg ) {
    osCreateViManager( OS_PRIORITY_VIMGR );
    osCreatePiManager( OS_PRIORITY_PIMGR, &boot_piQueue, boot_piMsgs, 8 );
    boot_romHandle = osCartRomInit();
    osCreateThread( &boot_mainThread, 3, Main_Thread, arg, &boot_mainStack[BOOT_STACK_SIZE / sizeof( u64 )], 20 );
    osStartThread( &boot_mainThread );
    osSetThreadPri( NULL, OS_PRIORITY_IDLE );
    for ( ;; ) {
    }
}

/*
================
Main_StartAudioTask

Runs the sched_audioProgram program on the RSP as an audio task; then counts down sound_bgmDelay and starts sequence
sound_bgmStartVolume when it reaches zero.
================
*/
void Main_StartAudioTask( void ) {
    sched_audioState = 2;
    sched_audioTask.t.type = M_AUDTASK;
    sched_audioTask.t.flags = 0;
    sched_audioTask.t.ucode_boot = rspbootTextStart;
    sched_audioTask.t.ucode_boot_size = (u8 *)rspbootTextEnd - (u8 *)rspbootTextStart;
    sched_audioTask.t.ucode = sched_audioProgram->ucode;
    sched_audioTask.t.ucode_data = sched_audioProgram->ucodeData;
    sched_audioTask.t.ucode_size = 0x1000;
    sched_audioTask.t.ucode_data_size = 0x800;
    sched_audioTask.t.data_ptr = sched_audioProgram->dataPtr;
    sched_rspBusy = 1;
    sched_audioTask.t.data_size = sched_audioProgram->dataSize;
    osSpTaskLoad( &sched_audioTask );
    osSpTaskStartGo( &sched_audioTask );
    if ( sound_bgmStartVolume != 0 ) {
        if ( --sound_bgmDelay == 0 ) {
            sound_bgmId = (u32)Snd_PlayBgm( sound_nextBgm );
            Snd_SetVolume( sound_bgmId, sound_bgmStartVolume );
            sound_bgmStartVolume = 0;
        }
    }
}

/*
================
Main_Thread

Main thread: creates the message queues, loads the file table, IDX and KNSTATICSYS, starts the controller
and game threads, then schedules the RSP and RDP for ever: it swaps the framebuffers on retrace, runs the
audio task when asked (making the graphics task yield), and starts each display list the game thread builds.
================
*/
void Main_Thread( void *arg ) {
    OSMesg msg;
    s32 index;

    osCreateMesgQueue( &boot_dmaQueue, boot_dmaMsgs, 1 );
    osCreateMesgQueue( &sched_queue, sched_msgs, 8 );
    osCreateMesgQueue( &sched_retraceQueue, sched_retraceMsgs, 1 );
    osCreateMesgQueue( &sound_frameQueue, sound_frameMsgs, 1 );
    osCreateMesgQueue( &D_800E8078, D_8009CF98, 1 );
    osCreateMesgQueue( &sound_doneQueue, sound_doneMsgs, 1 );
    osCreateMesgQueue( &cont_requestQueue, cont_requestMsgs, 1 );
    osCreateMesgQueue( &cont_replyQueue, cont_replyMsgs, 1 );
    osCreateMesgQueue( &game_drawQueue, game_drawMsgs, 1 );
    sched_msgSpDone = SCHED_MSG_SP_DONE;
    sched_msgDpDone = SCHED_MSG_DP_DONE;
    sched_msgPreNmi = SCHED_MSG_PRENMI;
    sched_msgAudio = SCHED_MSG_AUDIO;
    sched_msgRetrace = SCHED_MSG_RETRACE;
    sched_msgGfx = SCHED_MSG_GFX;
    osSetEventMesg( OS_EVENT_SP, &sched_queue, &sched_msgSpDone );
    osSetEventMesg( OS_EVENT_DP, &sched_queue, &sched_msgDpDone );
    osSetEventMesg( OS_EVENT_PRENMI, &sched_queue, &sched_msgPreNmi );
    osViSetEvent( &sched_queue, &sched_msgRetrace, 1 );

    osInvalDCache( &fat_table, fat_ROM_END - fat_ROM_START );
    boot_dmaRequest.hdr.pri = OS_MESG_PRI_NORMAL;
    boot_dmaRequest.hdr.retQueue = &boot_dmaQueue;
    boot_dmaRequest.dramAddr = &fat_table;
    boot_dmaRequest.devAddr = (u32)fat_ROM_START;
    boot_dmaRequest.size = fat_ROM_END - fat_ROM_START;
    osEPiStartDma( boot_romHandle, &boot_dmaRequest, OS_READ );
    osRecvMesg( &boot_dmaQueue, NULL, OS_MESG_BLOCK );
    FAT_ReadFile( "IDX     SYS", str_files, FAT_GetFileSize( "IDX     SYS" ), 0 );
    osInvalDCache( boot_staticFile, knstatic_ROM_END - knstatic_ROM_START );
    FAT_ReadFile( "KNSTATICSYS", boot_staticFile, FAT_GetFileSize( "KNSTATICSYS" ), 0 );
    Cont_Init();
    osContStartReadData( &cont_siQueue );
    osCreateThread( &boot_contThread, 2, Cont_Thread, arg, &boot_contStack[CONT_STACK_SIZE / sizeof( u64 )], 0x11 );
    osStartThread( &boot_contThread );
    osCreateThread( &boot_gameThread, 5, Game_Thread, arg, &boot_gameStack[GAME_STACK_SIZE / sizeof( u64 )], 8 );
    osStartThread( &boot_gameThread );
    Sound_Init();

    for ( ;; ) {
        osRecvMesg( &sched_queue, &msg, OS_MESG_BLOCK );
        switch ( *(s32 *)msg ) {
        case SCHED_MSG_PRENMI:
            sched_reset[0] = TRUE;
            if ( osAfterPreNMI() == 0 ) {
                sched_reset[1] = TRUE;
            }
            break;
        case SCHED_MSG_RETRACE:
            if ( sched_frameCount & 1 ) {
                osSendMesg( &sound_frameQueue, &msg, OS_MESG_NOBLOCK );
            }
            osSendMesg( &sched_retraceQueue, &sched_msgRetrace, OS_MESG_NOBLOCK );
            ++sched_frameCount;
            if ( sched_frameCount % 60 == 0 ) {
                sched_frameRate = sched_swaps;
                sched_swaps = 0;
            }
            if ( sched_retraces++ != 0 ) {
                sched_fbState[sched_shownFb] = 4;
                if ( sched_fbState[sched_nextFb] == 1 ) {
                    osViSwapBuffer( sched_framebuffers[sched_nextFb] );
                    sched_shownFb = sched_nextFb;
                    if ( ++sched_nextFb == 3 ) {
                        sched_nextFb = 0;
                    }
                    sched_retraces = 0;
                    sched_swaps++;
                }
            }
            break;
        case SCHED_MSG_SP_DONE:
            sched_rspBusy = 0;
            if ( sched_reset[0] == TRUE && sched_reset[1] == FALSE ) {
                if ( osAfterPreNMI() == 0 ) {
                    sched_reset[1] = TRUE;
                }
            } else if ( sched_audioState == 2 ) {
                sched_audioState = 0;
                if ( sched_gfxState == 2 ) {
                    sched_rspBusy = 1;
                    sched_gfxState = 1;
                    osSpTaskLoad( &sched_gfxTask );
                    osSpTaskStartGo( &sched_gfxTask );
                }
                osSendMesg( &sound_doneQueue, &msg, OS_MESG_NOBLOCK );
            } else if ( sched_audioState == 1 ) {
                if ( sched_yieldAsked != 0 ) {
                    sched_yieldAsked = 0;
                    if ( osSpTaskYielded( &sched_gfxTask ) ) {
                        sched_gfxState = 2;
                    }
                }
                Main_StartAudioTask();
            } else if ( sched_gfxState == 1 ) {
                sched_rspTime = OS_CYCLES_TO_USEC( osGetTime() - sched_gfxStartTime );
            }
            break;
        case SCHED_MSG_DP_DONE:
            sched_listState[sched_runList] = 3;
            sched_fbState[sched_gfxFrames[sched_runList].fbIndex] = 1;
            sched_gfxState = 0;
            if ( sched_fbState[sched_nextFb] != 1 ) {
                sched_nextFb = sched_gfxFrames[sched_runList].fbIndex;
            }
            sched_rdpTime = OS_CYCLES_TO_USEC( osGetTime() - sched_gfxStartTime );
            if ( cache_hostMode != 0 && game_scene != 1 && ( cont_allHeld & Z_TRIG ) && ( cont_allPressed & L_TRIG ) ) {
                Dbg_ScreenShot();
            }
            break;
        case SCHED_MSG_AUDIO:
            if ( sched_reset[0] == FALSE ) {
                if ( sched_rspBusy != 0 ) {
                    osSpTaskYield();
                    sched_yieldAsked = 1;
                    sched_audioState = 1;
                    continue;
                }
                Main_StartAudioTask();
            }
            break;
        case SCHED_MSG_GFX:
            sched_building = 0;
            sched_listState[sched_bufferIndex] = 1;
            break;
        }

        index = sched_drawFb + 1;
        if ( index == 3 ) {
            index = 0;
        }
        if ( sched_building == 0 && sched_listState[( sched_bufferIndex + 1 ) & 1] == 3 ) {
            if ( ++sched_drawFb == 3 ) {
                sched_drawFb = 0;
            }
            if ( ++sched_bufferIndex == 2 ) {
                sched_bufferIndex = 0;
            }
            sched_building = 1;
            sched_listState[sched_bufferIndex] = 0;
            sched_gfxFrames[sched_bufferIndex].fbIndex = sched_drawFb;
            osSendMesg( &game_drawQueue, &index, OS_MESG_NOBLOCK );
            sched_drawStartTime = osGetTime();
        }
        if ( sched_rspBusy == 0 && sched_gfxState == 0 && sched_reset[0] == FALSE ) {
            index = ( sched_runList + 1 ) & 1;
            if ( sched_listState[index] != 0 &&
                 sched_framebuffers[sched_gfxFrames[index].fbIndex] != osViGetCurrentFramebuffer() &&
                 sched_framebuffers[sched_gfxFrames[index].fbIndex] != osViGetNextFramebuffer() ) {
                sched_runList = ( sched_runList + 1 ) & 1;
                sched_listState[sched_runList] = 2;
                sched_gfxTask.t.flags = OS_TASK_LOADABLE;
                sched_gfxTask.t.ucode_boot_size = (u8 *)rspbootTextEnd - (u8 *)rspbootTextStart;
                sched_gfxTask.t.ucode_boot = (u64 *)rspbootTextStart;
                sched_gfxTask.t.ucode = (u64 *)gspF3DEX2_Rej_fifoTextStart;
                sched_gfxTask.t.ucode_data = (u64 *)gspF3DEX2_Rej_fifoDataStart;
                sched_gfxTask.t.data_ptr = (u64 *)sched_gfxFrames[sched_runList].list;
                sched_gfxTask.t.data_size = sched_gfxFrames[sched_runList].size;
                sched_rspBusy = 1;
                sched_gfxState = 1;
                osWritebackDCacheAll();
                osSpTaskLoad( &sched_gfxTask );
                osSpTaskStartGo( &sched_gfxTask );
                sched_gfxStartTime = osGetTime();
            }
        }
    }
}

/*
================
Rumble_StartBoth

Sets cont_rumble of both ports to 8.
================
*/
inline void Rumble_StartBoth( void ) {
    cont_rumble[0] = cont_rumble[1] = 8;
}

/*
================
Game_StartPulse
================
*/
inline void Game_StartPulse( void ) {
    game_pulseAlpha = 75;
    game_pulseStep = 12;
}

/*
================
Game_Thread

Game thread: finishes the first three frames empty, sets the VI mode for the TV type, loads the floor grid,
then draws a frame each time the main thread asks: it moves the cursor by the stick, starts the display list,
and when the scene changed, draws two blank frames, then loads the scene's overlay and runs its init; then
the scene's per-frame function, the demo's titles, the fade of game_leaveState with the master volume, and the
CPU time of the frame.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: the frame is 0x48 bytes, retail's 0x58 (16 bytes of locals no instruction uses); retail's lbu
// and separate andi of game_scene before each switch need it volatile (game.h); otherwise only relocation
// names differ (game_cursorY is retail's game_cursorX + 2). One local holds the counter, the file size and the
// second switch's index, as retail's s2 does. An unused 16-byte local gives retail's frame (not adopted, R17);
// the inlined Rumble_StartBoth and Game_StartPulse have no locals, so their frames add nothing.
void Game_Thread( void *arg ) {
    s32 i;

    Game_StartPulse();
    for ( i = 0; i < 3; i++ ) {
        osRecvMesg( &game_drawQueue, NULL, OS_MESG_BLOCK );
        sched_buildFrame = &sched_gfxFrames[sched_bufferIndex];
        game_gfx = sched_buildFrame->list;
        gDPFullSync( game_gfx++ );
        gSPEndDisplayList( game_gfx++ );
        osSendMesg( &sched_queue, &sched_msgGfx, OS_MESG_NOBLOCK );
    }
    if ( osTvType == OS_TV_NTSC ) {
        osViSetMode( &osViModeTable[OS_VI_NTSC_LPN1] );
    } else if ( osTvType == OS_TV_MPAL ) {
        osViSetMode( &osViModeTable[OS_VI_MPAL_LPN1] );
    } else {
        for ( ;; ) {
        }
    }
    osViSetSpecialFeatures( OS_VI_GAMMA_OFF | OS_VI_DITHER_FILTER_ON );
    Cont_ReadPads();
    i = Str_ReadFile( 0xB5C, (u8 *)&game_floorGrids[0] );
    memcpy( &game_floorGrids[1], &game_floorGrids[0], i );

    for ( ;; ) {
        osRecvMesg( &game_drawQueue, NULL, OS_MESG_BLOCK );
        guRandom();
        Cont_ReadPads();
        game_pulseAlpha += game_pulseStep;
        if ( game_pulseAlpha == 255 || game_pulseAlpha == 75 ) {
            game_pulseStep *= -1;
        }

        if ( game_cursorLocked == 0 ) {
            if ( cont_allStickX > 60 ) {
                game_cursorX += game_cursorSpeeds[game_cursorSpeed][3];
            } else if ( cont_allStickX > 45 ) {
                game_cursorX += game_cursorSpeeds[game_cursorSpeed][2];
            } else if ( cont_allStickX > 35 ) {
                game_cursorX += game_cursorSpeeds[game_cursorSpeed][1];
            } else if ( cont_allStickX > 25 ) {
                game_cursorX += game_cursorSpeeds[game_cursorSpeed][0];
            } else if ( cont_allStickX > 10 ) {
                game_cursorX += 1;
            } else if ( cont_allStickX < -9 ) {
                if ( cont_allStickX > -25 ) {
                    game_cursorX -= 1;
                } else if ( cont_allStickX > -35 ) {
                    game_cursorX -= game_cursorSpeeds[game_cursorSpeed][0];
                } else if ( cont_allStickX > -45 ) {
                    game_cursorX -= game_cursorSpeeds[game_cursorSpeed][1];
                } else if ( cont_allStickX > -60 ) {
                    game_cursorX -= game_cursorSpeeds[game_cursorSpeed][2];
                } else {
                    game_cursorX -= game_cursorSpeeds[game_cursorSpeed][3];
                }
            }
            if ( cont_allStickY > 60 ) {
                game_cursorY -= game_cursorSpeeds[game_cursorSpeed][3];
            } else if ( cont_allStickY > 45 ) {
                game_cursorY -= game_cursorSpeeds[game_cursorSpeed][2];
            } else if ( cont_allStickY > 35 ) {
                game_cursorY -= game_cursorSpeeds[game_cursorSpeed][1];
            } else if ( cont_allStickY > 25 ) {
                game_cursorY -= game_cursorSpeeds[game_cursorSpeed][0];
            } else if ( cont_allStickY > 10 ) {
                game_cursorY -= 1;
            } else if ( cont_allStickY < -9 ) {
                if ( cont_allStickY > -25 ) {
                    game_cursorY += 1;
                } else if ( cont_allStickY > -35 ) {
                    game_cursorY += game_cursorSpeeds[game_cursorSpeed][0];
                } else if ( cont_allStickY > -45 ) {
                    game_cursorY += game_cursorSpeeds[game_cursorSpeed][1];
                } else if ( cont_allStickY > -60 ) {
                    game_cursorY += game_cursorSpeeds[game_cursorSpeed][2];
                } else {
                    game_cursorY += game_cursorSpeeds[game_cursorSpeed][3];
                }
            }
            if ( game_cursorX < CURSOR_MIN_X ) {
                game_cursorX = CURSOR_MIN_X;
            }
            if ( game_cursorX > CURSOR_MAX_X ) {
                game_cursorX = CURSOR_MAX_X;
            }
            if ( game_cursorY < CURSOR_MIN_Y ) {
                game_cursorY = CURSOR_MIN_Y;
            }
            if ( game_cursorY > CURSOR_MAX_Y ) {
                game_cursorY = CURSOR_MAX_Y;
            }
        }

        sched_buildFrame = &sched_gfxFrames[sched_bufferIndex];
        game_gfx = sched_buildFrame->list;
        gSPSegment( game_gfx++, 0, 0 );
        gSPDisplayList( game_gfx++, D_8010A2C8 );
        gSPDisplayList( game_gfx++, D_8010A2A0 );
        gDPSetDepthImage( game_gfx++, osVirtualToPhysical( scene_depthBuffer ) );
        gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                          osVirtualToPhysical( sched_framebuffers[sched_drawFb] ) );
        gDPPipeSync( game_gfx++ );

        if ( game_loadedScene != game_scene ) {
            if ( game_blankFrames < 2 ) {
                gDPSetCycleType( game_gfx++, G_CYC_FILL );
                gDPSetFillColor( game_gfx++, ( GPACK_RGBA5551( 192, 192, 192, 1 ) << 16 ) |
                                               GPACK_RGBA5551( 192, 192, 192, 1 ) );
                gDPFillRectangle( game_gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
                gDPPipeSync( game_gfx++ );
                gDPFullSync( game_gfx++ );
                gSPEndDisplayList( game_gfx++ );
                sched_buildFrame->size = ( game_gfx - sched_buildFrame->list ) * sizeof( Gfx );
                osSendMesg( &sched_queue, &sched_msgGfx, OS_MESG_NOBLOCK );
                game_blankFrames++;
                continue;
            }
            game_blankFrames = 0;
            Mem_Reset( NULL, NULL );
            Cache_Clear();
            Obj_ClearList();
            switch ( game_scene ) {
            case SCENE_GACODE:
                i = FAT_GetFileSize( "GACODE  SYS" );
                memset( gacode_VRAM + i, 0, gacode_BSS_END - gacode_VRAM - i );
                osInvalICache( gacode_VRAM, i );
                osInvalDCache( gacode_VRAM, i );
                FAT_ReadFile( "GACODE  SYS", gacode_VRAM, i, 0 );
                Rumble_StartBoth();
                GA_Init();
                break;
            case SCENE_RSCODE:
                i = FAT_GetFileSize( "RSCODE  SYS" );
                memset( rscode_VRAM + i, 0, rscode_BSS_END - rscode_VRAM - i );
                osInvalICache( rscode_VRAM, i );
                osInvalDCache( rscode_VRAM, i );
                FAT_ReadFile( "RSCODE  SYS", rscode_VRAM, i, 0 );
                Rumble_StartBoth();
                RS_Init();
                break;
            case SCENE_GSCODE:
                i = FAT_GetFileSize( "GSCODE  SYS" );
                memset( gscode_VRAM + i, 0, gscode_BSS_END - gscode_VRAM - i );
                osInvalICache( gscode_VRAM, i );
                osInvalDCache( gscode_VRAM, i );
                FAT_ReadFile( "GSCODE  SYS", gscode_VRAM, i, 0 );
                Rumble_StartBoth();
                GS_Init();
                break;
            case SCENE_CACODE:
                i = FAT_GetFileSize( "CACODE  SYS" );
                memset( cacode_VRAM + i, 0, cacode_BSS_END - cacode_VRAM - i );
                osInvalICache( cacode_VRAM, i );
                osInvalDCache( cacode_VRAM, i );
                FAT_ReadFile( "CACODE  SYS", cacode_VRAM, i, 0 );
                Rumble_StartBoth();
                CA_Init();
                break;
            case SCENE_PVCODE:
                i = FAT_GetFileSize( "PVCODE  SYS" );
                memset( pvcode_VRAM + i, 0, pvcode_BSS_END - pvcode_VRAM - i );
                osInvalICache( pvcode_VRAM, i );
                osInvalDCache( pvcode_VRAM, i );
                FAT_ReadFile( "PVCODE  SYS", pvcode_VRAM, i, 0 );
                Rumble_StartBoth();
                PV_Init();
                break;
            case SCENE_MGCODE:
                i = FAT_GetFileSize( "MGCODE  SYS" );
                memset( mgcode_VRAM + i, 0, mgcode_BSS_END - mgcode_VRAM - i );
                osInvalICache( mgcode_VRAM, i );
                osInvalDCache( mgcode_VRAM, i );
                FAT_ReadFile( "MGCODE  SYS", mgcode_VRAM, i, 0 );
                Rumble_StartBoth();
                MG_Init();
                break;
            case SCENE_OPCODE:
                i = FAT_GetFileSize( "OPCODE  SYS" );
                memset( opcode_VRAM + i, 0, opcode_BSS_END - opcode_VRAM - i );
                osInvalICache( opcode_VRAM, i );
                osInvalDCache( opcode_VRAM, i );
                FAT_ReadFile( "OPCODE  SYS", opcode_VRAM, i, 0 );
                Rumble_StartBoth();
                OP_Init();
                break;
            }
            game_loadedScene = game_scene;
        }

        i = game_scene;
        switch ( i ) {
        case SCENE_GACODE:
            GA_Frame();
            break;
        case SCENE_RSCODE:
            RS_Frame();
            break;
        case SCENE_GSCODE:
            GS_Frame();
            break;
        case SCENE_CACODE:
            CA_Frame();
            break;
        case SCENE_PVCODE:
            PV_Frame();
            break;
        case SCENE_MGCODE:
            MG_Frame();
            break;
        case SCENE_OPCODE:
            OP_Frame();
            break;
        }

        if ( cont_demoMode != 0 ) {
            draw_rdpMode = DRAW2D_FILL;
            Draw_ImageScaled( 0xA71, 80, 120, game_pulseAlpha, 1.0f, 1.0f );
            Draw_Image( 0x87F, 200, 152 );
            Draw_Image( game_demoTitles[game_demo], 200, 200 );
            if ( --cont_demoFrames < 40 ) {
                game_leaveState = 1;
                if ( game_fade >= 248 ) {
                    GA_FreezeScene();
                    game_leaveState = 2;
                    game_fade = 263;
                    game_scene = SCENE_OPCODE;
                    cont_demoMode = 0;
                    save_data.music = 1;
                }
            }
        }

        if ( game_leaveState != 0 ) {
            if ( game_leaveState == 1 ) {
                game_fade += 8;
                if ( game_fade > 255 ) {
                    game_fade = 255;
                }
            } else {
                game_fade -= 8;
                if ( game_fade < 0 ) {
                    game_fade = 0;
                    game_leaveState = 0;
                }
            }
            Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 192, 192, 192, game_fade );
            Snd_SetMasterVolume( SND_BGM, ( 255 - game_fade ) * 96 + 0x2000 );
        }

        gDPFullSync( game_gfx++ );
        gSPEndDisplayList( game_gfx++ );
        sched_buildFrame->size = ( game_gfx - sched_buildFrame->list ) * sizeof( Gfx );
        sched_cpuTime = OS_CYCLES_TO_USEC( osGetTime() - sched_drawStartTime );
        osSendMesg( &sched_queue, &sched_msgGfx, OS_MESG_NOBLOCK );
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/1060", Game_Thread);
#endif
