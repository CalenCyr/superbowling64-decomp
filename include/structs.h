#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"
#include "ultra64.h"

/*
==============================================================================

SOUND DRIVER (166B0.c)

Names in this section are invented from behaviour unless config/names.txt says otherwise.
Times are in sequencer units: 0x100 per tick, a tick being 1 / snd_tickRate seconds.
Volumes, velocities, pans and FX mixes are 0 to 0x7F; pitches are in semitones.

==============================================================================
*/

/* Message in the snd_msgs ring buffer: posted by Snd_PostMessage, run by Snd_HandleMessage on the next tick. */
typedef struct {
    /* 0x00 */ u8 cmd;              /* SNDMSG_* */
    /* 0x04 */ s32 arg;             /* sound id, or the effect setting for SNDMSG_SETFX */
} sndMsg_t;

/* Drum map entry (seqHeader_t.drumMaps): what a note plays while a drum map is selected (command 0x9E). */
typedef struct {
    /* 0x00 */ u16 program;         /* replaces sndChannel_t.program */
    /* 0x02 */ u16 envelope;        /* index in seqHeader_t.envelopes */
    /* 0x04 */ u8 pan;              /* 0 to 0xFF, halved */
    /* 0x05 */ u8 note;             /* the note actually played */
} drumEntry_t;

/*
 * Envelope (seqHeader_t.envelopes, or inline after command 0x84), in the order SetEnvelope reads it. Levels
 * are 0 to 0x7F; times count envelope steps of interval ticks.
 */
typedef struct {
    /* 0x00 */ u8 interval;         /* ticks per envelope step; 0 reads as 1 */
    /* 0x01 */ u8 startLevel;
    /* 0x02 */ u8 attackTime;       /* steps from startLevel to peakLevel */
    /* 0x03 */ u8 peakLevel;
    /* 0x04 */ u8 decayTime;        /* steps from peakLevel to sustainLevel */
    /* 0x05 */ u8 sustainLevel;
    /* 0x06 */ u8 releaseTime;      /* steps from the release level to 0 */
} envelope_t;

/*
 * BGM sequence (from BGMPTR). Snd_StartBgm turns its offsets (trackCmds to masterCmds, and the three track
 * tables) into pointers in place, once, and starts one master channel plus one channel per track.
 */
typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 numTracks;       /* entries in trackCmds, trackVolStreams and trackBendStreams */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ u8 **trackCmds;      /* command stream of each track; NULL: unused track */
    /* 0x10 */ u8 **trackVolStreams;    /* volume step stream of each track (Snd_StepVolumeStream) */
    /* 0x14 */ u8 **trackBendStreams;   /* pitch bend step stream of each track (Snd_StepBendStream) */
    /* 0x18 */ envelope_t *envelopes;   /* selected by command 0x90 and drum maps */
    /* 0x1C */ drumEntry_t *drumMaps;   /* selected by command 0x9E */
    /* 0x20 */ u16 *programs;       /* wave of each program; 0xFFFF: none (notes rest) */
    /* 0x24 */ u8 *masterCmds;      /* command stream of the master channel */
    /* 0x28 */ s32 relocated;       /* set once the offsets are pointers */
} seqHeader_t;

/*
 * Tuning of one wave (waveBank_t.tunes): a signed byte of hundredths of a semitone that Snd_RelocateWaveBank
 * turns in place into a float in semitones, adding the matching digit of waveBank_t.unk_24.
 */
typedef union {
    u8 raw;
    f32 value;
} waveTune_t;

/*
 * Wave bank (BGMWBK or SEWBK control data). Snd_RelocateWaveBank relocates it in place once; its samples stay
 * in ROM. Channels play from sndChannel_t.waveBank.
 */
typedef struct {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ s32 flags;           /* WAVEBANK_RELOCATED */
    /* 0x14 */ u8 unk_14[0x20 - 0x14];
    /* 0x20 */ s32 numWaves;        /* entries in unk_24, tunes and waves */
    /* 0x24 */ u8 *unk_24;          /* '0'-based digits added to the tunes */
    /* 0x28 */ waveTune_t *tunes;
    /* 0x2C */ ALWaveTable **waves;
} waveBank_t;

