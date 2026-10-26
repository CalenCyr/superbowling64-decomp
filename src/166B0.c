#include "common.h"
#include "ultra64.h"
#include "functions.h"
#include "structs.h"
#include "variables.h"

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B2B0);

/*
================
func_8003B3FC

Sets up the driver from config: tick rate from the TV type, heap, channel and voice arrays, message queue, default banks, then registers the player and allocates the voices.
================
*/
void func_8003B3FC( UnkAudioConfig *config ) {
    s32 i;
    UnkVoiceConfig voiceConfig;

    D_800693D8 = config->unk_40;
    D_800DF6B0 = config->unk_00;
    D_80064334 = config->unk_04 + 4;
    if ( osTvType == OS_TV_PAL ) {
        D_80064344 = 50;
    } else {
        D_80064344 = 60;
    }
    D_80064348 = 1000000 / D_80064344;
    func_8003FC60( config->unk_10, config->unk_14 );
    func_8003F7F0( config->unk_08 );
    D_80064338 = func_8003FCAC( ( D_80064334 - 4 ) * sizeof( UnkVoice ) );
    D_8006433C = func_8003FCAC( D_80064334 * sizeof( UnkChannel ) );
    D_80064340 = D_8006433C + 4;
    func_8003C2F0( config->unk_24 );
    D_80064358 = NULL;
    D_8006435C = NULL;
    if ( config->unk_18 != NULL && config->unk_1C != 0 ) {
        func_8003BF74( config->unk_18, config->unk_1C );
    }
    D_8006436C = NULL;
    D_80064368 = NULL;
    if ( config->unk_20 != NULL ) {
        func_8003C13C( config->unk_20 );
    }
    D_80064370 = NULL;
    D_80064364 = 2;
    func_8003F9A0( config, D_80064344, 2 );
    func_8003B684( 3, 0x7FFF );
    D_80064350 = 1;
    D_80064354 = 0x12345678;
    D_80064320.unk_00 = NULL;
    D_80064320.unk_08 = func_8003C46C;
    D_80064320.unk_04 = &D_80064320;
    func_800421D0( &D_80064320 );
    for ( i = 0; i < D_80064334; i++ ) {
        D_8006433C[i].unk_C9 = 0;
        func_8003D90C( &D_8006433C[i] );
        voiceConfig.unk_04 = 0;
        voiceConfig.unk_00 = config->unk_0C;
        voiceConfig.unk_02 = 0;
        if ( i >= 4 ) {
            func_80042220( &D_80064338[i - 4], &voiceConfig );
        }
    }
    func_8003FCDC();
}

void func_8003B684(s32 flags, s16 value) {
    if (flags & 1) {
        D_8006434C = value;
    }
    if (flags & 2) {
        D_8006434E = value;
    }
}

/*
================
func_8003B6B0

Starts the channels of hdr through func_8003DEA0, queues message 1 for them; returns their id.
================
*/
s32 func_8003B6B0( UnkSeqHeader *hdr ) {
    s32 id;

    id = func_8003DEA0( hdr );
    func_8003C088( id );
    return id;
}

/*
================
func_8003B6E0

Starts the channels of hdr like func_8003B6B0, but first runs each of them silently up to the marker command 0xAB with the given argument.
================
*/
s32 func_8003B6E0( UnkSeqHeader *hdr, s32 marker ) {
    s32 id;
    s32 i;
    UnkChannel *chan;
    u8 *src;
    u8 cmd;
    u8 value;
    s32 length;

    id = func_8003DEA0( hdr );
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 != id || chan->unk_74 != hdr || chan->unk_04 == NULL ) {
            continue;
        }
        // RULE-EXCEPTION: R14, a do/while here lets gcc hoist the handler table and 0xAB out of the loop;
        // retail reloads both every pass, so this loop was built from labels.
    next:
        src = chan->unk_04;
        cmd = *src;
        if ( cmd >= 0x80 ) {
            if ( cmd == 0xAB && src[1] == marker ) {
                goto found;
            }
            chan->unk_04 = D_8005CB70[cmd & 0x7F]( chan, src + 1 );
        } else {
            chan->unk_04 = src + 1;
            if ( chan->unk_D2 ) {
                chan->unk_04 = src + 2;
                value = src[1];
                chan->unk_BB = value;
                if ( value >= 0x80 ) {
                    chan->unk_BB = value & 0x7F;
                    chan->unk_D2 = 0;
                    chan->unk_D3 = chan->unk_BB;
                }
            } else {
                chan->unk_BB = chan->unk_D3;
            }
            if ( chan->unk_AC != 0 && chan->unk_B7 == 0 ) {
                chan->unk_9A = chan->unk_AC;
            } else {
                chan->unk_B7 = 0;
                cmd = *chan->unk_04++;
                if ( cmd < 0x80 ) {
                    chan->unk_9A = cmd;
                } else {
                    chan->unk_9A = *chan->unk_04++ + ( ( cmd & 0x7F ) << 8 );
                }
            }
            chan->unk_0C += chan->unk_9A << 8;
        }
        if ( chan->unk_04 != NULL ) {
            goto next;
        }
    found:
        chan->unk_3C = chan->unk_0C;
        if ( chan->unk_04 != NULL ) {
            src = chan->unk_04 + 2;
            length = *src++;
            if ( length >= 0x80 ) {
                length &= 0x7F;
                length <<= 8;
                length |= *src++;
            }
            chan->unk_AA = 0;
            chan->unk_9A = length;
            chan->unk_04 = src;
            chan->unk_0C -= length << 8;
        }
        chan->unk_40 = chan->unk_0C;
        if ( chan->unk_38 != NULL ) {
            func_8003D374( chan );
        }
        if ( chan->unk_34 != NULL ) {
            func_8003D438( chan );
        }
    }
    func_8003C088( id );
    return id;
}

