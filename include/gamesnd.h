#ifndef GAMESND_H
#define GAMESND_H

/* gamesnd.h -- the game's use of the sound driver: SE, BGM files and the audio task (unit 12C70) */

#include "common.h"
#include "libultra.h"
#include "structs.h"
#include "boot.h"

#define BGM_BUFFER_SIZE     0x4650      /* bytes of each BGM sequence buffer */
#define BGM_START_DELAY     10          /* frames from loading a BGM to starting it */
#define BGM_STOPPED         0x100       /* D_8005C9E8: the BGM is not playing */

/* func_80037870 flags */
#define SE_PAN_MASK         0x03        /* 0: centre, 1: left, 2: right, 3: random pan and volume */
#define SE_NO_REUSE         0x80

#define SCREENSHOT_WIDTH    320
#define SCREENSHOT_HEIGHT   240
#define SCREENSHOT_FILE_SIZE 0x38438   /* bytes written, as the header's bfSize: 2 more than the header and pixels */

/* The 54 bytes in front of a screenshot's pixels: a BMP file and info header, 24-bit, bottom-up. */
typedef struct {
    /* 0x00 */ u8 data[0x36];
} bmpHeader_t;

/* A screenshot as func_80037E30 writes it to the host. */
typedef struct {
    /* 0x00000 */ bmpHeader_t header;
    /* 0x00036 */ u8 pixels[SCREENSHOT_HEIGHT][SCREENSHOT_WIDTH][3];  /* blue, green, red */
} bmpFile_t;

extern s32 D_8005C9D0;              /* the D_800EA440 buffer of the last BGM loaded */
extern u16 D_8005C9E8;              /* index of the current BGM, | BGM_STOPPED */
extern u16 D_8005C9EC[];            /* BGM volume of each D_8009D098.unk_7A setting */
extern u8 D_8005C9F8[];             /* nonzero: the BGM is skipped while D_8009D098.unk_79 is 0 */
extern s16 D_8005CA0C[];            /* str file of each BGM */
extern u8 D_800EA440[2][BGM_BUFFER_SIZE];
extern bmpHeader_t D_8005CA40;
extern s16 D_8005CA76;              /* number of the next screenshot */
extern OSMesgQueue D_800E6520;
extern OSMesgQueue D_80099428;      /* takes the audio task requests */
extern OSMesgQueue D_8009D040;      /* where the audio task reports completion */
extern u8 D_800DF6A4[];             /* the audio task request message */
extern u8 D_800ABD70[];             /* audio heap */
extern OSTime D_80099450;           /* when the last audio task started */
extern s32 D_8005B0F0;              /* microseconds the last audio task took */

/* sound driver functions (unit 166B0) this unit calls that functions.h does not declare */
void Snd_Init( sndConfig_t *config );
u32 Snd_PlaySeEx( s32 index, s32 volume, s32 pan, s32 reuse, s32 priority );
s32 Snd_Stop( s32 id, s32 ticks );
void Snd_SetSeWaveBank( seTable_t *table, waveBank_t *bank );

u64 func_80037870( s32 index, s32 flags );
void func_800379A8( u64 id );
void func_800379C8( s32 index );
void func_80037B18( void );
void func_80037B84( void );
void func_80037BD8( void );
void func_80037BE0( void );
void func_80037C08( UnkRspProgram *program );
void func_80037CD0( void );
void func_80037E30( void );

#endif
