#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"

/*
 * Element of the D_80064338 array, one per channel above the first four. Its size (0x1C) and the
 * { s16, s16, u8 } configuration func_8003B3FC passes to func_80042220 when it sets one up match libultra's
 * ALVoice and alSynAllocVoice; unconfirmed.
 */
typedef struct {
    /* 0x00 */ u8 unk_00[0x1C];
} UnkVoice;

/* Message passed to func_8003C3D4; identity unknown. */
typedef struct {
    /* 0x00 */ u8 cmd;
    /* 0x04 */ s32 arg;
} UnkMsg8003C3D4;

/* 6-byte entry of UnkSeqHeader.unk_1C; identity unknown. */
typedef struct {
    /* 0x00 */ u16 unk_00;
    /* 0x02 */ u16 unk_02;
    /* 0x04 */ u8 unk_04;
    /* 0x05 */ u8 unk_05;
} UnkSeqEntry;

/*
 * Header that UnkChannel.unk_74 points to. func_8003DEA0 turns its offsets (unk_0C to unk_24) into pointers in
 * place and starts one channel per entry of unk_0C; identity unknown.
 */
typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 unk_04;          /* entries in unk_0C, unk_10 and unk_14 */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ u8 **unk_0C;
    /* 0x10 */ u8 **unk_10;
    /* 0x14 */ u8 **unk_14;
    /* 0x18 */ s32 unk_18;
    /* 0x1C */ UnkSeqEntry *unk_1C;
    /* 0x20 */ u16 *unk_20;
    /* 0x24 */ u8 *unk_24;
    /* 0x28 */ s32 unk_28;          /* set once the offsets are relocated */
} UnkSeqHeader;

/*
 * Bank that func_8003D6E0 relocates in place (bit 31 of unk_10 marks it done); D_80064358 and D_8006435C point
 * to one; identity unknown.
 */
typedef struct {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ s32 unk_10;          /* bit 31: relocated */
    /* 0x14 */ u8 unk_14[0x20 - 0x14];
    /* 0x20 */ s32 unk_20;          /* entries in unk_28 and unk_2C */
    /* 0x24 */ u8 *unk_24;
    /* 0x28 */ f32 *unk_28;
    /* 0x2C */ s32 *unk_2C;
} UnkSoundBank;

/* Entry of UnkSeqBank.unk_18. */
typedef struct {
    /* 0x00 */ u8 *unk_00;          /* command stream; an offset until func_8003C13C relocates it */
    /* 0x04 */ s32 unk_04;
} UnkSeqBankEntry;

/*
 * Table that UnkChannel.unk_78 points to; func_8003C13C relocates it in place, func_8003DC48 and func_8003DCF8
 * start a channel on one of its entries; identity unknown.
 */
typedef struct {
    /* 0x00 */ s32 unk_00;          /* entries in unk_18 */
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ s32 unk_0C;          /* bit 0: relocated */
    /* 0x10 */ UnkSoundBank *unk_10;
    /* 0x14 */ u8 *unk_14;
    /* 0x18 */ UnkSeqBankEntry unk_18[1];  /* unk_00 entries */
} UnkSeqBank;

/* Object written by func_8003C1EC and read by func_8003C1C0 and func_8003C1F4; identity unknown. */
typedef struct {
    /* 0x00 */ u8 unk_00[0x04];
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ u8 unk_08[0x10 - 0x08];
    /* 0x10 */ UnkSoundBank *unk_10;
} UnkStruct8003C1EC;

/*
 * One element of the D_8006433C array (D_80064334 entries, 0x13C bytes each). Its byte-code read position
 * (unk_04) is advanced by the command handlers in D_8005CB70, so it is a sequence channel of some sort;
 * identity unknown.
 */
