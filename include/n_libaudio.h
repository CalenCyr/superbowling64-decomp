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
struct ALParam_s;

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
    /* decoder (driven by Synth_DecodeSamples and Synth_DecoderParam) */
    /* 0x0C */ ADPCM_STATE *decState;   /* the ADPCM decoder's state */
    /* 0x10 */ ADPCM_STATE *loopState;  /* the decoder's state at the loop start, from the wave table's loop */
    /* 0x14 */ u32 loopStart;           /* the wave table's loop, in samples; loopCount 0: none, ~0: forever */
    /* 0x18 */ u32 loopEnd;
    /* 0x1C */ u32 loopCount;
    /* 0x20 */ ALWaveTable *table;      /* the wave table it plays, NULL: none */
    /* 0x24 */ s32 bookSize;            /* bytes of the table's ADPCM codebook */
    /* 0x28 */ ALDMAproc dma;           /* reads ROM: returns the RDRAM address of the bytes asked for */
    /* 0x2C */ void *dmaState;
    /* 0x30 */ u32 sample;              /* samples decoded since the start or the last loop */
    /* 0x34 */ s32 lastsam;             /* samples of the last 16-sample ADPCM frame already used */
    /* 0x38 */ s32 first;               /* TRUE: the next decode starts the table, from a reset decoder state */
    /* 0x3C */ s32 memin;               /* the table's ROM address the next frames are read from */
    /* resampler */
    /* 0x40 */ RESAMPLE_STATE *rsState;
    /* 0x44 */ f32 ratio;               /* pitch ratio, below N_MAX_RATIO */
    /* 0x48 */ s32 unityPitch;          /* TRUE: no resampling, the decoded samples are moved as they are */
    /* 0x4C */ f32 rsDelta;             /* fraction of a sample carried to the next frame */
    /* 0x50 */ s32 rsFirst;             /* TRUE until the resampler's first frame */
    /* envelope mixer */
    /* 0x54 */ ENVMIX_STATE *emState;
    /* 0x58 */ s16 pan;                 /* 0-127, indexes the gain table both ways */
    /* 0x5A */ s16 volume;              /* the voice's volume squared, >> 15 */
    /* 0x5C */ s16 volL;                /* left and right volume now */
    /* 0x5E */ s16 volR;
    /* 0x60 */ s16 dryAmt;              /* gains from the effect mix */
    /* 0x62 */ s16 wetAmt;
    /* 0x64 */ u16 rateLFrac;           /* per-8-sample ramp step: fraction, whole */
    /* 0x66 */ s16 rateL;
    /* 0x68 */ s16 targetL;             /* volume the ramp ends at */
    /* 0x6A */ u16 rateRFrac;
    /* 0x6C */ s16 rateR;
    /* 0x6E */ s16 targetR;
    /* 0x70 */ s32 envDelta;            /* samples into the volume ramp */
    /* 0x74 */ s32 envLength;           /* samples the volume ramp lasts */
    /* 0x78 */ s32 envFirst;            /* TRUE: the ramp changed, set the volumes again */
    /* 0x7C */ struct ALParam_s *ctrlList;     /* queued updates, in time order */
    /* 0x80 */ struct ALParam_s *ctrlTail;
    /* 0x84 */ s32 motion;              /* N_MOTION_* */
    /* 0x88 */ s32 offset;              /* samples its updates wait: N_PVOICE_STEAL_DELAY while a stolen voice fades */
} N_PVoice;

#define N_MOTION_STOPPED        0       /* N_PVoice.motion */
#define N_MOTION_PLAYING        1

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

/* modulation rate range, and hundredths of a cent per natural-log unit of pitch ratio (120000 / ln 2) */
#define N_FX_RANGE      2.0
#define N_FX_CONVERT    173123.404906676

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
    /* 0x48 */ s16 *outBuf;          /* where the frame being made is saved: set per pass by Synth_MakeFrame */
    /* 0x4C */ s32 unk_4C;
} N_ALSynth;

typedef struct {
    /* 0x00 */ N_ALSynth drvr;
} N_ALGlobals;

#define AL_MAX_RSP_SAMPLES 184          /* N_ALSynth.maxOutSamples */

