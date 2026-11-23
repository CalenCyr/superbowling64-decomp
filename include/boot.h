#ifndef BOOT_H
#define BOOT_H

/* boot.h -- boot, threads and the cartridge PI handle (units 1060 and 2DF0) */

#include "common.h"
#include "libultra.h"
#include "structs.h"
#include "functions.h"

#define BOOT_STACK_SIZE     0x200       /* bytes of stack of each boot thread */
#define RAM_END             0x80400000  /* end of the 4 MB of RDRAM, in KSEG0 */
#define SCREEN_WIDTH        320         /* pixels of the framebuffers of D_80066C38 */
#define SCREEN_HEIGHT       240

/* Record of 0x1C bytes kept four at a time in D_8009D098 and D_800E44E8; its purpose is unknown. */
typedef struct {
    /* 0x00 */ u16 name[6];                 /* a player's name, characters ended by 0xFFFF; 0xFF bytes at boot */
    /* 0x0C */ u8 games;                    /* games played, to 255 */
    /* 0x0E */ u16 total;                   /* sum of their scores, to 0xFFFF */
    /* 0x10 */ s16 high;                    /* highest score */
    /* 0x12 */ s16 low;                     /* lowest score; 300 at boot */
    /* 0x14 */ u16 unk_14;                  /* counts to 9999: of 12 a game */
    /* 0x16 */ u16 unk_16;                  /* of 10 a game */
    /* 0x18 */ u16 unk_18;                  /* of 10 a game */
    /* 0x1A */ u8 unk_1A[0x02];
} UnkRecord;                                /* size 0x1C */

typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ u32 unk_04;                  /* bit per character of gscode's character menu: unlocked */
    /* 0x08 */ UnkRecord unk_08[4];
    /* 0x78 */ u8 unk_78;
    /* 0x79 */ u8 unk_79;
    /* 0x7A */ u8 unk_7A;
    /* 0x7B */ u8 unk_7B;
    /* 0x7C */ u8 unk_7C[0x100 - 0x7C];
} UnkRecordSet;                             /* size 0x100; func_800373DC sums bytes 0x04-0xFF */

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
extern u16 *D_80066C38[3];          /* RGBA5551 320x240 framebuffers: 0x8038F800, 0x803B5000, 0x803DA800 at boot */
extern s32 D_8005B0F8;              /* index in D_80066C38 of the framebuffer being drawn */
extern s32 D_8005B0E4;              /* RSP time of the last frame (mgcode's meter shows it as RSP) */
extern s32 D_8005B0E8;              /* RDP time of the last frame (shown as RDP) */
extern s32 D_8005B0EC;              /* CPU time of the last frame (shown as CPU) */
extern s32 D_8005B0FC;              /* frame rate (shown as FRATE) */
/* 0 or 1: which of the double-buffered display lists and matrices is built */
// volatile: units 1060, 7250 and C650 read it with lbu and a separate sign extension, once per use (53B0 alone uses lb)
extern volatile s8 D_8005C0B1;
extern Gfx *D_800F8094;             /* next command of the display list being built */
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
extern s32 D_8005B100;
extern s32 D_8005B110;
extern u64 D_8005C9D8;              /* sound id of the playing BGM, zero-extended; 0: none */
extern s32 D_8005C9E0;              /* frames until the sequence D_8005C9E4 starts */
extern s32 D_8005C9E4;
extern seqHeader_t *D_800A3048;

/* The main thread (func_80025FEC) schedules the RSP and RDP; these are its messages, queues and state. */
#define SCHED_MSG_RETRACE   0           /* VI retrace (D_80069408) */
#define SCHED_MSG_SP_DONE   1           /* RSP task done (D_800E4558) */
#define SCHED_MSG_DP_DONE   2           /* RDP done (D_80066B90) */
#define SCHED_MSG_PRENMI    3           /* reset button pressed (D_8009D058) */
#define SCHED_MSG_GFX       4           /* the game thread finished a display list (D_800A2FD4) */
#define SCHED_MSG_AUDIO     5           /* the audio thread asks for its task (D_800DF6A4) */

/* each message is a pointer to its code */
extern s32 D_80069408;
extern s32 D_800E4558;
extern s32 D_80066B90;
extern s32 D_8009D058;
extern s32 D_800A2FD4;

extern OSMesg D_800E675C[1];        /* buffer of D_800DF578 */
extern OSMesg D_801000C0[8];        /* buffer of D_80099428 */
extern OSMesgQueue D_800E57E8;      /* gets the retrace message each VI */
extern OSMesg D_8009D060[1];
extern OSMesg D_8009CF8C[1];        /* buffer of D_800E6520 */
extern OSMesgQueue D_800E8078;
extern OSMesg D_8009CF98[1];
extern OSMesg D_800DF6A0[1];        /* buffer of D_8009D040 */
extern OSMesg D_80066C2C[1];        /* buffer of D_800E6690 */
extern OSMesg D_8009D02C[1];        /* buffer of D_800F8098 */
extern OSMesgQueue D_800DF688;      /* tells the game thread which framebuffer to draw next */
extern OSMesg D_8009D068[1];

#define CONT_STACK_SIZE     0x800       /* bytes of stack of the controller thread */
#define GAME_STACK_SIZE     0x2000      /* bytes of stack of the game thread */
extern OSThread D_800617F0;         /* controller thread, func_80028064 */
extern u64 D_80068250[CONT_STACK_SIZE / sizeof( u64 )];
extern OSThread D_80061640;         /* game thread, func_80026B44 */
extern u64 D_800E8090[GAME_STACK_SIZE / sizeof( u64 )];

/* A display list the game thread builds, two of them used in turn. */
#define GFX_LIST_LEN        0x3000
typedef struct {
    /* 0x00000 */ Gfx list[GFX_LIST_LEN];
    /* 0x18000 */ u32 size;             /* bytes of list, the task's data_size */
    /* 0x18004 */ s32 fbIndex;          /* index in D_80066C38 of the framebuffer it draws */
} GfxFrame;                             /* size 0x18008 */

extern GfxFrame D_80069410[2];
extern OSTask D_8005B140;           /* the graphics task */
extern u64 D_8005F3F0[];            /* the graphics microcode's data */
extern s32 D_8005B120;              /* index in D_80069410 of the list the RSP runs */
extern s32 D_8005B124[2];           /* state of each list of D_80069410: 0 building, 1 built, 2 running, 3 done */
extern s32 D_8005B114[3];           /* state of each framebuffer of D_80066C38: 1 drawn, 4 shown */
extern s8 D_8005C0B2;               /* framebuffer to show next */
extern s8 D_8005C0B3;               /* framebuffer shown */
extern u8 D_8005C0B4[2];            /* [0]: reset pressed; [1]: tasks stopped after it */
extern s32 D_8005B104;              /* the graphics task was asked to yield */
extern s32 D_8005B108;              /* the game thread is building a list */
extern s32 D_8005B10C;              /* graphics task state: 0 idle, 1 running, 2 yielded */
extern s32 D_8005B12C;              /* retraces since the last swap */
extern s32 D_8005B130;              /* swaps this second */
extern OSTime D_80066C48;           /* when the graphics task started */
extern OSTime D_80099440;           /* when the game thread was told to draw */

void func_80026B44( void *arg );

extern u8 D_8009CFB0;
extern u8 D_800619A0;

void func_80025E10( void *arg );
void func_80025EBC( void );
void func_80025FEC( void *arg );
void func_80026B10( void );
void func_80026B28( void );


#endif
