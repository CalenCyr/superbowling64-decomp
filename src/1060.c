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

#define HEAP_START  0x80150000
#define HEAP_END    0x8038F7FF

/*
================
main

Initialises libultra, clears memory from D_8010A250 to the end of RDRAM, sets up the heap and the
records, and starts the boot thread.
================
*/
void main( void ) {
    s32 i;

    __osInitialize_common();
    __osInitialize_autodetect();
    D_800A2FE4 = 0;
    memset( D_8010A250, 0, RAM_END - (u32)D_8010A250 );
    D_80066C38[0] = (u16 *)0x8038F800;
    D_80066C38[1] = (u16 *)0x803B5000;
    D_80066C38[2] = (u16 *)0x803DA800;
    func_80029DF0( (HeapBlock *)HEAP_START, (u8 *)HEAP_END );
    D_801000B8 = &D_8009D098;
    D_8009D098.unk_00 = 0;
    D_8009D098.unk_04 = 3;
    D_8009D098.unk_78 = 1;
    D_8009D098.unk_79 = 1;
    D_8009D098.unk_7A = 2;
    D_8009D098.unk_7B = 1;
    for ( i = 0; i < 4; i++ ) {
        memset( &D_8009D098.unk_08[i], 0, sizeof( UnkRecord ) );
        D_8009D098.unk_08[i].low = 300;
        memset( D_8009D098.unk_08[i].name, 0xFF, sizeof( D_8009D098.unk_08[i].name ) );
        memset( &D_800E44E8[i], 0, sizeof( UnkRecord ) );
        D_800E44E8[i].low = 300;
        memset( D_800E44E8[i].name, 0xFF, sizeof( D_800E44E8[i].name ) );
    }
    osCreateThread( &D_80060EE0, 1, func_80025E10, NULL, &D_80061090[BOOT_STACK_SIZE / sizeof( u64 )], 10 );
    osStartThread( &D_80060EE0 );
}

/*
================
func_80025E10

Boot thread: starts the VI and PI managers and the main thread, then idles.
================
*/
void func_80025E10( void *arg ) {
    osCreateViManager( OS_PRIORITY_VIMGR );
    osCreatePiManager( OS_PRIORITY_PIMGR, &D_801000E0, D_800693B8, 8 );
    D_800ABD60 = osCartRomInit();
    osCreateThread( &D_80061290, 3, func_80025FEC, arg, &D_80061440[BOOT_STACK_SIZE / sizeof( u64 )], 20 );
    osStartThread( &D_80061290 );
    osSetThreadPri( NULL, OS_PRIORITY_IDLE );
    for ( ;; ) {
    }
}

/*
================
func_80025EBC

Runs the D_80066B50 program on the RSP as an audio task; then counts down D_8005C9E0 and starts sequence
D_8005C9E4 when it reaches zero.
================
*/
void func_80025EBC( void ) {
    D_8005B110 = 2;
    D_80060EA0.t.type = M_AUDTASK;
    D_80060EA0.t.flags = 0;
    D_80060EA0.t.ucode_boot = rspbootTextStart;
    D_80060EA0.t.ucode_boot_size = (u8 *)rspbootTextEnd - (u8 *)rspbootTextStart;
    D_80060EA0.t.ucode = D_80066B50->ucode;
    D_80060EA0.t.ucode_data = D_80066B50->ucodeData;
    D_80060EA0.t.ucode_size = 0x1000;
    D_80060EA0.t.ucode_data_size = 0x800;
    D_80060EA0.t.data_ptr = D_80066B50->dataPtr;
    D_8005B100 = 1;
    D_80060EA0.t.data_size = D_80066B50->dataSize;
    osSpTaskLoad( &D_80060EA0 );
    osSpTaskStartGo( &D_80060EA0 );
    if ( D_8005C9E4 != 0 ) {
        if ( --D_8005C9E0 == 0 ) {
            D_8005C9D8 = (u32)Snd_PlayBgm( D_800A3048 );
            Snd_SetVolume( D_8005C9D8, D_8005C9E4 );
            D_8005C9E4 = 0;
        }
    }
}

