/* 166B0.c -- sequenced sound channels: channel commands, voice updates and the player callback */

#include "common.h"
#include "ultra64.h"
#include "functions.h"
#include "structs.h"
#include "variables.h"

/* The low byte of x as a signed value. */
#define SEXT8( x ) ( ( ( x ) & 0x80 ) ? ( ( x ) & 0xFF ) - 0x100 : ( ( x ) & 0xFF ) )

/*
================
SetEnvelope

Reads the seven envelope bytes at src into the channel (rates and levels of the attack, decay and release
phases); returns the position after them. Used by Snd_CmdEnvelopeIndex; Snd_CmdEnvelope repeats its body. The name is invented.
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

Channel command: applies envelope entry (one- or two-byte index) of the channel's header.
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
================
Snd_Init

Sets up the driver from config: tick rate from the TV type, heap, channel and voice arrays, message queue, default banks, then registers the player and allocates the voices.
================
*/
void Snd_Init( sndConfig_t *config ) {
    s32 i;
    ALVoiceConfig voiceConfig;

    D_800693D8 = config->unk_40;
    D_800DF6B0 = config->unk_00;
    snd_numChannels = config->numVoices + 4;
    if ( osTvType == OS_TV_PAL ) {
        snd_tickRate = 50;
    } else {
        snd_tickRate = 60;
    }
    snd_tickTime = 1000000 / snd_tickRate;
    Snd_InitHeap( config->heapBase, config->heapSize );
    func_8003F7F0( config->unk_08 );
    snd_voices = Snd_Alloc( ( snd_numChannels - 4 ) * sizeof( N_ALVoice ) );
    snd_channels = Snd_Alloc( snd_numChannels * sizeof( sndChannel_t ) );
    snd_voiceChannels = snd_channels + 4;
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
    snd_fxType = 2;
    func_8003F9A0( config, snd_tickRate, 2 );
    Snd_SetMasterVolume( 3, 0x7FFF );
    snd_nextId = 1;
    snd_randomSeed = 0x12345678;
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
        if ( i >= 4 ) {
            n_alSynAllocVoice( &snd_voices[i - 4], &voiceConfig );
        }
    }
    Snd_HeapUsed();
}

/*
================
Snd_SetMasterVolume

Sets the volume scale of bank channels (bit 0 of flags) and of the other channels (bit 1).
================
*/
void Snd_SetMasterVolume( s32 flags, s16 value ) {
    if ( flags & 1 ) {
        snd_seVolume = value;
    }
    if ( flags & 2 ) {
        snd_bgmVolume = value;
    }
}

/*
================
Snd_PlayBgm

Starts the channels of hdr through Snd_StartBgm, queues message 1 for them; returns their id.
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

Starts the channels of hdr like Snd_PlayBgm, but first runs each of them silently up to the marker command 0xAB with the given argument.
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
        // RULE-EXCEPTION: R14, a do/while here lets gcc hoist the handler table and 0xAB out of the loop;
        // retail reloads both every pass, so this loop was built from labels.
    next:
        src = chan->cmdPos;
        cmd = *src;
        if ( cmd >= 0x80 ) {
            if ( cmd == 0xAB && src[1] == marker ) {
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

Starts entry index of the one-shot bank snd_nextSeTable, or else of the default bank snd_seTable, on a free channel; returns the channel id, or 0 without a bank.
================
*/
s32 Snd_PlaySe( s32 index ) {
    seTable_t *bank;
    s32 id;

    bank = snd_nextSeTable;
    if ( bank != NULL ) {
        snd_nextSeTable = NULL;
    } else {
        bank = snd_seTable;
        if ( bank == NULL ) {
            snd_nextWaveBank = NULL;
            return 0;
        }
    }
    if ( snd_nextWaveBank == NULL ) {
        snd_nextWaveBank = bank->waveBank;
    }
    id = Snd_StartSe( bank, index, 0x80, 0x80, -1 );
    snd_nextWaveBank = NULL;
    return id;
}

