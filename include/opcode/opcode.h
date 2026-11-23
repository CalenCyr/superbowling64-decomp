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
extern u8 D_8005C0E8;               /* the next mode: 0xA on entry; the overlay sets 0xFE or 0xFD */

extern u8 D_801140B0_opcode;         /* step of func_80112958_opcode */
extern u16 D_801140C0_opcode[];     /* character code 0x10-0x94 -> entry (func_80113130_opcode) */

/* An image of the Controller Pak screen; a list of them ends with an id of -1. Name invented. */
typedef struct {
    /* 0x00 */ s16 id;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ u8 alpha;            /* 0: not drawn */
} opImage_t;                        /* size 0x8 */

/* A note's name in glyphs (func_80113130_opcode), for func_80029088. Name invented. */
typedef struct {
    /* 0x00 */ u16 name[16];
    /* 0x20 */ u16 ext;
    /* 0x22 */ u16 end;             /* 0xFFFF */
} opNoteName_t;                     /* size 0x24 */

#define PAK_MAX_NOTES   16
#define PAK_NOTES_SHOWN 5

extern opImage_t D_801141CC_opcode[];   /* images of the Controller Pak screen */
extern u16 D_8011429C_opcode[3];        /* cursor images, a frame each */
extern u8 D_801142A2_opcode;            /* step of func_80113168_opcode */
extern u8 D_801142A3_opcode;            /* choice of the first prompt */
extern u8 D_801142A4_opcode;            /* line of the cursor */
extern u8 D_801142A5_opcode;            /* state of the screen */

extern u16 D_80114350_opcode;     /* frames in the current mode */
extern s32 D_80114360_opcode;     /* message to and from the controller thread */
extern s32 D_80114370_opcode;     /* func_80113168_opcode's message to and from the controller thread */
extern u16 D_80114374_opcode;      /* notes listed */
extern u16 D_80114376_opcode;      /* first note shown */
extern u16 D_80114378_opcode[PAK_MAX_NOTES];    /* per note listed: its index on the pak */
extern u16 D_80114398_opcode[PAK_MAX_NOTES];    /* per note listed: its pages */
extern opNoteName_t D_801143B8_opcode[PAK_MAX_NOTES];
extern s32 D_801145F8_opcode;     /* free notes */
extern s32 D_801145FC_opcode;     /* free pages */

s32 func_80112958_opcode( void );
s32 func_80113168_opcode( void );

#endif
