#ifndef PVCODE_PVCODE_H
#define PVCODE_PVCODE_H

/* pvcode.h -- the pvcode overlay (file PVCODE  SYS, ROM 0x6A650, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "gamesnd.h"
#include "filecache.h"
#include "hostio.h"
#include "save.h"
#include "gridedit.h"

extern s32 pv_stepDone;    /* 0: Save_SaveSettings is still stepping */
extern u8 pv_sound;        /* sound the D-pad picks */
extern u64 pv_soundHandle; /* handle of the playing sound */

extern objClass_t pv_classes[];       /* the classes of the overlay's scene */
extern objParams_t pv_sceneObjects[]; /* the overlay's scene objects, unless read from the host */

#endif