typedef struct {
    /* 0x000 */ s32 unk_00;
    /* 0x004 */ u8 *unk_04;         /* command read position */
    /* 0x008 */ s32 unk_08;
    /* 0x00C */ s32 unk_0C;
    /* 0x010 */ s32 unk_10;          /* -1: none */
    /* 0x014 */ s32 unk_14;
    /* 0x018 */ u8 unk_18[0x1C - 0x18];
    /* 0x01C */ u32 unk_1C;
    /* 0x020 */ f32 unk_20;
    /* 0x024 */ f32 unk_24;
    /* 0x028 */ f32 unk_28;
    /* 0x02C */ f32 unk_2C;
    /* 0x030 */ f32 unk_30;
    /* 0x034 */ u8 *unk_34;
    /* 0x038 */ u8 *unk_38;
    /* 0x03C */ s32 unk_3C;
    /* 0x040 */ s32 unk_40;
    /* 0x044 */ s32 unk_44;
    /* 0x048 */ s32 unk_48;
    /* 0x04C */ u8 unk_4C[0x50 - 0x4C];
    /* 0x050 */ f32 unk_50;
    /* 0x054 */ s32 unk_54;
    /* 0x058 */ f32 unk_58;
    /* 0x05C */ f32 unk_5C;
    /* 0x060 */ f32 unk_60;
    /* 0x064 */ s32 unk_64;
    /* 0x068 */ f32 unk_68;
    /* 0x06C */ f32 unk_6C;
    /* 0x070 */ f32 unk_70;
    /* 0x074 */ UnkSeqHeader *unk_74;
    /* 0x078 */ UnkSeqBank *unk_78;
    /* 0x07C */ UnkSoundBank *unk_7C;
    /* 0x080 */ u8 *unk_80;
    /* 0x084 */ UnkSeqEntry *unk_84;
    /* 0x088 */ u8 *unk_88;
    /* 0x08C */ u8 *unk_8C;
    /* 0x090 */ f32 unk_90;
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s16 unk_98;
    /* 0x09A */ u16 unk_9A;
    /* 0x09C */ s16 unk_9C;
    /* 0x09E */ s16 unk_9E;
    /* 0x0A0 */ u16 unk_A0;
    /* 0x0A2 */ u16 unk_A2;
    /* 0x0A4 */ u16 unk_A4;
    /* 0x0A6 */ s16 unk_A6;          /* entry of unk_78->unk_18 the channel was started on */
    /* 0x0A8 */ s16 unk_A8;
    /* 0x0AA */ u16 unk_AA;
    /* 0x0AC */ s16 unk_AC;
    /* 0x0AE */ s16 unk_AE;
    /* 0x0B0 */ s16 unk_B0;
    /* 0x0B2 */ u16 unk_B2;
    /* 0x0B4 */ u16 unk_B4;
    /* 0x0B6 */ u8 unk_B6;
    /* 0x0B7 */ u8 unk_B7;
    /* 0x0B8 */ u8 unk_B8;
    /* 0x0B9 */ u8 unk_B9;
    /* 0x0BA */ u8 unk_BA;
    /* 0x0BB */ u8 unk_BB;
    /* 0x0BC */ u8 unk_BC;
    /* 0x0BD */ u8 unk_BD;
    /* 0x0BE */ u8 unk_BE;
    /* 0x0BF */ u8 unk_BF;
    /* 0x0C0 */ u8 unk_C0;
    /* 0x0C1 */ u8 unk_C1;
    /* 0x0C2 */ u8 unk_C2;
    /* 0x0C3 */ u8 unk_C3;
    /* 0x0C4 */ u8 unk_C4;
    /* 0x0C5 */ u8 unk_C5;
    /* 0x0C6 */ u8 unk_C6;
    /* 0x0C7 */ u8 unk_C7;
    /* 0x0C8 */ u8 unk_C8;
    /* 0x0C9 */ u8 unk_C9;
    /* 0x0CA */ u8 unk_CA;
    /* 0x0CB */ u8 unk_CB;
    /* 0x0CC */ u8 unk_CC;
    /* 0x0CD */ u8 unk_CD;
    /* 0x0CE */ u8 unk_CE;
    /* 0x0CF */ u8 unk_CF;
    /* 0x0D0 */ u8 unk_D0;
    /* 0x0D1 */ s8 unk_D1;
    /* 0x0D2 */ u8 unk_D2;
    /* 0x0D3 */ u8 unk_D3;
    /* 0x0D4 */ u8 unk_D4;
    /* 0x0D5 */ u8 unk_D5;
    /* 0x0D6 */ u8 unk_D6;
    /* 0x0D7 */ u8 unk_D7;
    /* 0x0D8 */ u8 unk_D8;
    /* 0x0D9 */ u8 unk_D9;
    /* 0x0DA */ u8 unk_DA;
    /* 0x0DB */ u8 unk_DB;          /* depth of the stack below */
    /* 0x0DC */ f32 unk_DC;
    /* stack of four entries, pushed by func_8003E604 and popped by func_8003E670 */
    /* 0x0E0 */ u8 *unk_E0[4];      /* read position after the push command */
    /* 0x0F0 */ u8 *unk_F0[4];      /* saved unk_38 */
    /* 0x100 */ u8 *unk_100[4];     /* saved unk_34 */
    /* 0x110 */ u16 unk_110[4];     /* saved unk_A2 */
    /* 0x118 */ u16 unk_118[4];     /* saved unk_A4 */
    /* 0x120 */ u8 unk_120[4];      /* repeat count; 0xFF repeats forever */
    /* 0x124 */ u8 unk_124[4];      /* saved unk_BC */
    /* 0x128 */ f32 unk_128[4];     /* saved unk_70 */
    /* 0x138 */ u8 unk_138[0x13C - 0x138];
} UnkChannel;

#endif