extern N_ALGlobals *snd_globals;     /* the n_audio globals, set once */
extern N_ALSynth *snd_synth;       /* &snd_globals->drvr: the synthesizer every n_alSyn* call uses */

Acmd *Synth_PullFxBus( s32 sampleOffset, Acmd *cmdList );    /* the effect bus pull: clears its mix, pulls each physical voice */
void Synth_InitPVoice( N_PVoice *pv, ALDMANew dma, ALHeap *hp );  /* sets up a physical voice's filters */
Acmd *Synth_PullMainBus( s32 sampleOffset, Acmd *cmdList );    /* main bus pull through the effect */
Acmd *Synth_MakeFrame( Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen );   /* synthesizes one frame */

s32 Synth_PVoiceParam( N_PVoice *pv, s32 paramID, void *param );   /* a physical voice's parameter handler */
ALParam *Synth_AllocUpdate( void );        /* takes an update off the free list, NULL: none left */
s32 Synth_FloorTime( ALMicroTime t );    /* t in output samples, rounded down to a multiple of 16 */

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

void Synth_InitGlobals( N_ALGlobals *g, ALSynConfig *c );  /* sets up the n_audio globals and the synthesizer, once */
void Synth_Teardown( void );            /* takes the synthesizer down and clears the n_audio globals */
void Synth_BuildSynth( ALSynConfig *c );
Acmd *Synth_PullOutput( s32 sampleOffset, Acmd *cmdList );    /* the output: mix, interleave, save */
ALFx *Synth_NewFx( s16 bus, ALSynConfig *c, ALHeap *hp );  /* creates the effect of bus bus */
Acmd *Synth_PullMix( s32 sampleOffset, Acmd *cmdList );    /* pulls the main bus and mixes the effect bus in */
void Synth_BuildFx( ALFx **fx, ALSynConfig *c, ALHeap *hp );   /* allocates and sets up the effect into *fx */
void Synth_FreeUpdate( ALParam *update ); /* puts an update back on the free list */
void Synth_RecycleRetired( void );            /* moves the retired physical voices back to the free list */
void Synth_RetirePVoice( N_PVoice *pv );    /* retires a physical voice until the end of the frame */
s32 Synth_RoundTime( ALMicroTime t );    /* t in output samples, rounded */

/* ===== end of the synthesizer frame and setup block ===== */

/* ===== the decoder: unit 1C260 ===== */

/* filter messages the decoder's handler Synth_DecoderParam takes */
#define N_FILTER_RESET          4       /* rewinds the decoder to the table's start */
#define N_FILTER_SET_WAVETABLE  5       /* param: the ALWaveTable to play */

/* flags of the ROM's ADPCM decode command */
#define N_ADPCM_INIT            1       /* reset the decoder state first */
#define N_ADPCM_LOOP            2       /* start from the loop state the set-loop command gave */

#define N_ADPCM_FRAME_BYTES     9       /* bytes of one 16-sample ADPCM frame */

/* the ROM's load command: the byte count and the DMEM address in w0 */
#define n_aLoadBuffer( pkt, c, d, s )                                                       \
{                                                                                           \
    Acmd *_a = (Acmd *)pkt;                                                                 \
                                                                                            \
    _a->words.w0 = _SHIFTL( A_LOADBUFF, 24, 8 ) | _SHIFTL( c, 12, 12 ) | _SHIFTL( d, 0, 12 ); \
    _a->words.w1 = (u32)( s );                                                              \
}

/*
the ROM's ADPCM decode command: the state's address in w0; in w1 the flags, the output bytes, the input's
alignment offset and the output's DMEM address
*/
#define n_aADPCMdec( pkt, s, f, c, a, d )                                                   \
{                                                                                           \
    Acmd *_a = (Acmd *)pkt;                                                                 \
                                                                                            \
    _a->words.w0 = _SHIFTL( A_ADPCM, 24, 8 ) | _SHIFTL( s, 0, 24 );                         \
    _a->words.w1 = _SHIFTL( f, 28, 4 ) | _SHIFTL( c, 16, 12 ) | _SHIFTL( a, 12, 4 ) | _SHIFTL( d, 0, 12 ); \
}