/*
================
func_8003B930

Starts entry index of the one-shot bank D_8006436C, or else of the default bank D_80064368, on a free channel; returns the channel id, or 0 without a bank.
================
*/
s32 func_8003B930( s32 index ) {
    UnkSeqBank *bank;
    s32 id;

    bank = D_8006436C;
    if ( bank != NULL ) {
        D_8006436C = NULL;
    } else {
        bank = D_80064368;
        if ( bank == NULL ) {
            D_80064358 = NULL;
            return 0;
        }
    }
    if ( D_80064358 == NULL ) {
        D_80064358 = bank->unk_10;
    }
    id = func_8003DCF8( bank, index, 0x80, 0x80, -1 );
    D_80064358 = NULL;
    return id;
}

/*
================
func_8003B9B0

Like func_8003B930 with explicit parameters; with reuse set, a channel already playing the entry from the same bank is restarted instead of taking a free one.
================
*/
s32 func_8003B9B0( s32 index, s32 arg1, s32 arg2, s32 reuse, s32 priority ) {
    UnkSeqBank *bank;
    s32 i;
    UnkChannel *chan;
    s32 id;

    if ( D_8006436C != NULL ) {
        bank = D_8006436C;
        D_8006436C = NULL;
    } else {
        bank = D_80064368;
        if ( bank == NULL ) {
            D_80064358 = NULL;
            return 0;
        }
    }
    if ( D_80064358 == NULL ) {
        D_80064358 = bank->unk_10;
    }
    if ( reuse ) {
        for ( i = 4, chan = D_80064340; i < D_80064334; i++, chan++ ) {
            if ( chan->unk_A6 == index && chan->unk_78 == bank ) {
                if ( priority == -1 ) {
                    priority = bank->unk_18[index].unk_04;
                }
                id = func_8003DC48( chan, bank, index, arg1, arg2, priority );
                D_80064358 = NULL;
                return id;
            }
        }
    }
    id = func_8003DCF8( bank, index, arg1, arg2, priority );
    D_80064358 = NULL;
    return id;
}

/*
================
func_8003BAC8

Starts a fade over value steps (at least 1) on every active channel not fading yet; bit 0 of flags selects bank channels, bit 1 the others. See func_8003BC14.
================
*/
void func_8003BAC8( s32 flags, s32 value ) {
    s32 i;
    s32 total;
    UnkChannel *chan;

    total = value;
    if ( value == 0 ) {
        total = 1;
    }
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_78 != NULL ? flags & 1 : flags & 2 ) {
            if ( chan->unk_04 != NULL && chan->unk_10 == -1 ) {
                if ( chan->unk_00 & 1 ) {
                    chan->unk_1C = 1;
                    chan->unk_10 = 0;
                    chan->unk_00 &= ~1;
                } else {
                    chan->unk_1C = total;
                    chan->unk_10 = value;
                }
            }
        }
    }
}

/*
================
func_8003BB94

Counts the active channels: bit 0 of flags counts those started from a bank, bit 1 the others.
================
*/
s32 func_8003BB94( s32 flags ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_04 != NULL ) {
            if ( chan->unk_78 != NULL ? flags & 1 : flags & 2 ) {
                count++;
            }
        }
    }
    return count;
}

/*
================
func_8003BC14

Starts a fade over value steps (at least 1) on every channel with the given id that is not fading yet; a channel with bit 0 of unk_00 set instead gets a zero-length fade and loses that bit. Returns how many channels it changed.
================
*/
s32 func_8003BC14( s32 id, s32 value ) {
    s32 i;
    s32 count;
    s32 total;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    total = value;
    if ( value == 0 ) {
        total = 1;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id && chan->unk_10 == -1 ) {
            if ( chan->unk_00 & 1 ) {
                chan->unk_1C = 1;
                chan->unk_10 = 0;
                chan->unk_00 &= ~1;
            } else {
                chan->unk_1C = total;
                chan->unk_10 = value;
            }
            count++;
        }
    }
    return count;
}

/*
================
func_8003BCC8

Counts the channels with the given id.
================
*/
s32 func_8003BCC8( s32 id ) {
    s32 i;
    UnkChannel *chan;
    s32 count;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        count += chan->unk_44 == id;
    }
    return count;
}

/*
================
func_8003BD20

Sets unk_9E on every channel with the given id; returns how many there were.
================
*/
s32 func_8003BD20( s32 id, s32 value ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            chan->unk_9E = value;
            count++;
        }
    }
    return count;
}

/*
================
func_8003BD80

Sets unk_B0 and unk_BE = 0xFF on every channel with the given id; returns how many there were.
================
*/
s32 func_8003BD80( s32 id, s32 value ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            count++;
            chan->unk_B0 = value;
            chan->unk_BE = 0xFF;
        }
    }
    return count;
}

/*
================
func_8003BDF0

Sets unk_30 to value + unk_90 on every channel with the given id; returns how many there were.
================
*/
s32 func_8003BDF0( s32 id, f32 value ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            chan->unk_30 = value + chan->unk_90;
            count++;
        }
    }
    return count;
}

/*
================
func_8003BE60

Sets the tempo scale unk_98 (clamped to [1, 0x100]) on every channel with the given id and rescales unk_9C from unk_A8; returns how many there were.
================
*/
s32 func_8003BE60( s32 id, s32 value ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    if ( value <= 0 ) {
        value = 1;
    } else if ( value > 0x100 ) {
        value = 0x100;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            count++;
            chan->unk_98 = value;
            chan->unk_9C = ( chan->unk_A8 * value ) >> 7;
        }
    }
    return count;
}

