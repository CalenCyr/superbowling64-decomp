#ifndef HOSTIO_H
#define HOSTIO_H

/* hostio.h -- byte link to a development host through PI I/O registers at 0x11000000 (unit 4890) */

#include "common.h"
#include "libultra.h"

/* PI I/O registers of the host link; the status bit is set when the link can take the next byte */
#define HOSTIO_DATA         0x11000000      /* read: the byte from the host, in the low 8 bits */
#define HOSTIO_SEND         0x11000004      /* write: the byte to the host, in bits 16-23 */
#define HOSTIO_STATUS       0x11000008
#define HOSTIO_STATUS_READY 0x01

/*
Request header sent before each transfer (D_800692D8). The names of the type and its members are invented
from their use in unit 4890.
*/
typedef struct {
    /* 0x00 */ s32 command;         /* 1, 2 or 3 */
    /* 0x04 */ s32 size;            /* byte count of the transfer that follows */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ u8 name[0x50];       /* NUL-terminated file name */
} HostPacket;                       /* size 0x5C */

extern HostPacket D_800692D8;
extern s32 D_800619F0;

void func_80029490( u8 *buf, s32 len );
void func_800294D8( u8 *buf, s32 len );
s32 func_80029524( void );
void func_80029578( u8 *dst, u8 *src );
void func_800295A0( u8 *name );
s32 func_8002961C( u8 *name );
void func_800296D4( u8 *buf );
s32 func_80029728( u8 *name, u8 *buf, s32 size );
void func_80029814( u8 *name, u8 *buf, s32 size );
u8 func_800298D4( u8 c );
u8 func_8002994C( void );

#endif
