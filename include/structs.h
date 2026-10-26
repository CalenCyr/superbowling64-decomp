#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"

/* Message passed to func_8003C3D4; identity unknown. */
typedef struct {
    /* 0x00 */ u8 cmd;
    /* 0x04 */ s32 arg;
} UnkMsg8003C3D4;

/* Object whose field 0xAE func_8003E180 sets from a 1- or 2-byte encoded value; identity unknown. */
typedef struct {
    /* 0x00 */ u8 unk_00[0xAE];
    /* 0xAE */ s16 unk_AE;
} UnkObj8003E180;

#endif