void Synth_DecoderParam( N_PVoice *pv, s32 paramID, void *param );         /* the decoder's parameter handler */
Acmd *Synth_DecodeAdpcm( Acmd *ptr, N_PVoice *pv, s32 tsam, s32 nbytes, s16 outp, s16 inp, s32 flags );

/* ===== end of the decoder block ===== */

/* ===== physical voice pull: 1B2D0, 1C980 ===== */

#define N_FRAME_SAMPLES         AL_MAX_RSP_SAMPLES  /* samples a voice makes per pull */
#define N_UNITY_PITCH           0x8000  /* the resample command's ratio for 1.0 */
#define N_MAX_RATIO             1.99996 /* the highest pitch ratio the resampler takes */
#define N_DECODE_BUFFER         0x170   /* DMEM where a voice's decoded samples start */

/* the ROM's resample command: like ultralib's aResample, with the input's DMEM address in w1 */
#define n_aResample(pkt, f, p, s, i)                                                    \
{                                                                                       \
    Acmd *_a = (Acmd *)pkt;                                                             \
                                                                                        \
    _a->words.w0 = _SHIFTL(A_RESAMPLE, 24, 8) | _SHIFTL(s, 0, 24);                      \
    _a->words.w1 = _SHIFTL(f, 30, 2) | _SHIFTL(p, 14, 16) | _SHIFTL(i, 2, 12);         \
}

/* filter message a physical voice's handler takes, beside AL_FILTER_ADD_UPDATE, N_FILTER_RESET and N_FILTER_SET_WAVETABLE */
#define N_FILTER_START          9       /* starts the voice playing */

/* ALParam.type values only the physical voice's pull handles (beside the N_PARAM_* above) */
#define N_PARAM_FREE_VOICE      0       /* data: the physical voice to free */
#define N_PARAM_SET_WAVETABLE   5       /* data: the wave table, passed to the decoder */
#define N_PARAM_UNITY_PITCH     8       /* plays at unity pitch from now on */
#define N_PARAM_START_VOICE_ALT 13      /* an N_ALStartParamAlt: starts with volume, pan and mix set */

/* the N_PARAM_START_VOICE_ALT update; 0x1C bytes */
typedef struct {
    /* 0x00 */ struct ALParam_s *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;                /* N_PARAM_START_VOICE_ALT */
    /* 0x0A */ s16 unityPitch;
    /* 0x0C */ f32 pitch;
    /* 0x10 */ s16 volume;
    /* 0x12 */ u8 pan;
    /* 0x13 */ u8 fxMix;
    /* 0x14 */ s32 envLength;           /* samples the volume ramps over, 0: none */
    /* 0x18 */ ALWaveTable *wavetable;
} N_ALStartParamAlt;

/* n samples rounded to the nearest whole frame */
#define N_ROUND_FRAMES(n)       ( ( ( n ) + N_FRAME_SAMPLES / 2 ) / N_FRAME_SAMPLES * N_FRAME_SAMPLES )


/* the ROM's envelope mixer command: like ultralib's aEnvMixer, with the right volume in w0 */
#define n_aEnvMixer(pkt, f, v, s)                                                       \
{                                                                                       \
    Acmd *_a = (Acmd *)pkt;                                                             \
                                                                                        \
    _a->words.w0 = _SHIFTL(A_ENVMIXER, 24, 8) | _SHIFTL(f, 16, 8) | _SHIFTL(v, 0, 16);  \
    _a->words.w1 = (unsigned int)(s);                                                   \
}

/* the library's effect presets, by ALSynConfig.fxType: section count, buffer length, then per section the
delay line, coefficients, modulation and low-pass (as Synth_SetupFx reads them) */
extern s32 snd_fxSmallRoom[];            /* AL_FX_SMALLROOM */
extern s32 snd_fxBigRoom[];            /* AL_FX_BIGROOM */
extern s32 snd_fxEcho[];            /* AL_FX_ECHO */
extern s32 snd_fxChorus[];            /* AL_FX_CHORUS */
extern s32 snd_fxFlange[];            /* AL_FX_FLANGE */
extern s32 snd_fxDefault[];            /* any other fxType */

