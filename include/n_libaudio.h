#ifndef N_LIBAUDIO_H
#define N_LIBAUDIO_H

/*
n_libaudio.h -- the n_audio synthesizer calls the game makes, reconstructed from the ROM. ultralib ships no
n_audio; this header holds only what the game's own code uses. The libaudio types (ALLink, ALPlayer,
ALVoiceConfig, ALWaveTable, ALMicroTime) come from ultralib's PR/libaudio.h through libultra.h.
n_alSyn* take no ALSynth argument: n_audio has a single synthesizer.
*/

#include "libultra.h"

struct N_PVoice_s;

/* n_audio voice: owned by the synthesizer once n_alSynAllocVoice has set it up */
typedef struct {
    /* 0x00 */ ALLink node;
    /* 0x08 */ struct N_PVoice_s *pvoice;  /* the physical voice it plays on, NULL: none */
    /* 0x0C */ ALWaveTable *table;
    /* 0x10 */ void *clientPrivate;
    /* 0x14 */ s16 state;
    /* 0x16 */ s16 priority;
    /* 0x18 */ s16 fxBus;
    /* 0x1A */ s16 unityPitch;
} N_ALVoice;

/* the physical voice: on the synthesizer's free list until a voice is allocated; 0x8C bytes */
typedef struct N_PVoice_s {
    /* 0x00 */ ALLink node;
    /* 0x08 */ N_ALVoice *vvoice;       /* the voice using it, NULL while free */
    /* 0x0C */ u8 unk_0C[0x88 - 0x0C];
    /* 0x88 */ s32 offset;              /* samples its updates wait: N_PVOICE_STEAL_DELAY while a stolen voice fades */
} N_PVoice;

#define N_PVOICE_STEAL_DELAY    552     /* N_PVoice.offset of a stolen voice: three frames */
#define N_PVOICE_STEAL_FADE     368     /* samples a stolen voice's volume ramps to 0 over: two frames */

/* parameter update queued for a later frame (libaudio ALParam shape); 0x1C bytes */
typedef struct ALParam_s {
    /* 0x00 */ struct ALParam_s *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;                /* N_PARAM_* */
    /* 0x0A */ s16 unityPitch;          /* N_PARAM_START_VOICE: the voice's unityPitch */
    /* 0x0C */ union {
                   s32 i;
                   f32 f;               /* N_PARAM_SET_PITCH */
                   void *p;             /* N_PARAM_START_VOICE */
               } data;
    /* 0x10 */ s32 moredata;
    /* 0x14 */ s32 stillmoredata;
    /* 0x18 */ s32 yetstillmoredata;
} ALParam;

/* ALParam.type values the n_alSyn* calls queue */
#define N_PARAM_SET_PITCH       7       /* data: the pitch ratio, as f32 */
#define N_PARAM_SET_VOLUME      11      /* data: the volume, moredata: the ramp time in samples */
#define N_PARAM_SET_PAN         12      /* data: the pan */
#define N_PARAM_START_VOICE     14      /* data: the wave table */
#define N_PARAM_STOP_VOICE      15
#define N_PARAM_SET_FXMIX       16      /* data: the effect mix, 0-127 */

/* pulls a bus's commands for one frame: appends them at cmdList and returns the new end */
typedef Acmd *(*N_ALPullHandler)( s32 sampleOffset, Acmd *cmdList );

/*
The effect (libaudio's reverb.o types, which the n_audio effect shares): delay lines over one sample buffer,
each with an optional modulating resampler and low-pass filter. Only the members the game sets are typed.
*/
typedef struct {
    /* 0x00 */ s16 fc;                  /* cutoff */
    /* 0x02 */ s16 fgain;
    /* 0x08 */ union {
                   s16 fccoef[16];      /* filter coefficients, built by _init_lpfilter */
                   s64 force_aligned;
               } fcvec;
    /* 0x28 */ POLEF_STATE *fstate;
    /* 0x2C */ s32 first;
} ALLowPass;

typedef struct {
    /* 0x00 */ u8 filter[0x14];         /* libaudio filter header, set up by the library */
    /* 0x14 */ RESAMPLE_STATE *state;
    /* 0x18 */ f32 ratio;
    /* 0x1C */ s32 upitch;
    /* 0x20 */ f32 delta;
    /* 0x24 */ s32 first;
    /* 0x28 */ ALParam *ctrlList;
    /* 0x2C */ ALParam *ctrlTail;
    /* 0x30 */ s32 motion;
} ALResampler;

