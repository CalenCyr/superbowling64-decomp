/*
 * 166B0.c -- sound driver: a byte-code sequencer for BGM (BGMPTR) and sound effects (SEPTR) that plays wave
 * banks (BGMWBK, SEWBK) on n_audio voices. The file names are the game's BW64FAT entries; every other name in
 * this unit is invented (config/names.txt).
 *
 * Snd_PlayerHandler runs once a tick (snd_tickRate per second) as the synthesizer's ALPlayer. Each channel
 * reads one command stream. A byte below CMD_FIRST is a note: the note, then a velocity byte while noteVelocity
 * is set, then its length in ticks (one byte, or two when the first has bit 7 set) unless fixedLength is set.
 * A byte from CMD_FIRST on is a command, run by snd_commands[byte - CMD_FIRST] (Snd_Cmd*), which returns the
 * read position after its operands, or NULL to end the stream.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "structs.h"
#include "variables.h"

/* The low byte of x as a signed value. */
#define SEXT8( x ) ( ( ( x ) & 0x80 ) ? ( ( x ) & 0xFF ) - 0x100 : ( ( x ) & 0xFF ) )

/* command stream bytes */
#define CMD_FIRST           0x80    /* bytes below are notes */
#define CMD_MARKER          0xAB    /* Snd_CmdMarker */
#define NOTE_REST           0x60    /* a note that releases the voice */
#define PROGRAM_NONE        0xFFFF  /* seqHeader_t.programs: no wave, notes rest */

#define TRACK_MASTER        ( -1 )  /* Snd_FindBgmChannel: the master stream */
#define SE_PRIORITY_DEFAULT ( -1 )  /* Snd_StartSe and Snd_PlaySeEx: the entry's own priority */

#define TICK_RATE_PAL       50      /* ticks per second */
#define TICK_RATE_NTSC      60
#define TEMPO_120_BPM       0x6000  /* sequencer time per second at 120 beats per minute (96 steps of 0x100) */
#define SCALE_ONE           0x80    /* playVolume, playPan and tempoScale: 1 */
#define VOLUME_MAX          0x7FFF  /* snd_seVolume, snd_bgmVolume and the voice volume */
#define PAN_CENTRE          0x40
#define RESEND              0xFF    /* lastPan, lastFxMix: send the next value whatever it is */
#define RANDOM_SEED         0x12345678
#define FX_DEFAULT          2       /* snd_fxType until Snd_SetFxType */

/* the message ring buffer */
#define MIN_MSGS            0x40
#define MAX_MSGS            0x400

/* sndMsg_t.cmd */
typedef enum {
    SNDMSG_PAUSE,                   /* sets CHAN_PAUSED on the channels of arg */
    SNDMSG_RESUME,                  /* clears it */
    SNDMSG_SETFX                    /* func_8003F340( arg ) */
} sndMsgType_t;

/*
==============================================================================

ENVELOPES

==============================================================================
*/

/*
================
SetEnvelope

Reads an envelope_t at src into the channel and derives the slopes of its attack, decay and release; returns
the position after it. Used by Snd_CmdEnvelopeIndex and Snd_PlayNextNote; Snd_CmdEnvelope repeats its body.
================
*/
static inline u8 *SetEnvelope( sndChannel_t *chan, u8 *src ) {
    s32 value;
    s32 level;

    value = *src++;
    if ( value == 0 ) {
        value = 1;
    }
    chan->envInterval = value;
    chan->envTimeScale = 0x400 / value;
    chan->envStartLevel = *src++;
    value = *src++;
    chan->envAttackTime = value;
    level = *src++;
    chan->attackSlope = 1.0 / (f32)value * (f32)( level - chan->envStartLevel );
    chan->envPeakLevel = level;
    value = *src++;
    chan->envDecayTime = value;
    level = *src++;
    chan->decaySlope = 1.0 / (f32)value * (f32)( level - chan->envPeakLevel );
    chan->envSustainLevel = level;
    value = *src++;
    chan->envReleaseTime = value;
    chan->releaseSlope = 1.0 / (f32)value;
    return src;
}

/*
================
Snd_CmdEnvelopeIndex

Command 0x90 <index:1-2>: applies envelope index of the channel's sequence (seqHeader_t.envelopes).
================
*/
u8 *Snd_CmdEnvelopeIndex( sndChannel_t *chan, u8 *src ) {
    s32 index;

    index = *src++;
    if ( index & 0x80 ) {
        index &= 0x7F;
        index <<= 8;
        index |= *src++;
    }
    SetEnvelope( chan, &chan->seq->envelopes[index].interval );
    return src;
}

/*
==============================================================================

SETUP AND SOUND CONTROL

==============================================================================
*/

/*
================
Snd_Init

Sets up the driver from config: the tick rate from the TV type, the audio heap, the channel and voice arrays,
the message ring buffer, the default wave bank and SE table, the synthesizer, full master volumes, then
registers snd_player and allocates a voice for every channel past the master channels. Called once.
================
*/
void Snd_Init( sndConfig_t *config ) {
    s32 i;
    ALVoiceConfig voiceConfig;

    D_800693D8 = config->unk_40;
    D_800DF6B0 = config->unk_00;
    snd_numChannels = config->numVoices + NUM_MASTER_CHANNELS;
    if ( osTvType == OS_TV_PAL ) {
        snd_tickRate = TICK_RATE_PAL;
    } else {
        snd_tickRate = TICK_RATE_NTSC;
    }
    snd_tickTime = 1000000 / snd_tickRate;
    Snd_InitHeap( config->heapBase, config->heapSize );
    func_8003F7F0( config->sched );
    snd_voices = Snd_Alloc( ( snd_numChannels - NUM_MASTER_CHANNELS ) * sizeof( N_ALVoice ) );
    snd_channels = Snd_Alloc( snd_numChannels * sizeof( sndChannel_t ) );
    snd_voiceChannels = snd_channels + NUM_MASTER_CHANNELS;
    Snd_InitMessages( config->numMsgs );
    snd_nextWaveBank = NULL;
    snd_defaultWaveBank = NULL;
    if ( config->waveBank != NULL && config->waveRomBase != 0 ) {
        Snd_LoadWaveBank( config->waveBank, config->waveRomBase );
    }
    snd_nextSeTable = NULL;
    snd_seTable = NULL;
    if ( config->seTable != NULL ) {
        Snd_LoadSeTable( config->seTable );
    }
    snd_markerCallback = NULL;
    snd_fxType = FX_DEFAULT;
    func_8003F9A0( config, snd_tickRate, 2 );
    Snd_SetMasterVolume( SND_SE | SND_BGM, VOLUME_MAX );
    snd_nextId = 1;
    snd_randomSeed = RANDOM_SEED;
    snd_player.next = NULL;
    snd_player.handler = Snd_PlayerHandler;
    snd_player.clientData = &snd_player;
    n_alSynAddPlayer( &snd_player );
    for ( i = 0; i < snd_numChannels; i++ ) {
        snd_channels[i].voiceActive = 0;
        Snd_ResetChannel( &snd_channels[i] );
        voiceConfig.unityPitch = 0;
        voiceConfig.priority = config->voicePriority;
        voiceConfig.fxBus = 0;
        if ( i >= NUM_MASTER_CHANNELS ) {
            n_alSynAllocVoice( &snd_voices[i - NUM_MASTER_CHANNELS], &voiceConfig );
        }
    }
    Snd_HeapUsed();     // NOTE: the result is unused
}

/*
================
Snd_SetMasterVolume

Sets the volume scale (VOLUME_MAX: 1) of the channel groups in flags (SND_SE, SND_BGM).
================
*/
void Snd_SetMasterVolume( s32 flags, s16 value ) {
    if ( flags & SND_SE ) {
        snd_seVolume = value;
    }
    if ( flags & SND_BGM ) {
        snd_bgmVolume = value;
    }
}

/*
================
Snd_PlayBgm

Starts BGM sequence hdr (paused) and posts SNDMSG_RESUME for it, so it plays from the next tick. Returns its
id.
================
*/
s32 Snd_PlayBgm( seqHeader_t *hdr ) {
    s32 id;

    id = Snd_StartBgm( hdr );
    Snd_Resume( id );
    return id;
}