/*
================
func_8003BEEC

Sets unk_CB (clamped to [0, 0x7F]) and unk_CC = 0xFF on every channel with the given id; returns how many there were.
================
*/
s32 func_8003BEEC( s32 id, s32 value ) {
    s32 i;
    s32 count;
    UnkChannel *chan;

    if ( id == 0 ) {
        return 0;
    }
    if ( value < 0 ) {
        value = 0;
    } else if ( value > 0x7F ) {
        value = 0x7F;
    }
    count = 0;
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            count++;
            chan->unk_CB = value;
            chan->unk_CC = 0xFF;
        }
    }
    return count;
}

/*
================
func_8003BF74

Relocates bank to base and makes it the default D_8006435C if there is none yet.
================
*/
void func_8003BF74( UnkSoundBank *bank, s32 base ) {
    func_8003D6E0( bank, base );
    if ( D_8006435C == NULL ) {
        D_8006435C = bank;
    }
}

/*
================
func_8003BFB0

Makes bank the current D_80064358 if it is relocated; returns D_80064358.
================
*/
UnkSoundBank *func_8003BFB0( UnkSoundBank *bank ) {
    if ( bank != NULL && bank->unk_10 < 0 ) {
        D_80064358 = bank;
    }
    return D_80064358;
}

/*
================
func_8003BFD8

Makes bank the default D_8006435C if it is relocated.
================
*/
void func_8003BFD8( UnkSoundBank *bank ) {
    if ( bank != NULL && bank->unk_10 < 0 ) {
        D_8006435C = bank;
    }
}

/*
================
func_8003BFFC

Returns the default bank D_8006435C.
================
*/
UnkSoundBank *func_8003BFFC( void ) {
    return D_8006435C;
}

/*
================
func_8003C008

Returns unk_7C of the first channel with the given id, or 0.
================
*/
UnkSoundBank *func_8003C008( s32 id ) {
    s32 i;
    UnkChannel *chan;

    if ( id == 0 ) {
        return NULL;
    }
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            return chan->unk_7C;
        }
    }
    return NULL;
}

/*
================
func_8003C064

Queues message 0 with arg.
================
*/
void func_8003C064( s32 arg ) {
    UnkMsg8003C3D4 msg;

    msg.cmd = 0;
    msg.arg = arg;
    func_8003C3D4( &msg );
}

/*
================
func_8003C088

Queues message 1 with arg.
================
*/
void func_8003C088( s32 arg ) {
    UnkMsg8003C3D4 msg;

    msg.cmd = 1;
    msg.arg = arg;
    func_8003C3D4( &msg );
}

/*
================
func_8003C0B0

Remembers arg in D_80064364 and queues message 2 with it; returns whether it was queued.
================
*/
inline s32 func_8003C0B0( s32 arg ) {
    UnkMsg8003C3D4 msg;

    msg.arg = arg;
    D_80064364 = arg;
    msg.cmd = 2;
    return func_8003C3D4( &msg );
}

/*
================
func_8003C0E0

Sets D_80064360 to arg; a zero arg first requeues message 2 with D_80064364 and is dropped if that fails.
================
*/
void func_8003C0E0( s32 arg ) {
    s32 queued;

    queued = 1;
    if ( arg == 0 ) {
        queued = func_8003C0B0( D_80064364 );
    }
    if ( queued ) {
        D_80064360 = arg;
    }
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C13C);

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
func_8003C1C8

Sets the default bank D_80064368.
================
*/
void func_8003C1C8( UnkSeqBank *bank ) {
    D_80064368 = bank;
}

/*
================
func_8003C1D4

Sets the one-shot bank D_8006436C.
================
*/
void func_8003C1D4( UnkSeqBank *bank ) {
    D_8006436C = bank;
}

/*
================
func_8003C1E0

Returns the default bank D_80064368.
================
*/
UnkSeqBank *func_8003C1E0( void ) {
    return D_80064368;
}

/*
================
func_8003C1EC

Sets obj->unk_10.
================
*/
void func_8003C1EC( UnkStruct8003C1EC *obj, UnkSoundBank *bank ) {
    obj->unk_10 = bank;
}

/*
================
func_8003C1F4

Returns obj->unk_10.
================
*/
UnkSoundBank *func_8003C1F4( UnkStruct8003C1EC *obj ) {
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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C208);

/*
================
func_8003C26C

Returns unk_20 of the header, or unk_14 of the bank, of the first channel with the given id, or NULL.
================
*/
void *func_8003C26C( s32 id ) {
    s32 i;
    UnkChannel *chan;

    if ( id == 0 ) {
        return NULL;
    }
    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            if ( chan->unk_74 != NULL ) {
                return chan->unk_74->unk_20;
            }
            return chan->unk_78->unk_14;
        }
    }
    return NULL;
}

/*
================
func_8003C2E4

Sets the D_80064370 callback.
================
*/
void func_8003C2E4( void (*callback)( s32 id, s32 value ) ) {
    D_80064370 = callback;
}

/*
================
func_8003C2F0

Allocates the message ring buffer with count entries, clamped to [0x40, 0x400].
================
*/
void func_8003C2F0( s32 count ) {
    if ( count < 0x40 ) {
        count = 0x40;
    } else if ( count > 0x400 ) {
        count = 0x400;
    }
    D_80064380 = func_8003FCAC( count * sizeof( UnkMsg8003C3D4 ) );
    D_8006437C = count;
    D_80064378 = 0;
    D_80064374 = 0;
}

