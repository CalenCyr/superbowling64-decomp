/* 1060.c -- boot: main, the boot threads, the RSP start-up task, and the main thread */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "heap.h"
#include "input.h"
#include "libc_boot.h"

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
    func_80056150( D_8010A250, 0, RAM_END - (u32)D_8010A250 );
    D_80066C38 = 0x8038F800;
    D_80066C3C = 0x803B5000;
    D_80066C40 = 0x803DA800;
    func_80029DF0( (HeapBlock *)HEAP_START, (u8 *)HEAP_END );
    D_801000B8 = &D_8009D098;
    D_8009D098.unk_00 = 0;
    D_8009D098.unk_04 = 3;
    D_8009D098.unk_78 = 1;
    D_8009D098.unk_79 = 1;
    D_8009D098.unk_7A = 2;
    D_8009D098.unk_7B = 1;
    for ( i = 0; i < 4; i++ ) {
        func_80056150( &D_8009D098.unk_08[i], 0, sizeof( UnkRecord ) );
        D_8009D098.unk_08[i].unk_12 = 300;
        func_80056150( D_8009D098.unk_08[i].unk_00, 0xFF, sizeof( D_8009D098.unk_08[i].unk_00 ) );
        func_80056150( &D_800E44E8[i], 0, sizeof( UnkRecord ) );
        D_800E44E8[i].unk_12 = 300;
        func_80056150( D_800E44E8[i].unk_00, 0xFF, sizeof( D_800E44E8[i].unk_00 ) );
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
#ifdef NON_MATCHING
// NON_MATCHING: register/schedule; retail sets up a2 = 0 and a3 = the id around the func_8003BD20 call (as if
// it took more arguments), and schedules the ucode store before ucode_size
void func_80025EBC( void ) {
    D_80060EA0.t.type = M_AUDTASK;
    D_80060EA0.t.flags = 0;
    D_80060EA0.t.ucode_boot = func_80056D00;
    D_80060EA0.t.ucode_boot_size = (u8 *)D_80056DD0 - (u8 *)func_80056D00;
    D_80060EA0.t.ucode = D_80066B50->ucode;
    D_80060EA0.t.ucode_size = 0x1000;
    D_80060EA0.t.ucode_data = D_80066B50->ucodeData;
    D_80060EA0.t.ucode_data_size = 0x800;
    D_8005B110 = 2;
    D_80060EA0.t.data_ptr = D_80066B50->dataPtr;
    D_8005B100 = 1;
    D_80060EA0.t.data_size = D_80066B50->dataSize;
    osSpTaskLoad( &D_80060EA0 );
    osSpTaskStartGo( &D_80060EA0 );
    if ( D_8005C9E4 != 0 ) {
        if ( --D_8005C9E0 == 0 ) {
            D_8005C9DC = func_8003B6B0( D_800A3048 );
            D_8005C9D8 = 0;
            func_8003BD20( D_8005C9DC, D_8005C9E4 );
            D_8005C9E4 = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/1060", func_80025EBC);
#endif

INCLUDE_ASM("asm/nonmatchings/1060", func_80025FEC);

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
