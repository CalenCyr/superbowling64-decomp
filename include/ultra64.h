#ifndef ULTRA64_H
#define ULTRA64_H

/* The libultra declarations this project's C units use, as the SDK declares them. */

#include "common.h"

#define OS_TV_PAL   0
#define OS_TV_NTSC  1
#define OS_TV_MPAL  2

extern s32 osTvType;

void osWritebackDCacheAll( void );
float __sinf( float x );

#endif
