/* 6A650.c -- pvcode overlay */

#include "common.h"
#include "libultra.h"
#include "pvcode/pvcode.h"

/*
================
func_80112350_pvcode

Entry of the overlay: Start switches to files from the development host; from the host, the scene's
objects come from its "sce.dat", else from the overlay's own records.
================
*/
void func_80112350_pvcode( void ) {
    if ( D_800994BC & START_BUTTON ) {
        D_8005C0BC = 1;
        func_80029524();
    }
    if ( D_8005C0BC ) {
        func_8002A7EC( "sce.dat" );
        D_8005C0BD = 1;
        func_8002A1EC( D_80112660_pvcode, (objParams_t *)D_800F30E0 );
    } else {
        func_8002A1EC( D_80112660_pvcode, D_801126C0_pvcode );
    }
}

INCLUDE_ASM("asm/nonmatchings/pvcode/6A650", func_801123D8_pvcode);
