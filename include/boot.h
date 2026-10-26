#ifndef BOOT_H
#define BOOT_H

/* boot.h -- boot, threads and the cartridge PI handle (units 1060 and 2DF0) */

#include "common.h"
#include "libultra.h"
#include "structs.h"

#define BOOT_STACK_SIZE     0x200       /* bytes of stack of each boot thread */
#define RAM_END             0x80400000  /* end of the 4 MB of RDRAM, in KSEG0 */

/* Record of 0x1C bytes kept four at a time in D_8009D098 and D_800E44E8; its purpose is unknown. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x0C];             /* set to 0xFF at boot */
    /* 0x0C */ u8 unk_0C[0x06];
    /* 0x12 */ s16 unk_12;                  /* 300 at boot */
    /* 0x14 */ u8 unk_14[0x08];
} UnkRecord;                                /* size 0x1C */

typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ UnkRecord unk_08[4];
    /* 0x78 */ u8 unk_78;
    /* 0x79 */ u8 unk_79;
    /* 0x7A */ u8 unk_7A;
    /* 0x7B */ u8 unk_7B;
} UnkRecordSet;

/* What func_80025EBC runs on the RSP; set through D_80066B50. Names invented. */
typedef struct {
    /* 0x00 */ u64 *dataPtr;
    /* 0x04 */ u32 dataSize;
    /* 0x08 */ u64 *ucode;
    /* 0x0C */ u64 *ucodeData;
} UnkRspProgram;

/*
libultra's raw PI register access, which the SDK headers of 2.0K declare only in the library's internal
piint.h; prototypes as there.
*/
s32 __osEPiRawReadIo( OSPiHandle *pihandle, u32 devAddr, u32 *data );
s32 __osEPiRawWriteIo( OSPiHandle *pihandle, u32 devAddr, u32 data );

extern OSPiHandle *D_800ABD60;      /* cartridge ROM handle */
extern OSIoMesg D_80066B70;         /* the request of each ROM DMA */
extern OSMesgQueue D_800DF578;      /* where ROM DMAs report completion */

extern u8 D_8010A250[];             /* start of the memory main clears at boot (the KNSTATICSYS file) */
extern u8 D_800A2FE4;
extern u32 D_80066C38;              /* 0x8038F800 at boot */
extern u32 D_80066C3C;              /* 0x803B5000 at boot */
extern u32 D_80066C40;              /* 0x803DA800 at boot */
extern UnkRecordSet D_8009D098;
extern UnkRecordSet *D_801000B8;
extern UnkRecord D_800E44E8[4];

extern OSThread D_80060EE0;         /* boot thread: starts the managers, then idles */
extern u64 D_80061090[BOOT_STACK_SIZE / sizeof( u64 )];
extern OSThread D_80061290;         /* main thread, func_80025FEC */
extern u64 D_80061440[BOOT_STACK_SIZE / sizeof( u64 )];
extern OSMesgQueue D_801000E0;      /* PI manager commands */
extern OSMesg D_800693B8[8];

extern OSTask D_80060EA0;
extern UnkRspProgram *D_80066B50;
extern u64 func_80056D00[];         /* rspboot: its text, up to D_80056DD0 */
extern u64 D_80056DD0[];
extern s32 D_8005B100;
extern s32 D_8005B110;
extern s32 D_8005C9D8;
extern s32 D_8005C9DC;
extern s32 D_8005C9E0;              /* frames until the sequence D_8005C9E4 starts */
extern s32 D_8005C9E4;
extern UnkSeqHeader *D_800A3048;

extern u8 D_8009CFB0;
extern u8 D_800619A0;

void func_80025E10( void *arg );
void func_80025EBC( void );
void func_80025FEC( void *arg );
void func_80026B10( void );
void func_80026B28( void );

/* unit 166B0 */
s32 func_8003B6B0( UnkSeqHeader *hdr );
s32 func_8003BD20( s32 id, s32 value );

#endif