/*
================
Snd_PlaySeEx

Like Snd_PlaySe with explicit parameters; with reuse set, a channel already playing the entry from the same bank is restarted instead of taking a free one.
================
*/
s32 Snd_PlaySeEx( s32 index, s32 arg1, s32 arg2, s32 reuse, s32 priority ) {
    seTable_t *bank;
    s32 i;
    sndChannel_t *chan;
    s32 id;

    if ( snd_nextSeTable != NULL ) {
        bank = snd_nextSeTable;
        snd_nextSeTable = NULL;
    } else {
        bank = snd_seTable;
        if ( bank == NULL ) {
            snd_nextWaveBank = NULL;
            return 0;
        }
    }
    if ( snd_nextWaveBank == NULL ) {
        snd_nextWaveBank = bank->waveBank;
    }
    if ( reuse ) {
        for ( i = 4, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
            if ( chan->seIndex == index && chan->seTable == bank ) {
                if ( priority == -1 ) {
                    priority = bank->entries[index].priority;
                }
                id = Snd_StartSeOnChannel( chan, bank, index, arg1, arg2, priority );
                snd_nextWaveBank = NULL;
                return id;
            }
        }
    }
    id = Snd_StartSe( bank, index, arg1, arg2, priority );
    snd_nextWaveBank = NULL;
    return id;
}

/*
================
Snd_StopAll

Starts a fade over value steps (at least 1) on every active channel not fading yet; bit 0 of flags selects bank channels, bit 1 the others. See Snd_Stop.
================
*/
void Snd_StopAll( s32 flags, s32 value ) {
    s32 i;
    s32 total;
    sndChannel_t *chan;

    total = value;
    if ( value == 0 ) {
        total = 1;
    }
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seTable != NULL ? flags & 1 : flags & 2 ) {
            if ( chan->cmdPos != NULL && chan->fadeLeft == -1 ) {
                if ( chan->flags & 1 ) {
                    chan->fadeTotal = 1;
                    chan->fadeLeft = 0;
                    chan->flags &= ~1;
                } else {
                    chan->fadeTotal = total;
                    chan->fadeLeft = value;
                }
            }
        }
    }
}

/*
================
Snd_NumActive

Counts the active channels: bit 0 of flags counts those started from a bank, bit 1 the others.
================
*/
s32 Snd_NumActive( s32 flags ) {
    s32 i;
    s32 count;
    sndChannel_t *chan;

    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos != NULL ) {
            if ( chan->seTable != NULL ? flags & 1 : flags & 2 ) {
                count++;
            }
        }
    }
    return count;
}

/*
================
Snd_Stop

Starts a fade over value steps (at least 1) on every channel with the given id that is not fading yet; a channel with bit 0 of unk_00 set instead gets a zero-length fade and loses that bit. Returns how many channels it changed.
================
*/
s32 Snd_Stop( s32 id, s32 value ) {
    s32 i;
    s32 count;
    s32 total;
    sndChannel_t *chan;

    if ( id == 0 ) {
        return 0;
    }
    total = value;
    if ( value == 0 ) {
        total = 1;
    }
    count = 0;
    for ( i = 0, chan = snd_channels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->id == id && chan->fadeLeft == -1 ) {
            if ( chan->flags & 1 ) {
                chan->fadeTotal = 1;
                chan->fadeLeft = 0;
                chan->flags &= ~1;
            } else {
                chan->fadeTotal = total;
                chan->fadeLeft = value;
            }
            count++;
        }
    }
    return count;
}

/*
================
Snd_NumChannels

Counts the channels with the given id.
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

Sets unk_9E on every channel with the given id; returns how many there were.
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

Sets unk_B0 and unk_BE = 0xFF on every channel with the given id; returns how many there were.
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
            chan->lastPan = 0xFF;
        }
    }
    return count;
}

/*
================
Snd_SetPitchOffset

Sets unk_30 to value + unk_90 on every channel with the given id; returns how many there were.
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

Sets the tempo scale unk_98 (clamped to [1, 0x100]) on every channel with the given id and rescales unk_9C from unk_A8; returns how many there were.
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

Sets unk_CB (clamped to [0, 0x7F]) and unk_CC = 0xFF on every channel with the given id; returns how many there were.
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
            chan->lastFxMix = 0xFF;
        }
    }
    return count;
}

/*
================
Snd_LoadWaveBank

Relocates bank to base and makes it the default snd_defaultWaveBank if there is none yet.
================
*/
void Snd_LoadWaveBank( waveBank_t *bank, s32 base ) {
    Snd_RelocateWaveBank( bank, base );
    if ( snd_defaultWaveBank == NULL ) {
        snd_defaultWaveBank = bank;
    }
}

/*
================
Snd_SetNextWaveBank

Makes bank the current snd_nextWaveBank if it is relocated; returns snd_nextWaveBank.
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

Makes bank the default snd_defaultWaveBank if it is relocated.
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

Returns the default bank snd_defaultWaveBank.
================
*/
waveBank_t *Snd_GetDefaultWaveBank( void ) {
    return snd_defaultWaveBank;
}

