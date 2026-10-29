#ifndef DBGTEXT_H
#define DBGTEXT_H

/* dbgtext.h -- text and number drawing into a display list (unit 3D50) */

#include "common.h"
#include "libultra.h"

#define GLYPH_DIGIT0    0xF0        /* glyph index of '0' in the D_800619E0 font */
#define GLYPH_END       0xFFFF      /* ends a glyph string */

extern u8 D_8005B1B0[];             /* palette of the D_8005B8B0 font */
extern u8 D_8005B8B0[];             /* 128x32 CI4 font, characters 0x20-0x5F in rows of 16 */
extern u8 D_8005C190;
extern u8 *D_800619E0;              /* CI4 glyph sheet, 64 pixels wide, 8x16 glyphs */

void func_80028950( Gfx **gdl, s32 palette );
void func_80028A0C( Gfx **gdl );
void func_80028BF4( Gfx **gdl, u8 mode, u8 primR, u8 primG, u8 primB, u8 envR, u8 envG, u8 envB, u8 primA );
void func_80028DA4( Gfx **gdl, s16 x, s16 y, u8 *str );
void func_80028EA8( Gfx **gdl );
void func_80029088( Gfx **gdl, s16 x, s16 y, u16 advance, u16 *glyphs );
void func_800292D4( u16 *glyphs, s32 value, s32 digits );

#endif
