#ifndef LIBC_BOOT_H
#define LIBC_BOOT_H

/*
libc_boot.h -- the KMC libc functions (ROM 0x30C20-0x32150) that units 1060-51F0 call, declared under their
current placeholder names until the libc units provide their own header.
*/

#include "common.h"

s32 func_80056530( const u8 *a, const u8 *b, s32 n );      /* strncmp */
void *func_80056150( void *dst, s32 c, u32 n );           /* memset */

#endif