/*
================
func_8003C354

Handles one queued message: 0 and 1 change bit 0 of unk_00 on the channels of msg->arg, 2 calls func_8003F340.
================
*/
void func_8003C354( UnkMsg8003C3D4 *msg ) {
    switch ( msg->cmd ) {
    case 0:
        func_8003E10C( msg->arg, ~1, 1 );
        break;
    case 1:
        func_8003E10C( msg->arg, ~1, 0 );
        break;
    case 2:
        func_8003F340( msg->arg );
        break;
    }
}

/*
================
func_8003C3D4

Appends msg to the ring buffer; returns 0 if it is full, else 1.
================
*/
s32 func_8003C3D4( UnkMsg8003C3D4 *msg ) {
    s32 next;

    next = ( D_80064378 + 1 ) % D_8006437C;
    if ( next == D_80064374 ) {
        return 0;
    }
    func_8003FD2C( &D_80064380[D_80064378], msg, sizeof( UnkMsg8003C3D4 ) );
    D_80064378 = next;
    return 1;
}

/*
================
func_8003C46C

Player callback: handles the queued messages, then advances every active channel by one tick (commands, step streams, fade, envelope, modulation, voice update). Returns the time to the next call.
================
*/
s32 func_8003C46C( void *clientData ) {
    s32 i;
    UnkChannel *chan;
    f32 pitch;

    while ( D_80064374 != D_80064378 ) {
        func_8003C354( &D_80064380[D_80064374] );
        if ( ++D_80064374 == D_8006437C ) {
            D_80064374 = 0;
        }
    }
    for ( i = -4, chan = D_8006433C; i < D_80064334 - 4; i++, chan++ ) {
        if ( chan->unk_04 == NULL || ( chan->unk_00 & 1 ) ) {
            continue;
        }
        if ( chan->unk_08 != 0 ) {
            func_8003CCA4( chan, i );
        }
        chan->unk_0C += chan->unk_9C;
        if ( chan->unk_9A != 0x7FFF ) {
            while ( chan->unk_3C - chan->unk_0C < 0 ) {
                if ( chan->unk_04 == NULL ) {
                    break;
                }
                func_8003C75C( chan, i );
            }
            if ( chan->unk_04 == NULL ) {
                continue;
            }
        }
        if ( chan->unk_38 != NULL && chan->unk_14 - chan->unk_0C < 0 ) {
            func_8003D374( chan );
        }
        if ( chan->unk_34 != NULL && chan->unk_18 - chan->unk_0C < 0 ) {
            func_8003D438( chan );
        }
        if ( chan->unk_10 != -1 && --chan->unk_10 == -1 ) {
            chan->unk_38 = NULL;
            chan->unk_34 = NULL;
            chan->unk_74 = NULL;
            chan->unk_78 = NULL;
            chan->unk_44 = 0;
            chan->unk_08 = 0;
            chan->unk_04 = NULL;
            if ( chan->unk_C9 ) {
                chan->unk_C9 = 0;
                func_80042710( &D_80064338[i] );
            }
        }
        if ( chan->unk_C9 ) {
            if ( chan->unk_C3 ) {
                func_8003CFF0( chan );
            }
            if ( chan->unk_D4 && chan->unk_94 - chan->unk_0C < 0 ) {
                func_8003D200( chan );
            }
            pitch = chan->unk_30;
            if ( chan->unk_D5 ) {
                pitch += func_8003D310( chan );
            }
            if ( chan->unk_CE ) {
                pitch += func_8003D2B8( chan );
            }
            if ( chan->unk_08 == 0 ) {
                func_8003CE74( chan, i, pitch );
                func_8003CD20( chan, i );
            }
        }
        chan->unk_AA = (u32)( chan->unk_0C - chan->unk_40 ) >> 8;
    }
    return D_80064348;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C75C);

/*
================
func_8003CCA4

Stops voice index if the channel had started it, then starts it again with unk_08 and clears unk_08.
================
*/
void func_8003CCA4( UnkChannel *chan, s32 index ) {
    if ( chan->unk_C9 ) {
        func_80042710( &D_80064338[index] );
    }
    chan->unk_C9 = 1;
    func_80042680( &D_80064338[index], chan->unk_08 );
    chan->unk_08 = 0;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CD20);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CE74);

/*
================
func_8003CF80

Sets unk_54 from unk_40 and unk_B2, or from unk_3C and unk_B4, or to unk_40 + 0x7FFFFFFF when unk_9A is 0x7FFF; copies unk_C0 and unk_BF to unk_C4 and unk_C5 and sets unk_C3.
================
*/
void func_8003CF80( UnkChannel *chan ) {
    if ( chan->unk_9A != 0x7FFF ) {
        if ( chan->unk_B2 != 0 ) {
            chan->unk_54 = chan->unk_40 + ( chan->unk_B2 << 8 );
        } else {
            chan->unk_54 = chan->unk_3C - ( chan->unk_B4 << 8 );
        }
    } else {
        chan->unk_54 = chan->unk_40 + 0x7FFFFFFF;
    }
    chan->unk_C3 = 1;
    chan->unk_C4 = chan->unk_C0;
    chan->unk_C5 = chan->unk_BF;
}

