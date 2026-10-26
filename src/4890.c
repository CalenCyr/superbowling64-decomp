/* 4890.c -- byte link to a development host: handshake, file requests and the byte transfers */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "hostio.h"

/*
================
func_80029490

Sends len bytes from buf.
================
*/
void func_80029490( u8 *buf, s32 len ) {
    s32 i;

    for ( i = 0; i < len; i++ ) {
        func_800298D4( buf[i] );
    }
}

/*
================
func_800294D8

Receives len bytes into buf.
================
*/
void func_800294D8( u8 *buf, s32 len ) {
    s32 i;

    for ( i = 0; i < len; i++ ) {
        buf[i] = func_8002994C();
    }
}

/*
================
func_80029524

Exchanges 0x20 / 0x10 with the host until it answers 0x10 / 0x20.
================
*/
s32 func_80029524( void ) {
    while ( func_800298D4( 0x20 ) != 0x10 || func_800298D4( 0x10 ) != 0x20 ) {
    }
    return 0;
}

/*
================
func_80029578

Copies the string src to dst.
================
*/
void func_80029578( u8 *dst, u8 *src ) {
    while ( *src != '\0' ) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

/*
================
func_800295A0

Sends request 1 for name.
================
*/
void func_800295A0( u8 *name ) {
    D_800692D8.command = 1;
    func_80029578( D_800692D8.name, name );
    func_80029490( (u8 *)&D_800692D8, sizeof( D_800692D8 ) );
}

/*
================
func_8002961C

Sends request 2 for name and receives the file's size.
================
*/
s32 func_8002961C( u8 *name ) {
    D_800692D8.command = 2;
    func_80029578( D_800692D8.name, name );
    D_800692D8.size = 0x100000;
    func_80029490( (u8 *)&D_800692D8, sizeof( D_800692D8 ) );
    func_800294D8( (u8 *)&D_800619F0, sizeof( D_800619F0 ) );
    return D_800619F0;
}

/*
================
func_800296D4

Receives the D_800619F0 bytes announced by func_8002961C into buf.
================
*/
void func_800296D4( u8 *buf ) {
    func_800294D8( buf, D_800619F0 );
}

/*
================
func_80029728

Sends request 2 for name with size, then receives the size the host returns and that many bytes into buf.
================
*/
s32 func_80029728( u8 *name, u8 *buf, s32 size ) {
    D_800692D8.command = 2;
    func_80029578( D_800692D8.name, name );
    D_800692D8.size = size;
    func_80029490( (u8 *)&D_800692D8, sizeof( D_800692D8 ) );
    func_800294D8( (u8 *)&size, sizeof( size ) );
    func_800294D8( buf, size );
    return size;
}

/*
================
func_80029814

Sends request 3 for name, then the size bytes of buf.
================
*/
void func_80029814( u8 *name, u8 *buf, s32 size ) {
    D_800692D8.command = 3;
    func_80029578( D_800692D8.name, name );
    D_800692D8.size = size;
    func_80029490( (u8 *)&D_800692D8, sizeof( D_800692D8 ) );
    func_80029490( buf, size );
}

/*
================
func_800298D4

Sends c once the link is ready; returns the byte read from the link at the same time.
================
*/
u8 func_800298D4( u8 c ) {
    u32 data;

    do {
        __osEPiRawReadIo( D_800ABD60, HOSTIO_STATUS, &data );
    } while ( !( data & HOSTIO_STATUS_READY ) );
    __osEPiRawReadIo( D_800ABD60, HOSTIO_DATA, &data );
    __osEPiRawWriteIo( D_800ABD60, HOSTIO_SEND, c << 16 );
    return data;
}

/*
================
func_8002994C

Receives a byte once the link is ready, echoing it back.
================
*/
u8 func_8002994C( void ) {
    u32 data;

    do {
        __osEPiRawReadIo( D_800ABD60, HOSTIO_STATUS, &data );
    } while ( !( data & HOSTIO_STATUS_READY ) );
    __osEPiRawReadIo( D_800ABD60, HOSTIO_DATA, &data );
    __osEPiRawWriteIo( D_800ABD60, HOSTIO_SEND, data << 16 );
    return data;
}
