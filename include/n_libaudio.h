#ifndef N_LIBAUDIO_H
#define N_LIBAUDIO_H

/*
n_libaudio.h -- the n_audio synthesizer calls the game makes, reconstructed from the ROM. ultralib ships no
n_audio; this header holds only what the game's own code uses. The libaudio types (ALLink, ALPlayer,
ALVoiceConfig, ALWaveTable, ALMicroTime) come from ultralib's PR/libaudio.h through libultra.h.
n_alSyn* take no ALSynth argument: n_audio has a single synthesizer.
*/

#include "libultra.h"

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