/*
================
Snd_PlayBgmFromMarker

Like Snd_PlayBgm, but first runs each channel of hdr silently, commands and notes, up to its marker command
(CMD_MARKER) whose value is marker, and starts it there; the marker's length becomes the wait for the next
event. A channel without that marker runs to its end.
================
*/
s32 Snd_PlayBgmFromMarker( seqHeader_t *hdr, s32 marker ) {
    s32 id;
    s32 i;
    sndChannel_t *chan;
    u8 *src;
    u8 cmd;
    u8 value;
    s32 length;

    id = Snd_StartBgm( hdr );
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id != id || chan->seq != hdr || chan->cmdPos == NULL ) {
            continue;
        }
        // RULE-EXCEPTION: R14, a do/while here lets gcc hoist the handler table and CMD_MARKER out of the loop;
        // retail reloads both every pass, so this loop was built from labels.
    next:
        src = chan->cmdPos;
        cmd = *src;
        if ( cmd >= CMD_FIRST ) {
            if ( cmd == CMD_MARKER && src[1] == marker ) {
                goto found;
            }
            chan->cmdPos = snd_commands[cmd & 0x7F]( chan, src + 1 );
        } else {
            chan->cmdPos = src + 1;
            if ( chan->noteVelocity ) {
                chan->cmdPos = src + 2;
                value = src[1];
                chan->velocity = value;
                if ( value >= 0x80 ) {
                    chan->velocity = value & 0x7F;
                    chan->noteVelocity = 0;
                    chan->fixedVelocity = chan->velocity;
                }
            } else {
                chan->velocity = chan->fixedVelocity;
            }
            if ( chan->fixedLength != 0 && chan->explicitLength == 0 ) {
                chan->noteLength = chan->fixedLength;
            } else {
                chan->explicitLength = 0;
                cmd = *chan->cmdPos++;
                if ( cmd < 0x80 ) {
                    chan->noteLength = cmd;
                } else {
                    chan->noteLength = *chan->cmdPos++ + ( ( cmd & 0x7F ) << 8 );
                }
            }
            chan->time += chan->noteLength << 8;
        }
        if ( chan->cmdPos != NULL ) {
            goto next;
        }
    found:
        chan->nextNoteTime = chan->time;
        if ( chan->cmdPos != NULL ) {
            src = chan->cmdPos + 2;
            length = *src++;
            if ( length >= 0x80 ) {
                length &= 0x7F;
                length <<= 8;
                length |= *src++;
            }
            chan->noteAge = 0;
            chan->noteLength = length;
            chan->cmdPos = src;
            chan->time -= length << 8;
        }
        chan->noteTime = chan->time;
        if ( chan->volStream != NULL ) {
            Snd_StepVolumeStream( chan );
        }
        if ( chan->bendStream != NULL ) {
            Snd_StepBendStream( chan );
        }
    }
    Snd_Resume( id );
    return id;
}

/*
================
Snd_PlaySe

Starts entry index of snd_nextSeTable (used once) or else snd_seTable at full volume and pan scale and the
entry's own priority, playing from the table's wave table. Returns the id, or 0 without a table or a channel.
================
*/
s32 Snd_PlaySe( s32 index ) {
    seTable_t *table;
    s32 id;

    table = snd_nextSeTable;
    if ( table != NULL ) {
        snd_nextSeTable = NULL;
    } else {
        table = snd_seTable;
        if ( table == NULL ) {
            snd_nextWaveBank = NULL;
            return 0;
        }
    }
    if ( snd_nextWaveBank == NULL ) {
        snd_nextWaveBank = table->waveBank;
    }
    id = Snd_StartSe( table, index, SCALE_ONE, SCALE_ONE, SE_PRIORITY_DEFAULT );
    snd_nextWaveBank = NULL;
    return id;
}

/*
================
Snd_PlaySeEx

Snd_PlaySe with a volume and pan scale (SCALE_ONE: 1) and a priority (SE_PRIORITY_DEFAULT: the entry's own).
With reuse set, a channel already playing this entry of this table is restarted instead of taking another.
================
*/
s32 Snd_PlaySeEx( s32 index, s32 volume, s32 pan, s32 reuse, s32 priority ) {
    seTable_t *table;
    s32 i;
    sndChannel_t *chan;
    s32 id;

    if ( snd_nextSeTable != NULL ) {
        table = snd_nextSeTable;
        snd_nextSeTable = NULL;
    } else {
        table = snd_seTable;
        if ( table == NULL ) {
            snd_nextWaveBank = NULL;
            return 0;
        }
    }
    if ( snd_nextWaveBank == NULL ) {
        snd_nextWaveBank = table->waveBank;
    }
    if ( reuse ) {
        for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
            if ( chan->seIndex == index && chan->seTable == table ) {
                if ( priority == SE_PRIORITY_DEFAULT ) {
                    priority = table->entries[index].priority;
                }
                id = Snd_StartSeOnChannel( chan, table, index, volume, pan, priority );
                snd_nextWaveBank = NULL;
                return id;
            }
        }
    }
    id = Snd_StartSe( table, index, volume, pan, priority );
    snd_nextWaveBank = NULL;
    return id;
}

/*
================
Snd_StopAll

Fades out over ticks (at least 1) every active channel of the groups in flags that is not fading yet; a
paused channel stops on the next tick instead. A faded channel is freed.
================
*/
void Snd_StopAll( s32 flags, s32 ticks ) {
    s32 i;
    s32 total;
    sndChannel_t *chan;

    total = ticks;
    if ( ticks == 0 ) {
        total = 1;
    }
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seTable != NULL ? flags & SND_SE : flags & SND_BGM ) {
            if ( chan->cmdPos != NULL && chan->fadeLeft == FADE_NONE ) {
                if ( chan->flags & CHAN_PAUSED ) {
                    chan->fadeTotal = 1;
                    chan->fadeLeft = 0;
                    chan->flags &= ~CHAN_PAUSED;
                } else {
                    chan->fadeTotal = total;
                    chan->fadeLeft = ticks;
                }
            }
        }
    }
}

/*
================
Snd_NumActive

Counts the active channels of the groups in flags (SND_SE, SND_BGM).
================
*/
s32 Snd_NumActive( s32 flags ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos != NULL ) {
            if ( chan->seTable != NULL ? flags & SND_SE : flags & SND_BGM ) {
                count++;
            }
        }
    }
    return count;
}

/*
================
Snd_Stop

Fades out over ticks (at least 1) every channel of sound id that is not fading yet; a paused channel stops on
the next tick instead and is unpaused. Returns how many channels it changed.
================
*/
s32 Snd_Stop( s32 id, s32 ticks ) {
    s32 i;
    s32 count;
    s32 total;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    total = ticks;
    if ( ticks == 0 ) {
        total = 1;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id && chan->fadeLeft == FADE_NONE ) {
            if ( chan->flags & CHAN_PAUSED ) {
                chan->fadeTotal = 1;
                chan->fadeLeft = 0;
                chan->flags &= ~CHAN_PAUSED;
            } else {
                chan->fadeTotal = total;
                chan->fadeLeft = ticks;
            }
            count++;
        }
    }
    return count;
}

/*
================
Snd_NumChannels

Counts the channels of sound id.
================
*/
s32 Snd_NumChannels( s32 id ) {
    s32 i;
    sndChannel_t *chan;
    s32 count;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        count += chan->id == id;
    }
    return count;
}

/*
================
Snd_SetVolume

Sets the volume scale (SCALE_ONE: 1) of every channel of sound id; returns how many there were.
================
*/
s32 Snd_SetVolume( s32 id, s32 value ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            chan->playVolume = value;
            count++;
        }
    }
    return count;
}

/*
================
Snd_SetPan

Sets the pan scale (SCALE_ONE: 1) of every channel of sound id and has its pan resent; returns how many there
were.
================
*/
s32 Snd_SetPan( s32 id, s32 value ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            count++;
            chan->playPan = value;
            chan->lastPan = RESEND;
        }
    }
    return count;
}

/*
================
Snd_SetPitchOffset

Sets the pitch offset (semitones, added to the channel's detune) of every channel of sound id; returns how
many there were.
================
*/
s32 Snd_SetPitchOffset( s32 id, f32 value ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            chan->pitchOffset = value + chan->detune;
            count++;
        }
    }
    return count;
}

/*
================
Snd_SetTempoScale

Sets the tempo scale (SCALE_ONE: 1, clamped to [1, 2 * SCALE_ONE]) of every channel of sound id and
recomputes its tempo from baseTempo; returns how many there were.
================
*/
s32 Snd_SetTempoScale( s32 id, s32 value ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    if ( value <= 0 ) {
        value = 1;
    } else if ( value > 0x100 ) {
        value = 0x100;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            count++;
            chan->tempoScale = value;
            chan->tempo = ( chan->baseTempo * value ) >> 7;
        }
    }
    return count;
}

/*
================
Snd_SetFxBase

Sets the minimum FX mix (clamped to [0, 0x7F]) of every channel of sound id and has it resent; returns how
many there were.
================
*/
s32 Snd_SetFxBase( s32 id, s32 value ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    if ( value < 0 ) {
        value = 0;
    } else if ( value > 0x7F ) {
        value = 0x7F;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            count++;
            chan->fxBase = value;
            chan->lastFxMix = RESEND;
        }
    }
    return count;
}

/*
==============================================================================

WAVE BANKS

==============================================================================
*/

/*
================
Snd_LoadWaveBank

Relocates wave bank bank, whose samples start at ROM address romBase, and makes it the default if there is none.
================
*/
void Snd_LoadWaveBank( waveBank_t *bank, s32 romBase ) {
    Snd_RelocateWaveBank( bank, romBase );
    if ( snd_defaultWaveBank == NULL ) {
        snd_defaultWaveBank = bank;
    }
}

/*
================
Snd_SetNextWaveBank

Makes bank, if it is relocated, the wave bank of the next started sound; returns that bank.
================
*/
waveBank_t *Snd_SetNextWaveBank( waveBank_t *bank ) {
    if ( bank != NULL && bank->flags < 0 ) {
        snd_nextWaveBank = bank;
    }
    return snd_nextWaveBank;
}

/*
================
Snd_SetDefaultWaveBank

Makes bank the default wave bank if it is relocated.
================
*/
void Snd_SetDefaultWaveBank( waveBank_t *bank ) {
    if ( bank != NULL && bank->flags < 0 ) {
        snd_defaultWaveBank = bank;
    }
}

/*
================
Snd_GetDefaultWaveBank
================
*/
waveBank_t *Snd_GetDefaultWaveBank( void ) {
    return snd_defaultWaveBank;
}

