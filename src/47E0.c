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
func_800293E0

Sets up the SRAM handle and adds it to the PI handle table, once; returns it.
================
*/
OSPiHandle *func_800293E0( void ) {
    if ( D_80069338.baseAddress == PHYS_TO_K1( SRAM_START_ADDR ) ) {
        return &D_80069338;
    }
    D_80069338.type = DEVICE_TYPE_SRAM;
    D_80069338.baseAddress = PHYS_TO_K1( SRAM_START_ADDR );
    D_80069338.latency = SRAM_LATENCY;
    D_80069338.pulse = SRAM_PULSE;
    D_80069338.pageSize = SRAM_PAGE_SIZE;
    D_80069338.relDuration = SRAM_REL_DURATION;
    D_80069338.domain = PI_DOMAIN2;
    D_80069338.speed = 0;
    bzero( &D_80069338.transferInfo, sizeof( D_80069338.transferInfo ) );
    osEPiLinkHandle( &D_80069338 );
    return &D_80069338;
}
