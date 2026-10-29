#ifndef VARIABLES_H
#define VARIABLES_H

#include "common.h"
#include "structs.h"

extern u8 *(*snd_commands[])( sndChannel_t *chan, u8 *src );  /* command handlers, by command - CMD_FIRST */
extern OSPiHandle *D_800693D8;
extern s32 D_800DF6B0;
extern ALPlayer snd_player;
extern u16 snd_seVolume;            /* volume scale of SE channels, 0x7FFF: 1 */
extern u16 snd_bgmVolume;           /* volume scale of BGM channels */
extern s32 snd_numChannels;         /* entries in snd_channels */
extern sndChannel_t *snd_channels;
extern sndChannel_t *snd_voiceChannels;  /* &snd_channels[NUM_MASTER_CHANNELS] */
extern N_ALVoice *snd_voices;        /* one per channel from snd_voiceChannels on */
extern s32 snd_tickRate;            /* ticks per second */
extern s32 snd_tickTime;            /* microseconds per tick */
extern s32 snd_nextId;              /* id of the next started sound */
extern s32 snd_randomSeed;          /* Snd_Random shift register */
extern waveBank_t *snd_nextWaveBank;     /* wave bank of the next started channel, then NULL */
extern waveBank_t *snd_defaultWaveBank;  /* wave bank while snd_nextWaveBank is NULL */
extern s32 snd_seqFxEnabled;        /* 1: Snd_CmdFxType may change the effect */
extern s32 snd_fxType;              /* effect setting last given to Snd_SetFxType */
extern seTable_t *snd_seTable;      /* default SE table */
extern seTable_t *snd_nextSeTable;  /* used once, by the next Snd_PlaySe, instead of snd_seTable */
extern void (*snd_markerCallback)( s32 id, s32 value );   /* called by Snd_CmdMarker */
/* ring buffer of snd_numMsgs messages: Snd_PostMessage writes at snd_msgWrite, Snd_PlayerHandler reads at snd_msgRead */
extern s32 snd_msgRead;
extern s32 snd_msgWrite;
extern s32 snd_numMsgs;
extern sndMsg_t *snd_msgs;

/* audio heap and frame sizes (1B060.c) */
extern ALHeap snd_heap;
extern u32 snd_frameSize;           /* samples per audio frame, a multiple of AUDIO_FRAME_SAMPLES */
extern u32 snd_minFrameSize;        /* snd_frameSize - AUDIO_FRAME_SAMPLES */
extern u32 snd_maxFrameSize;        /* snd_frameSize + AUDIO_FRAME_SAMPLES */
extern u32 snd_extraSamples;        /* queued samples kept ahead of the AI */
extern s32 snd_frameSettled;        /* 0 after a short or long frame, until the queue is back in range */

/* audio manager: synthesizer and effect setup (19F50.c) */
extern N_ALSynth *D_800EA098;      /* the synthesizer, as Snd_InitSynth set it up */
extern fxStorage_t D_800EA0A8;
extern s32 D_8005CC98[];            /* effect preset: the default */
extern s32 D_8005CE20;              /* presets in D_8005CE24 */
extern s32 *D_8005CE24[];           /* effect presets, NULL-terminated */

/* audio manager: ROM sample cache and scheduler client (1A7A0.c) */
extern dmaBuffer_t *D_80064390;     /* buffers in use, by startAddr */
extern dmaBuffer_t *D_80064394;     /* free buffers */
extern dmaBuffer_t *D_80064398;     /* every buffer */
extern OSIoMesg *D_8006439C;        /* DMA requests, two per buffer */
extern OSMesg *D_800643A0;          /* D_800643B0's messages */
extern s32 D_800643A4;              /* bytes per buffer */
extern s32 D_800643A8;              /* DMAs started this frame */
extern OSMesgQueue D_800643B0;      /* DMA completions */
extern OSPiHandle *D_800643C8;      /* the cartridge ROM */
extern OSSched *D_800643D0;
extern audioClient_t *D_800643D4;
extern audioTaskFuncs_t D_8005CE40; /* func_8003F7FC, func_8003F860, func_8003F8C0 */
extern audioTaskFuncs_t *D_8005CE4C;    /* the routines the audio thread uses, &D_8005CE40 by default */

#endif