/*
================
Snd_GetWaveBank

Returns the wave bank of sound id, or NULL.
================
*/
waveBank_t *Snd_GetWaveBank( s32 id ) {
    s32 i;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return NULL;
    }
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            return chan->waveBank;
        }
    }
    return NULL;
}

/*
==============================================================================

PAUSE AND EFFECT

==============================================================================
*/

/*
================
Snd_Pause

Pauses the channels of sound id from the next tick.
================
*/
void Snd_Pause( s32 id ) {
    sndMsg_t msg;

    msg.cmd = SNDMSG_PAUSE;
    msg.arg = id;
    Snd_PostMessage( &msg );
}

/*
================
Snd_Resume

Resumes the channels of sound id from the next tick.
================
*/
void Snd_Resume( s32 id ) {
    sndMsg_t msg;

    msg.cmd = SNDMSG_RESUME;
    msg.arg = id;
    Snd_PostMessage( &msg );
}

/*
================
Snd_SetFxType

Remembers the effect setting fxType and posts SNDMSG_SETFX for it; returns 0 if the ring buffer was full.
================
*/
inline s32 Snd_SetFxType( s32 fxType ) {
    sndMsg_t msg;

    msg.arg = fxType;
    snd_fxType = fxType;
    msg.cmd = SNDMSG_SETFX;
    return Snd_PostMessage( &msg );
}

/*
================
Snd_EnableSeqFx

Sets snd_seqFxEnabled (1: Snd_CmdFxType may change the effect). Disabling it first restores the effect last
given to Snd_SetFxType, and is dropped if that cannot be posted.
================
*/
void Snd_EnableSeqFx( s32 enable ) {
    s32 queued;

    queued = 1;
    if ( enable == 0 ) {
        queued = Snd_SetFxType( snd_fxType );
    }
    if ( queued ) {
        snd_seqFxEnabled = enable;
    }
}

/*
==============================================================================

SE TABLES AND QUERIES

==============================================================================
*/

/*
================
Snd_LoadSeTable

Turns the offsets of SE table data into pointers, once, and makes it the default if there is none. Its wave
table is cleared; Snd_SetSeWaveBank sets it.
================
*/
void Snd_LoadSeTable( void *data ) {
    s32 i;
    seTable_t *table;

    table = data;
    if ( !( table->flags & SETABLE_RELOCATED ) ) {
        if ( snd_seTable == NULL ) {
            snd_seTable = table;
        }
        table->flags = SETABLE_RELOCATED;
        table->waveBank = 0;
        table->programs = (u16 *)( (u8 *)table + (s32)table->programs );
        for ( i = 0; i < table->numEntries; i++ ) {
            table->entries[i].cmds = (u8 *)data + (s32)table->entries[i].cmds;
        }
    }
}

/*
================
func_8003C1C0

Returns table->unk_04.
================
*/
s32 func_8003C1C0( seTable_t *table ) {
    return table->unk_04;
}

/*
================
Snd_SetSeTable
================
*/
void Snd_SetSeTable( seTable_t *table ) {
    snd_seTable = table;
}

/*
================
Snd_SetNextSeTable

Sets the SE table of the next Snd_PlaySe only.
================
*/
void Snd_SetNextSeTable( seTable_t *table ) {
    snd_nextSeTable = table;
}

/*
================
Snd_GetSeTable
================
*/
seTable_t *Snd_GetSeTable( void ) {
    return snd_seTable;
}

/*
================
Snd_SetSeWaveBank

Sets the wave bank of an SE table's sounds.
================
*/
void Snd_SetSeWaveBank( seTable_t *table, waveBank_t *bank ) {
    table->waveBank = bank;
}

/*
================
Snd_GetSeWaveBank
================
*/
waveBank_t *Snd_GetSeWaveBank( seTable_t *table ) {
    return table->waveBank;
}

/*
================
func_8003C1FC

Sets the task routines the audio thread uses (D_8005CE40 by default).
================
*/
void func_8003C1FC( audioTaskFuncs_t *funcs ) {
    D_8005CE4C = funcs;
}

#ifdef NON_MATCHING
// NON_MATCHING: registers: retail loads snd_numChannels into v1 and snd_channels into v0 (swapped here); the seTable_t cast reads unk_08 at the same offset
/*
================
func_8003C208

Returns unk_08 of the sequence (or else the SE table) of the first channel of sound id, or 0.
================
*/
s32 func_8003C208( s32 id ) {
    s32 i;
    sndChannel_t *chan;
    seqHeader_t *hdr;

    if ( id != 0 ) {
        for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
            if ( chan->id == id ) {
                hdr = chan->seq;
                if ( hdr == NULL ) {
                    hdr = (seqHeader_t *)chan->seTable;
                }
                return hdr->unk_08;
            }
        }
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C208);
#endif

/*
================
Snd_GetProgramTable

Returns the program-to-wave table of the sequence, or else the SE table, of sound id, or NULL.
================
*/
u16 *Snd_GetProgramTable( s32 id ) {
    s32 i;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return NULL;
    }
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            if ( chan->seq != NULL ) {
                return chan->seq->programs;
            }
            return chan->seTable->programs;
        }
    }
    return NULL;
}

/*
================
Snd_SetMarkerCallback

Sets the function Snd_CmdMarker calls with the sound id and the marker value.
================
*/
void Snd_SetMarkerCallback( void (*callback)( s32 id, s32 value ) ) {
    snd_markerCallback = callback;
}

/*
==============================================================================

MESSAGES

==============================================================================
*/

/*
================
Snd_InitMessages

Allocates the message ring buffer with count entries, clamped to [MIN_MSGS, MAX_MSGS].
================
*/
void Snd_InitMessages( s32 count ) {
    if ( count < MIN_MSGS ) {
        count = MIN_MSGS;
    } else if ( count > MAX_MSGS ) {
        count = MAX_MSGS;
    }
    snd_msgs = Snd_Alloc( count * sizeof( sndMsg_t ) );
    snd_numMsgs = count;
    snd_msgWrite = 0;
    snd_msgRead = 0;
}

/*
================
Snd_HandleMessage

Runs one posted message.
================
*/
void Snd_HandleMessage( sndMsg_t *msg ) {
    switch ( msg->cmd ) {
    case SNDMSG_PAUSE:
        Snd_SetChannelFlags( msg->arg, ~CHAN_PAUSED, CHAN_PAUSED );
        break;
    case SNDMSG_RESUME:
        Snd_SetChannelFlags( msg->arg, ~CHAN_PAUSED, 0 );
        break;
    case SNDMSG_SETFX:
        func_8003F340( msg->arg );
        break;
    }
}

/*
================
Snd_PostMessage

Appends msg to the ring buffer, to run on the next tick; returns 0 if it is full, else 1.
================
*/
s32 Snd_PostMessage( sndMsg_t *msg ) {
    s32 next;

    next = ( snd_msgWrite + 1 ) % snd_numMsgs;
    if ( next == snd_msgRead ) {
        return 0;
    }
    Snd_MemMove( &snd_msgs[snd_msgWrite], msg, sizeof( sndMsg_t ) );
    snd_msgWrite = next;
    return 1;
}

/*
==============================================================================

PLAYER AND VOICES

==============================================================================
*/

/*
================
Snd_PlayerHandler

The synthesizer calls this once a tick: it runs the posted messages, then advances every unpaused channel by
one tick (pending voice start, notes and commands, step streams, fade out, envelope, auto pan, vibrato, trill,
then the voice's pitch, volume and pan). Returns the time to the next call, in microseconds.
================
*/
ALMicroTime Snd_PlayerHandler( void *clientData ) {
    s32 i;
    sndChannel_t *chan;
    f32 pitch;

    while ( snd_msgRead != snd_msgWrite ) {
        Snd_HandleMessage( &snd_msgs[snd_msgRead] );
        if ( ++snd_msgRead == snd_numMsgs ) {
            snd_msgRead = 0;
        }
    }
    for ( i = -NUM_MASTER_CHANNELS, chan = snd_channels; i < snd_numChannels - NUM_MASTER_CHANNELS; i++, chan++ ) {
        if ( chan->cmdPos == NULL || ( chan->flags & CHAN_PAUSED ) ) {
            continue;
        }
        if ( chan->pendingWave != NULL ) {
            Snd_StartVoice( chan, i );
        }
        chan->time += chan->tempo;
        if ( chan->noteLength != NOTE_LENGTH_HOLD ) {
            while ( chan->nextNoteTime - chan->time < 0 ) {
                if ( chan->cmdPos == NULL ) {
                    break;
                }
                Snd_PlayNextNote( chan, i );
            }
            if ( chan->cmdPos == NULL ) {
                continue;
            }
        }
        if ( chan->volStream != NULL && chan->volStreamTime - chan->time < 0 ) {
            Snd_StepVolumeStream( chan );
        }
        if ( chan->bendStream != NULL && chan->bendStreamTime - chan->time < 0 ) {
            Snd_StepBendStream( chan );
        }
        if ( chan->fadeLeft != FADE_NONE && --chan->fadeLeft == FADE_NONE ) {
            chan->volStream = NULL;
            chan->bendStream = NULL;
            chan->seq = NULL;
            chan->seTable = NULL;
            chan->id = 0;
            chan->pendingWave = NULL;
            chan->cmdPos = NULL;
            if ( chan->voiceActive ) {
                chan->voiceActive = 0;
                n_alSynStopVoice( &snd_voices[i] );
            }
        }
        if ( chan->voiceActive ) {
            if ( chan->envPhase ) {
                Snd_UpdateEnvelope( chan );
            }
            if ( chan->autoPanSpeed && chan->autoPanTime - chan->time < 0 ) {
                Snd_UpdateAutoPan( chan );
            }
            pitch = chan->pitchOffset;
            if ( chan->vibratoPeriod ) {
                pitch += Snd_UpdateVibrato( chan );
            }
            if ( chan->trillOnTicks ) {
                pitch += Snd_UpdateTrill( chan );
            }
            if ( chan->pendingWave == NULL ) {
                Snd_UpdatePitch( chan, i, pitch );
                Snd_UpdateVolumePan( chan, i );
            }
        }
        chan->noteAge = (u32)( chan->time - chan->noteTime ) >> 8;
    }
    return snd_tickTime;
}

