/* 3D50.c -- text drawing: the 8x8 font of characters 0x20-0x5F, and the 8x16 glyph sheet loaded from file */

#include "common.h"
#include "libultra.h"
#include "dbgtext.h"
#include "filecache.h"

/*
================
Txt_SetupFont

Render state for the 8x8 font, drawn with the given palette of txt_fontPalette.
================
*/
void Txt_SetupFont( Gfx **gdl, s32 palette ) {
    gDPSetTextureLUT( ( *gdl )++, G_TT_RGBA16 );
    gDPSetRenderMode( ( *gdl )++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    gDPSetAlphaCompare( ( *gdl )++, G_AC_THRESHOLD );
    gDPSetCombineMode( ( *gdl )++, G_CC_DECALRGBA, G_CC_DECALRGBA );
    gDPSetTile( ( *gdl )++, G_IM_FMT_CI, G_IM_SIZ_4b, 8, 0, G_TX_RENDERTILE, palette,
                G_TX_NOMIRROR | G_TX_CLAMP, 6, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, 7, G_TX_NOLOD );
}

/*
================
Txt_LoadFont

Loads the 8x8 font and its palettes.
================
*/
void Txt_LoadFont( Gfx **gdl ) {
    gDPLoadTextureBlock_4b( ( *gdl )++, txt_font, G_IM_FMT_CI, 128, 32, 1,
                            G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, 7, 6, G_TX_NOLOD, G_TX_NOLOD );
    gDPLoadTLUT_pal256( ( *gdl )++, txt_fontPalette );
    gDPSetTextureFilter( ( *gdl )++, G_TF_POINT );
    gDPSetBlendColor( ( *gdl )++, 0xFF, 0xFF, 0xFF, 0xA0 );
    gDPSetTexturePersp( ( *gdl )++, G_TP_NONE );
    gDPSetTextureLUT( ( *gdl )++, G_TT_NONE );
}

/*
================
Txt_SetTint

Sets the primitive and environment colours, and for mode 0 or 1 the render and combine modes that tint the
font with them; mode 1 blends when primA is not 0xFF.
================
*/
void Txt_SetTint( Gfx **gdl, u8 mode, u8 primR, u8 primG, u8 primB, u8 envR, u8 envG, u8 envB, u8 primA ) {
    gDPSetEnvColor( ( *gdl )++, envR, envG, envB, 0xFF );
    gDPSetPrimColor( ( *gdl )++, 0, 0, primR, primG, primB, primA );
    switch ( mode ) {
    case 0:
        gDPSetRenderMode( ( *gdl )++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
        gDPSetAlphaCompare( ( *gdl )++, G_AC_THRESHOLD );
        gDPSetCombineMode( ( *gdl )++, G_CC_BLENDPEDECALA, G_CC_BLENDPEDECALA );
        break;
    case 1:
        gDPSetAlphaCompare( ( *gdl )++, G_AC_NONE );
        if ( primA == 0xFF ) {
            gDPSetRenderMode( ( *gdl )++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
            gDPSetCombineMode( ( *gdl )++, G_CC_BLENDPEDECALA, G_CC_BLENDPEDECALA );
        } else {
            gDPSetRenderMode( ( *gdl )++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
            gDPSetCombineLERP( ( *gdl )++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT, PRIMITIVE,
                               TEXEL0, PRIMITIVE, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, ENVIRONMENT,
                               PRIMITIVE, TEXEL0, PRIMITIVE );
        }
        break;
    }
}

/*
================
Txt_DrawString

Draws str in the 8x8 font at x, y.
================
*/
void Txt_DrawString( Gfx **gdl, s16 x, s16 y, u8 *str ) {
    s32 i;

    for ( i = 0; *str != '\0'; i += 8, str++ ) {
        gSPTextureRectangle( ( *gdl )++, ( x + i ) << 2, y << 2, ( x + i + 8 ) << 2, ( y + 8 ) << 2,
                             G_TX_RENDERTILE, ( ( *str & 0xF ) * 8 ) << 5, ( ( *str - 0x20 ) / 16 * 8 ) << 5,
                             1 << 10, 1 << 10 );
    }
    gDPPipeSync( ( *gdl )++ );
}

/*
================
Txt_LoadGlyphSheet

Loads the glyph sheet (file 0x9CE) and its palette, and sets the render state for drawing it.
================
*/
void Txt_LoadGlyphSheet( Gfx **gdl ) {
    txt_glyphs = Cache_GetFile( 0x9CE, 0 );
    gDPLoadTLUT_pal16( ( *gdl )++, 0, txt_glyphs + 0x10 );
    txt_glyphs += 0x30;
    gDPSetTextureFilter( ( *gdl )++, G_TF_POINT );
    gDPSetTexturePersp( ( *gdl )++, G_TP_NONE );
    gDPSetTextureLUT( ( *gdl )++, G_TT_RGBA16 );
    gDPSetRenderMode( ( *gdl )++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    gDPSetAlphaCompare( ( *gdl )++, G_AC_THRESHOLD );
    gDPSetCombineMode( ( *gdl )++, G_CC_DECALRGBA, G_CC_DECALRGBA );
    gDPSetCycleType( ( *gdl )++, G_CYC_COPY );
    draw_rdpMode = 0xFF;
}

/*
================
Txt_DrawGlyphs

Draws the glyph string glyphs from the glyph sheet at x, y, advance pixels apart.
================
*/
void Txt_DrawGlyphs( Gfx **gdl, s16 x, s16 y, u16 advance, u16 *glyphs ) {
    s32 i;

    for ( i = 0; *glyphs != GLYPH_END; i += advance, glyphs++ ) {
        gDPLoadTextureTile_4b( ( *gdl )++, txt_glyphs, G_IM_FMT_CI, 64, 0, *glyphs % 8 * 8, *glyphs / 8 * 16,
                               *glyphs % 8 * 8 + 8, *glyphs / 8 * 16 + 16, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                               G_TX_NOMIRROR | G_TX_CLAMP, 3, 4, G_TX_NOLOD, G_TX_NOLOD );
        gSPTextureRectangle( ( *gdl )++, ( x + i ) << 2, y << 2, ( x + i + 7 ) << 2, ( y + 15 ) << 2,
                             G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10 );
    }
    gDPPipeSync( ( *gdl )++ );
}

/*
================
Txt_NumberToGlyphs

Writes the last digits decimal digits of value to glyphs as glyph indices, without leading zeros (a lone
zero for 0), and ends the string.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with one local doing two jobs (marked R30 below); separate locals
// change the register allocation, and a reused local is not counted as a match
void Txt_NumberToGlyphs( u16 *glyphs, s32 value, s32 digits ) {
    s32 i;
    s32 div;

    // RULE-EXCEPTION: R30, retail keeps the digit counter and the glyph index in one register: one i
    if ( value == 0 ) {
        glyphs[0] = GLYPH_DIGIT0;
        i = 1;
    } else {
        div = 1;
        for ( i = 0; i < digits; i++ ) {
            div *= 10;
        }
        i = 0;
        while ( digits > 0 ) {
            div /= 10;
            digits--;
            if ( value >= div ) {
                glyphs[i++] = value / div % 10 + GLYPH_DIGIT0;
            }
        }
    }
    glyphs[i] = GLYPH_END;
}
#else
INCLUDE_ASM("asm/nonmatchings/3D50", Txt_NumberToGlyphs);
#endif
