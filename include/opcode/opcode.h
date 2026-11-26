#ifndef OPCODE_OPCODE_H
#define OPCODE_OPCODE_H

/* opcode.h -- the opcode overlay (file OPCODE  SYS, ROM 0x6BE70, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "gamesnd.h"
#include "game.h"
#include "sprite.h"

/* main (owner unit not identified yet) */
extern u8 op_nextMode; /* the next mode: 0xA on entry; the overlay sets 0xFE or 0xFD */

/* An image of the Controller Pak screen; a list of them ends with an id of -1. Name invented. */
typedef struct {
    /* 0x00 */ s16 id;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ u8 alpha;            /* 0: not drawn */
} opImage_t;                        /* size 0x8 */

/* A note's name in glyphs (OP_ConvertPakChar), for Txt_DrawGlyphs. Name invented. */
typedef struct {
    /* 0x00 */ u16 name[16];
    /* 0x20 */ u16 ext;
    /* 0x22 */ u16 end;             /* 0xFFFF */
} opNoteName_t;                     /* size 0x24 */

#define PAK_MAX_NOTES   16
#define PAK_NOTES_SHOWN 5

s32 OP_CheckSettings( void );
s32 OP_PakScreen( void );

#endif