/*
================
Snd_PlayNextNote

Runs the channel's commands up to its next note, then plays it: velocity, length, drum map entry, envelope
and voice restart (unless held), wave, pitch and FX mix. NOTE_REST, or a program without a wave, releases the
note instead. A channel whose stream ended stops its voice.
================
*/
void Snd_PlayNextNote( sndChannel_t *chan, s32 index ) {
    u8 *src;
    s32 note;
    // RULE-EXCEPTION: R30, the command, length and transpose bytes share one local; separate locals
    // change the register allocation.
    u8 byte;
    u8 value;
    u16 sound;
    waveBank_t *bank;
    u8 fx;

    src = chan->cmdPos;
    if ( src != NULL ) {
    next:
        byte = *src;
        if ( byte >= CMD_FIRST ) {
            src = snd_commands[byte & 0x7F]( chan, src + 1 );
            if ( src != NULL ) {
                goto next;
            }
        }
    }
    chan->cmdPos = src;
    if ( src != NULL ) {
        chan->glideFrom = chan->glidePitch;
        note = *chan->cmdPos++;
        if ( chan->noteVelocity ) {
            value = *chan->cmdPos++;
            chan->velocity = value;
            if ( value >= 0x80 ) {
                chan->velocity = value & 0x7F;
                chan->noteVelocity = 0;
                chan->fixedVelocity = chan->velocity;
            }
        } else {
            chan->velocity = chan->fixedVelocity;
        }
        if ( chan->fixedLength != 0 ) {
            if ( chan->explicitLength == 0 ) {
                chan->noteLength = chan->fixedLength;
            } else {
                chan->explicitLength = 0;
                byte = *chan->cmdPos++;
                if ( byte < 0x80 ) {
                    chan->noteLength = byte;
                } else {
                    chan->noteLength = *chan->cmdPos++ + ( ( byte & 0x7F ) << 8 );
                }
            }
        } else {
            byte = *chan->cmdPos++;
            if ( byte < 0x80 ) {
                chan->noteLength = byte;
            } else {
                chan->noteLength = *chan->cmdPos++ + ( ( byte & 0x7F ) << 8 );
            }
        }
        chan->noteTime = chan->nextNoteTime;
        chan->nextNoteTime += chan->noteLength << 8;
        chan->noteAge = 0;
        chan->trillPitch = 0;
        chan->trillWait = chan->trillOffTicks;
        if ( chan->seq != NULL && chan->drumMap == NULL && chan->seq->programs[chan->program] == PROGRAM_NONE ) {
            note = NOTE_REST;
        }
        if ( note != NOTE_REST ) {
            bank = chan->waveBank;
            if ( chan->drumMap != NULL ) {
                chan->program = chan->drumMap[note].program;
                chan->pan = chan->drumMap[note].pan >> 1;
                SetEnvelope( chan, &chan->seq->envelopes[chan->drumMap[note].envelope].interval );
                note = chan->drumMap[note].note;
            }
            if ( !chan->noEnvRestart ) {
                Snd_StartEnvelope( chan );
            }
            if ( chan->autoPanSpeed ) {
                Snd_StartAutoPan( chan );
            }
            sound = chan->program;
            if ( chan->seq != NULL ) {
                sound = chan->seq->programs[sound];
            } else {
                sound = chan->seTable->programs[sound];
            }
            if ( !chan->noVoiceRestart ) {
                chan->pendingWave = bank->waves[sound];
                if ( chan->voiceActive && chan->lastVolume != 0 ) {
                    chan->lastVolume = 0;
                    n_alSynSetVol( &snd_voices[index], 0, snd_tickTime );
                } else {
                    Snd_StartVoice( chan, index );
                }
            }
            chan->notePitch = note + bank->tunes[sound].value;
            byte = chan->transpose * ( 1 - chan->transposeOff );
            chan->notePitch += SEXT8( byte );
            fx = chan->fxMix;
            if ( fx != chan->lastFxMix ) {
                chan->lastFxMix = fx;
                n_alSynSetFXMix( &snd_voices[index], chan->fxBase + ( ( ( 0x80 - chan->fxBase ) * fx ) >> 7 ) );
            }
        } else if ( chan->envPhase < ENV_RELEASE ) {
            chan->envPhase = ENV_RELEASE;
            chan->envWait = 1;
            chan->releaseTime = chan->time;
            chan->envReleaseLevel = chan->envLevel;
        }
    } else if ( chan->voiceActive ) {
        chan->voiceActive = 0;
        n_alSynSetVol( &snd_voices[index], 0, snd_tickTime );
        n_alSynStopVoice( &snd_voices[index] );
    }
}

/*
================
Snd_StartVoice

Stops voice index if the channel had started it, then starts it on the pending wave.
================
*/
void Snd_StartVoice( sndChannel_t *chan, s32 index ) {
    if ( chan->voiceActive ) {
        n_alSynStopVoice( &snd_voices[index] );
    }
    chan->voiceActive = 1;
    n_alSynStartVoice( &snd_voices[index], chan->pendingWave );
    chan->pendingWave = NULL;
}

/*
================
Snd_UpdateVolumePan

Sends the volume (track volume * envelope * velocity * volume scale, then the group's master volume and the
fade) and the pan (pan * pan scale) of voice index, each only when it changed.
================
*/
void Snd_UpdateVolumePan( sndChannel_t *chan, s32 index ) {
    // RULE-EXCEPTION: R30, the volume and the pan share one local; separate locals change the register
    // allocation.
    u32 value;

    value = (u32)( chan->trackVolume * chan->envLevel * chan->velocity * chan->playVolume ) >> 13;
    if ( value > VOLUME_MAX ) {
        value = VOLUME_MAX;
    }
    value *= chan->seTable == NULL ? snd_bgmVolume : snd_seVolume;
    value >>= 15;
    if ( chan->fadeLeft != FADE_NONE ) {
        value = value * chan->fadeLeft / chan->fadeTotal;
    }
    if ( value != chan->lastVolume ) {
        chan->lastVolume = value;
        n_alSynSetVol( &snd_voices[index], (s16)value, snd_tickTime );
    }
    value = ( ( chan->pan * chan->playPan ) >> 7 ) & 0x7F;
    if ( value != chan->lastPan ) {
        chan->lastPan = value;
        n_alSynSetPan( &snd_voices[index], value );
    }
}

/*
================
Snd_UpdatePitch

Sends the pitch of voice index: the note pitch (gliding from glideFrom over the portamento ticks) plus offset
and the bend, in semitones, as a ratio; only when it changed. A ratio above 2 is clamped and silences the note.
================
*/
void Snd_UpdatePitch( sndChannel_t *chan, s32 index, f32 offset ) {
    f32 pitch;

    pitch = chan->notePitch;
    if ( chan->portamento != 0 ) {
        if ( chan->portamento < chan->noteAge ) {
            chan->glidePitch = pitch;
        } else {
            pitch = chan->glideFrom + ( pitch - chan->glideFrom ) / chan->portamento * chan->noteAge;
            chan->glidePitch = pitch;
        }
    }
    pitch += offset + chan->bendPitch;
    if ( pitch != chan->lastPitch ) {
        chan->lastPitch = pitch;
        pitch = Snd_Exp2( pitch * ( 1.0 / 12 ) );
        if ( pitch > 2.0 ) {
            pitch = 2.0f;
            chan->velocity = 0;
        }
        n_alSynSetPitch( &snd_voices[index], pitch );
    }
}

/*
==============================================================================

ENVELOPE AND MODULATION

==============================================================================
*/

/*
================
Snd_StartEnvelope

Starts the envelope at the note: the release time from releaseAfter, or else from releaseBefore, or never for
a held note; then ENV_ATTACK from the start level.
================
*/
void Snd_StartEnvelope( sndChannel_t *chan ) {
    if ( chan->noteLength != NOTE_LENGTH_HOLD ) {
        if ( chan->releaseAfter != 0 ) {
            chan->releaseTime = chan->noteTime + ( chan->releaseAfter << 8 );
        } else {
            chan->releaseTime = chan->nextNoteTime - ( chan->releaseBefore << 8 );
        }
    } else {
        chan->releaseTime = chan->noteTime + 0x7FFFFFFF;
    }
    chan->envPhase = ENV_ATTACK;
    chan->envLevel = chan->envStartLevel;
    chan->envWait = chan->envInterval;
}