/*
================
func_8003CFF0

Steps the envelope: phase unk_C3 (1 attack, 2 decay, 3 sustain, 4 release) and level unk_C4, every unk_BF calls; enters release once unk_0C passes unk_54.
================
*/
void func_8003CFF0( UnkChannel *chan ) {
    s32 time;

    if ( chan->unk_54 - chan->unk_0C < 0 && chan->unk_C3 < 4 ) {
        chan->unk_C3 = 4;
        chan->unk_C5 = 1;
        chan->unk_CD = chan->unk_C4;
    }
    if ( --chan->unk_C5 == 0 ) {
        chan->unk_C5 = chan->unk_BF;
        switch ( chan->unk_C3 ) {
        case 1:
            time = ( (u32)( chan->unk_0C - chan->unk_40 ) >> 8 ) * chan->unk_64 >> 10;
            if ( time < chan->unk_C6 ) {
                chan->unk_C4 = chan->unk_C0 + (s32)( chan->unk_58 * time );
            } else {
                chan->unk_C3++;
                chan->unk_C4 = chan->unk_C1;
            }
            break;
        case 2:
            time = ( ( (u32)( chan->unk_0C - chan->unk_40 ) >> 8 ) - chan->unk_C6 ) * chan->unk_64 >> 10;
            if ( time < chan->unk_C7 ) {
                chan->unk_C4 = chan->unk_C1 + (s32)( chan->unk_5C * time );
            } else {
                chan->unk_C3++;
                chan->unk_C4 = chan->unk_C2;
            }
            break;
        case 3:
            break;
        case 4:
            time = ( (u32)( chan->unk_0C - chan->unk_54 ) >> 8 ) * chan->unk_64 >> 10;
            if ( time < chan->unk_C8 ) {
                chan->unk_C4 = chan->unk_CD - (s32)( chan->unk_60 * time * chan->unk_CD );
            } else {
                chan->unk_C3++;
                chan->unk_C4 = 0;
            }
            break;
        }
    }
}

/*
================
func_8003D1E4

Copies unk_40 to unk_94, clears unk_D9 and sets unk_DA from bit 6 of unk_BD.
================
*/
void func_8003D1E4( UnkChannel *chan ) {
    chan->unk_D9 = 0;
    chan->unk_94 = chan->unk_40;
    chan->unk_DA = chan->unk_BD & 0x40;
}

/*
================
func_8003D200

Advances unk_94 in steps of 0x100 up to unk_0C, moving unk_BD up to 0x7F or down to 0 by unk_D4 / 64 per step, reversing at either end.
================
*/
void func_8003D200( UnkChannel *chan ) {
    u32 step;

    do {
        step = chan->unk_D9 + chan->unk_D4;
        chan->unk_94 += 0x100;
        if ( step < 0x40 ) {
            chan->unk_D9 = step;
        } else {
            chan->unk_D9 = step & 0x3F;
            step >>= 6;
            if ( !chan->unk_DA ) {
                chan->unk_BD += step;
                if ( chan->unk_BD >= 0x80 ) {
                    chan->unk_BD = 0x7F;
                    chan->unk_DA = 1;
                }
            } else {
                chan->unk_BD -= step;
                if ( chan->unk_BD >= 0x80 || chan->unk_BD == 0 ) {
                    chan->unk_BD = 0;
                    chan->unk_DA = 0;
                }
            }
        }
    } while ( chan->unk_94 - chan->unk_0C < 0 );
}

/*
================
func_8003D2B8

Counts down unk_D0; at zero toggles unk_D1 between 0 (for unk_CF steps) and unk_D8 (for unk_CE steps). Returns unk_D1.
================
*/
f32 func_8003D2B8( UnkChannel *chan ) {
    if ( --chan->unk_D0 == 0 ) {
        if ( chan->unk_D1 == 0 ) {
            chan->unk_D1 = chan->unk_D8;
            chan->unk_D0 = chan->unk_CE;
        } else {
            chan->unk_D1 = 0;
            chan->unk_D0 = chan->unk_CF;
        }
    }
    return chan->unk_D1;
}

/*
================
func_8003D310

Sets unk_68 to unk_20 * sin( ( unk_AA - unk_B6 ) * unk_DC ) and returns it; 0 while unk_AA has not passed unk_B6.
================
*/
f32 func_8003D310( UnkChannel *chan ) {
    s32 time;

    time = chan->unk_AA - chan->unk_B6;
    if ( time > 0 ) {
        return chan->unk_68 = __sinf( time * chan->unk_DC ) * chan->unk_20;
    }
    return 0.0f;
}

/*
================
func_8003D374

Advances unk_14 in steps of 0x100 up to unk_0C, reading a new unk_BC and step length from the unk_38 stream each time unk_A2 runs out.
================
*/
void func_8003D374( UnkChannel *chan ) {
    u8 value;

    do {
        chan->unk_14 += 0x100;
        if ( --chan->unk_A2 == 0 ) {
            value = *chan->unk_38++;
            if ( value >= 0x80 ) {
                chan->unk_BC = value & 0x7F;
                value = *chan->unk_38++;
                if ( value >= 0x80 ) {
                    chan->unk_A2 = ( value & 0x7F ) << 8;
                    chan->unk_A2 += *chan->unk_38++ + 2;
                } else {
                    chan->unk_A2 = value + 2;
                }
            } else {
                chan->unk_BC = value;
                chan->unk_A2 = 1;
            }
        }
    } while ( chan->unk_14 - chan->unk_0C < 0 );
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D438);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D54C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D6E0);

/*
================
func_8003D878

Returns a random number in [0, range), from eight steps of the shift register D_80064354.
================
*/
s32 func_8003D878( s32 range ) {
    s32 i;
    s32 bits;

    for ( i = 0; i < 8; i++ ) {
        bits = D_80064354 & 0x48000000;
        D_80064354 <<= 1;
        if ( bits == 0x48000000 || bits == 0x08000000 ) {
            D_80064354 |= 1;
        }
    }
    return range * ( (f32)D_80064354 / 65536.0f / 65536.0f );
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D90C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DA48);

/*
================
func_8003DC10

Adds base to every non-zero entry of a table of count offsets.
================
*/
void func_8003DC10( s32 *table, s32 base, s32 count ) {
    s32 i;

    for ( i = 0; i < count; i++ ) {
        if ( table[i] != 0 ) {
            table[i] += base;
        }
    }
}

