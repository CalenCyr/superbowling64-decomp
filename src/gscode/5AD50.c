/* 5AD50.c -- game-select overlay: the title of the game select */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"
#include "filecache.h"
#include "dbgtext.h"

/*
================
func_80114160_gscode

Draws the image of file 0x960 tinted with the colour r, g, b, a: its pixels are three 4-bit intensity
textures behind the image's 16-colour palette, two 128x32 strips one above the other and a 32x64 strip on
their right, from (x - 8, y - 8).
================
*/
void func_80114160_gscode( s16 x, s16 y, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL; /* neither cached mode: the next image or text sets its mode up again */
    if ( D_800F80B0[0x960].refCount == 0 ) {
        func_80030C04( 0x960, FILE_RAW );
    }
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    gDPSetCombineLERP( D_800F8094++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                       PRIMITIVE, 0 );
    gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
    gDPSetPrimColor( D_800F8094++, 0, 0, r, g, b, a );

    gDPLoadTextureBlock_4b( D_800F8094++, ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE,
                            G_IM_FMT_I, 128, 32, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x - 8 ) << 2, ( y - 8 ) << 2, ( x + 120 ) << 2, ( y + 24 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPLoadTextureBlock_4b( D_800F8094++,
                            ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE + 128 * 32 / 2,
                            G_IM_FMT_I, 128, 32, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x - 8 ) << 2, ( y + 24 ) << 2, ( x + 120 ) << 2, ( y + 56 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPLoadTextureBlock_4b( D_800F8094++,
                            ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE + 128 * 32,
                            G_IM_FMT_I, 32, 64, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x + 120 ) << 2, ( y - 8 ) << 2, ( x + 152 ) << 2, ( y + 56 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPPipeSync( D_800F8094++ );
    gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
}

INCLUDE_ASM("asm/nonmatchings/gscode/5AD50", func_80114680_gscode);

INCLUDE_ASM("asm/nonmatchings/gscode/5AD50", func_80114D1C_gscode);