/*
================
Snd_UpdateEnvelope

Steps the envelope every envInterval ticks through ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN and ENV_RELEASE, which
it enters once the release time has passed.
================
*/
void Snd_UpdateEnvelope( sndChannel_t *chan ) {
    s32 time;

    if ( chan->releaseTime - chan->time < 0 && chan->envPhase < ENV_RELEASE ) {
        chan->envPhase = ENV_RELEASE;
        chan->envWait = 1;
        chan->envReleaseLevel = chan->envLevel;
    }
    if ( --chan->envWait == 0 ) {
        chan->envWait = chan->envInterval;
        switch ( chan->envPhase ) {
        case ENV_ATTACK:
            time = ( (u32)( chan->time - chan->noteTime ) >> 8 ) * chan->envTimeScale >> 10;
            if ( time < chan->envAttackTime ) {
                chan->envLevel = chan->envStartLevel + (s32)( chan->attackSlope * time );
            } else {
                chan->envPhase++;
                chan->envLevel = chan->envPeakLevel;
            }
            break;
        case ENV_DECAY:
            time = ( ( (u32)( chan->time - chan->noteTime ) >> 8 ) - chan->envAttackTime ) * chan->envTimeScale >> 10;
            if ( time < chan->envDecayTime ) {
                chan->envLevel = chan->envPeakLevel + (s32)( chan->decaySlope * time );
            } else {
                chan->envPhase++;
                chan->envLevel = chan->envSustainLevel;
            }
            break;
        case ENV_SUSTAIN:
            break;
        case ENV_RELEASE:
            time = ( (u32)( chan->time - chan->releaseTime ) >> 8 ) * chan->envTimeScale >> 10;
            if ( time < chan->envReleaseTime ) {
                chan->envLevel = chan->envReleaseLevel - (s32)( chan->releaseSlope * time * chan->envReleaseLevel );
            } else {
                chan->envPhase++;
                chan->envLevel = 0;
            }
            break;
        }
    }
}

/*
================
Snd_StartAutoPan

Restarts the pan sweep at the note, going down if the pan is right of centre.
================
*/
void Snd_StartAutoPan( sndChannel_t *chan ) {
    chan->autoPanFrac = 0;
    chan->autoPanTime = chan->noteTime;
    chan->autoPanDown = chan->pan & PAN_CENTRE;
}

/*
================
Snd_UpdateAutoPan

Advances the pan sweep, one step per 0x100 of time up to the channel's time, autoPanSpeed / 64 pan units per
step, turning at 0 and 0x7F.
================
*/
void Snd_UpdateAutoPan( sndChannel_t *chan ) {
    u32 step;

    do {
        step = chan->autoPanFrac + chan->autoPanSpeed;
        chan->autoPanTime += 0x100;
        if ( step < 0x40 ) {
            chan->autoPanFrac = step;
        } else {
            chan->autoPanFrac = step & 0x3F;
            step >>= 6;
            if ( !chan->autoPanDown ) {
                chan->pan += step;
                if ( chan->pan >= 0x80 ) {
                    chan->pan = 0x7F;
                    chan->autoPanDown = 1;
                }
            } else {
                chan->pan -= step;
                if ( chan->pan >= 0x80 || chan->pan == 0 ) {
                    chan->pan = 0;
                    chan->autoPanDown = 0;
                }
            }
        }
    } while ( chan->autoPanTime - chan->time < 0 );
}

/*
================
Snd_UpdateTrill

Counts down the trill; at zero switches between 0 (for trillOffTicks) and trillInterval (for trillOnTicks).
Returns the trill pitch.
================
*/
f32 Snd_UpdateTrill( sndChannel_t *chan ) {
    if ( --chan->trillWait == 0 ) {
        if ( chan->trillPitch == 0 ) {
            chan->trillPitch = chan->trillInterval;
            chan->trillWait = chan->trillOnTicks;
        } else {
            chan->trillPitch = 0;
            chan->trillWait = chan->trillOffTicks;
        }
    }
    return chan->trillPitch;
}

/*
================
Snd_UpdateVibrato

Returns the vibrato pitch, depth * sin( ( noteAge - delay ) * step ), and keeps it in vibratoPitch; 0 during the
delay.
================
*/
f32 Snd_UpdateVibrato( sndChannel_t *chan ) {
    s32 time;

    time = chan->noteAge - chan->vibratoDelay;
    if ( time > 0 ) {
        return chan->vibratoPitch = sinf( time * chan->vibratoStep ) * chan->vibratoDepth;
    }
    return 0.0f;
}

/*
================
Snd_StepVolumeStream

Advances the volume stream, one step per 0x100 of time up to the channel's time. Each entry is a track volume,
held one step; with bit 7 set, a length (one byte, or two when it has bit 7 set) follows, plus 2 steps.
================
*/
void Snd_StepVolumeStream( sndChannel_t *chan ) {
    u8 value;

    do {
        chan->volStreamTime += 0x100;
        if ( --chan->volStreamWait == 0 ) {
            value = *chan->volStream++;
            if ( value >= 0x80 ) {
                chan->trackVolume = value & 0x7F;
                value = *chan->volStream++;
                if ( value >= 0x80 ) {
                    chan->volStreamWait = ( value & 0x7F ) << 8;
                    chan->volStreamWait += *chan->volStream++ + 2;
                } else {
                    chan->volStreamWait = value + 2;
                }
            } else {
                chan->trackVolume = value;
                chan->volStreamWait = 1;
            }
        }
    } while ( chan->volStreamTime - chan->time < 0 );
}

/*
================
Snd_StepBendStream

Advances the pitch bend stream as Snd_StepVolumeStream does the volume stream; entries are bends, biased by 64.
================
*/
void Snd_StepBendStream( sndChannel_t *chan ) {
    u8 value;

    do {
        chan->bendStreamTime += 0x100;
        if ( --chan->bendStreamWait == 0 ) {
            value = *chan->bendStream++;
            if ( value >= 0x80 ) {
                chan->bend = (f32)( value & 0x7F ) - 64.0;
                chan->bendPitch = chan->bend * chan->bendRange;
                value = *chan->bendStream++;
                if ( value >= 0x80 ) {
                    chan->bendStreamWait = ( value & 0x7F ) << 8;
                    chan->bendStreamWait += *chan->bendStream++ + 2;
                } else {
                    chan->bendStreamWait = value + 2;
                }
            } else {
                chan->bend = (f32)value - 64.0;
                chan->bendPitch = chan->bend * chan->bendRange;
                chan->bendStreamWait = 1;
            }
        }
    } while ( chan->bendStreamTime - chan->time < 0 );
}

/*
==============================================================================

UTILITIES

==============================================================================
*/