Acmd *Synth_PullPVoice( void *filter, s32 sampleOffset, Acmd *p );        /* pulls one frame of a physical voice */
Acmd *Synth_MixFrame( void *filter, s16 *inp, s16 *outp, s32 outCount, Acmd *p ); /* mixes one frame */
s16 Synth_RampStep( f64 vol, f64 tgt, s32 count, u16 *ratel );         /* the ramp step from vol to tgt */
s16 Synth_RampVolume( s16 vol, s32 count, s16 ratem, u16 ratel );        /* the volume count samples into a ramp */

Acmd *Synth_DecodeSamples( N_PVoice *pv, s16 *outp, s32 outCount, Acmd *p );  /* decodes outCount samples to *outp */
Acmd *Synth_ResampleFrame( N_PVoice *pv, s16 *outp, Acmd *p );              /* resamples one frame into *outp */
s32 Synth_ResamplerParam( N_PVoice *pv, s32 paramID, void *param );          /* the resampler's parameter handler */

/* ===== end 1B2D0, 1C980 ===== */

/* ===== the effect: unit 1CB60 ===== */

/* DMEM buffers the effect's delay lines are worked in, one frame each, swapped from line to line */
#define N_FX_BUFFER_A           0x000
#define N_FX_BUFFER_B           0x170
#define N_FX_RESAMPLE_BUFFER    0x2E0   /* where a modulated line's input is loaded before resampling */

/* gains of the effect's input mix: the effect bus's left channel scaled in place, then the right mixed in */
#define N_FX_INPUT_GAIN         0xDA83
#define N_FX_RIGHT_GAIN         0x5A82

/* the ROM's save command: like n_aLoadBuffer */
#define n_aSaveBuffer( pkt, c, d, s )                                                       \
{                                                                                           \
    Acmd *_a = (Acmd *)pkt;                                                                 \
                                                                                            \
    _a->words.w0 = _SHIFTL( A_SAVEBUFF, 24, 8 ) | _SHIFTL( c, 12, 12 ) | _SHIFTL( d, 0, 12 ); \
    _a->words.w1 = (u32)( s );                                                              \
}

/* the ROM's resample command with an output buffer: its DMEM page (address >> 8) in w1's low two bits */
#define n_aResampleOut( pkt, f, p, s, i, o )                                                \
{                                                                                           \
    Acmd *_a = (Acmd *)pkt;                                                                 \
                                                                                            \
    _a->words.w0 = _SHIFTL( A_RESAMPLE, 24, 8 ) | _SHIFTL( s, 0, 24 );                      \
    _a->words.w1 = _SHIFTL( f, 30, 2 ) | _SHIFTL( p, 14, 16 ) | _SHIFTL( i, 2, 12 ) | _SHIFTL( o, 0, 2 );        \
}

/* the ROM's pole filter command: the buffer's DMEM page (address >> 8) beside the state's address in w1 */
#define n_aPoleFilter( pkt, f, g, t, s )                                                    \
{                                                                                           \
    Acmd *_a = (Acmd *)pkt;                                                                 \
                                                                                            \
    _a->words.w0 = _SHIFTL( A_POLEF, 24, 8 ) | _SHIFTL( f, 16, 8 ) | _SHIFTL( g, 0, 16 );   \
    _a->words.w1 = _SHIFTL( t, 24, 8 ) | _SHIFTL( s, 0, 24 );                              \
}

f32 _doModFunc( ALDelay *d, s32 count );    /* a modulated delay line's pitch offset for count samples */

s32 Synth_FxParam( void *filter, s32 paramID, void *param );          /* the effect's parameter handler */
Acmd *Synth_ReadDelayLine( ALFx *fx, ALDelay *delay, s32 buff, Acmd *p );        /* reads a delay line's output into buff */
Acmd *Synth_LoadDelay( ALFx *fx, s16 *pos, s32 buff, s32 count, Acmd *p );  /* loads count samples of the delay buffer */
Acmd *Synth_SaveDelay( ALFx *fx, s16 *pos, s32 buff, Acmd *p );     /* saves one frame into the delay buffer */
Acmd *Synth_RunLowPass( ALLowPass *lp, s32 buff, Acmd *p );              /* runs a low-pass over buff */

/* ===== end of the effect block ===== */

#endif