/* waveBank_t.flags */
#define WAVEBANK_RELOCATED  0x80000000

/* seTable_t.flags */
#define SETABLE_RELOCATED   0x00000001

/* Sound effect of an SE table. */
typedef struct {
    /* 0x00 */ u8 *cmds;            /* command stream; an offset until Snd_LoadSeTable relocates it */
    /* 0x04 */ s32 priority;        /* default priority of the channel */
} seEntry_t;

/* SE table (from SEPTR): sound effects started by index (Snd_PlaySe); Snd_LoadSeTable relocates it once. */
typedef struct {
    /* 0x00 */ s32 numEntries;      /* entries in entries */
    /* 0x04 */ s32 unk_04;          /* returned by func_8003C1C0 */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ s32 flags;           /* SETABLE_RELOCATED */
    /* 0x10 */ waveBank_t *waveBank;    /* set by Snd_SetSeWaveBank; NULL: snd_nextWaveBank or the default */
    /* 0x14 */ u16 *programs;       /* wave of each program */
    /* 0x18 */ seEntry_t entries[1];    /* numEntries entries */
} seTable_t;

/* Configuration passed to Snd_Init; filled on the stack by its caller. 0x28 to 0x3C go to func_8003F9A0. */
typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 numVoices;       /* NUM_MASTER_CHANNELS more channels are added */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ s32 voicePriority;
    /* 0x10 */ s32 heapBase;        /* audio heap (Snd_InitHeap) */
    /* 0x14 */ s32 heapSize;
    /* 0x18 */ waveBank_t *waveBank;    /* default wave bank, NULL: none */
    /* 0x1C */ s32 waveRomBase;     /* ROM address of its samples */
    /* 0x20 */ seTable_t *seTable;  /* default SE table, NULL: none */
    /* 0x24 */ s32 numMsgs;         /* message ring buffer size */
    /* 0x28 */ u8 unk_28[0x40 - 0x28];
    /* 0x40 */ s32 unk_40;
} sndConfig_t;

#define NUM_MASTER_CHANNELS 4       /* the first channels: BGM master streams only, no voice */
#define MAX_LOOP_DEPTH      4       /* entries of the channel loop stack */
#define FADE_NONE           ( -1 )  /* sndChannel_t.fadeLeft: not fading */
#define NOTE_LENGTH_HOLD    0x7FFF  /* sndChannel_t.noteLength: the note lasts until a command */
#define LOOP_FOREVER        0xFF    /* sndChannel_t.loopCount */

/* sndChannel_t.flags */
#define CHAN_PAUSED         0x00000001  /* skipped by Snd_PlayerHandler; every channel starts paused */
#define CHAN_MASTER         0x00000002  /* the master stream of a BGM sequence (seqHeader_t.masterCmds) */

/* sndChannel_t.envPhase */
typedef enum {
    ENV_OFF,
    ENV_ATTACK,
    ENV_DECAY,
    ENV_SUSTAIN,
    ENV_RELEASE,
    ENV_DONE
} envPhase_t;

/* channel groups, for Snd_SetMasterVolume, Snd_StopAll and Snd_NumActive */
#define SND_SE              0x00000001  /* channels started from an SE table */
#define SND_BGM             0x00000002  /* BGM channels */

/*
 * Sequencer channel: one command stream (a BGM track, a BGM master stream or a sound effect) and the state of
 * the note it plays on its voice. snd_channels holds snd_numChannels of them; the first NUM_MASTER_CHANNELS
 * have no voice. Snd_ResetChannel clears one when a sound starts on it; Snd_CmdEnd and the end of a fade free
 * it (cmdPos NULL).
 */
