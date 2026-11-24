#ifndef SRAM_H
#define SRAM_H

/* sram.h -- the SRAM PI handle (unit 47E0) */

#include "common.h"
#include "libultra.h"

extern OSPiHandle D_80069338;

OSPiHandle *Sram_GetHandle( void );

#endif