#ifdef NON_MATCHING
// NON_MATCHING: FPU registers: x lives in $f6 in retail, $f14 here, and the x^6 term takes a different register
/*
================
Snd_Exp2

Returns 2 to the power x, from the series sum of ( x ln 2 )^n / n! to n = 6; a negative
x takes the reciprocal.
================
*/
f32 Snd_Exp2( f32 x ) {
    f32 x2;
    f32 x3;
    f32 x4;
    f32 x5;
    f32 x6;

    if ( x == 0.0f ) {
        return 1.0f;
    }
    if ( x > 0.0f ) {
        x2 = x * x;
        x3 = x2 * x;
        x4 = x2 * x2;
        x5 = x4 * x;
        x6 = x4 * x2;
        return 1.0 + x * 0.69314718055994495 + x2 * 0.240226506959101 + x3 * 0.0555041086648216
               + x4 * 0.0096181291076284804 + x5 * 0.00133335581464284 + x6 * 0.000154035303933816;
    }
    x = -x;
    x2 = x * x;
    x3 = x2 * x;
    x4 = x2 * x2;
    x5 = x4 * x;
    x6 = x4 * x2;
    return 1.0 / ( 1.0 + x * 0.69314718055994495 + x2 * 0.240226506959101 + x3 * 0.0555041086648216
                   + x4 * 0.0096181291076284804 + x5 * 0.00133335581464284 + x6 * 0.000154035303933816 );
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", Snd_Exp2);
#endif

#ifdef NON_MATCHING
// NON_MATCHING: register/schedule: retail has a 0x30 frame (0x28 here), reloads tune->value before the add and copies the digit before SEXT8; an inline SetTune( tune, digit ) helper gave a worse schedule
/*
================
Snd_RelocateWaveBank

Turns the offsets of wave bank data into pointers, once (WAVEBANK_RELOCATED), converts its tunings to
semitones, and patches each wave table once: the sample address by romBase (a ROM address), unless its top byte
is 0xFF, and the loop and ADPCM book by the bank's address. Writes back the data cache.
================
*/
void Snd_RelocateWaveBank( void *data, s32 romBase ) {
    waveBank_t *bank;
    s32 i;
    s32 value;
    waveTune_t *tune;

    bank = data;
    if ( bank->flags < 0 ) {
        return;
    }
    bank->flags |= WAVEBANK_RELOCATED;
    Snd_RelocateTable( (s32 *)&bank->unk_24, (s32)bank, 3 );
    Snd_RelocateTable( (s32 *)bank->waves, (s32)bank, bank->numWaves );
    for ( i = 0; i < bank->numWaves; i++ ) {
        tune = &bank->tunes[i];
        value = tune->raw;
        tune->value = (f32)SEXT8( value ) / 100.0;
        value = bank->unk_24[i] - '0';
        tune->value += (f32)SEXT8( value );
        if ( bank->waves[i]->flags == 0 ) {
            if ( ( (u32)bank->waves[i]->base & 0xFF000000 ) != 0xFF000000 ) {
                bank->waves[i]->base += romBase;
            }
            bank->waves[i]->flags = 1;
            if ( bank->waves[i]->waveInfo.adpcmWave.loop != NULL ) {
                bank->waves[i]->waveInfo.adpcmWave.loop =
                    (ALADPCMloop *)( (u8 *)bank->waves[i]->waveInfo.adpcmWave.loop + (s32)data );
            }
            if ( bank->waves[i]->type == AL_ADPCM_WAVE ) {
                bank->waves[i]->waveInfo.adpcmWave.book =
                    (ALADPCMBook *)( (u8 *)bank->waves[i]->waveInfo.adpcmWave.book + (s32)data );
            }
        }
    }
    osWritebackDCacheAll();
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", Snd_RelocateWaveBank);
#endif

/*
================
Snd_Random

Returns a number in [0, range), from eight steps of the shift register snd_randomSeed.
================
*/
s32 Snd_Random( s32 range ) {
    s32 i;
    s32 bits;

    for ( i = 0; i < 8; i++ ) {
        bits = snd_randomSeed & 0x48000000;
        snd_randomSeed <<= 1;
        if ( bits == 0x48000000 || bits == 0x08000000 ) {
            snd_randomSeed |= 1;
        }
    }
    return range * ( (f32)snd_randomSeed / 65536.0f / 65536.0f );
}

/*
==============================================================================

CHANNEL ALLOCATION

==============================================================================
*/

#ifdef NON_MATCHING
// NON_MATCHING: registers: retail puts the 0xFF constant in a1, moving chan and started to a2/a3 (store order now matches)
/*
================
Snd_ResetChannel

Clears data, a channel, to its defaults, keeping only voiceActive: centre pan, full levels, scales of 1, a
120 bpm tempo, no fade and the next (or default) wave bank.
================
*/
void Snd_ResetChannel( sndChannel_t *data ) {
    sndChannel_t *chan;
    u8 started;
    s32 tempo;
    u32 i;

    chan = data;
    started = chan->voiceActive;
    chan->cmdPos = NULL;
    for ( i = 0; i < sizeof( sndChannel_t ); i++ ) {
        *( (u8 *)data )++ = 0;
    }
    chan->lastPan = chan->lastFxMix = RESEND;
    tempo = TEMPO_120_BPM / snd_tickRate;
    chan->fixedVelocity = 0x7F;
    chan->trackVolume = 0x7F;
    chan->envPeakLevel = 0x7F;
    chan->envDecayTime = 0xFF;
    chan->envSustainLevel = 0x7F;
    chan->pan = PAN_CENTRE;
    chan->envInterval = 1;
    chan->envAttackTime = 1;
    chan->envReleaseTime = 15;
    chan->lastVolume = 0xFFFF;
    chan->noteLength = 1;
    chan->volStreamWait = 1;
    chan->bendStreamWait = 1;
    chan->fadeLeft = FADE_NONE;
    chan->playVolume = SCALE_ONE;
    chan->playPan = SCALE_ONE;
    chan->tempoScale = SCALE_ONE;
    chan->lastPitch = 99.9f;
    chan->bendRange = 1.0f / 32;
    chan->attackSlope = 1.0f;
    chan->decaySlope = 1.0f / 255;
    chan->releaseSlope = 1.0f / 15;
    chan->baseTempo = tempo;
    chan->tempo = tempo;
    chan->waveBank = snd_nextWaveBank != NULL ? snd_nextWaveBank : snd_defaultWaveBank;
    chan->voiceActive = started;
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", Snd_ResetChannel);
#endif

/*
================
Snd_FindBgmChannel

Picks the channel for track of hdr (TRACK_MASTER: the master stream, which first tries the master channels).
Takes a free voice channel, else the SE channel of lowest priority, else a BGM channel of another sequence,
else the channel already on this track, else one by track number.
================
*/
s32 Snd_FindBgmChannel( seqHeader_t *hdr, s32 track ) {
    s32 i;
    s32 best;
    s32 lowest;
    sndChannel_t *chan;

    if ( track < 0 ) {
        for ( i = 0, chan = snd_channels; i < NUM_MASTER_CHANNELS; i++, chan++ ) {
            if ( chan->cmdPos == NULL ) {
                return i;
            }
        }
    }
    for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos == NULL ) {
            return i;
        }
    }
    for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels, lowest = 0x7FFFFFFF, best = NUM_MASTER_CHANNELS - 1; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seTable != NULL && chan->priority <= lowest ) {
            lowest = chan->priority;
            best = i;
        }
    }
    if ( best >= NUM_MASTER_CHANNELS ) {
        return best;
    }
    // BUG: chan is never advanced here, so every pass tests the first voice channel
    for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels; i < snd_numChannels; i++ ) {
        if ( chan->seTable == NULL && chan->seq != hdr ) {
            return i;
        }
    }
    for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seq == hdr && hdr->trackCmds[track] == chan->cmdStart ) {
            return i;
        }
    }
    return track % ( snd_numChannels - NUM_MASTER_CHANNELS ) + NUM_MASTER_CHANNELS;
}

/*
================
Snd_RelocateTable

Adds base to every non-zero entry of a table of count offsets.
================
*/
void Snd_RelocateTable( s32 *table, s32 base, s32 count ) {
    s32 i;

    for ( i = 0; i < count; i++ ) {
        if ( table[i] != 0 ) {
            table[i] += base;
        }
    }
}

/*
================
Snd_StartSeOnChannel

Resets chan and starts entry index of SE table table on it with the given volume and pan scales and priority;
returns the new sound id.
================
*/
s32 Snd_StartSeOnChannel( sndChannel_t *chan, seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority ) {
    Snd_ResetChannel( chan );
    chan->seIndex = index;
    chan->seTable = table;
    chan->playVolume = volume;
    chan->playPan = pan;
    chan->id = snd_nextId++;
    chan->priority = priority;
    if ( table->waveBank != 0 ) {
        chan->waveBank = table->waveBank;
    }
    chan->cmdPos = chan->cmdStart = table->entries[index].cmds;
    return chan->id;
}

/*
================
Snd_StartSe

Starts entry index of SE table table on a free voice channel, or else on the SE channel of lowest priority
below priority (SE_PRIORITY_DEFAULT: the entry's own). Returns the new sound id, or 0.
================
*/
s32 Snd_StartSe( seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority ) {
    s32 i;
    s32 lowest;
    sndChannel_t *chan;
    sndChannel_t *victim;

    if ( priority == SE_PRIORITY_DEFAULT ) {
        priority = table->entries[index].priority;
    }
    lowest = priority + 1;
    for ( i = NUM_MASTER_CHANNELS, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos == NULL ) {
            Snd_ResetChannel( chan );
            chan->seIndex = index;
            chan->seTable = table;
            chan->playVolume = volume;
            chan->playPan = pan;
            chan->priority = priority;
            chan->id = snd_nextId++;
            if ( table->waveBank != NULL ) {
                chan->waveBank = table->waveBank;
            }
            chan->cmdPos = chan->cmdStart = table->entries[index].cmds;
            return chan->id;
        }
        if ( chan->seTable != NULL && chan->priority < lowest ) {
            lowest = chan->priority;
            victim = chan;
        }
    }
    if ( lowest < priority ) {
        Snd_ResetChannel( victim );
        victim->seIndex = index;
        victim->seTable = table;
        victim->playVolume = volume;
        victim->playPan = pan;
        victim->priority = priority;
        victim->id = snd_nextId++;
        if ( table->waveBank != NULL ) {
            victim->waveBank = table->waveBank;
        }
        victim->cmdPos = victim->cmdStart = table->entries[index].cmds;
        return victim->id;
    }
    return 0;
}

#ifdef NON_MATCHING
// NON_MATCHING: registers: retail inlines Snd_RelocateTable (declaring it inline gives retail's 0x50 frame and loops), then keeps a separate base copy per relocation loop and swaps s1/s2
/*
================
Snd_StartBgm

Relocates hdr once, then starts its master stream and each of its tracks on a channel, all paused and under one
new sound id, which it returns.
================
*/
s32 Snd_StartBgm( seqHeader_t *hdr ) {
    s32 count;
    s32 id;
    s32 i;
    sndChannel_t *chan;

    count = hdr->numTracks;
    if ( hdr->relocated == 0 ) {
        hdr->relocated = 1;
        Snd_RelocateTable( (s32 *)&hdr->trackCmds, (s32)hdr, 7 );
        Snd_RelocateTable( (s32 *)hdr->trackCmds, (s32)hdr, count );
        Snd_RelocateTable( (s32 *)hdr->trackVolStreams, (s32)hdr, count );
        Snd_RelocateTable( (s32 *)hdr->trackBendStreams, (s32)hdr, count );
    }
    id = snd_nextId++;
    chan = &snd_channels[Snd_FindBgmChannel( hdr, TRACK_MASTER )];
    Snd_ResetChannel( chan );
    chan->noteVelocity = 1;
    chan->seq = hdr;
    chan->flags |= CHAN_PAUSED | CHAN_MASTER;
    chan->id = id;
    chan->cmdPos = chan->cmdStart = hdr->masterCmds;
    for ( i = 0; i < count; i++ ) {
        if ( hdr->trackCmds[i] != NULL ) {
            chan = &snd_channels[Snd_FindBgmChannel( hdr, i )];
            Snd_ResetChannel( chan );
            chan->noteVelocity = 1;
            chan->seq = hdr;
            chan->flags |= CHAN_PAUSED;
            chan->volStream = chan->volStreamStart = hdr->trackVolStreams[i];
            chan->bendStream = chan->bendStreamStart = hdr->trackBendStreams[i];
            chan->id = id;
            chan->cmdPos = chan->cmdStart = hdr->trackCmds[i];
        }
    }
    snd_nextWaveBank = NULL;
    return id;
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", Snd_StartBgm);
#endif

/*
================
Snd_SetChannelFlags

Sets flags to ( flags & mask ) | bits on every channel of sound id.
================
*/
void Snd_SetChannelFlags( s32 id, s32 mask, s32 bits ) {
    s32 i;
    sndChannel_t *chan;

    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id ) {
            chan->flags = ( chan->flags & mask ) | bits;
        }
    }
}

