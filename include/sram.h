#ifndef SRAM_H
#define SRAM_H

/* sram.h -- the SRAM PI handle (unit 47E0) */

#include "common.h"
#include "libultra.h"

extern OSPiHandle sram_handle;      /* the SRAM's PI handle, set up on first use */

OSPiHandle *Sram_GetHandle( void );

#endif