typedef struct {
    /* 0x000 */ s32 flags;          /* CHAN_* */
    /* 0x004 */ u8 *cmdPos;         /* command read position; NULL: free */
    /* 0x008 */ ALWaveTable *pendingWave;   /* wave to start on the voice next tick, NULL: none */
    /* 0x00C */ s32 time;           /* sequencer clock, + tempo each tick */
    /* 0x010 */ s32 fadeLeft;       /* ticks left of the fade out, FADE_NONE: not fading */
    /* 0x014 */ s32 volStreamTime;  /* time of the next volume stream step */
    /* 0x018 */ s32 bendStreamTime; /* time of the next bend stream step */
    /* 0x01C */ u32 fadeTotal;      /* ticks of the whole fade */
    /* 0x020 */ f32 vibratoDepth;   /* semitones */
    /* 0x024 */ f32 bendPitch;      /* bend * bendRange, semitones */
    /* 0x028 */ f32 lastPitch;      /* last pitch sent to the voice */
    /* 0x02C */ f32 notePitch;      /* note + wave tuning + transpose */
    /* 0x030 */ f32 pitchOffset;    /* detune plus Snd_SetPitchOffset */
    /* 0x034 */ u8 *bendStream;     /* pitch bend step stream, NULL: none */
    /* 0x038 */ u8 *volStream;      /* volume step stream, NULL: none */
    /* 0x03C */ s32 nextNoteTime;   /* time of the next note */
    /* 0x040 */ s32 noteTime;       /* time the current note started */
    /* 0x044 */ s32 id;             /* shared by every channel of one started sound; 0: none */
    /* 0x048 */ s32 priority;       /* SE channels: lower ones are taken first */
    /* 0x04C */ f32 glideFrom;      /* portamento start pitch */
    /* 0x050 */ f32 glidePitch;     /* portamento pitch reached so far */
    /* 0x054 */ s32 releaseTime;    /* time the envelope enters ENV_RELEASE */
    /* 0x058 */ f32 attackSlope;    /* level per step */
    /* 0x05C */ f32 decaySlope;
    /* 0x060 */ f32 releaseSlope;   /* fraction of the release level per step */
    /* 0x064 */ s32 envTimeScale;   /* 0x400 / envInterval: ticks to envelope steps, << 10 */
    /* 0x068 */ f32 vibratoPitch;   /* last vibrato offset */
    /* 0x06C */ f32 bendRange;      /* semitones per bend unit */
    /* 0x070 */ f32 bend;           /* -64 to 63 */
    /* 0x074 */ seqHeader_t *seq;   /* BGM channels */
    /* 0x078 */ seTable_t *seTable; /* SE channels; NULL on BGM channels */
    /* 0x07C */ waveBank_t *waveBank;
    /* 0x080 */ u8 *cmdStart;       /* start of the command stream, base of Snd_CmdJump */
    /* 0x084 */ drumEntry_t *drumMap;   /* NULL: none */
    /* 0x088 */ u8 *bendStreamStart;
    /* 0x08C */ u8 *volStreamStart;
    /* 0x090 */ f32 detune;         /* semitones, part of pitchOffset */
    /* 0x094 */ s32 autoPanTime;    /* time of the next pan sweep step */
    /* 0x098 */ s16 tempoScale;     /* 0x80: 1 */
    /* 0x09A */ u16 noteLength;     /* ticks; NOTE_LENGTH_HOLD: until the next command */
    /* 0x09C */ u16 tempo;          /* added to time each tick: baseTempo * tempoScale >> 7 */
    /* 0x09E */ s16 playVolume;     /* 0x80: 1; set when the sound starts and by Snd_SetVolume */
    /* 0x0A0 */ u16 lastVolume;     /* last volume sent to the voice */
    /* 0x0A2 */ u16 volStreamWait;  /* steps until the next volume stream entry */
    /* 0x0A4 */ u16 bendStreamWait;
    /* 0x0A6 */ u16 seIndex;        /* entry of seTable the channel was started on */
    /* 0x0A8 */ u16 baseTempo;      /* from Snd_CmdTempo */
    /* 0x0AA */ u16 noteAge;        /* ticks since the note started */
    /* 0x0AC */ u16 fixedLength;    /* note length when notes carry none, 0: they do */
    /* 0x0AE */ u16 program;
    /* 0x0B0 */ s16 playPan;        /* pan scale, 0x80: 1; set when the sound starts and by Snd_SetPan */
    /* 0x0B2 */ u16 releaseAfter;   /* release this many ticks after note on, 0: use releaseBefore */
    /* 0x0B4 */ u16 releaseBefore;  /* release this many ticks before the note ends */
    /* 0x0B6 */ u8 vibratoDelay;    /* ticks */
    /* 0x0B7 */ u8 explicitLength;  /* the next note carries a length despite fixedLength */
    /* 0x0B8 */ u8 portamento;      /* glide ticks, 0: off */
    /* 0x0B9 */ u8 transpose;       /* signed semitones */
    /* 0x0BA */ u8 transposeOff;
    /* 0x0BB */ u8 velocity;
    /* 0x0BC */ u8 trackVolume;
    /* 0x0BD */ u8 pan;             /* 0x40: centre */
    /* 0x0BE */ u8 lastPan;         /* last pan sent to the voice, 0xFF: resend */
    /* the envelope (envelope_t, and its running state) */
    /* 0x0BF */ u8 envInterval;
    /* 0x0C0 */ u8 envStartLevel;
    /* 0x0C1 */ u8 envPeakLevel;
    /* 0x0C2 */ u8 envSustainLevel;
    /* 0x0C3 */ u8 envPhase;        /* ENV_* */
    /* 0x0C4 */ u8 envLevel;
    /* 0x0C5 */ u8 envWait;         /* ticks until the next envelope step */
    /* 0x0C6 */ u8 envAttackTime;
    /* 0x0C7 */ u8 envDecayTime;
    /* 0x0C8 */ u8 envReleaseTime;
    /* 0x0C9 */ u8 voiceActive;     /* the channel started its voice */
    /* 0x0CA */ u8 fxMix;           /* from Snd_CmdFxMix, scales the part above fxBase */
    /* 0x0CB */ u8 fxBase;          /* minimum FX mix, from Snd_SetFxBase */
    /* 0x0CC */ u8 lastFxMix;       /* last fxMix sent, 0xFF: resend */
    /* 0x0CD */ u8 envReleaseLevel; /* envLevel when the release started */
    /* trill: notes alternate between their pitch and trillInterval semitones above */
    /* 0x0CE */ u8 trillOnTicks;    /* 0: trill off */
    /* 0x0CF */ u8 trillOffTicks;
    /* 0x0D0 */ u8 trillWait;
    /* 0x0D1 */ s8 trillPitch;      /* 0 or trillInterval */
    /* 0x0D2 */ u8 noteVelocity;    /* each note carries a velocity byte; one >= 0x80 fixes it */
    /* 0x0D3 */ u8 fixedVelocity;
    /* 0x0D4 */ u8 autoPanSpeed;    /* 64ths of a pan unit per tick, 0: off */
    /* 0x0D5 */ u8 vibratoPeriod;   /* ticks, 0: vibrato off */
    /* 0x0D6 */ u8 noEnvRestart;    /* notes keep the running envelope */
    /* 0x0D7 */ u8 noVoiceRestart;  /* notes keep the running voice */
    /* 0x0D8 */ u8 trillInterval;   /* semitones */
    /* 0x0D9 */ u8 autoPanFrac;     /* 64ths carried to the next step */
    /* 0x0DA */ u8 autoPanDown;     /* sweep direction */
    /* 0x0DB */ u8 loopDepth;       /* entries on the loop stack below */
    /* 0x0DC */ f32 vibratoStep;    /* radians per tick */
    /* loop stack, pushed by Snd_CmdLoopStart and popped by Snd_CmdLoopEnd */
    /* 0x0E0 */ u8 *loopPos[MAX_LOOP_DEPTH];        /* read position after the loop start command */
    /* 0x0F0 */ u8 *loopVolStream[MAX_LOOP_DEPTH];  /* saved volStream */
    /* 0x100 */ u8 *loopBendStream[MAX_LOOP_DEPTH]; /* saved bendStream */
    /* 0x110 */ u16 loopVolWait[MAX_LOOP_DEPTH];    /* saved volStreamWait */
    /* 0x118 */ u16 loopBendWait[MAX_LOOP_DEPTH];   /* saved bendStreamWait */
    /* 0x120 */ u8 loopCount[MAX_LOOP_DEPTH];       /* passes left; LOOP_FOREVER repeats forever */
    /* 0x124 */ u8 loopVolume[MAX_LOOP_DEPTH];      /* saved trackVolume */
    /* 0x128 */ f32 loopBend[MAX_LOOP_DEPTH];       /* saved bend */
    /* 0x138 */ u8 unk_138[0x13C - 0x138];
} sndChannel_t;

#endif
