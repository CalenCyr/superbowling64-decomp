/* 4890.c -- byte link to a development host: handshake, file requests and the byte transfers */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "hostio.h"

/*
================
Host_Send

Sends len bytes from buf.
================
*/
void Host_Send( u8 *buf, s32 len ) {
    s32 i;

    for ( i = 0; i < len; i++ ) {
        Host_ExchangeByte( buf[i] );
    }
}

/*
================
Host_Recv

Receives len bytes into buf.
================
*/
void Host_Recv( u8 *buf, s32 len ) {
    s32 i;

    for ( i = 0; i < len; i++ ) {
        buf[i] = Host_RecvByte();
    }
}

/*
================
Host_Sync

Exchanges 0x20 / 0x10 with the host until it answers 0x10 / 0x20.
================
*/
s32 Host_Sync( void ) {
    while ( Host_ExchangeByte( 0x20 ) != 0x10 || Host_ExchangeByte( 0x10 ) != 0x20 ) {
    }
    return 0;
}

/*
================
strcpy

Copies the string src to dst.
================
*/
void strcpy( u8 *dst, const u8 *src ) {
    while ( *src != '\0' ) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

/*
================
Host_RequestFile

Sends request 1 for name.
================
*/
void Host_RequestFile( u8 *name ) {
    host_packet.command = 1;
    strcpy( host_packet.name, name );
    Host_Send( (u8 *)&host_packet, sizeof( host_packet ) );
}

/*
================
Host_GetFileSize

Sends request 2 for name and receives the file's size.
================
*/
s32 Host_GetFileSize( u8 *name ) {
    host_packet.command = 2;
    strcpy( host_packet.name, name );
    host_packet.size = 0x100000;
    Host_Send( (u8 *)&host_packet, sizeof( host_packet ) );
    Host_Recv( (u8 *)&host_fileSize, sizeof( host_fileSize ) );
    return host_fileSize;
}

/*
================
Host_ReadFile

Receives the host_fileSize bytes announced by Host_GetFileSize into buf.
================
*/
void Host_ReadFile( u8 *buf ) {
    Host_Recv( buf, host_fileSize );
}

/*
================
Host_LoadFile

Sends request 2 for name with size, then receives the size the host returns and that many bytes into buf.
================
*/
s32 Host_LoadFile( u8 *name, u8 *buf, s32 size ) {
    host_packet.command = 2;
    strcpy( host_packet.name, name );
    host_packet.size = size;
    Host_Send( (u8 *)&host_packet, sizeof( host_packet ) );
    Host_Recv( (u8 *)&size, sizeof( size ) );
    Host_Recv( buf, size );
    return size;
}

/*
================
Host_WriteFile

Sends request 3 for name, then the size bytes of buf.
================
*/
void Host_WriteFile( u8 *name, u8 *buf, s32 size ) {
    host_packet.command = 3;
    strcpy( host_packet.name, name );
    host_packet.size = size;
    Host_Send( (u8 *)&host_packet, sizeof( host_packet ) );
    Host_Send( buf, size );
}

/*
================
Host_ExchangeByte

Sends c once the link is ready; returns the byte read from the link at the same time.
================
*/
u8 Host_ExchangeByte( u8 c ) {
    u32 data;

    do {
        __osEPiRawReadIo( boot_romHandle, HOSTIO_STATUS, &data );
    } while ( !( data & HOSTIO_STATUS_READY ) );
    __osEPiRawReadIo( boot_romHandle, HOSTIO_DATA, &data );
    __osEPiRawWriteIo( boot_romHandle, HOSTIO_SEND, c << 16 );
    return data;
}

/*
================
Host_RecvByte

Receives a byte once the link is ready, echoing it back.
================
*/
u8 Host_RecvByte( void ) {
    u32 data;

    do {
        __osEPiRawReadIo( boot_romHandle, HOSTIO_STATUS, &data );
    } while ( !( data & HOSTIO_STATUS_READY ) );
    __osEPiRawReadIo( boot_romHandle, HOSTIO_DATA, &data );
    __osEPiRawWriteIo( boot_romHandle, HOSTIO_SEND, data << 16 );
    return data;
}