/*
================
Snd_GetWaveBank

Returns unk_7C of the first channel with the given id, or 0.
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
================
Snd_Pause

Queues message 0 with arg.
================
*/
void Snd_Pause( s32 arg ) {
    sndMsg_t msg;

    msg.cmd = 0;
    msg.arg = arg;
    Snd_PostMessage( &msg );
}

/*
================
Snd_Resume

Queues message 1 with arg.
================
*/
void Snd_Resume( s32 arg ) {
    sndMsg_t msg;

    msg.cmd = 1;
    msg.arg = arg;
    Snd_PostMessage( &msg );
}

/*
================
Snd_SetFxType

Remembers arg in snd_fxType and queues message 2 with it; returns whether it was queued.
================
*/
inline s32 Snd_SetFxType( s32 arg ) {
    sndMsg_t msg;

    msg.arg = arg;
    snd_fxType = arg;
    msg.cmd = 2;
    return Snd_PostMessage( &msg );
}

/*
================
Snd_EnableSeqFx

Sets snd_seqFxEnabled to arg; a zero arg first requeues message 2 with snd_fxType and is dropped if that fails.
================
*/
void Snd_EnableSeqFx( s32 arg ) {
    s32 queued;

    queued = 1;
    if ( arg == 0 ) {
        queued = Snd_SetFxType( snd_fxType );
    }
    if ( queued ) {
        snd_seqFxEnabled = arg;
    }
}

/*
================
Snd_LoadSeTable

Turns the offsets of bank into pointers, once, and makes it the default snd_seTable if there is none yet.
================
*/
void Snd_LoadSeTable( void *data ) {
    s32 i;
    seTable_t *bank;

    bank = data;
    if ( !( bank->flags & 1 ) ) {
        if ( snd_seTable == NULL ) {
            snd_seTable = bank;
        }
        bank->flags = 1;
        bank->waveBank = 0;
        bank->programs = (u16 *)( (u8 *)bank + (s32)bank->programs );
        for ( i = 0; i < bank->numEntries; i++ ) {
            bank->entries[i].cmds = (u8 *)data + (s32)bank->entries[i].cmds;
        }
    }
}

/*
================
func_8003C1C0

Returns obj->unk_04.
================
*/
s32 func_8003C1C0( UnkStruct8003C1EC *obj ) {
    return obj->unk_04;
}

/*
================
Snd_SetSeTable

Sets the default bank snd_seTable.
================
*/
void Snd_SetSeTable( seTable_t *bank ) {
    snd_seTable = bank;
}

/*
================
Snd_SetNextSeTable

Sets the one-shot bank snd_nextSeTable.
================
*/
void Snd_SetNextSeTable( seTable_t *bank ) {
    snd_nextSeTable = bank;
}

/*
================
Snd_GetSeTable

Returns the default bank snd_seTable.
================
*/
seTable_t *Snd_GetSeTable( void ) {
    return snd_seTable;
}

/*
================
Snd_SetSeWaveBank

Sets obj->unk_10.
================
*/
void Snd_SetSeWaveBank( UnkStruct8003C1EC *obj, waveBank_t *bank ) {
    obj->unk_10 = bank;
}

/*
================
Snd_GetSeWaveBank

Returns obj->unk_10.
================
*/
waveBank_t *Snd_GetSeWaveBank( UnkStruct8003C1EC *obj ) {
    return obj->unk_10;
}

/*
================
func_8003C1FC

Sets D_8005CE4C.
================
*/
void func_8003C1FC( s32 value ) {
    D_8005CE4C = value;
}

#ifdef NON_MATCHING
// NON_MATCHING: referent/type: retail reads unk_08 once through one type covering seqHeader_t and seTable_t; the cast leaves a register difference
/*
================
func_8003C208

Returns unk_08 of the header (or else the bank) of the first channel with the given id, or 0.
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

Returns unk_20 of the header, or unk_14 of the bank, of the first channel with the given id, or NULL.
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

Sets the snd_markerCallback callback.
================
*/
void Snd_SetMarkerCallback( void (*callback)( s32 id, s32 value ) ) {
    snd_markerCallback = callback;
}

/*
================
Snd_InitMessages

Allocates the message ring buffer with count entries, clamped to [0x40, 0x400].
================
*/
void Snd_InitMessages( s32 count ) {
    if ( count < 0x40 ) {
        count = 0x40;
    } else if ( count > 0x400 ) {
        count = 0x400;
    }
    snd_msgs = Snd_Alloc( count * sizeof( sndMsg_t ) );
    snd_numMsgs = count;
    snd_msgWrite = 0;
    snd_msgRead = 0;
}

