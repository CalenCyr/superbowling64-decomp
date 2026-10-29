/* libultra.h -- the SDK's own declarations (lib/ultralib/include/PR), on top of common.h's integer types */

#ifndef LIBULTRA_H
#define LIBULTRA_H

#include "common.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif

/*
common.h defines s32 and u32 as int, where the SDK's ultratypes.h uses long: the same 32-bit types under this
ABI, but C rejects a second typedef of one name. ultratypes.h is therefore skipped, and the rest of what it
declares is supplied here.
*/
#define _ULTRATYPES_H_

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* the game's microcode is F3DEX 2.08 */
#define F3DEX_GBI_2

/* libultra 2.0K: selects the SDK's 2.0J-and-later declarations (osMotorStart as a macro, and so on) */
#include <PR/os_version.h>
#define BUILD_VERSION VERSION_K

#include <PR/rcp.h>
#include <PR/os.h>
#include <PR/mbi.h>
#include <PR/sptask.h>
#include <PR/ucode.h>
#include <PR/gu.h>
#include <PR/libaudio.h>
#include <PR/sched.h>

#endif
