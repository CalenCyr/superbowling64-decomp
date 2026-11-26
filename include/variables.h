#ifndef VARIABLES_H
#define VARIABLES_H

#include "common.h"
#include "structs.h"

extern u8 *(*snd_commands[])( sndChannel_t *chan, u8 *src );  /* command handlers, by command - CMD_FIRST */
extern OSPiHandle *audio_samplePiHandle;  /* device of tagged sample addresses (Snd_Init sets it) */
extern s32 audio_sampleFlags;       /* SAMPLES_IN_RAM etc. (Snd_Init sets it) */

/* audio heap and frame sizes (1B060.c) */
extern s32 snd_frameSettled;        /* 0 after a short or long frame, until the queue is back in range */

/* audio manager: synthesizer and effect setup (19F50.c) */
extern N_ALSynth *synth_driver;     /* the synthesizer, as Snd_InitSynth set it up */
extern fxStorage_t synth_fxStorage;  /* the effect's delay, buffer and filter storage */

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
extern audioTaskFuncs_t *audio_taskFuncs;  /* the routines the audio thread uses, &audio_defaultTaskFuncs by default */

/* audio manager: audio thread (1ADA0.c) */
extern N_ALGlobals audio_globals;   /* the n_audio globals */

#endif
