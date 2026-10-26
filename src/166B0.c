#include "common.h"
#include "functions.h"
#include "structs.h"
#include "variables.h"

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B2B0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B3FC);

void func_8003B684(s32 flags, s16 value) {
    if (flags & 1) {
        D_8006434C = value;
    }
    if (flags & 2) {
        D_8006434E = value;
    }
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B6B0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B6E0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B930);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003B9B0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BAC8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BB94);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BC14);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BCC8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BD20);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BD80);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BDF0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BE60);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BEEC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BF74);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BFB0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BFD8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003BFFC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C008);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C064);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C088);

void func_8003C0B0(s32 arg) {
    UnkMsg8003C3D4 msg;

    msg.arg = arg;
    D_80064364 = arg;
    msg.cmd = 2;
    func_8003C3D4(&msg);
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C0E0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C13C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1C0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1C8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1D4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1E0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1EC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1F4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C1FC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C208);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C26C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C2E4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C2F0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C354);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C3D4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C46C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003C75C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CCA4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CD20);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CE74);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CF80);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003CFF0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D1E4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D200);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D2B8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D310);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D374);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D438);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D54C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D6E0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D878);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003D90C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DA48);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DC10);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DC48);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DCF8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003DEA0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E10C);

/*
================
func_8003E160

Channel command: ends the channel's command stream.
================
*/
u8 *func_8003E160( UnkChannel *chan, u8 *src ) {
    chan->unk_38 = NULL;
    chan->unk_34 = NULL;
    chan->unk_74 = 0;
    chan->unk_78 = 0;
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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E304);

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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E604);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E670);

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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E78C);

/*
================
func_8003E7D4

Channel command: clears unk_84.
================
*/
u8 *func_8003E7D4( UnkChannel *chan, u8 *src ) {
    chan->unk_84 = 0;
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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E7E8);

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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E868);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E8B8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E908);

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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E968);

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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EA84);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EAC8);

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
