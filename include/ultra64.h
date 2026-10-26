#ifndef ULTRA64_H
#define ULTRA64_H

/* The libultra declarations this project's C units use, as the SDK declares them. */

#include "common.h"

#define OS_TV_PAL   0
#define OS_TV_NTSC  1
#define OS_TV_MPAL  2

extern s32 osTvType;

void osWritebackDCacheAll( void );
float __sinf( float x );

/*
 * Audio: libaudio types and the n_audio synthesizer calls (n_alSyn* take no ALSynth: n_audio's single
 * synthesizer is the global n_syn). Members this project's code does not touch keep the SDK's names.
 */
typedef s32 ALMicroTime;

typedef struct ALLink_s {
    struct ALLink_s *next;
    struct ALLink_s *prev;
} ALLink;

/* client of the synthesizer, called back every callTime microseconds */
typedef struct ALPlayer_s {
    /* 0x00 */ struct ALPlayer_s *next;
    /* 0x04 */ void *clientData;
    /* 0x08 */ ALMicroTime (*handler)( void *clientData );   /* returns the time to its next call */
    /* 0x0C */ ALMicroTime callTime;
    /* 0x10 */ s32 samplesLeft;
} ALPlayer;

typedef struct {
    /* 0x00 */ s16 priority;
    /* 0x02 */ s16 fxBus;
    /* 0x04 */ u8 unityPitch;
} ALVoiceConfig;

/*
 * Wave table (the SDK's ALWaveTable: base, len, type, flags, then the loop and book of the wave info). The
 * addresses are declared as integers: the game's banks keep them as offsets until Snd_RelocateWaveBank.
 * Snd_RelocateWaveBank patches it once, as the SDK's alBnkfNew patches its wave tables.
 */
typedef struct {
    /* 0x00 */ u32 base;            /* ROM offset of the samples, relocated unless the top byte is 0xFF */
    /* 0x04 */ u8 unk_04[0x08 - 0x04];
    /* 0x08 */ u8 type;             /* 0: ADPCM (has a book) */
    /* 0x09 */ u8 flags;            /* set once relocated */
    /* 0x0A */ u8 unk_0A[0x0C - 0x0A];
    /* 0x0C */ s32 loop;            /* offset in the bank, 0: none */
    /* 0x10 */ s32 book;            /* offset in the bank, ADPCM only */
} ALWaveTable;

/* n_audio voice: owned by the synthesizer once n_alSynAllocVoice has set it up */
typedef struct {
    /* 0x00 */ ALLink node;
    /* 0x08 */ void *pvoice;
    /* 0x0C */ ALWaveTable *table;
    /* 0x10 */ void *clientPrivate;
    /* 0x14 */ s16 state;
    /* 0x16 */ s16 priority;
    /* 0x18 */ s16 fxBus;
    /* 0x1A */ s16 unityPitch;
} N_ALVoice;

void n_alSynAddPlayer( ALPlayer *client );
void n_alSynAllocVoice( N_ALVoice *voice, ALVoiceConfig *vc );
void n_alSynSetFXMix( N_ALVoice *voice, u8 fxmix );
void n_alSynSetPan( N_ALVoice *voice, s32 pan );
void n_alSynSetPitch( N_ALVoice *voice, f32 pitch );
void n_alSynSetVol( N_ALVoice *voice, s32 volume, ALMicroTime t );
void n_alSynStartVoice( N_ALVoice *voice, ALWaveTable *table );
void n_alSynStopVoice( N_ALVoice *voice );

#endif