/*
================
func_8003DC48

Starts entry index of bank on chan; returns the new channel id.
================
*/
s32 func_8003DC48( UnkChannel *chan, UnkSeqBank *bank, s32 index, s32 arg3, s32 arg4, s32 priority ) {
    func_8003D90C( chan );
    chan->unk_A6 = index;
    chan->unk_78 = bank;
    chan->unk_9E = arg3;
    chan->unk_B0 = arg4;
    chan->unk_44 = D_80064350++;
    chan->unk_48 = priority;
    if ( bank->unk_10 != 0 ) {
        chan->unk_7C = bank->unk_10;
    }
    chan->unk_04 = chan->unk_80 = bank->unk_18[index].unk_00;
    return chan->unk_44;
}

/*
================
func_8003DCF8

Starts entry index of bank on a free channel above the first four, or else on the bank channel of lowest priority below priority (-1: the entry's own); returns the channel id, or 0.
================
*/
s32 func_8003DCF8( UnkSeqBank *bank, s32 index, s32 arg2, s32 arg3, s32 priority ) {
    s32 i;
    s32 lowest;
    UnkChannel *chan;
    UnkChannel *victim;

    if ( priority == -1 ) {
        priority = bank->unk_18[index].unk_04;
    }
    lowest = priority + 1;
    for ( i = 4, chan = D_80064340; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_04 == NULL ) {
            func_8003D90C( chan );
            chan->unk_A6 = index;
            chan->unk_78 = bank;
            chan->unk_9E = arg2;
            chan->unk_B0 = arg3;
            chan->unk_48 = priority;
            chan->unk_44 = D_80064350++;
            if ( bank->unk_10 != NULL ) {
                chan->unk_7C = bank->unk_10;
            }
            chan->unk_04 = chan->unk_80 = bank->unk_18[index].unk_00;
            return chan->unk_44;
        }
        if ( chan->unk_78 != NULL && chan->unk_48 < lowest ) {
            lowest = chan->unk_48;
            victim = chan;
        }
    }
    if ( lowest < priority ) {
        func_8003D90C( victim );
        victim->unk_A6 = index;
        victim->unk_78 = bank;
        victim->unk_9E = arg2;
        victim->unk_B0 = arg3;
        victim->unk_48 = priority;
        victim->unk_44 = D_80064350++;
        if ( bank->unk_10 != NULL ) {
            victim->unk_7C = bank->unk_10;
        }
        victim->unk_04 = victim->unk_80 = bank->unk_18[index].unk_00;
        return victim->unk_44;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DEA0);

/*
================
func_8003E10C

Sets unk_00 to ( unk_00 & mask ) | bits on every channel with the given id.
================
*/
void func_8003E10C( s32 id, s32 mask, s32 bits ) {
    s32 i;
    UnkChannel *chan;

    for ( i = 0, chan = D_8006433C; i < D_80064334; i++, chan++ ) {
        if ( chan->unk_44 == id ) {
            chan->unk_00 = ( chan->unk_00 & mask ) | bits;
        }
    }
}

/*
================
func_8003E160

Channel command: ends the channel's command stream.
================
*/
u8 *func_8003E160( UnkChannel *chan, u8 *src ) {
    chan->unk_38 = NULL;
    chan->unk_34 = NULL;
    chan->unk_74 = NULL;
    chan->unk_78 = NULL;
    chan->unk_44 = 0;
    chan->unk_08 = 0;
    return NULL;
}

/* Reads a value encoded as one byte, or as two bytes when the first has bit 7 set; returns the next read position. */
u8 *func_8003E180(UnkChannel *obj, u8 *src) {
    u32 value = *src++;

    if (value & 0x80) {
        value = *src++ | ((value & 0x7F) << 8);
    }
    obj->unk_AE = value;
    return src;
}

/*
================
func_8003E1B0

Channel command: sets unk_B8 from one byte; a non-zero value also copies unk_2C to unk_50.
================
*/
u8 *func_8003E1B0( UnkChannel *chan, u8 *src ) {
    chan->unk_B8 = *src++;
    if ( chan->unk_B8 != 0 ) {
        chan->unk_50 = chan->unk_2C;
    }
    return src;
}

/*
================
func_8003E1D0

Channel command: clears unk_B8.
================
*/
u8 *func_8003E1D0( UnkChannel *chan, u8 *src ) {
    chan->unk_B8 = 0;
    return src;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E1DC);

/*
================
func_8003E304

Channel command: sets the tempo from one byte; a bank channel takes it alone, otherwise every channel of the same header does.
================
*/
u8 *func_8003E304( UnkChannel *chan, u8 *src ) {
    s32 tempo;
    s32 scaled;
    s32 i;
    UnkChannel *other;

    tempo = ( *src++ * 0x6000 / 120 ) / D_80064344;
    scaled = ( tempo * chan->unk_98 ) >> 7;
    if ( chan->unk_78 != NULL ) {
        chan->unk_9C = tempo;
    } else {
        for ( i = 0, other = D_8006433C; i < D_80064334; i++, other++ ) {
            if ( other->unk_74 == chan->unk_74 ) {
                other->unk_A8 = tempo;
                other->unk_9C = scaled;
            }
        }
    }
    return src;
}

/*
================
func_8003E3E8

Channel command: sets unk_B4 from one byte and clears unk_B2.
================
*/
u8 *func_8003E3E8( UnkChannel *chan, u8 *src ) {
    u8 value;

    value = *src++;
    chan->unk_B2 = 0;
    chan->unk_B4 = value;
    return src;
}

