#ifndef PVCODE_PVCODE_H
#define PVCODE_PVCODE_H

/* pvcode.h -- the pvcode overlay (file PVCODE  SYS, ROM 0x6A650, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "gamesnd.h"
#include "filecache.h"
#include "hostio.h"

extern objClass_t D_80112660_pvcode[];  /* the classes of the overlay's scene */
extern objParams_t D_801126C0_pvcode[]; /* the overlay's scene objects, unless read from the host */

#endif
