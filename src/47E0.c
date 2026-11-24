/* 47E0.c -- the SRAM PI handle */

#include "common.h"
#include "libultra.h"
#include "sram.h"

/* PI domain 2 timing of the SRAM, as in the SDK's SRAM sample */
#define SRAM_START_ADDR     0x08000000
#define SRAM_LATENCY        0x05
#define SRAM_PULSE          0x0C
#define SRAM_PAGE_SIZE      0x0D
#define SRAM_REL_DURATION   0x02

/*
================
Sram_GetHandle

Sets up the SRAM handle and adds it to the PI handle table, once; returns it.
================
*/
OSPiHandle *Sram_GetHandle( void ) {
    if ( sram_handle.baseAddress == PHYS_TO_K1( SRAM_START_ADDR ) ) {
        return &sram_handle;
    }
    sram_handle.type = DEVICE_TYPE_SRAM;
    sram_handle.baseAddress = PHYS_TO_K1( SRAM_START_ADDR );
    sram_handle.latency = SRAM_LATENCY;
    sram_handle.pulse = SRAM_PULSE;
    sram_handle.pageSize = SRAM_PAGE_SIZE;
    sram_handle.relDuration = SRAM_REL_DURATION;
    sram_handle.domain = PI_DOMAIN2;
    sram_handle.speed = 0;
    bzero( &sram_handle.transferInfo, sizeof( sram_handle.transferInfo ) );
    osEPiLinkHandle( &sram_handle );
    return &sram_handle;
}
