#ifndef VARIABLES_H
#define VARIABLES_H

#include "common.h"
#include "structs.h"

extern u8 *(*snd_commands[])( sndChannel_t *chan, u8 *src );  /* command handlers, by command - CMD_FIRST */
extern OSPiHandle *audio_samplePiHandle;  /* device of tagged sample addresses (Snd_Init sets it) */
extern s32 audio_sampleFlags;       /* SAMPLES_IN_RAM etc. (Snd_Init sets it) */
extern ALPlayer snd_player;
extern u16 snd_seVolume;            /* volume scale of SE channels, 0x7FFF: 1 */
extern u16 snd_bgmVolume;           /* volume scale of BGM channels */
extern s32 snd_numChannels;         /* entries in snd_channels */
extern sndChannel_t *snd_channels;
extern sndChannel_t *snd_voiceChannels;  /* &snd_channels[NUM_MASTER_CHANNELS] */
extern N_ALVoice *snd_voices;       /* one per channel from snd_voiceChannels on */
extern s32 snd_tickRate;            /* ticks per second */
extern s32 snd_tickTime;            /* microseconds per tick */
extern s32 snd_nextId;              /* id of the next started sound */
extern s32 snd_randomSeed;          /* Snd_Random shift register */
extern waveBank_t *snd_nextWaveBank;  /* wave bank of the next started channel, then NULL */
extern waveBank_t *snd_defaultWaveBank;  /* wave bank while snd_nextWaveBank is NULL */
extern s32 snd_seqFxEnabled;        /* 1: Snd_CmdFxType may change the effect */
extern s32 snd_fxType;              /* effect setting last given to Snd_SetFxType */
extern seTable_t *snd_seTable;      /* default SE table */
extern seTable_t *snd_nextSeTable;  /* used once, by the next Snd_PlaySe, instead of snd_seTable */
extern void (*snd_markerCallback)( s32 id, s32 value );  /* called by Snd_CmdMarker */
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
extern N_ALSynth *synth_driver;     /* the synthesizer, as Snd_InitSynth set it up */
extern fxStorage_t synth_fxStorage;  /* the effect's delay, buffer and filter storage */
extern s32 synth_defaultFx[];       /* effect preset: the default */
extern s32 synth_numFxPresets;      /* presets in synth_fxPresets */
extern s32 *synth_fxPresets[];      /* effect presets, NULL-terminated */

/* audio manager: ROM sample cache and scheduler client (1A7A0.c) */
extern dmaBuffer_t *audio_dmaUsed;  /* buffers in use, by startAddr */
extern dmaBuffer_t *audio_dmaFree;  /* free buffers */
extern dmaBuffer_t *audio_dmaBuffers;  /* every buffer */
extern OSIoMesg *audio_dmaRequests;  /* DMA requests, two per buffer */
extern OSMesg *audio_dmaMsgs;       /* audio_dmaQueue's messages */
extern s32 audio_dmaBufferSize;     /* bytes per buffer */
extern s32 audio_dmaCount;          /* DMAs started this frame */
extern OSMesgQueue audio_dmaQueue;  /* DMA completions */
extern OSPiHandle *audio_romHandle;  /* the cartridge ROM */
extern OSSched *audio_sched;        /* the scheduler, as Audio_SetScheduler set it */
extern audioClient_t *audio_client;  /* the audio thread's scheduler client */
extern audioTaskFuncs_t audio_defaultTaskFuncs;  /* Audio_ClientInit, Audio_ClientWaitFrame, Audio_ClientRunTask */
extern audioTaskFuncs_t *audio_taskFuncs;  /* the routines the audio thread uses, &audio_defaultTaskFuncs by default */

/* audio manager: audio thread (1ADA0.c) */
extern N_ALGlobals audio_globals;   /* the n_audio globals */
extern OSThread audio_thread;       /* the audio thread */
extern u8 *audio_stack;             /* its stack */
extern audioBuffer_t *audio_buffers;  /* NUM_AUDIO_BUFFERS output buffers */
extern Acmd *audio_cmdList;         /* the frame's command list */
extern audioBuffer_t *audio_lastBuffer;  /* the buffer the last task filled, for the AI next; NULL: none yet */

#endif
