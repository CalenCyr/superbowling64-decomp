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

/*
================
func_80114680_gscode

One frame of the name entry of player D_80120793_gscode: shows the grid of the character set and case, writes
the character under the cursor after the name, and moves the cursor. The C buttons change the case, B erases
a character (or backs out), A and Z take the character, or, on the row below the grid, the end.
Declared s32 but returns nothing (the caller ignores it): the original's return register is live at the
exit, which keeps the delay slots of the branches to it empty.
================
*/
s32 func_80114680_gscode( void ) {
    s32 player;

    player = ( ( D_8005C0E4[D_80120793_gscode] >> 4 ) & 7 ) - 1;

    if ( D_80120795_gscode == -1 && D_80120794_gscode >= 4 && D_80120794_gscode <= 6 ) {
        D_80120796_gscode = D_80120794_gscode - 4;
    }
    func_8003893C( D_8011BA08_gscode, 0x17, 0x937, 0x30, 0x30, 0xFF );
    switch ( D_80120796_gscode ) {
    case 0:
        if ( D_80120797_gscode == 0 ) {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x93B, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x93A, 0x40, 0x58, 0xFF );
        }
        break;
    case 1:
        if ( D_80120797_gscode == 0 ) {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x939, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x938, 0x40, 0x58, 0xFF );
        }
        break;
    case 2:
        if ( D_80120797_gscode == 0 ) {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x93D, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( D_8011BA08_gscode, 0x18, 0x93C, 0x40, 0x58, 0xFF );
        }
        break;
    }

    if ( D_80120795_gscode == -1 ) {
        D_8009D098.unk_08[player].name[D_80120798_gscode] = 0xFFFF;
        if ( D_80120794_gscode == 7 ) {
            func_8003893C( D_8011BA08_gscode, 0x19, 0xA8C, 0xC0, 0x40, D_8009CFB0 );
        } else {
            func_8003893C( D_8011BA08_gscode, 0x19, 0xA8B, D_80120794_gscode * 16 + 0x3E, 0x3E, D_8009CFB0 );
        }
    } else {
        if ( D_80120798_gscode < 5 ) {
            D_8009D098.unk_08[player].name[D_80120798_gscode] =
                D_8011BA70_gscode[D_80120795_gscode * 12 +
                                  ( ( D_80120796_gscode * 2 + D_80120797_gscode ) * 60 + D_80120794_gscode )];
            D_8009D098.unk_08[player].name[D_80120798_gscode + 1] = 0xFFFF;
        }
        func_8003893C( D_8011BA08_gscode, 0x19, 0xA8B, D_80120794_gscode * 16 + 0x3E, D_80120795_gscode * 16 + 0x56,
                       D_8009CFB0 );
    }

    if ( D_800E6758 & U_JPAD ) {
        func_80037870( 0, 0 );
        D_80120795_gscode--;
        if ( D_80120795_gscode == -1 ) {
            D_8011BD40_gscode = D_80120794_gscode;
            D_80120794_gscode = 7;
        }
        if ( D_80120795_gscode == -2 ) {
            D_80120795_gscode = 4;
            D_80120794_gscode = D_8011BD40_gscode;
        }
    }
    if ( D_800E6758 & D_JPAD ) {
        func_80037870( 0, 0 );
        D_80120795_gscode++;
        if ( D_80120795_gscode == 0 ) {
            D_80120794_gscode = D_8011BD40_gscode;
        }
        if ( D_80120795_gscode == 5 ) {
            D_80120795_gscode = 0;
        }
    }
    if ( ( D_800E6758 & L_JPAD ) && D_80120795_gscode != -1 ) {
        func_80037870( 0, 0 );
        D_80120794_gscode--;
        if ( D_80120794_gscode == -1 ) {
            D_80120794_gscode = 11;
        }
    }
    if ( D_800E6758 & R_JPAD ) {
        if ( D_80120795_gscode == -1 ) {
            if ( D_80120794_gscode < 7 ) {
                D_80120794_gscode++;
                func_80037870( 0, 0 );
            }
        } else {
            func_80037870( 0, 0 );
            D_80120794_gscode++;
            if ( D_80120794_gscode == 12 ) {
                D_80120794_gscode = 0;
            }
        }
    }

    if ( ( D_800E4D62 & R_CBUTTONS ) && D_80120797_gscode == 0 ) {
        func_80037870( 0, 0 );
        D_80120797_gscode = 1;
    }
    if ( ( D_800E4D62 & L_CBUTTONS ) && D_80120797_gscode == 1 ) {
        func_80037870( 0, 0 );
        D_80120797_gscode = 0;
    }
    if ( D_800E4D62 & B_BUTTON ) {
        func_80037870( 2, 0 );
        if ( D_80120798_gscode != 0 ) {
            D_80120798_gscode--;
        } else {
            D_8011BA6C_gscode = 2;
            D_8009D098.unk_08[player].name[0] = 0xFFFF;
        }
    }
    if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
        if ( D_80120795_gscode == -1 ) {
            if ( D_80120794_gscode == 7 ) {
                if ( D_80120798_gscode == 0 ) {
                    D_8009D098.unk_08[player].name[0] = 0x57;
                    D_8009D098.unk_08[player].name[1] = 0xFFFF;
                }
                func_80037870( 6, 0 );
                if ( D_80120793_gscode < D_8005C161 - 1 ) {
                    D_80120793_gscode++;
                    D_8011BA6C_gscode = 1;
                } else {
                    D_8011BA6C_gscode = 5;
                }
            }
        } else {
            func_80037870( 5, 0 );
            D_80120798_gscode++;
            if ( D_80120798_gscode >= 5 ) {
                D_80120798_gscode = 5;
                D_80120795_gscode = -1;
                D_80120794_gscode = 7;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/gscode/5AD50", func_80114D1C_gscode);