/*
==============================================================================

COMMANDS

==============================================================================
*/

/*
================
Snd_CmdEnd

Command 0x80: ends the stream and frees the channel; its voice is stopped by Snd_PlayNextNote.
================
*/
u8 *Snd_CmdEnd( sndChannel_t *chan, u8 *src ) {
    chan->volStream = NULL;
    chan->bendStream = NULL;
    chan->seq = NULL;
    chan->seTable = NULL;
    chan->id = 0;
    chan->pendingWave = NULL;
    return NULL;
}

/*
================
Snd_CmdProgram

Command 0x81 <program:1-2>.
================
*/
u8 *Snd_CmdProgram( sndChannel_t *chan, u8 *src ) {
    u32 value = *src++;

    if ( value & 0x80 ) {
        value = *src++ | ( ( value & 0x7F ) << 8 );
    }
    chan->program = value;
    return src;
}

/*
================
Snd_CmdPortamento

Command 0x82 <ticks>: glides each note from the previous one over ticks.
================
*/
u8 *Snd_CmdPortamento( sndChannel_t *chan, u8 *src ) {
    chan->portamento = *src++;
    if ( chan->portamento != 0 ) {
        chan->glidePitch = chan->notePitch;
    }
    return src;
}

/*
================
Snd_CmdPortamentoOff

Command 0x83.
================
*/
u8 *Snd_CmdPortamentoOff( sndChannel_t *chan, u8 *src ) {
    chan->portamento = 0;
    return src;
}

/*
================
Snd_CmdEnvelope

Command 0x84 <envelope_t>: an envelope inline.
================
*/
u8 *Snd_CmdEnvelope( sndChannel_t *chan, u8 *src ) {
    s32 value;
    s32 level;

    // RULE-EXCEPTION: R16, SetEnvelope( chan, src ) inlined here folds the read position into offsets;
    // retail advances src byte by byte, so this copy of the helper's body was written out.
    value = *src++;
    if ( value == 0 ) {
        value = 1;
    }
    chan->envInterval = value;
    chan->envTimeScale = 0x400 / value;
    chan->envStartLevel = *src++;
    value = *src++;
    chan->envAttackTime = value;
    level = *src++;
    chan->attackSlope = 1.0 / (f32)value * (f32)( level - chan->envStartLevel );
    chan->envPeakLevel = level;
    value = *src++;
    chan->envDecayTime = value;
    level = *src++;
    chan->decaySlope = 1.0 / (f32)value * (f32)( level - chan->envPeakLevel );
    chan->envSustainLevel = level;
    value = *src++;
    chan->envReleaseTime = value;
    chan->releaseSlope = 1.0 / (f32)value;
    return src;
}

/*
================
Snd_CmdTempo

Command 0x85 <bpm>: sets the tempo. An SE channel takes it alone, unscaled; on a BGM channel every channel of
the sequence takes it, with this channel's tempo scale.
================
*/
u8 *Snd_CmdTempo( sndChannel_t *chan, u8 *src ) {
    s32 tempo;
    s32 scaled;
    s32 i;
    sndChannel_t *other;

    tempo = ( *src++ * TEMPO_120_BPM / 120 ) / snd_tickRate;
    scaled = ( tempo * chan->tempoScale ) >> 7;
    if ( chan->seTable != NULL ) {
        chan->tempo = tempo;
    } else {
        for ( i = 0, other = snd_channels; i < snd_numChannels; i++, other++ ) {
            if ( other->seq == chan->seq ) {
                other->baseTempo = tempo;
                other->tempo = scaled;
            }
        }
    }
    return src;
}

/*
================
Snd_CmdReleaseBefore

Command 0x87 <ticks>: notes release ticks before they end.
================
*/
u8 *Snd_CmdReleaseBefore( sndChannel_t *chan, u8 *src ) {
    u8 value;

    value = *src++;
    chan->releaseAfter = 0;
    chan->releaseBefore = value;
    return src;
}

/*
================
Snd_CmdReleaseAfter

Command 0x86 <ticks:2>: notes release ticks after they start.
================
*/
u8 *Snd_CmdReleaseAfter( sndChannel_t *chan, u8 *src ) {
    s32 hi;
    s32 lo;

    hi = *src++;
    lo = *src++;
    chan->releaseBefore = 0;
    chan->releaseAfter = lo | ( hi << 8 );
    return src;
}

/*
================
Snd_CmdVibrato

Command 0x88 <delay> <period> <depth>: vibrato after delay ticks, period ticks long, depth / 50 semitones.
================
*/
u8 *Snd_CmdVibrato( sndChannel_t *chan, u8 *src ) {
    chan->vibratoDelay = *src++;
    chan->vibratoPeriod = *src++;
    chan->vibratoDepth = (f32)*src++ / 50.0;
    chan->vibratoStep = 6.2831852 / (f32)chan->vibratoPeriod;
    return src;
}

#ifdef NON_MATCHING
// NON_MATCHING: schedule: retail reloads vibratoPeriod right after storing it and interleaves the two divisions
/*
================
Snd_CmdVibratoInverted

Command 0x89 <delay> <period> <depth>: as 0x88, starting downwards.
================
*/
u8 *Snd_CmdVibratoInverted( sndChannel_t *chan, u8 *src ) {
    chan->vibratoDelay = *src++;
    chan->vibratoStep = 6.2831852 / (f32)( chan->vibratoPeriod = *src++ );
    chan->vibratoDepth = -(f32)*src++ / 50.0;
    return src;
}
#else
INCLUDE_ASM("asm/nonmatchings/166B0", Snd_CmdVibratoInverted);
#endif

/*
================
Snd_CmdVibratoOff

Command 0x8A.
================
*/
u8 *Snd_CmdVibratoOff( sndChannel_t *chan, u8 *src ) {
    chan->vibratoPeriod = 0;
    chan->vibratoPitch = 0;
    return src;
}

/*
================
Snd_CmdFixedLength

Command 0x8B <ticks:1-2>: notes no longer carry a length and last ticks; 0 ends that.
================
*/
u8 *Snd_CmdFixedLength( sndChannel_t *chan, u8 *src ) {
    s32 value;

    value = *src++;
    if ( value >= 0x80 ) {
        value &= 0x7F;
        value <<= 8;
        value |= *src++;
    }
    chan->fixedLength = value;
    return src;
}

/*
================
Snd_CmdExplicitLength

Command 0x8C: the next note carries a length despite a fixed length.
================
*/
u8 *Snd_CmdExplicitLength( sndChannel_t *chan, u8 *src ) {
    chan->explicitLength = 1;
    return src;
}

/*
================
Snd_CmdTranspose

Command 0x8D <semitones>: signed.
================
*/
u8 *Snd_CmdTranspose( sndChannel_t *chan, u8 *src ) {
    chan->transpose = *src++;
    return src;
}

/*
================
Snd_CmdTransposeOff

Command 0x8E: notes ignore the transposition from now on.
================
*/
u8 *Snd_CmdTransposeOff( sndChannel_t *chan, u8 *src ) {
    chan->transposeOff = 1;
    return src;
}

/*
================
Snd_CmdDetune

Command 0x8F <hundredths>: signed hundredths of a semitone.
================
*/
u8 *Snd_CmdDetune( sndChannel_t *chan, u8 *src ) {
    s32 value;
    f32 offset;

    value = *src++;
    if ( value & 0x80 ) {
        value |= ~0xFF;
    }
    offset = (f32)value / 100.0;
    chan->pitchOffset = chan->pitchOffset - chan->detune + offset;
    chan->detune = offset;
    return src;
}

/*
================
Snd_CmdEnvRestartOff

Command 0x91: notes keep the running envelope.
================
*/
u8 *Snd_CmdEnvRestartOff( sndChannel_t *chan, u8 *src ) {
    chan->noEnvRestart = 1;
    return src;
}

/*
================
Snd_CmdEnvRestartOn

Command 0x92.
================
*/
u8 *Snd_CmdEnvRestartOn( sndChannel_t *chan, u8 *src ) {
    chan->noEnvRestart = 0;
    return src;
}

/*
================
Snd_CmdVoiceRestartOff

Command 0x93: notes keep the running voice and wave.
================
*/
u8 *Snd_CmdVoiceRestartOff( sndChannel_t *chan, u8 *src ) {
    chan->noVoiceRestart = 1;
    return src;
}

/*
================
Snd_CmdVoiceRestartOn

Command 0x94.
================
*/
u8 *Snd_CmdVoiceRestartOn( sndChannel_t *chan, u8 *src ) {
    chan->noVoiceRestart = 0;
    return src;
}

/*
================
Snd_CmdLoopStart

Command 0x95 <count>: pushes a loop of count passes (LOOP_FOREVER: endless), saving the read position and the
volume and bend streams.
================
*/
u8 *Snd_CmdLoopStart( sndChannel_t *chan, u8 *src ) {
    s32 depth;

    depth = chan->loopDepth;
    chan->loopCount[depth] = *src++;
    chan->loopPos[depth] = src;
    chan->loopVolStream[depth] = chan->volStream;
    chan->loopBendStream[depth] = chan->bendStream;
    chan->loopVolume[depth] = chan->trackVolume;
    chan->loopBend[depth] = chan->bend;
    chan->loopVolWait[depth] = chan->volStreamWait;
    chan->loopBendWait[depth] = chan->bendStreamWait;
    chan->loopDepth++;
    return src;
}

