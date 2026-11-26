#ifndef DBGTEXT_H
#define DBGTEXT_H

/* dbgtext.h -- text and number drawing into a display list (unit 3D50) */

#include "common.h"
#include "libultra.h"

#define GLYPH_DIGIT0    0xF0        /* glyph index of '0' in the txt_glyphs font */
#define GLYPH_END       0xFFFF      /* ends a glyph string */

extern u16 txt_fontPalette[0x380];  /* palettes of the txt_font font, RGBA5551 */
extern u8 txt_font[0x800];             /* 128x32 CI4 font, characters 0x20-0x5F in rows of 16 */
extern u8 draw_rdpMode;             /* the RDP mode the last 2D drawing set up, DRAW2D_*; 0xFF: none */
extern u8 *txt_glyphs;              /* CI4 glyph sheet, 64 pixels wide, 8x16 glyphs */

void Txt_SetupFont( Gfx **gdl, s32 palette );
void Txt_LoadFont( Gfx **gdl );
void Txt_SetTint( Gfx **gdl, u8 mode, u8 primR, u8 primG, u8 primB, u8 envR, u8 envG, u8 envB, u8 primA );
void Txt_DrawString( Gfx **gdl, s16 x, s16 y, u8 *str );
void Txt_LoadGlyphSheet( Gfx **gdl );
void Txt_DrawGlyphs( Gfx **gdl, s16 x, s16 y, u16 advance, u16 *glyphs );
void Txt_NumberToGlyphs( u16 *glyphs, s32 value, s32 digits );

#endif