/*
================
func_8003E3FC

Channel command: sets unk_B2 from a big-endian 16-bit value and clears unk_B4.
================
*/
u8 *func_8003E3FC( UnkChannel *chan, u8 *src ) {
    s32 hi;
    s32 lo;

    hi = *src++;
    lo = *src++;
    chan->unk_B4 = 0;
    chan->unk_B2 = lo | ( hi << 8 );
    return src;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E420);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E490);

/*
================
func_8003E504

Channel command: clears unk_D5 and unk_68.
================
*/
u8 *func_8003E504( UnkChannel *chan, u8 *src ) {
    chan->unk_D5 = 0;
    chan->unk_68 = 0;
    return src;
}

/*
================
func_8003E514

Channel command: sets unk_AC from a value encoded as one byte, or as two bytes when the first is 0x80 or more.
================
*/
u8 *func_8003E514( UnkChannel *chan, u8 *src ) {
    s32 value;

    value = *src++;
    if ( value >= 0x80 ) {
        value &= 0x7F;
        value <<= 8;
        value |= *src++;
    }
    chan->unk_AC = value;
    return src;
}

/*
================
func_8003E544

Channel command: sets unk_B7.
================
*/
u8 *func_8003E544( UnkChannel *chan, u8 *src ) {
    chan->unk_B7 = 1;
    return src;
}

/*
================
func_8003E554

Channel command: sets unk_B9 from one byte.
================
*/
u8 *func_8003E554( UnkChannel *chan, u8 *src ) {
    chan->unk_B9 = *src++;
    return src;
}

/*
================
func_8003E564

Channel command: sets unk_BA.
================
*/
u8 *func_8003E564( UnkChannel *chan, u8 *src ) {
    chan->unk_BA = 1;
    return src;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E574);

/*
================
func_8003E5CC

Channel command: sets unk_D6.
================
*/
u8 *func_8003E5CC( UnkChannel *chan, u8 *src ) {
    chan->unk_D6 = 1;
    return src;
}

/*
================
func_8003E5DC

Channel command: clears unk_D6.
================
*/
u8 *func_8003E5DC( UnkChannel *chan, u8 *src ) {
    chan->unk_D6 = 0;
    return src;
}

/*
================
func_8003E5E8

Channel command: sets unk_D7.
================
*/
u8 *func_8003E5E8( UnkChannel *chan, u8 *src ) {
    chan->unk_D7 = 1;
    return src;
}

/*
================
func_8003E5F8

Channel command: clears unk_D7.
================
*/
u8 *func_8003E5F8( UnkChannel *chan, u8 *src ) {
    chan->unk_D7 = 0;
    return src;
}

/*
================
func_8003E604

Channel command: pushes the read position and the channel state onto the repeat stack, with a repeat count from one byte.
================
*/
u8 *func_8003E604( UnkChannel *chan, u8 *src ) {
    s32 depth;

    depth = chan->unk_DB;
    chan->unk_120[depth] = *src++;
    chan->unk_E0[depth] = src;
    chan->unk_F0[depth] = chan->unk_38;
    chan->unk_100[depth] = chan->unk_34;
    chan->unk_124[depth] = chan->unk_BC;
    chan->unk_128[depth] = chan->unk_70;
    chan->unk_110[depth] = chan->unk_A2;
    chan->unk_118[depth] = chan->unk_A4;
    chan->unk_DB++;
    return src;
}

/*
================
func_8003E670

Channel command: counts down the top repeat entry; jumps back to it and restores its state unless the count ran out, which pops it.
================
*/
u8 *func_8003E670( UnkChannel *chan, u8 *src ) {
    s32 depth;

    depth = chan->unk_DB - 1;
    if ( chan->unk_120[depth] != 0xFF ) {
        if ( --chan->unk_120[depth] == 0 ) {
            chan->unk_DB = depth;
            depth = -1;
        }
    }
    if ( depth >= 0 ) {
        src = chan->unk_E0[depth];
        chan->unk_38 = chan->unk_F0[depth];
        chan->unk_34 = chan->unk_100[depth];
        chan->unk_BC = chan->unk_124[depth];
        chan->unk_70 = chan->unk_128[depth];
        chan->unk_A2 = chan->unk_110[depth];
        chan->unk_A4 = chan->unk_118[depth];
        chan->unk_24 = chan->unk_70 * chan->unk_6C;
    }
    return src;
}

/*
================
func_8003E70C

Channel command: sets unk_D8, unk_CE and unk_CF from three bytes.
================
*/
u8 *func_8003E70C( UnkChannel *chan, u8 *src ) {
    chan->unk_D8 = *src++;
    chan->unk_CE = *src++;
    chan->unk_CF = *src++;
    return src;
}

/*
================
func_8003E734

Channel command: clears unk_CE.
================
*/
u8 *func_8003E734( UnkChannel *chan, u8 *src ) {
    chan->unk_CE = 0;
    return src;
}

/*
================
func_8003E740

Channel command: sets unk_D2.
================
*/
u8 *func_8003E740( UnkChannel *chan, u8 *src ) {
    chan->unk_D2 = 1;
    return src;
}

/*
================
func_8003E750

Channel command: clears unk_D2.
================
*/
u8 *func_8003E750( UnkChannel *chan, u8 *src ) {
    chan->unk_D2 = 0;
    return src;
}

/*
================
func_8003E75C

Channel command: sets unk_D3 from one byte and clears unk_D2.
================
*/
u8 *func_8003E75C( UnkChannel *chan, u8 *src ) {
    u8 value;

    value = *src++;
    chan->unk_D2 = 0;
    chan->unk_D3 = value;
    return src;
}

