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

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E160);

/* Reads a value encoded as one byte, or as two bytes when the first has bit 7 set; returns the next read position. */
u8 *func_8003E180(UnkObj8003E180 *obj, u8 *src) {
    u32 value = *src++;

    if (value & 0x80) {
        value = *src++ | ((value & 0x7F) << 8);
    }
    obj->unk_AE = value;
    return src;
}

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E1B0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E1D0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E1DC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E304);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E3E8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E3FC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E420);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E490);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E504);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E514);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E544);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E554);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E564);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E574);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E5CC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E5DC);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E5E8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E5F8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E604);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E670);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E70C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E734);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E740);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E750);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E75C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E770);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E784);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E78C);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E7D4);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E7E0);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E7E8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E858);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E868);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E8B8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E908);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E958);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003E968);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EA38);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EA74);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EA84);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EAC8);

INCLUDE_ASM("asm/nonmatchings/166B0", func_8003EB38);