/*
================
func_80025FEC

Main thread: creates the message queues, loads the file table, IDX and KNSTATICSYS, starts the controller
and game threads, then schedules the RSP and RDP for ever: it swaps the framebuffers on retrace, runs the
audio task when asked (making the graphics task yield), and starts each display list the game thread builds.
================
*/
void func_80025FEC( void *arg ) {
    OSMesg msg;
    s32 index;

    osCreateMesgQueue( &D_800DF578, D_800E675C, 1 );
    osCreateMesgQueue( &D_80099428, D_801000C0, 8 );
    osCreateMesgQueue( &D_800E57E8, D_8009D060, 1 );
    osCreateMesgQueue( &D_800E6520, D_8009CF8C, 1 );
    osCreateMesgQueue( &D_800E8078, D_8009CF98, 1 );
    osCreateMesgQueue( &D_8009D040, D_800DF6A0, 1 );
    osCreateMesgQueue( &D_800E6690, D_80066C2C, 1 );
    osCreateMesgQueue( &D_800F8098, D_8009D02C, 1 );
    osCreateMesgQueue( &D_800DF688, D_8009D068, 1 );
    D_800E4558 = SCHED_MSG_SP_DONE;
    D_80066B90 = SCHED_MSG_DP_DONE;
    D_8009D058 = SCHED_MSG_PRENMI;
    D_800DF6A4 = SCHED_MSG_AUDIO;
    D_80069408 = SCHED_MSG_RETRACE;
    D_800A2FD4 = SCHED_MSG_GFX;
    osSetEventMesg( OS_EVENT_SP, &D_80099428, &D_800E4558 );
    osSetEventMesg( OS_EVENT_DP, &D_80099428, &D_80066B90 );
    osSetEventMesg( OS_EVENT_PRENMI, &D_80099428, &D_8009D058 );
    osViSetEvent( &D_80099428, &D_80069408, 1 );

    osInvalDCache( &D_8010A080, fat_ROM_END - fat_ROM_START );
    D_80066B70.hdr.pri = OS_MESG_PRI_NORMAL;
    D_80066B70.hdr.retQueue = &D_800DF578;
    D_80066B70.dramAddr = &D_8010A080;
    D_80066B70.devAddr = (u32)fat_ROM_START;
    D_80066B70.size = fat_ROM_END - fat_ROM_START;
    osEPiStartDma( D_800ABD60, &D_80066B70, OS_READ );
    osRecvMesg( &D_800DF578, NULL, OS_MESG_BLOCK );
    func_80028744( "IDX     SYS", D_800A30C0, func_8002889C( "IDX     SYS" ), 0 );
    osInvalDCache( D_8010A250, knstatic_ROM_END - knstatic_ROM_START );
    func_80028744( "KNSTATICSYS", D_8010A250, func_8002889C( "KNSTATICSYS" ), 0 );
    func_800279F0();
    osContStartReadData( &D_80066B38 );
    osCreateThread( &D_800617F0, 2, func_80028064, arg, &D_80068250[CONT_STACK_SIZE / sizeof( u64 )], 0x11 );
    osStartThread( &D_800617F0 );
    osCreateThread( &D_80061640, 5, func_80026B44, arg, &D_800E8090[GAME_STACK_SIZE / sizeof( u64 )], 8 );
    osStartThread( &D_80061640 );
    func_80037CD0();

    for ( ;; ) {
        osRecvMesg( &D_80099428, &msg, OS_MESG_BLOCK );
        switch ( *(s32 *)msg ) {
        case SCHED_MSG_PRENMI:
            D_8005C0B4[0] = TRUE;
            if ( osAfterPreNMI() == 0 ) {
                D_8005C0B4[1] = TRUE;
            }
            break;
        case SCHED_MSG_RETRACE:
            if ( D_8005B0F4 & 1 ) {
                osSendMesg( &D_800E6520, &msg, OS_MESG_NOBLOCK );
            }
            osSendMesg( &D_800E57E8, &D_80069408, OS_MESG_NOBLOCK );
            ++D_8005B0F4;
            if ( D_8005B0F4 % 60 == 0 ) {
                D_8005B0FC = D_8005B130;
                D_8005B130 = 0;
            }
            if ( D_8005B12C++ != 0 ) {
                D_8005B114[D_8005C0B3] = 4;
                if ( D_8005B114[D_8005C0B2] == 1 ) {
                    osViSwapBuffer( D_80066C38[D_8005C0B2] );
                    D_8005C0B3 = D_8005C0B2;
                    if ( ++D_8005C0B2 == 3 ) {
                        D_8005C0B2 = 0;
                    }
                    D_8005B12C = 0;
                    D_8005B130++;
                }
            }
            break;
        case SCHED_MSG_SP_DONE:
            D_8005B100 = 0;
            if ( D_8005C0B4[0] == TRUE && D_8005C0B4[1] == FALSE ) {
                if ( osAfterPreNMI() == 0 ) {
                    D_8005C0B4[1] = TRUE;
                }
            } else if ( D_8005B110 == 2 ) {
                D_8005B110 = 0;
                if ( D_8005B10C == 2 ) {
                    D_8005B100 = 1;
                    D_8005B10C = 1;
                    osSpTaskLoad( &D_8005B140 );
                    osSpTaskStartGo( &D_8005B140 );
                }
                osSendMesg( &D_8009D040, &msg, OS_MESG_NOBLOCK );
            } else if ( D_8005B110 == 1 ) {
                if ( D_8005B104 != 0 ) {
                    D_8005B104 = 0;
                    if ( osSpTaskYielded( &D_8005B140 ) ) {
                        D_8005B10C = 2;
                    }
                }
                func_80025EBC();
            } else if ( D_8005B10C == 1 ) {
                D_8005B0E4 = OS_CYCLES_TO_USEC( osGetTime() - D_80066C48 );
            }
            break;
        case SCHED_MSG_DP_DONE:
            D_8005B124[D_8005B120] = 3;
            D_8005B114[D_80069410[D_8005B120].fbIndex] = 1;
            D_8005B10C = 0;
            if ( D_8005B114[D_8005C0B2] != 1 ) {
                D_8005C0B2 = D_80069410[D_8005B120].fbIndex;
            }
            D_8005B0E8 = OS_CYCLES_TO_USEC( osGetTime() - D_80066C48 );
            if ( D_8005C0BC != 0 && D_8005C0B6 != 1 && ( D_800994BC & Z_TRIG ) && ( D_800E4D62 & L_TRIG ) ) {
                func_80037E30();
            }
            break;
        case SCHED_MSG_AUDIO:
            if ( D_8005C0B4[0] == FALSE ) {
                if ( D_8005B100 != 0 ) {
                    osSpTaskYield();
                    D_8005B104 = 1;
                    D_8005B110 = 1;
                    continue;
                }
                func_80025EBC();
            }
            break;
        case SCHED_MSG_GFX:
            D_8005B108 = 0;
            D_8005B124[D_8005C0B1] = 1;
            break;
        }

        index = D_8005B0F8 + 1;
        if ( index == 3 ) {
            index = 0;
        }
        if ( D_8005B108 == 0 && D_8005B124[( D_8005C0B1 + 1 ) & 1] == 3 ) {
            if ( ++D_8005B0F8 == 3 ) {
                D_8005B0F8 = 0;
            }
            if ( ++D_8005C0B1 == 2 ) {
                D_8005C0B1 = 0;
            }
            D_8005B108 = 1;
            D_8005B124[D_8005C0B1] = 0;
            D_80069410[D_8005C0B1].fbIndex = D_8005B0F8;
            osSendMesg( &D_800DF688, &index, OS_MESG_NOBLOCK );
            D_80099440 = osGetTime();
        }
        if ( D_8005B100 == 0 && D_8005B10C == 0 && D_8005C0B4[0] == FALSE ) {
            index = ( D_8005B120 + 1 ) & 1;
            if ( D_8005B124[index] != 0 &&
                 D_80066C38[D_80069410[index].fbIndex] != osViGetCurrentFramebuffer() &&
                 D_80066C38[D_80069410[index].fbIndex] != osViGetNextFramebuffer() ) {
                D_8005B120 = ( D_8005B120 + 1 ) & 1;
                D_8005B124[D_8005B120] = 2;
                D_8005B140.t.flags = OS_TASK_LOADABLE;
                D_8005B140.t.ucode_boot_size = (u8 *)rspbootTextEnd - (u8 *)rspbootTextStart;
                D_8005B140.t.ucode_boot = (u64 *)rspbootTextStart;
                D_8005B140.t.ucode = (u64 *)gspF3DEX2_Rej_fifoTextStart;
                D_8005B140.t.ucode_data = (u64 *)gspF3DEX2_Rej_fifoDataStart;
                D_8005B140.t.data_ptr = (u64 *)D_80069410[D_8005B120].list;
                D_8005B140.t.data_size = D_80069410[D_8005B120].size;
                D_8005B100 = 1;
                D_8005B10C = 1;
                osWritebackDCacheAll();
                osSpTaskLoad( &D_8005B140 );
                osSpTaskStartGo( &D_8005B140 );
                D_80066C48 = osGetTime();
            }
        }
    }
}

/*
================
func_80026B10

Sets D_800DF680 of both ports to 8.
================
*/
void func_80026B10( void ) {
    D_800DF680[1] = 8;
    D_800DF680[0] = 8;
}

/*
================
func_80026B28
================
*/
void func_80026B28( void ) {
    D_8009CFB0 = 75;
    D_800619A0 = 12;
}

INCLUDE_ASM("asm/nonmatchings/1060", func_80026B44);
