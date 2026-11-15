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
#include "save.h"

/* main unit 111A0 (no header yet) */
extern u8 D_8005C980;               /* set: func_80035DA0 runs each frame; it clears it when done */
void func_80035DA0( void );

extern s32 D_80112650_pvcode;       /* 0: func_80037410 is still stepping */
extern u8 D_80112654_pvcode;        /* sound the D-pad picks */
extern u64 D_80112658_pvcode;       /* handle of the playing sound */

extern objClass_t D_80112660_pvcode[];  /* the classes of the overlay's scene */
extern objParams_t D_801126C0_pvcode[]; /* the overlay's scene objects, unless read from the host */

#endif
