#ifndef HOSTIO_H
#define HOSTIO_H

/* hostio.h -- byte link to a development host through PI I/O registers at 0x11000000 (unit 4890) */

#include "common.h"
#include "libultra.h"
#include "libc.h"

/* PI I/O registers of the host link; the status bit is set when the link can take the next byte */
#define HOSTIO_DATA         0x11000000      /* read: the byte from the host, in the low 8 bits */
#define HOSTIO_SEND         0x11000004      /* write: the byte to the host, in bits 16-23 */
#define HOSTIO_STATUS       0x11000008
#define HOSTIO_STATUS_READY 0x01

/*
Request header sent before each transfer (host_packet). The names of the type and its members are invented
from their use in unit 4890.
*/
typedef struct {
    /* 0x00 */ s32 command;         /* 1, 2 or 3 */
    /* 0x04 */ s32 size;            /* byte count of the transfer that follows */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ u8 name[0x50];       /* NUL-terminated file name */
} HostPacket;                       /* size 0x5C */

extern HostPacket host_packet;      /* the request header of the next transfer */

void Host_Send( u8 *buf, s32 len );
void Host_Recv( u8 *buf, s32 len );
s32 Host_Sync( void );
void Host_RequestFile( u8 *name );
s32 Host_GetFileSize( u8 *name );
void Host_ReadFile( u8 *buf );
s32 Host_LoadFile( u8 *name, u8 *buf, s32 size );
void Host_WriteFile( u8 *name, u8 *buf, s32 size );
u8 Host_ExchangeByte( u8 c );
u8 Host_RecvByte( void );

#endif
