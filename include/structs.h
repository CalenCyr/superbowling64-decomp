#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"

/* Message passed to func_8003C3D4; identity unknown. */
typedef struct {
    /* 0x00 */ u8 cmd;
    /* 0x04 */ s32 arg;
} UnkMsg8003C3D4;

/*
 * One element of the D_8006433C array (D_80064334 entries, 0x13C bytes each). Its byte-code read position
 * (unk_04) is advanced by the command handlers in D_8005CB70, so it is a sequence channel of some sort;
 * identity unknown.
 */
typedef struct {
    /* 0x000 */ s32 unk_00;
    /* 0x004 */ u8 *unk_04;         /* command read position */
    /* 0x008 */ s32 unk_08;
    /* 0x00C */ u8 unk_0C[0x20 - 0x0C];
    /* 0x020 */ f32 unk_20;
    /* 0x024 */ f32 unk_24;
    /* 0x028 */ u8 unk_28[0x2C - 0x28];
    /* 0x02C */ f32 unk_2C;
    /* 0x030 */ f32 unk_30;
    /* 0x034 */ u8 *unk_34;
    /* 0x038 */ u8 *unk_38;
    /* 0x03C */ u8 unk_3C[0x40 - 0x3C];
    /* 0x040 */ s32 unk_40;
    /* 0x044 */ s32 unk_44;
    /* 0x048 */ s32 unk_48;
    /* 0x04C */ u8 unk_4C[0x50 - 0x4C];
    /* 0x050 */ f32 unk_50;
    /* 0x054 */ u8 unk_54[0x58 - 0x54];
    /* 0x058 */ f32 unk_58;
    /* 0x05C */ f32 unk_5C;
    /* 0x060 */ f32 unk_60;
    /* 0x064 */ s32 unk_64;
    /* 0x068 */ s32 unk_68;
    /* 0x06C */ f32 unk_6C;
    /* 0x070 */ f32 unk_70;
    /* 0x074 */ s32 unk_74;
    /* 0x078 */ s32 unk_78;
    /* 0x07C */ s32 unk_7C;
    /* 0x080 */ u8 *unk_80;
    /* 0x084 */ s32 unk_84;
    /* 0x088 */ u8 *unk_88;
    /* 0x08C */ u8 *unk_8C;
    /* 0x090 */ f32 unk_90;
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s16 unk_98;
    /* 0x09A */ u8 unk_9A[0x9C - 0x9A];
    /* 0x09C */ s16 unk_9C;
    /* 0x09E */ s16 unk_9E;
    /* 0x0A0 */ u8 unk_A0[0xA2 - 0xA0];
    /* 0x0A2 */ u16 unk_A2;
    /* 0x0A4 */ u16 unk_A4;
    /* 0x0A6 */ u8 unk_A6[0xA8 - 0xA6];
    /* 0x0A8 */ s16 unk_A8;
    /* 0x0AA */ u8 unk_AA[0xAC - 0xAA];
    /* 0x0AC */ s16 unk_AC;
    /* 0x0AE */ s16 unk_AE;
    /* 0x0B0 */ s16 unk_B0;
    /* 0x0B2 */ s16 unk_B2;
    /* 0x0B4 */ s16 unk_B4;
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
    /* 0x0C3 */ u8 unk_C3[0xC6 - 0xC3];
    /* 0x0C6 */ u8 unk_C6;
    /* 0x0C7 */ u8 unk_C7;
    /* 0x0C8 */ u8 unk_C8;
    /* 0x0C9 */ u8 unk_C9;
    /* 0x0CA */ u8 unk_CA;
    /* 0x0CB */ u8 unk_CB[0xCE - 0xCB];
    /* 0x0CE */ u8 unk_CE;
    /* 0x0CF */ u8 unk_CF;
    /* 0x0D0 */ u8 unk_D0[0xD2 - 0xD0];
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