/*
================
Snd_CmdLoopEnd

Command 0x96: ends a pass of the innermost loop; jumps back to its start and restores the streams unless it was
the last pass, which pops the loop.
================
*/
u8 *Snd_CmdLoopEnd( sndChannel_t *chan, u8 *src ) {
    s32 depth;

    depth = chan->loopDepth - 1;
    if ( chan->loopCount[depth] != LOOP_FOREVER ) {
        if ( --chan->loopCount[depth] == 0 ) {
            chan->loopDepth = depth;
            depth = -1;
        }
    }
    if ( depth >= 0 ) {
        src = chan->loopPos[depth];
        chan->volStream = chan->loopVolStream[depth];
        chan->bendStream = chan->loopBendStream[depth];
        chan->trackVolume = chan->loopVolume[depth];
        chan->bend = chan->loopBend[depth];
        chan->volStreamWait = chan->loopVolWait[depth];
        chan->bendStreamWait = chan->loopBendWait[depth];
        chan->bendPitch = chan->bend * chan->bendRange;
    }
    return src;
}

/*
================
Snd_CmdTrill

Command 0x97 <semitones> <on ticks> <off ticks>.
================
*/
u8 *Snd_CmdTrill( sndChannel_t *chan, u8 *src ) {
    chan->trillInterval = *src++;
    chan->trillOnTicks = *src++;
    chan->trillOffTicks = *src++;
    return src;
}

/*
================
Snd_CmdTrillOff

Command 0x98.
================
*/
u8 *Snd_CmdTrillOff( sndChannel_t *chan, u8 *src ) {
    chan->trillOnTicks = 0;
    return src;
}

/*
================
Snd_CmdNoteVelocity

Command 0x99: notes carry a velocity byte, until one with bit 7 set makes it the fixed velocity.
================
*/
u8 *Snd_CmdNoteVelocity( sndChannel_t *chan, u8 *src ) {
    chan->noteVelocity = 1;
    return src;
}

/*
================
Snd_CmdNoteVelocityOff

Command 0x9A.
================
*/
u8 *Snd_CmdNoteVelocityOff( sndChannel_t *chan, u8 *src ) {
    chan->noteVelocity = 0;
    return src;
}

/*
================
Snd_CmdVelocity

Command 0x9B <velocity>: a fixed velocity for the following notes.
================
*/
u8 *Snd_CmdVelocity( sndChannel_t *chan, u8 *src ) {
    u8 value;

    value = *src++;
    chan->noteVelocity = 0;
    chan->fixedVelocity = value;
    return src;
}

/*
================
Snd_CmdPan

Command 0x9C <pan>: 0 to 0xFF, halved.
================
*/
u8 *Snd_CmdPan( sndChannel_t *chan, u8 *src ) {
    chan->pan = *src++ >> 1;
    return src;
}

/*
================
Snd_CmdSkip2

Command 0x9D <2 bytes>: ignored.
================
*/
u8 *Snd_CmdSkip2( sndChannel_t *chan, u8 *src ) {
    return src + 2;
}

/*
================
Snd_CmdDrumMap

Command 0x9E <index:1-2>: selects the drum map from entry index of the sequence's drum maps; each note then
indexes it.
================
*/
u8 *Snd_CmdDrumMap( sndChannel_t *chan, u8 *src ) {
    s32 index;

    index = *src++;
    if ( index >= 0x80 ) {
        index &= 0x7F;
        index <<= 8;
        index |= *src++;
    }
    chan->drumMap = &chan->seq->drumMaps[index];
    return src;
}

/*
================
Snd_CmdDrumMapOff

Command 0x9F.
================
*/
u8 *Snd_CmdDrumMapOff( sndChannel_t *chan, u8 *src ) {
    chan->drumMap = NULL;
    return src;
}

/*
================
Snd_CmdSkip1

Command 0xA0 <1 byte>: ignored.
================
*/
u8 *Snd_CmdSkip1( sndChannel_t *chan, u8 *src ) {
    return src + 1;
}

/*
================
Snd_CmdJump

Command 0xA1 <cmds:2> <volume:2> <bend:2>: jumps the command, volume and bend streams to the given offsets
(big-endian) from their starts.
================
*/
u8 *Snd_CmdJump( sndChannel_t *chan, u8 *src ) {
    s32 jump;
    s32 offset;

    jump = *src++ << 8;
    jump += *src++;
    offset = *src++ << 8;
    offset += *src++;
    chan->volStreamWait = 1;
    chan->volStream = chan->volStreamStart + offset;
    offset = *src++ << 8;
    offset += *src++;
    chan->bendStreamWait = 1;
    chan->bendStream = chan->bendStreamStart + offset;
    return chan->cmdStart + jump;
}

/*
================
Snd_CmdFxMix

Command 0xA2 <mix>: the FX mix above the channel's minimum.
================
*/
u8 *Snd_CmdFxMix( sndChannel_t *chan, u8 *src ) {
    chan->fxMix = *src++;
    return src;
}

/*
================
Snd_CmdRandomTranspose

Command 0xA3 <range> <base>: transposes by base plus a random number below range.
================
*/
u8 *Snd_CmdRandomTranspose( sndChannel_t *chan, u8 *src ) {
    chan->transpose = Snd_Random( *src++ );
    chan->transpose += *src++;
    return src;
}

/*
================
Snd_CmdRandomVolume

Command 0xA4 <range> <base>: the track volume is base plus a random number below range.
================
*/
u8 *Snd_CmdRandomVolume( sndChannel_t *chan, u8 *src ) {
    chan->trackVolume = Snd_Random( *src++ );
    chan->trackVolume += *src++;
    return src;
}

/*
================
Snd_CmdRandomPan

Command 0xA5 <range> <base>: the pan is base plus a random number below range.
================
*/
u8 *Snd_CmdRandomPan( sndChannel_t *chan, u8 *src ) {
    chan->pan = Snd_Random( *src++ );
    chan->pan += *src++;
    return src;
}

/*
================
Snd_CmdVolume

Command 0xA6 <volume>: the track volume.
================
*/
u8 *Snd_CmdVolume( sndChannel_t *chan, u8 *src ) {
    chan->trackVolume = *src++;
    return src;
}

/*
================
Snd_CmdPlaySe

Command 0xA7 <index:1-2>: starts entry index of the channel's SE table above this channel's priority, and makes
it part of this sound (same id and wave bank).
================
*/
u8 *Snd_CmdPlaySe( sndChannel_t *chan, u8 *src ) {
    s32 index;
    s32 id;
    s32 i;
    sndChannel_t *other;

    index = *src++;
    if ( index >= 0x80 ) {
        index = ( ( index & 0x7F ) << 8 ) + *src++;
    }
    chan->priority++;
    id = Snd_StartSe( chan->seTable, index, chan->playVolume, chan->playPan, chan->priority );
    chan->priority--;
    if ( id != 0 ) {
        for ( i = 0, other = snd_channels; i < snd_numChannels; i++, other++ ) {
            if ( other->id == id ) {
                other->id = chan->id;
                other->waveBank = chan->waveBank;
            }
        }
    }
    return src;
}

/*
================
Snd_CmdBendRange

Command 0xA8 <range>: semitones per bend unit, in 64ths.
================
*/
u8 *Snd_CmdBendRange( sndChannel_t *chan, u8 *src ) {
    chan->bendRange = (f32)*src++ * 0.015625;
    chan->bendPitch = chan->bend * chan->bendRange;
    return src;
}

/*
================
Snd_CmdAutoPan

Command 0xA9 <speed>: sweeps the pan, speed / 64 pan units per tick; 0 stops it.
================
*/
u8 *Snd_CmdAutoPan( sndChannel_t *chan, u8 *src ) {
    chan->autoPanSpeed = *src++;
    return src;
}

/*
================
Snd_CmdFxType

Command 0xAA <setting>: passes setting to func_8003F340 (the effect) while snd_seqFxEnabled is 1.
================
*/
u8 *Snd_CmdFxType( sndChannel_t *chan, u8 *src ) {
    s32 arg;

    arg = *src++;
    if ( snd_seqFxEnabled == 1 ) {
        func_8003F340( arg );
    }
    return src;
}

/*
================
Snd_CmdMarker

Command 0xAB <value> <length:1-2>: on an unpaused master channel, calls the marker callback with the sound id
and value. Snd_PlayBgmFromMarker seeks these; length is only used there.
================
*/
u8 *Snd_CmdMarker( sndChannel_t *chan, u8 *src ) {
    s32 value;
    s32 length;

    value = *src++;
    length = *src++;
    if ( length & 0x80 ) {
        length = ( ( length & 0x7F ) << 8 ) + *src++;
    }
    if ( ( chan->flags & ( CHAN_PAUSED | CHAN_MASTER ) ) == CHAN_MASTER && snd_markerCallback != NULL ) {
        snd_markerCallback( chan->id, value );
    }
    return src;
}

/*
================
Snd_CmdFixedLengthOff

Command 0xAC: notes carry a length again.
================
*/
u8 *Snd_CmdFixedLengthOff( sndChannel_t *chan, u8 *src ) {
    chan->fixedLength = 0;
    return src;
}