typedef struct {
    /* 0x00 */ u32 input;               /* delay line start and end, in samples into the buffer */
    /* 0x04 */ u32 output;
    /* 0x08 */ s16 ffcoef;
    /* 0x0A */ s16 fbcoef;
    /* 0x0C */ s16 gain;
    /* 0x10 */ f32 rsinc;               /* modulation rate */
    /* 0x14 */ f32 rsval;
    /* 0x18 */ s32 rsdelta;
    /* 0x1C */ f32 rsgain;              /* modulation depth */
    /* 0x20 */ ALLowPass *lp;           /* NULL: no low-pass */
    /* 0x24 */ ALResampler *rs;         /* NULL: no modulation */
} ALDelay;

typedef s32 (*ALSetFXParam)( void *fx, s32 paramID, void *param );

typedef struct {
    /* 0x00 */ u8 filter[0x14];         /* libaudio filter header, set up by the library */
    /* 0x14 */ s16 *base;               /* sample buffer, length samples */
    /* 0x18 */ s16 *input;
    /* 0x1C */ u32 length;
    /* 0x20 */ ALDelay *delay;          /* section_count delay lines */
    /* 0x24 */ u8 section_count;
    /* 0x28 */ ALSetFXParam paramHdl;
} ALFx;

/* filter message ids, libaudio's first two */
#define AL_FILTER_FREE_VOICE    0
#define AL_FILTER_SET_SOURCE    1
#define AL_FILTER_ADD_UPDATE    3       /* param: an ALParam to append to the physical voice's queue */

s32 alFxParam( void *filter, s32 paramID, void *param );
s32 alFxParamHdl( void *filter, s32 paramID, void *param );
void _init_lpfilter( ALLowPass *lp );

/* the main bus: mixes the effect bus into the output; 0x14 bytes */
typedef struct {
    /* 0x00 */ void *source;
    /* 0x04 */ N_ALPullHandler handler;
    /* 0x08 */ u8 unk_08[0x14 - 0x08];
} N_ALMainBus;

#define N_AL_MAX_FX 8                   /* N_ALAuxBus.fxSlots */

/* the effect bus: sums the physical voices, then runs the effect; 0x44 bytes */
typedef struct {
    /* 0x00 */ u8 unk_00[0x14];
    /* 0x14 */ s32 sourceCount;
    /* 0x18 */ s32 maxSources;
    /* 0x1C */ N_PVoice **sources;      /* maxSources entries, sourceCount used */
    /* 0x20 */ ALFx *fx;                /* the effect it runs, NULL: none */
    /* 0x24 */ ALFx *fxSlots[N_AL_MAX_FX];
} N_ALAuxBus;

/* the n_audio synthesizer (libaudio ALSynth shape, without an output filter); 0x50 bytes */
typedef struct {
    /* 0x00 */ ALPlayer *head;          /* client list */
    /* 0x04 */ ALLink pFreeList;        /* free physical voices */
    /* 0x0C */ ALLink pAllocList;       /* allocated physical voices */
    /* 0x14 */ ALLink pLameList;        /* physical voices to free */
    /* 0x1C */ s32 paramSamples;
    /* 0x20 */ s32 curSamples;          /* samples since the start */
    /* 0x24 */ ALDMANew dma;
    /* 0x28 */ ALHeap *heap;
    /* 0x2C */ ALParam *paramList;      /* free parameter updates */
    /* 0x30 */ N_ALMainBus *mainBus;
    /* 0x34 */ N_ALAuxBus *auxBus;
    /* 0x38 */ s32 numPVoices;
    /* 0x3C */ s32 unk_3C;
    /* 0x40 */ s32 outputRate;          /* output sample rate */
    /* 0x44 */ s32 maxOutSamples;       /* the most samples the RSP makes at once */
    /* 0x48 */ s16 *outBuf;          /* where the frame being made is saved: set per pass by func_80042A00 */
    /* 0x4C */ s32 unk_4C;
} N_ALSynth;

typedef struct {
    /* 0x00 */ N_ALSynth drvr;
} N_ALGlobals;

#define AL_MAX_RSP_SAMPLES 184          /* N_ALSynth.maxOutSamples */