/*
================
func_8003E770

Channel command: sets unk_BD from half of one byte.
================
*/
u8 *func_8003E770( UnkChannel *chan, u8 *src ) {
    chan->unk_BD = *src++ >> 1;
    return src;
}

/*
================
func_8003E784

Channel command: skips two bytes.
================
*/
u8 *func_8003E784( UnkChannel *chan, u8 *src ) {
    return src + 2;
}

/*
================
func_8003E78C

Channel command: selects entry unk_84 of unk_74->unk_1C by a one- or two-byte index.
================
*/
u8 *func_8003E78C( UnkChannel *chan, u8 *src ) {
    s32 index;

    index = *src++;
    if ( index >= 0x80 ) {
        index &= 0x7F;
        index <<= 8;
        index |= *src++;
    }
    chan->unk_84 = &chan->unk_74->unk_1C[index];
    return src;
}

/*
================
func_8003E7D4

Channel command: clears unk_84.
================
*/
u8 *func_8003E7D4( UnkChannel *chan, u8 *src ) {
    chan->unk_84 = NULL;
    return src;
}

/*
================
func_8003E7E0

Channel command: skips one byte.
================
*/
u8 *func_8003E7E0( UnkChannel *chan, u8 *src ) {
    return src + 1;
}

/*
================
func_8003E7E8

Channel command: reads three big-endian 16-bit offsets; sets unk_38 and unk_34 from the last two and jumps to the first, relative to unk_80.
================
*/
u8 *func_8003E7E8( UnkChannel *chan, u8 *src ) {
    s32 jump;
    s32 offset;

    jump = *src++ << 8;
    jump += *src++;
    offset = *src++ << 8;
    offset += *src++;
    chan->unk_A2 = 1;
    chan->unk_38 = chan->unk_8C + offset;
    offset = *src++ << 8;
    offset += *src++;
    chan->unk_A4 = 1;
    chan->unk_34 = chan->unk_88 + offset;
    return chan->unk_80 + jump;
}

/*
================
func_8003E858

Channel command: sets unk_CA from one byte.
================
*/
u8 *func_8003E858( UnkChannel *chan, u8 *src ) {
    chan->unk_CA = *src++;
    return src;
}

/*
================
func_8003E868

Channel command: sets unk_B9 to a random number below the first byte plus the second byte.
================
*/
u8 *func_8003E868( UnkChannel *chan, u8 *src ) {
    chan->unk_B9 = func_8003D878( *src++ );
    chan->unk_B9 += *src++;
    return src;
}

/*
================
func_8003E8B8

Channel command: sets unk_BC to a random number below the first byte plus the second byte.
================
*/
u8 *func_8003E8B8( UnkChannel *chan, u8 *src ) {
    chan->unk_BC = func_8003D878( *src++ );
    chan->unk_BC += *src++;
    return src;
}

/*
================
func_8003E908

Channel command: sets unk_BD to a random number below the first byte plus the second byte.
================
*/
u8 *func_8003E908( UnkChannel *chan, u8 *src ) {
    chan->unk_BD = func_8003D878( *src++ );
    chan->unk_BD += *src++;
    return src;
}

/*
================
func_8003E958

Channel command: sets unk_BC from one byte.
================
*/
u8 *func_8003E958( UnkChannel *chan, u8 *src ) {
    chan->unk_BC = *src++;
    return src;
}

/*
================
func_8003E968

Channel command: starts entry (one- or two-byte index) of the channel's bank on another channel at a higher priority, and gives every channel started under the new id this channel's id and unk_7C.
================
*/
u8 *func_8003E968( UnkChannel *chan, u8 *src ) {
    s32 index;
    s32 id;
    s32 i;
    UnkChannel *other;

    index = *src++;
    if ( index >= 0x80 ) {
        index = ( ( index & 0x7F ) << 8 ) + *src++;
    }
    chan->unk_48++;
    id = func_8003DCF8( chan->unk_78, index, chan->unk_9E, chan->unk_B0, chan->unk_48 );
    chan->unk_48--;
    if ( id != 0 ) {
        for ( i = 0, other = D_8006433C; i < D_80064334; i++, other++ ) {
            if ( other->unk_44 == id ) {
                other->unk_44 = chan->unk_44;
                other->unk_7C = chan->unk_7C;
            }
        }
    }
    return src;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EA38);

/*
================
func_8003EA74

Channel command: sets unk_D4 from one byte.
================
*/
u8 *func_8003EA74( UnkChannel *chan, u8 *src ) {
    chan->unk_D4 = *src++;
    return src;
}

/*
================
func_8003EA84

Channel command: passes one byte to func_8003F340 while D_80064360 is 1.
================
*/
u8 *func_8003EA84( UnkChannel *chan, u8 *src ) {
    s32 arg;

    arg = *src++;
    if ( D_80064360 == 1 ) {
        func_8003F340( arg );
    }
    return src;
}

/*
================
func_8003EAC8

Channel command: passes the channel id and one byte to the D_80064370 callback when unk_00 & 3 is 2; skips a one- or two-byte value after it.
================
*/
u8 *func_8003EAC8( UnkChannel *chan, u8 *src ) {
    s32 value;
    s32 length;

    value = *src++;
    length = *src++;
    if ( length & 0x80 ) {
        length = ( ( length & 0x7F ) << 8 ) + *src++;
    }
    if ( ( chan->unk_00 & 3 ) == 2 && D_80064370 != NULL ) {
        D_80064370( chan->unk_44, value );
    }
    return src;
}

/*
================
func_8003EB38

Channel command: clears unk_AC.
================
*/
u8 *func_8003EB38( UnkChannel *chan, u8 *src ) {
    chan->unk_AC = 0;
    return src;
}