/*
================
Snd_HandleMessage

Handles one queued message: 0 and 1 change bit 0 of unk_00 on the channels of msg->arg, 2 calls func_8003F340.
================
*/
void Snd_HandleMessage( sndMsg_t *msg ) {
    switch ( msg->cmd ) {
    case 0:
        Snd_SetChannelFlags( msg->arg, ~1, 1 );
        break;
    case 1:
        Snd_SetChannelFlags( msg->arg, ~1, 0 );
        break;
    case 2:
        func_8003F340( msg->arg );
        break;
    }
}

/*
================
Snd_PostMessage

Appends msg to the ring buffer; returns 0 if it is full, else 1.
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
================
Snd_PlayerHandler

Player callback: handles the queued messages, then advances every active channel by one tick (commands, step streams, fade, envelope, modulation, voice update). Returns the time to the next call.
================
*/
s32 Snd_PlayerHandler( void *clientData ) {
    s32 i;
    sndChannel_t *chan;
    f32 pitch;

    while ( snd_msgRead != snd_msgWrite ) {
        Snd_HandleMessage( &snd_msgs[snd_msgRead] );
        if ( ++snd_msgRead == snd_numMsgs ) {
            snd_msgRead = 0;
        }
    }
    for ( i = -4, chan = snd_channels; i < snd_numChannels - 4; i++, chan++ ) {
        if ( chan->cmdPos == NULL || ( chan->flags & 1 ) ) {
            continue;
        }
        if ( chan->pendingWave != NULL ) {
            Snd_StartVoice( chan, i );
        }
        chan->time += chan->tempo;
        if ( chan->noteLength != 0x7FFF ) {
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
        if ( chan->fadeLeft != -1 && --chan->fadeLeft == -1 ) {
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

Runs the channel's commands up to its next note, then plays it: length, velocity, program (or drum map entry), sound, pitch and effect; the rest note 0x60 releases the voice. A channel without commands left stops its voice.
================
*/
void Snd_PlayNextNote( sndChannel_t *chan, s32 index ) {
    u8 *src;
    s32 note;
    // RULE-EXCEPTION: R30, the command, length and detune bytes share one local; separate locals
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
        if ( byte >= 0x80 ) {
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
        if ( chan->seq != NULL && chan->drumMap == NULL && chan->seq->programs[chan->program] == 0xFFFF ) {
            note = 0x60;
        }
        if ( note != 0x60 ) {
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
        } else if ( chan->envPhase < 4 ) {
            chan->envPhase = 4;
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

Stops voice index if the channel had started it, then starts it again with unk_08 and clears unk_08.
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

Updates the volume and pan of voice index from the channel, sending only values that changed.
================
*/
void Snd_UpdateVolumePan( sndChannel_t *chan, s32 index ) {
    // RULE-EXCEPTION: R30, the volume and the pan share one local; separate locals change the register
    // allocation.
    u32 value;

    value = (u32)( chan->trackVolume * chan->envLevel * chan->velocity * chan->playVolume ) >> 13;
    if ( value > 0x7FFF ) {
        value = 0x7FFF;
    }
    value *= chan->seTable == NULL ? snd_bgmVolume : snd_seVolume;
    value >>= 15;
    if ( chan->fadeLeft != -1 ) {
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

Updates the pitch of voice index: the note unk_2C (gliding from unk_4C over unk_B8 ticks) plus offset and unk_24, in semitones; sent only when it changed, capped at a ratio of 2.
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
================
Snd_StartEnvelope

Sets unk_54 from unk_40 and unk_B2, or from unk_3C and unk_B4, or to unk_40 + 0x7FFFFFFF when unk_9A is 0x7FFF; copies unk_C0 and unk_BF to unk_C4 and unk_C5 and sets unk_C3.
================
*/
void Snd_StartEnvelope( sndChannel_t *chan ) {
    if ( chan->noteLength != 0x7FFF ) {
        if ( chan->releaseAfter != 0 ) {
            chan->releaseTime = chan->noteTime + ( chan->releaseAfter << 8 );
        } else {
            chan->releaseTime = chan->nextNoteTime - ( chan->releaseBefore << 8 );
        }
    } else {
        chan->releaseTime = chan->noteTime + 0x7FFFFFFF;
    }
    chan->envPhase = 1;
    chan->envLevel = chan->envStartLevel;
    chan->envWait = chan->envInterval;
}

/*
================
Snd_UpdateEnvelope

Steps the envelope: phase unk_C3 (1 attack, 2 decay, 3 sustain, 4 release) and level unk_C4, every unk_BF calls; enters release once unk_0C passes unk_54.
================
*/
void Snd_UpdateEnvelope( sndChannel_t *chan ) {
    s32 time;

    if ( chan->releaseTime - chan->time < 0 && chan->envPhase < 4 ) {
        chan->envPhase = 4;
        chan->envWait = 1;
        chan->envReleaseLevel = chan->envLevel;
    }
    if ( --chan->envWait == 0 ) {
        chan->envWait = chan->envInterval;
        switch ( chan->envPhase ) {
        case 1:
            time = ( (u32)( chan->time - chan->noteTime ) >> 8 ) * chan->envTimeScale >> 10;
            if ( time < chan->envAttackTime ) {
                chan->envLevel = chan->envStartLevel + (s32)( chan->attackSlope * time );
            } else {
                chan->envPhase++;
                chan->envLevel = chan->envPeakLevel;
            }
            break;
        case 2:
            time = ( ( (u32)( chan->time - chan->noteTime ) >> 8 ) - chan->envAttackTime ) * chan->envTimeScale >> 10;
            if ( time < chan->envDecayTime ) {
                chan->envLevel = chan->envPeakLevel + (s32)( chan->decaySlope * time );
            } else {
                chan->envPhase++;
                chan->envLevel = chan->envSustainLevel;
            }
            break;
        case 3:
            break;
        case 4:
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

Copies unk_40 to unk_94, clears unk_D9 and sets unk_DA from bit 6 of unk_BD.
================
*/
void Snd_StartAutoPan( sndChannel_t *chan ) {
    chan->autoPanFrac = 0;
    chan->autoPanTime = chan->noteTime;
    chan->autoPanDown = chan->pan & 0x40;
}

/*
================
Snd_UpdateAutoPan

Advances unk_94 in steps of 0x100 up to unk_0C, moving unk_BD up to 0x7F or down to 0 by unk_D4 / 64 per step, reversing at either end.
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

Counts down unk_D0; at zero toggles unk_D1 between 0 (for unk_CF steps) and unk_D8 (for unk_CE steps). Returns unk_D1.
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

Sets unk_68 to unk_20 * sin( ( unk_AA - unk_B6 ) * unk_DC ) and returns it; 0 while unk_AA has not passed unk_B6.
================
*/
f32 Snd_UpdateVibrato( sndChannel_t *chan ) {
    s32 time;

    time = chan->noteAge - chan->vibratoDelay;
    if ( time > 0 ) {
        return chan->vibratoPitch = __sinf( time * chan->vibratoStep ) * chan->vibratoDepth;
    }
    return 0.0f;
}

/*
================
Snd_StepVolumeStream

Advances unk_14 in steps of 0x100 up to unk_0C, reading a new unk_BC and step length from the unk_38 stream each time unk_A2 runs out.
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

Advances unk_18 in steps of 0x100 up to unk_0C, reading a new pitch offset unk_70 (and unk_24) and step length from the unk_34 stream each time unk_A4 runs out.
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

#ifdef NON_MATCHING
// NON_MATCHING: FPU registers: x lives in $f6 in retail, $f14 here, and the x^6 term takes a different register
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
// NON_MATCHING: register/schedule: retail has a 0x30 frame (0x28 here) and reloads tune->value; likely an inlined helper not yet recovered
/*
================
Snd_RelocateWaveBank

TODO
================
*/
void Snd_RelocateWaveBank( void *data, s32 base ) {
    waveBank_t *bank;
    s32 i;

    bank = data;
    if ( bank->flags < 0 ) {
        return;
    }
    bank->flags |= 0x80000000;
    Snd_RelocateTable( (s32 *)&bank->unk_24, (s32)bank, 3 );
    Snd_RelocateTable( (s32 *)bank->waves, (s32)bank, bank->numWaves );
    for ( i = 0; i < bank->numWaves; i++ ) {
        SetTune( &bank->tunes[i], &bank->unk_24[i] );
        if ( bank->waves[i]->flags == 0 ) {
            if ( ( bank->waves[i]->base & 0xFF000000 ) != 0xFF000000 ) {
                bank->waves[i]->base += base;
            }
            bank->waves[i]->flags = 1;
            if ( bank->waves[i]->loop != 0 ) {
                bank->waves[i]->loop += (s32)data;
            }
            if ( bank->waves[i]->type == 0 ) {
                bank->waves[i]->book += (s32)data;
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

Returns a random number in [0, range), from eight steps of the shift register snd_randomSeed.
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

#ifdef NON_MATCHING
// NON_MATCHING: registers: retail puts the 0xFF constant in a1, moving chan and started to a2/a3
/*
================
Snd_ResetChannel

TODO
================
*/
void Snd_ResetChannel( void *data ) {
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
    chan->lastFxMix = chan->lastPan = 0xFF;
    tempo = 0x6000 / snd_tickRate;
    chan->fixedVelocity = 0x7F;
    chan->trackVolume = 0x7F;
    chan->envPeakLevel = 0x7F;
    chan->envDecayTime = 0xFF;
    chan->envSustainLevel = 0x7F;
    chan->pan = 0x40;
    chan->envInterval = 1;
    chan->envAttackTime = 1;
    chan->envReleaseTime = 15;
    chan->lastVolume = 0xFFFF;
    chan->noteLength = 1;
    chan->volStreamWait = 1;
    chan->bendStreamWait = 1;
    chan->fadeLeft = -1;
    chan->playVolume = 0x80;
    chan->playPan = 0x80;
    chan->tempoScale = 0x80;
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

Picks the channel index for track of hdr (-1: the master channel, which prefers the first four). Takes a free channel, else the bank channel of lowest priority, else a channel of another header, else one already on this track, else one by track number.
================
*/
s32 Snd_FindBgmChannel( seqHeader_t *hdr, s32 track ) {
    s32 i;
    s32 best;
    s32 lowest;
    sndChannel_t *chan;

    if ( track < 0 ) {
        for ( i = 0, chan = snd_channels; i < 4; i++, chan++ ) {
            if ( chan->cmdPos == NULL ) {
                return i;
            }
        }
    }
    for ( i = 4, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos == NULL ) {
            return i;
        }
    }
    for ( i = 4, chan = snd_voiceChannels, lowest = 0x7FFFFFFF, best = 3; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seTable != NULL && chan->priority <= lowest ) {
            lowest = chan->priority;
            best = i;
        }
    }
    if ( best >= 4 ) {
        return best;
    }
    // the original never advances chan here, so this tests the first channel each time
    for ( i = 4, chan = snd_voiceChannels; i < snd_numChannels; i++ ) {
        if ( chan->seTable == NULL && chan->seq != hdr ) {
            return i;
        }
    }
    for ( i = 4, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->seq == hdr && hdr->trackCmds[track] == chan->cmdStart ) {
            return i;
        }
    }
    return track % ( snd_numChannels - 4 ) + 4;
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

Starts entry index of bank on chan; returns the new channel id.
================
*/
s32 Snd_StartSeOnChannel( sndChannel_t *chan, seTable_t *bank, s32 index, s32 arg3, s32 arg4, s32 priority ) {
    Snd_ResetChannel( chan );
    chan->seIndex = index;
    chan->seTable = bank;
    chan->playVolume = arg3;
    chan->playPan = arg4;
    chan->id = snd_nextId++;
    chan->priority = priority;
    if ( bank->waveBank != 0 ) {
        chan->waveBank = bank->waveBank;
    }
    chan->cmdPos = chan->cmdStart = bank->entries[index].cmds;
    return chan->id;
}

/*
================
Snd_StartSe

Starts entry index of bank on a free channel above the first four, or else on the bank channel of lowest priority below priority (-1: the entry's own); returns the channel id, or 0.
================
*/
s32 Snd_StartSe( seTable_t *bank, s32 index, s32 arg2, s32 arg3, s32 priority ) {
    s32 i;
    s32 lowest;
    sndChannel_t *chan;
    sndChannel_t *victim;

    if ( priority == -1 ) {
        priority = bank->entries[index].priority;
    }
    lowest = priority + 1;
    for ( i = 4, chan = snd_voiceChannels; i < snd_numChannels; i++, chan++ ) {
        if ( chan->cmdPos == NULL ) {
            Snd_ResetChannel( chan );
            chan->seIndex = index;
            chan->seTable = bank;
            chan->playVolume = arg2;
            chan->playPan = arg3;
            chan->priority = priority;
            chan->id = snd_nextId++;
            if ( bank->waveBank != NULL ) {
                chan->waveBank = bank->waveBank;
            }
            chan->cmdPos = chan->cmdStart = bank->entries[index].cmds;
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
        victim->seTable = bank;
        victim->playVolume = arg2;
        victim->playPan = arg3;
        victim->priority = priority;
        victim->id = snd_nextId++;
        if ( bank->waveBank != NULL ) {
            victim->waveBank = bank->waveBank;
        }
        victim->cmdPos = victim->cmdStart = bank->entries[index].cmds;
        return victim->id;
    }
    return 0;
}

#ifdef NON_MATCHING
// NON_MATCHING: registers: retail keeps a separate base copy per relocation loop (as if Snd_RelocateTable were inlined) and swaps s1/s2
/*
================
Snd_StartBgm

Relocates hdr once, then starts its master channel and one channel per non-empty entry of unk_0C, all under one new id, which it returns.
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
    chan = &snd_channels[Snd_FindBgmChannel( hdr, -1 )];
    Snd_ResetChannel( chan );
    chan->noteVelocity = 1;
    chan->seq = hdr;
    chan->flags |= 3;
    chan->id = id;
    chan->cmdPos = chan->cmdStart = hdr->masterCmds;
    for ( i = 0; i < count; i++ ) {
        if ( hdr->trackCmds[i] != NULL ) {
            chan = &snd_channels[Snd_FindBgmChannel( hdr, i )];
            Snd_ResetChannel( chan );
            chan->noteVelocity = 1;
            chan->seq = hdr;
            chan->flags |= 1;
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

Sets unk_00 to ( unk_00 & mask ) | bits on every channel with the given id.
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
================
Snd_CmdEnd

Channel command: ends the channel's command stream.
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

Channel command: sets the program unk_AE from a value encoded as one byte, or as two bytes when the first has
bit 7 set.
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

Channel command: sets unk_B8 from one byte; a non-zero value also copies unk_2C to unk_50.
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

Channel command: clears unk_B8.
================
*/
u8 *Snd_CmdPortamentoOff( sndChannel_t *chan, u8 *src ) {
    chan->portamento = 0;
    return src;
}

/*
================
Snd_CmdEnvelope

Channel command: reads an envelope from the command stream.
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

Channel command: sets the tempo from one byte; a bank channel takes it alone, otherwise every channel of the same header does.
================
*/
u8 *Snd_CmdTempo( sndChannel_t *chan, u8 *src ) {
    s32 tempo;
    s32 scaled;
    s32 i;
    sndChannel_t *other;

    tempo = ( *src++ * 0x6000 / 120 ) / snd_tickRate;
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

Channel command: sets unk_B4 from one byte and clears unk_B2.
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

Channel command: sets unk_B2 from a big-endian 16-bit value and clears unk_B4.
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

Channel command: sets unk_B6 and unk_D5 from two bytes, unk_20 to a third byte / 50 and the phase step unk_DC to 2 pi / unk_D5.
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
// NON_MATCHING: schedule: retail reloads unk_D5 right after storing it and interleaves the two divisions
/*
================
Snd_CmdVibratoInverted

TODO
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

Channel command: clears unk_D5 and unk_68.
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

Channel command: sets unk_AC from a value encoded as one byte, or as two bytes when the first is 0x80 or more.
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

Channel command: sets unk_B7.
================
*/
u8 *Snd_CmdExplicitLength( sndChannel_t *chan, u8 *src ) {
    chan->explicitLength = 1;
    return src;
}

/*
================
Snd_CmdTranspose

Channel command: sets unk_B9 from one byte.
================
*/
u8 *Snd_CmdTranspose( sndChannel_t *chan, u8 *src ) {
    chan->transpose = *src++;
    return src;
}

/*
================
Snd_CmdTransposeOff

Channel command: sets unk_BA.
================
*/
u8 *Snd_CmdTransposeOff( sndChannel_t *chan, u8 *src ) {
    chan->transposeOff = 1;
    return src;
}

/*
================
Snd_CmdDetune

Channel command: replaces the offset unk_90 within unk_30 by a signed byte / 100.
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

Channel command: sets unk_D6.
================
*/
u8 *Snd_CmdEnvRestartOff( sndChannel_t *chan, u8 *src ) {
    chan->noEnvRestart = 1;
    return src;
}

/*
================
Snd_CmdEnvRestartOn

Channel command: clears unk_D6.
================
*/
u8 *Snd_CmdEnvRestartOn( sndChannel_t *chan, u8 *src ) {
    chan->noEnvRestart = 0;
    return src;
}

/*
================
Snd_CmdVoiceRestartOff

Channel command: sets unk_D7.
================
*/
u8 *Snd_CmdVoiceRestartOff( sndChannel_t *chan, u8 *src ) {
    chan->noVoiceRestart = 1;
    return src;
}

/*
================
Snd_CmdVoiceRestartOn

Channel command: clears unk_D7.
================
*/
u8 *Snd_CmdVoiceRestartOn( sndChannel_t *chan, u8 *src ) {
    chan->noVoiceRestart = 0;
    return src;
}

/*
================
Snd_CmdLoopStart

Channel command: pushes the read position and the channel state onto the repeat stack, with a repeat count from one byte.
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

Channel command: counts down the top repeat entry; jumps back to it and restores its state unless the count ran out, which pops it.
================
*/
u8 *Snd_CmdLoopEnd( sndChannel_t *chan, u8 *src ) {
    s32 depth;

    depth = chan->loopDepth - 1;
    if ( chan->loopCount[depth] != 0xFF ) {
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

Channel command: sets unk_D8, unk_CE and unk_CF from three bytes.
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

Channel command: clears unk_CE.
================
*/
u8 *Snd_CmdTrillOff( sndChannel_t *chan, u8 *src ) {
    chan->trillOnTicks = 0;
    return src;
}

/*
================
Snd_CmdNoteVelocity

Channel command: sets unk_D2.
================
*/
u8 *Snd_CmdNoteVelocity( sndChannel_t *chan, u8 *src ) {
    chan->noteVelocity = 1;
    return src;
}

/*
================
Snd_CmdNoteVelocityOff

Channel command: clears unk_D2.
================
*/
u8 *Snd_CmdNoteVelocityOff( sndChannel_t *chan, u8 *src ) {
    chan->noteVelocity = 0;
    return src;
}

/*
================
Snd_CmdVelocity

Channel command: sets unk_D3 from one byte and clears unk_D2.
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

Channel command: sets unk_BD from half of one byte.
================
*/
u8 *Snd_CmdPan( sndChannel_t *chan, u8 *src ) {
    chan->pan = *src++ >> 1;
    return src;
}

/*
================
Snd_CmdSkip2

Channel command: skips two bytes.
================
*/
u8 *Snd_CmdSkip2( sndChannel_t *chan, u8 *src ) {
    return src + 2;
}

/*
================
Snd_CmdDrumMap

Channel command: selects entry unk_84 of unk_74->unk_1C by a one- or two-byte index.
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

Channel command: clears unk_84.
================
*/
u8 *Snd_CmdDrumMapOff( sndChannel_t *chan, u8 *src ) {
    chan->drumMap = NULL;
    return src;
}

/*
================
Snd_CmdSkip1

Channel command: skips one byte.
================
*/
u8 *Snd_CmdSkip1( sndChannel_t *chan, u8 *src ) {
    return src + 1;
}

/*
================
Snd_CmdJump

Channel command: reads three big-endian 16-bit offsets; sets unk_38 and unk_34 from the last two and jumps to the first, relative to unk_80.
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

Channel command: sets unk_CA from one byte.
================
*/
u8 *Snd_CmdFxMix( sndChannel_t *chan, u8 *src ) {
    chan->fxMix = *src++;
    return src;
}

/*
================
Snd_CmdRandomTranspose

Channel command: sets unk_B9 to a random number below the first byte plus the second byte.
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

Channel command: sets unk_BC to a random number below the first byte plus the second byte.
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

Channel command: sets unk_BD to a random number below the first byte plus the second byte.
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

Channel command: sets unk_BC from one byte.
================
*/
u8 *Snd_CmdVolume( sndChannel_t *chan, u8 *src ) {
    chan->trackVolume = *src++;
    return src;
}

/*
================
Snd_CmdPlaySe

Channel command: starts entry (one- or two-byte index) of the channel's bank on another channel at a higher priority, and gives every channel started under the new id this channel's id and unk_7C.
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

Channel command: sets unk_6C to one byte / 64 and unk_24 to unk_70 * unk_6C.
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

Channel command: sets unk_D4 from one byte.
================
*/
u8 *Snd_CmdAutoPan( sndChannel_t *chan, u8 *src ) {
    chan->autoPanSpeed = *src++;
    return src;
}

/*
================
Snd_CmdFxType

Channel command: passes one byte to func_8003F340 while snd_seqFxEnabled is 1.
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

Channel command: passes the channel id and one byte to the snd_markerCallback callback when unk_00 & 3 is 2; skips a one- or two-byte value after it.
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
    if ( ( chan->flags & 3 ) == 2 && snd_markerCallback != NULL ) {
        snd_markerCallback( chan->id, value );
    }
    return src;
}

/*
================
Snd_CmdFixedLengthOff

Channel command: clears unk_AC.
================
*/
u8 *Snd_CmdFixedLengthOff( sndChannel_t *chan, u8 *src ) {
    chan->fixedLength = 0;
    return src;
}
