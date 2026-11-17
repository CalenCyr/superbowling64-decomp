#ifndef OPCODE_OPCODE_H
#define OPCODE_OPCODE_H

/* opcode.h -- the opcode overlay (file OPCODE  SYS, ROM 0x6BE70, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "gamesnd.h"
#include "game.h"
#include "sprite.h"

/* main (owner unit not identified yet) */
extern u8 D_8005C0E8;               /* the next mode: 0xA on entry; the overlay sets 0xFE or 0xFD */

extern u8 D_801140B0_opcode;         /* step of func_80112958_opcode */
extern u16 D_801140C0_opcode[];     /* character code 0x10-0x94 -> entry (func_80113130_opcode) */

extern u16 D_80114350_opcode;     /* frames in the current mode */
extern s32 D_80114360_opcode;     /* message to and from the controller thread */

s32 func_80112958_opcode( void );
s32 func_80113168_opcode( void );

#endif
