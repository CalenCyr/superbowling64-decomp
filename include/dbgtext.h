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

void Txt_SetupFont( Gfx **gdl, s32 palette );
void Txt_LoadFont( Gfx **gdl );
void Txt_SetTint( Gfx **gdl, u8 mode, u8 primR, u8 primG, u8 primB, u8 envR, u8 envG, u8 envB, u8 primA );
void Txt_DrawString( Gfx **gdl, s16 x, s16 y, u8 *str );
void Txt_LoadGlyphSheet( Gfx **gdl );
void Txt_DrawGlyphs( Gfx **gdl, s16 x, s16 y, u16 advance, u16 *glyphs );
void Txt_NumberToGlyphs( u16 *glyphs, s32 value, s32 digits );

#endif