extern N_ALGlobals *D_8005D110;     /* the n_audio globals, set once */
extern N_ALSynth *D_8005D114;       /* &D_8005D110->drvr: the synthesizer every n_alSyn* call uses */

Acmd *func_8003FED0( s32 sampleOffset, Acmd *cmdList );    /* main bus pull without an effect */
void func_8004035C( N_PVoice *pv, ALDMANew dma, ALHeap *hp );  /* sets up a physical voice's filters */
Acmd *func_80041760( s32 sampleOffset, Acmd *cmdList );    /* main bus pull through the effect */
Acmd *func_80042A00( Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen );   /* synthesizes one frame */

s32 func_80040A08( N_PVoice *pv, s32 paramID, void *param );   /* a physical voice's parameter handler */
ALParam *func_80042B68( void );        /* takes an update off the free list, NULL: none left */
s32 func_80042C94( ALMicroTime t );    /* t in output samples, rounded down to a multiple of 16 */

void n_alSynAddPlayer( ALPlayer *client );
void Synth_Destroy( void );
s32 Synth_AllocPVoice( N_PVoice **pvoice, s16 priority );
s32 n_alSynAllocVoice( N_ALVoice *voice, ALVoiceConfig *vc );
void n_alSynSetFXMix( N_ALVoice *voice, u8 fxmix );
void n_alSynSetPan( N_ALVoice *voice, u8 pan );
void n_alSynSetPitch( N_ALVoice *voice, f32 pitch );
void n_alSynSetVol( N_ALVoice *voice, s16 volume, ALMicroTime t );
void n_alSynStartVoice( N_ALVoice *voice, ALWaveTable *table );
void n_alSynStopVoice( N_ALVoice *voice );

/* ===== synthesizer frame and setup (units 1D550, 1DB90, 1E0F0, 1E140) ===== */

/* DMEM buffers of one frame's output: AL_MAX_RSP_SAMPLES samples each */
#define N_DMEM_MAIN_L           0x4E0   /* the main bus, left */
#define N_DMEM_MAIN_R           0x650   /* the main bus, right */
#define N_DMEM_AUX_L            0x7C0   /* the effect bus, left */
#define N_DMEM_AUX_R            0x930   /* the effect bus, right */
#define N_DMEM_FRAME_BYTES      ( 2 * AL_MAX_RSP_SAMPLES * sizeof( s16 ) )  /* one channel pair */

/*
The output commands as n_audio's microcode takes them (the ROM's command words): the interleave has no
operands, and the save carries its byte count, in units of 16, beside the opcode.
*/
#define aInterleaveFrame( pkt )                                                 \
{                                                                               \
    Acmd *_a = (Acmd *)pkt;                                                     \
                                                                                \
    _a->words.w0 = _SHIFTL( A_INTERLEAVE, 24, 8 );                              \
}

#define aSaveFrame( pkt, c, s )                                                 \
{                                                                               \
    Acmd *_a = (Acmd *)pkt;                                                     \
                                                                                \
    _a->words.w0 = _SHIFTL( A_SAVEBUFF, 24, 8 ) | _SHIFTL( ( c ) >> 4, 16, 8 ); \
    _a->words.w1 = (u32)( s );                                                  \
}

void func_80042150( N_ALGlobals *g, ALSynConfig *c );  /* sets up the n_audio globals and the synthesizer, once */
void func_80042194( void );            /* takes the synthesizer down and clears the n_audio globals */
void func_80042790( ALSynConfig *c );
Acmd *func_80042CF0( s32 sampleOffset, Acmd *cmdList );    /* the output: mix, interleave, save */
ALFx *func_80042D40( s16 bus, ALSynConfig *c, ALHeap *hp );  /* creates the effect of bus bus */
Acmd *func_80042D90( s32 sampleOffset, Acmd *cmdList );    /* pulls the main bus and mixes the effect bus in */
void func_8003FF70( ALFx **fx, ALSynConfig *c, ALHeap *hp );   /* allocates and sets up the effect into *fx */
void func_80042B94( ALParam *update ); /* puts an update back on the free list */
void func_80042BAC( void );            /* moves the retired physical voices back to the free list */
void func_80042C0C( N_PVoice *pv );    /* retires a physical voice until the end of the frame */
s32 func_80042C44( ALMicroTime t );    /* t in output samples, rounded */

/* ===== end of the synthesizer frame and setup block ===== */

#endif
