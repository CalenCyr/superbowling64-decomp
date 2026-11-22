/* C650.c -- the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "ground.h"
#include "filecache.h"
#include "object.h"
#include "sprite.h"

/*
================
func_80031250

Empties the draw list: no entries, and every bucket empty.
================
*/
void func_80031250( void ) {
    D_800E8070 = 0;
    D_800DF6A8 = DRAW_NONE;
    memset( D_800E4FE0, 0xFF, sizeof( D_800E4FE0 ) );
}

#ifdef NON_MATCHING
// NON_MATCHING: the inline helper and its s16 *link store reproduce retail's frame (0x18) and its per-iteration
// reloads; still differs in the insertion's block order (retail places the append arm before the search loop and
// takes &D_8009D1A0[entry] in a register for the insert) and in the key expressions' schedule; retail also re-masks
// D_80066E14 (lhu, then andi 0xffff) for the test and the key but shifts the unmasked value for the bucket, so it is
// probably read into a wider local (a u16 local alone does not do it)
/*
================
func_8003128C_insert

Adds entry D_800E8070 of the draw list to bucket, after the entries of a lower key.
================
*/
static inline void func_8003128C_insert( u16 key, u16 bucket ) {
    s16 *link;
    u16 entry;
    u16 next;

    D_8009D1A0[D_800E8070].key = key;
    if ( D_800E4FE0[bucket] == DRAW_NONE ) {
        D_800E4FE0[bucket] = D_800E8070;
        D_800E4560[bucket] = D_800E8070;
        D_8009D1A0[D_800E8070].next = DRAW_NONE;
    } else {
        entry = D_800E4FE0[bucket];
        if ( D_8009D1A0[entry].key >= key ) {
            D_800E4FE0[bucket] = D_800E8070;
            D_8009D1A0[D_800E8070].next = entry;
        } else {
            for ( ;; ) {
                if ( D_8009D1A0[entry].next == DRAW_NONE ) {
                    D_8009D1A0[entry].next = D_800E8070;
                    D_8009D1A0[D_800E8070].next = DRAW_NONE;
                    D_800E4560[bucket] = D_800E8070;
                    break;
                }
                link = &D_8009D1A0[entry].next;
                next = *link;
                if ( D_8009D1A0[next].key >= key ) {
                    *link = D_800E8070;
                    D_8009D1A0[D_800E8070].next = next;
                    break;
                }
                entry = next;
            }
        }
    }
    D_800E8070++;
}

/*
================
func_8003128C

Adds an entry to the draw list for each camera that draws, keeping its bucket in key order.
================
*/
void func_8003128C( void ) {
    u8 mask;
    u32 i;
    u32 key;
    u32 bucket;

    mask = D_8009CF89;
    for ( i = 0; i < D_80099480; i++ ) {
        if ( ( mask & 1 ) && D_8009CFA0[i]->height != 0 ) {
            D_8009D1A0[D_800E8070].unk_04 = D_8009D064 | ( D_8009CF9C << 11 ) | ( i << 14 );
            D_8009D1A0[D_800E8070].unk_06 = D_800DF6BC | ( D_800F8090 << 11 ) | ( D_800E6580 << 13 ) | ( D_801000B0 << 14 );
            if ( D_80099448 == 0 ) {
                D_8009D1A0[D_800E8070].unk_06 |= 0x1000;
            }
            D_8009D1A0[D_800E8070].unk_08 = D_80099420;
            if ( D_80066E14 != 0 ) {
                key = ( D_80066E14 << 15 ) | ( D_8009D064 << 4 ) | ( D_801000B0 << 2 ) | i;
                bucket = ( D_80066E14 << 3 ) | ( D_8009D064 & 3 );
            } else if ( D_8009CF9C >= 3 ) {
                if ( D_800693AC != 0 ) {
                    key = ( D_8009D064 << 4 ) | ( ( D_801000B0 << 2 ) | 0x8000 ) | i;
                    bucket = ( D_8009D064 & 0x7F ) | 0x80;
                } else {
                    key = ( D_801000B0 << 2 ) | ( i | 0x20 );
                    bucket = key << 3;
                }
            } else {
                if ( D_800693AC != 0 ) {
                    key = ( D_8009D064 << 4 ) | ( ( D_801000B0 << 2 ) | 0x18000 ) | i;
                    bucket = ( D_8009D064 & 0x7F ) | 0x180;
                } else {
                    key = ( D_801000B0 << 2 ) | ( i | 0x40 );
                    bucket = key << 3;
                }
            }
            func_8003128C_insert( key, bucket );
        }
        mask >>= 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/C650", func_8003128C);
#endif

/*
================
func_80031650

Chains the buckets of the draw list, in key order, into one list from D_800DF6A8.
================
*/
inline void func_80031650( void ) {
    s16 *link;
    s32 i;

    link = &D_800DF6A8;
    for ( i = 0; i < DRAW_BUCKETS; i++ ) {
        if ( D_800E4FE0[i] != DRAW_NONE ) {
            *link = D_800E4FE0[i];
            link = &D_8009D1A0[D_800E4560[i]].next;
        }
    }
}

/*
================
func_800316C0

Starts drawing the view of cam: the depth and colour images at its corner, its fog (turning the fog setting on or
off when it changes), its matrices, look-at, viewport, scissor and lights.
================
*/
void func_800316C0( sceneCamera_t *cam ) {
    gDPPipeSync( D_800F8094++ );
    gDPSetDepthImage( D_800F8094++,
                      osVirtualToPhysical( D_80000400 + ( cam->unk_2C & 0xFFC ) + cam->unk_2E * SCREEN_WIDTH ) );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( D_80066C38[D_8005B0F8] + ( cam->unk_2C & 0xFFC ) +
                                           cam->unk_2E * SCREEN_WIDTH ) );
    if ( cam->path->unk_38 & FILE3_FOG ) {
        if ( D_800E6588 == 0 ) {
            D_800E6588 = 1;
            D_800DF682 = 1;
            D_80066B30 = 1;
            func_80031CD0();
        }
        gSPFogPosition( D_800F8094++, cam->path->fogNear, cam->path->fogFar );
        gDPSetFogColor( D_800F8094++, cam->path->fogColor >> 24, cam->path->fogColor >> 16, cam->path->fogColor >> 8,
                        cam->path->fogColor );
    } else {
        if ( D_800E6588 == 1 ) {
            D_800E6588 = 0;
            D_800DF682 = 1;
            D_80066B30 = 1;
            func_80031CD0();
        }
        gDPSetFogColor( D_800F8094++, 0, 0, 0, 0 );
        gSPFogPosition( D_800F8094++, 1000, 1200 );
    }
    gSPMatrix( D_800F8094++, &cam->projL[D_8005C0B1], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPMatrix( D_800F8094++, &cam->viewL[D_8005C0B1], G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH );
    gSPLookAt( D_800F8094++, &cam->lookAt[D_8005C0B1] );
    gSPViewport( D_800F8094++, &cam->vp[D_8005C0B1] );
    gDPSetScissor( D_800F8094++, G_SC_NON_INTERLACE, 0, 0, cam->width, cam->height );
    gSPSetLights2( D_800F8094++, cam->lights[D_8005C0B1] );
}

/*
================
func_80031BB0

Starts the frame's drawing: viewport and scissor of the screen, the framebuffer D_8005B0F8 and the depth buffer.
================
*/
void func_80031BB0( void ) {
    gDPPipeSync( D_800F8094++ );
    gSPViewport( D_800F8094++, &D_8010A290 );
    gDPSetScissor( D_800F8094++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( D_80066C38[D_8005B0F8] ) );
    gDPSetDepthImage( D_800F8094++, osVirtualToPhysical( D_80000400 ) );
}

/*
================
func_80031C9C

Returns the n for which 1 << n, from 2 to 512 in D_8005C1D0, is the smallest power at least value; 10 if none is.
================
*/
inline s32 func_80031C9C( u32 value ) {
    s32 i;

    for ( i = 0; i < 9; i++ ) {
        if ( D_8005C1D0[i] >= value ) {
            break;
        }
    }
    return i + 1;
}

/*
================
func_80031CD0

Picks the render modes of the frame's two cycles, D_801000B4 and D_800E4D64, from the fog setting and the
surface type D_800A30B4.
================
*/
void func_80031CD0( void ) {
    if ( D_800E4D6C == 0 ) {
        if ( D_800E4D60 != 0 ) {
            return;
        }
        switch ( D_800A30B4 ) {
        case 2:
            D_801000B4 = G_RM_XLU_SURF;
            D_800E4D64 = G_RM_XLU_SURF2;
            break;
        case 3:
            D_801000B4 = G_RM_AA_TEX_EDGE;
            D_800E4D64 = G_RM_AA_TEX_EDGE2;
            break;
        case 6:
            if ( D_800E6588 != 0 ) {
                D_801000B4 = 0xC8000200;
                D_800E4D64 = 0x32004200;
            } else {
                D_801000B4 = G_RM_OPA_SURF;
                D_800E4D64 = G_RM_OPA_SURF2;
            }
            break;
        }
    } else {
        if ( D_800E6588 != 0 ) {
            D_801000B4 = G_RM_FOG_SHADE_A;
        } else {
            D_801000B4 = G_RM_PASS;
        }
        if ( D_800E4D60 == 0 ) {
            return;
        }
        switch ( D_800A30B4 ) {
        case 0:
            D_800E4D64 = G_RM_ZB_XLU_DECAL2;
            break;
        case 2:
            D_800E4D64 = G_RM_ZB_XLU_SURF2;
            break;
        case 3:
            D_800E4D64 = G_RM_AA_ZB_TEX_EDGE2;
            break;
        case 4:
            D_800E4D64 = G_RM_ZB_XLU_DECAL2;
            break;
        case 6:
            D_800E4D64 = G_RM_ZB_OPA_SURF2;
            break;
        }
    }
}

#ifdef NON_MATCHING
// NON_MATCHING: complete body, same calls and skeleton (func_80031650 and func_80031C9C expand inline). Differs in:
// the second texture's loads, where retail's render tile is at tmem 0 (only the load tile takes D_8009CF68), unlike
// gDPLoadMultiBlock; the clear colour, which retail reads as one word at fileType3_t 0x44 (clearColor is bytes);
// cms/cmt trade $s4/$s5; the order of the initial colour-byte stores; register choice around the load macros
/*
================
func_80031E40

Draws the frame's scene: clears the depth buffer and the background of camera 0, then walks the draw list,
setting up for each entry its microcode, camera, cycle and render modes, material (colours, combiner, geometry
mode and textures) and model matrix only where they change, and drawing its mesh.
================
*/
void func_80031E40( void ) {
    s16 *link;
    fileRefA_t *mesh;
    fileRefB_t *mat;
    fileType3_t *path;
    u32 ucode;
    u32 view;
    u32 size;
    u32 cmt, cms;
    Gfx *gfx;

    func_80031650();

    /* clear the depth buffer */
    gDPSetCycleType( D_800F8094++, G_CYC_FILL );
    gDPSetRenderMode( D_800F8094++, G_RM_NOOP, G_RM_NOOP2 );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical( D_80000400 ) );
    gDPSetFillColor( D_800F8094++, ( GPACK_ZDZ( G_MAXFBZ, 0 ) << 16 ) | GPACK_ZDZ( G_MAXFBZ, 0 ) );
    gDPFillRectangle( D_800F8094++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPPipeSync( D_800F8094++ );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( D_80066C38[D_8005B0F8] ) );

    /* the background of camera 0: its path's image, or its clear colour */
    if ( !( D_8009CFA0[0]->path->unk_38 & 4 ) ) {
        if ( D_8009CFA0[0]->path->unk_38 & 2 ) {
            gDPSetCycleType( D_800F8094++, G_CYC_COPY );
            gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
            gDPSetTexturePersp( D_800F8094++, G_TP_NONE );
            /* (0 - 0) * K5 + TEXEL0: the texel, whatever the multiplier */
            gDPSetCombineLERP( D_800F8094++, 0, 0, K5, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, K5, TEXEL0, 0, 0, 0, TEXEL0 );
            gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
            func_8002CE24( IMAGE_CI8, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           ( (fileImage_t *)D_8009CFA0[0]->path->unk_50 )->data,
                           ( (fileImage_t *)D_8009CFA0[0]->path->unk_50 )->data + CI8_PALETTE_SIZE );
        } else {
            path = D_8009CFA0[0]->path;
            gDPSetCycleType( D_800F8094++, G_CYC_FILL );
            gDPSetFillColor( D_800F8094++,
                             ( GPACK_RGBA5551( path->clearColor[0], path->clearColor[1], path->clearColor[2], 1 )
                               << 16 ) |
                                 GPACK_RGBA5551( path->clearColor[0], path->clearColor[1], path->clearColor[2], 1 ) );
            gDPFillRectangle( D_800F8094++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
        }
        gDPPipeSync( D_800F8094++ );
    }

    /* the state the entries change from */
    D_801000B4 = G_RM_FOG_PRIM_A;
    D_800E4D64 = G_RM_ZB_OPA_SURF2;
    D_800E4D6C = 1;
    D_8006940C = 0;
    D_800A30B4 = 0;
    D_80066B31 = 1;
    D_800E4D60 = 1;
    D_800E6579 = 1;
    D_80066B32 = 1;
    D_800F7BE0 = 0;
    D_800E6588 = 0;
    D_80066C34 = 0;
    D_800DF5D4 = 0xFFFF;
    gDPSetCycleType( D_800F8094++, G_CYC_2CYCLE );
    gDPSetTextureFilter( D_800F8094++, G_TF_BILERP );
    gDPSetTexturePersp( D_800F8094++, G_TP_PERSP );
    gSPPerspNormalize( D_800F8094++, D_8009CF9E );
    gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
    gDPSetAlphaCompare( D_800F8094++, G_AC_THRESHOLD );
    D_8009CF70.a0 = 0xFF;
    D_80068ACC.r = 0x80;
    D_80068ACC.g = 0x80;
    D_80068ACC.b = 0x80;
    D_80068ACC.a = 0xFF;
    gDPSetPrimColor( D_800F8094++, 0, 0, D_80068ACC.r, D_80068ACC.g, D_80068ACC.b, D_80068ACC.a );
    D_800EA0C8.r = 0x80;
    D_800EA0C8.g = 0x80;
    D_800EA0C8.b = 0x80;
    D_800EA0C8.a = 0xFF;
    gDPSetEnvColor( D_800F8094++, D_800EA0C8.r, D_800EA0C8.g, D_800EA0C8.b, D_800EA0C8.a );
    D_800A3018 = 0;
    func_800316C0( D_8009CFA0[0] );
    D_800DF682 = 1;
    D_80066B30 = 1;

    for ( link = &D_800DF6A8; *link != DRAW_NONE; link = &D_8009D1A0[*link].next ) {
        mesh = D_800F80B0[D_8009D1A0[*link].unk_06 & 0x7FF].data;
        mat = D_800F80B0[D_8009D1A0[*link].unk_04 & 0x7FF].data;

        ucode = D_8009D1A0[*link].unk_06 >> 14;
        if ( D_80066C34 != ucode ) {
            D_800DF682 = 1;
            D_80066C34 = ucode & 3;
            D_800A3018 = 0xFF;
            switch ( D_80066C34 ) {
            case 0:
                gSPLoadUcode( D_800F8094++, D_56DD0, D_5F3F0 );
                gSPClipRatio( D_800F8094++, FRUSTRATIO_3 );
                break;
            case 1:
                gSPLoadUcode( D_800F8094++, D_57F60, D_5F800 );
                gSPClipRatio( D_800F8094++, FRUSTRATIO_1 );
                break;
            case 2:
                gSPLoadUcode( D_800F8094++, D_592F0, D_5FC20 );
                gSPClipRatio( D_800F8094++, FRUSTRATIO_1 );
                break;
            }
            if ( D_8006940C ) {
                gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
            }
        }

        view = D_8009D1A0[*link].unk_04 >> 14;
        if ( D_800A3018 != view ) {
            D_800A3018 = view;
            func_800316C0( D_8009CFA0[view] );
        }

        if ( D_800E4D6C != ( ( D_8009D1A0[*link].unk_06 >> 13 ) & 1 ) ) {
            D_800E4D6C = ( D_8009D1A0[*link].unk_06 >> 13 ) & 1;
            if ( D_800E4D6C == 0 ) {
                gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
            } else {
                gDPSetCycleType( D_800F8094++, G_CYC_2CYCLE );
            }
            D_80066B30 = 1;
            func_80031CD0();
            D_800EA0CC = 1;
        }
        if ( D_800E4D60 != ( ( D_8009D1A0[*link].unk_06 >> 12 ) & 1 ) ) {
            D_800E4D60 = ( D_8009D1A0[*link].unk_06 >> 12 ) & 1;
            D_800DF682 = 1;
            D_80066B30 = 1;
            D_800EA0CC = 1;
            func_80031CD0();
        }
        if ( D_80066B31 != ( ( D_8009D1A0[*link].unk_06 >> 11 ) & 1 ) ) {
            D_80066B31 = ( D_8009D1A0[*link].unk_06 >> 11 ) & 1;
            D_800DF682 = 1;
        }
        if ( D_800A30B4 != ( ( D_8009D1A0[*link].unk_04 >> 11 ) & 7 ) ) {
            D_800A30B4 = ( D_8009D1A0[*link].unk_04 >> 11 ) & 7;
            D_80066B30 = 1;
            func_80031CD0();
        }

        if ( D_800DF5D4 != ( D_8009D1A0[*link].unk_04 & 0x7FF ) ) {
            D_800DF5D4 = D_8009D1A0[*link].unk_04 & 0x7FF;
            if ( memcmp( &D_80068ACC, &mat->prim, sizeof( rgba8_t ) ) ) {
                D_80068ACC = mat->prim;
                gDPSetPrimColor( D_800F8094++, 0, 0, D_80068ACC.r, D_80068ACC.g, D_80068ACC.b, D_80068ACC.a );
            }
            if ( memcmp( &D_800EA0C8, &mat->env, sizeof( rgba8_t ) ) ) {
                D_800EA0C8 = mat->env;
                gDPSetEnvColor( D_800F8094++, D_800EA0C8.r, D_800EA0C8.g, D_800EA0C8.b, D_800EA0C8.a );
            }
            if ( memcmp( &D_8009CF70, &mat->combine, sizeof( combine_t ) ) ) {
                D_8009CF70 = mat->combine;
                D_800EA0CC = 1;
            }
            if ( ( ( mat->flags & MAT_SMOOTH ) >> 3 ) != D_800E6579 ) {
                D_800E6579 = ( mat->flags & MAT_SMOOTH ) >> 3;
                D_800DF682 = 1;
            }
            if ( ( ( mat->flags & MAT_LIGHTING ) >> 2 ) != D_80066B32 ) {
                D_80066B32 = ( mat->flags & MAT_LIGHTING ) >> 2;
                D_800DF682 = 1;
            }
            if ( ( ( mat->flags & MAT_TEXGEN ) >> 4 ) != D_800F7BE0 ) {
                D_800F7BE0 = ( mat->flags & MAT_TEXGEN ) >> 4;
                if ( D_800F7BE0 ) {
                    D_8006940C = 1;
                    gSPTexture( D_800F8094++, 0x07C0, 0x07C0, 0, G_TX_RENDERTILE, G_ON );
                } else if ( !D_8006940C ) {
                    gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
                } else {
                    gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
                }
                D_800DF682 = 1;
            }

            if ( mat->flags & MAT_TEXTURE ) {
                switch ( mat->wrap & 3 ) {
                case 0:
                    cms = G_TX_WRAP;
                    break;
                case 1:
                    cms = G_TX_CLAMP;
                    break;
                case 2:
                    cms = G_TX_MIRROR;
                    break;
                }
                switch ( ( mat->wrap >> 2 ) & 3 ) {
                case 0:
                    cmt = G_TX_WRAP;
                    break;
                case 1:
                    cmt = G_TX_CLAMP;
                    break;
                case 2:
                    cmt = G_TX_MIRROR;
                    break;
                }
                if ( D_800F7BE0 ) {
                    cmt = cms = G_TX_WRAP;
                }
                if ( D_8006940C != 1 ) {
                    D_8006940C = 1;
                    gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
                }
                switch ( mat->fmt ) {
                case MAT_I4:
                    gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
                    gDPLoadTextureBlock_4b( D_800F8094++, mat->unk_30, G_IM_FMT_I, mat->width, mat->height, 0, cms,
                                            cmt, func_80031C9C( mat->width ), func_80031C9C( mat->height ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI4:
                    gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
                    gDPLoadTLUT_pal16( D_800F8094++, 15, mat->unk_2C );
                    gDPLoadTextureBlock_4b( D_800F8094++, mat->unk_30, G_IM_FMT_CI, mat->width, mat->height, 15, cms,
                                            cmt, func_80031C9C( mat->width ), func_80031C9C( mat->height ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI8:
                    gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
                    gDPLoadTLUT_pal256( D_800F8094++, mat->unk_2C );
                    gDPLoadTextureBlock( D_800F8094++, mat->unk_30, G_IM_FMT_CI, G_IM_SIZ_8b, mat->width, mat->height,
                                         0, cms, cmt, func_80031C9C( mat->width ), func_80031C9C( mat->height ),
                                         G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_RGBA16:
                    gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
                    gDPLoadTextureBlock( D_800F8094++, mat->unk_30, G_IM_FMT_RGBA, G_IM_SIZ_16b, mat->width,
                                         mat->height, 0, cms, cmt, func_80031C9C( mat->width ),
                                         func_80031C9C( mat->height ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_I8:
                    gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
                    gDPLoadTextureBlock( D_800F8094++, mat->unk_30, G_IM_FMT_I, G_IM_SIZ_8b, mat->width, mat->height,
                                         0, cms, cmt, func_80031C9C( mat->width ), func_80031C9C( mat->height ),
                                         G_TX_NOLOD, G_TX_NOLOD );
                    break;
                }
            } else if ( mat->flags & MAT_TEXTURE2 ) {
                size = mat->width * mat->height;
                D_8009CF68 = size;
                switch ( mat->fmt ) {
                case MAT_I4:
                case MAT_CI4:
                    D_8009CF68 = size >> 4;
                    break;
                case MAT_CI8:
                    D_8009CF68 = size >> 3;
                    break;
                case MAT_RGBA16:
                    D_8009CF68 = size >> 2;
                    break;
                }
                switch ( mat->wrap2 & 3 ) {
                case 0:
                    cms = G_TX_WRAP;
                    break;
                case 1:
                    cms = G_TX_CLAMP;
                    break;
                case 2:
                    cms = G_TX_MIRROR;
                    break;
                }
                switch ( ( mat->wrap2 >> 2 ) & 3 ) {
                case 0:
                    cmt = G_TX_WRAP;
                    break;
                case 1:
                    cmt = G_TX_CLAMP;
                    break;
                case 2:
                    cmt = G_TX_MIRROR;
                    break;
                }
                if ( D_800F7BE0 ) {
                    cmt = cms = G_TX_WRAP;
                }
                switch ( mat->fmt2 ) {
                case MAT_I4:
                    /* loaded over the first texture, into tile 0 with palette 1: the tmem and tile of the others
                       went to the palette */
                    gDPLoadTextureBlock_4b( D_800F8094++, mat->unk_38, G_IM_FMT_I, mat->width2, mat->height2, 1, cms,
                                            cmt, func_80031C9C( mat->width2 ), func_80031C9C( mat->height2 ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI4:
                    gDPLoadTLUT_pal16( D_800F8094++, 14, mat->unk_34 );
                    gDPLoadMultiBlock_4b( D_800F8094++, mat->unk_38, D_8009CF68, 1, G_IM_FMT_CI, mat->width2,
                                          mat->height2, 14, cms, cmt, func_80031C9C( mat->width2 ),
                                          func_80031C9C( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI8:
                    gDPLoadMultiBlock( D_800F8094++, mat->unk_38, D_8009CF68, 1, G_IM_FMT_CI, G_IM_SIZ_8b, mat->width2,
                                       mat->height2, 0, cms, cmt, func_80031C9C( mat->width2 ),
                                       func_80031C9C( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_RGBA16:
                    gDPLoadMultiBlock( D_800F8094++, mat->unk_38, D_8009CF68, 1, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                       mat->width2, mat->height2, 0, cms, cmt, func_80031C9C( mat->width2 ),
                                       func_80031C9C( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_I8:
                    /* the first texture again, not the second */
                    gDPLoadMultiBlock( D_800F8094++, mat->unk_30, D_8009CF68, 1, G_IM_FMT_I, G_IM_SIZ_8b, mat->width,
                                       mat->height, 0, cms, cmt, func_80031C9C( mat->width ),
                                       func_80031C9C( mat->height ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                }
            } else if ( D_8006940C ) {
                D_8006940C = 0;
                gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
            }
        }

        if ( ( mat->flags & MAT_TEXTURE ) && ( mat->scroll[0] != 0 || mat->scroll[1] != 0 ) ) {
            gDPSetTileSize( D_800F8094++, G_TX_RENDERTILE, mat->scroll[0], mat->scroll[1], mat->scroll[0] + 64,
                            mat->scroll[1] + 32 );
        }

        if ( D_800DF682 ) {
            D_800DF682 = 0;
            D_800DF570 = G_SHADE;
            if ( D_80066B31 ) {
                D_800DF570 = G_SHADE | G_CULL_BACK;
            } else {
                D_800DF570 = G_SHADE | G_CULL_FRONT;
            }
            if ( D_800E4D60 ) {
                D_800DF570 |= G_ZBUFFER;
            }
            if ( D_800E6579 ) {
                D_800DF570 |= G_SHADING_SMOOTH;
            }
            if ( D_80066B32 ) {
                D_800DF570 |= G_LIGHTING;
            }
            if ( D_800F7BE0 ) {
                D_800DF570 |= G_TEXTURE_GEN;
            }
            if ( D_800E6588 ) {
                D_800DF570 |= G_FOG;
            }
            gSPClearGeometryMode( D_800F8094++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING |
                                                G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH );
            gSPSetGeometryMode( D_800F8094++, D_800DF570 );
        }

        if ( D_800EA0CC ) {
            D_800EA0CC = 0;
            gfx = D_800F8094++;
            if ( D_800E4D60 ) {
                gfx->words.w0 = _SHIFTL( G_SETCOMBINE, 24, 8 ) |
                                _SHIFTL( GCCc0w0( mat->combine.a0, mat->combine.c0, mat->combine.Aa0, mat->combine.Ac0 ) |
                                             GCCc1w0( mat->combine.a1, mat->combine.c1 ),
                                         0, 24 );
                gfx->words.w1 = GCCc0w1( mat->combine.b0, mat->combine.d0, mat->combine.Ab0, mat->combine.Ad0 ) |
                                GCCc1w1( mat->combine.b1, mat->combine.Aa1, mat->combine.Ac1, mat->combine.d1,
                                         mat->combine.Ab1, mat->combine.Ad1 );
            } else {
                gfx->words.w0 = _SHIFTL( G_SETCOMBINE, 24, 8 ) |
                                _SHIFTL( GCCc0w0( mat->combine.a0, mat->combine.c0, mat->combine.Aa0, mat->combine.Ac0 ) |
                                             GCCc1w0( mat->combine.a0, mat->combine.c0 ),
                                         0, 24 );
                gfx->words.w1 = GCCc0w1( mat->combine.b0, mat->combine.d0, mat->combine.Ab0, mat->combine.Ad0 ) |
                                GCCc1w1( mat->combine.b0, mat->combine.Aa0, mat->combine.Ac0, mat->combine.d0,
                                         mat->combine.Ab0, mat->combine.Ad0 );
            }
        }

        if ( D_80066B30 ) {
            D_80066B30 = 0;
            gDPSetRenderMode( D_800F8094++, D_801000B4, D_800E4D64 );
        }

        if ( D_800A30B0 != D_8009D1A0[*link].unk_08 ) {
            D_800A30B0 = D_8009D1A0[*link].unk_08;
            gSPMatrix( D_800F8094++, D_8009D1A0[*link].unk_08, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
        }

        gSPVertex( D_800F8094++, mesh->unk_04, mesh->numVerts, 0 );
        gSPDisplayList( D_800F8094++, mesh->unk_08 );
        gDPPipeSync( D_800F8094++ );
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/C650", func_80031E40);
#endif

/*
================
func_80035074

Frees every puff, loads the puff images, and makes the puff squares of each size from the first.
================
*/
void func_80035074( void ) {
    s32 i;

    for ( i = 0; i < NUM_PUFFS; i++ ) {
        D_800E4D70[i].life = 0;
    }
    for ( i = 0; i < 2; i++ ) {
        func_80030C04( D_8005C1E4[i], FILE_RAW );
    }
    for ( i = 1; i < NUM_PUFF_SIZES; i++ ) {
        D_8005C1F0[i * 4] = D_8005C1F0[0];
        D_8005C1F0[i * 4].v.ob[0] = -( i + 1 );
        D_8005C1F0[i * 4].v.ob[1] = i + 1;
        D_8005C1F0[i * 4 + 1] = D_8005C1F0[1];
        D_8005C1F0[i * 4 + 1].v.ob[0] = i + 1;
        D_8005C1F0[i * 4 + 1].v.ob[1] = i + 1;
        D_8005C1F0[i * 4 + 2] = D_8005C1F0[2];
        D_8005C1F0[i * 4 + 2].v.ob[0] = -( i + 1 );
        D_8005C1F0[i * 4 + 2].v.ob[1] = -( i + 1 );
        D_8005C1F0[i * 4 + 3] = D_8005C1F0[3];
        D_8005C1F0[i * 4 + 3].v.ob[0] = i + 1;
        D_8005C1F0[i * 4 + 3].v.ob[1] = -( i + 1 );
    }
}

/*
================
func_80035220

Sets m to stand at (x, y, z) and face (tx, ty, tz), turned about y then tilted.
================
*/
inline void func_80035220( f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz ) {
    f32 dx, dy, dz;
    f32 dist, distXZ;
    f32 cosPitch, sinPitch, cosYaw, sinYaw;

    dx = tx - x;
    dy = ty - y;
    dz = tz - z;
    dist = sqrtf( dx * dx + dy * dy + dz * dz );
    distXZ = sqrtf( dx * dx + dz * dz );
    sinPitch = ( y - ty ) / dist;
    cosPitch = distXZ / dist;
    sinYaw = dx / distXZ;
    cosYaw = dz / distXZ;
    m[0][0] = cosYaw;
    m[0][1] = 0.0f;
    m[0][2] = -sinYaw;
    m[0][3] = 0.0f;
    m[1][0] = sinYaw * sinPitch;
    m[1][1] = cosPitch;
    m[1][2] = cosYaw * sinPitch;
    m[1][3] = 0.0f;
    m[2][0] = cosPitch * sinYaw;
    m[2][1] = -sinPitch;
    m[2][2] = cosPitch * cosYaw;
    m[2][3] = 0.0f;
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    m[3][3] = 1.0f;
}

/*
================
func_80035380

Starts a puff of flags at (x, y, z), in the first free slot, facing each camera.
================
*/
void func_80035380( u16 flags, f32 x, f32 y, f32 z ) {
    f32 m[4][4];
    u32 i;

    for ( i = 0; i < NUM_PUFFS; i++ ) {
        if ( D_800E4D70[i].life == 0 ) {
            break;
        }
    }
    D_800E4D70[i].flags = flags;
    switch ( flags & PUFF_KIND ) {
    case 0:
        D_800E4D70[i].alpha = 0xFF;
        D_800E4D70[i].alphaStep = -8;
        D_800E4D70[i].size = 15;
        D_800E4D70[i].sizeStep = 2;
        D_800E4D70[i].life = 15;
        break;
    case 1:
        D_800E4D70[i].alpha = 0xFF;
        D_800E4D70[i].alphaStep = -15;
        D_800E4D70[i].size = 10;
        D_800E4D70[i].sizeStep = 2;
        D_800E4D70[i].life = 7;
        break;
    }
    func_80035220( m, x, y, z, D_8009CFA0[0]->eye[0], D_8009CFA0[0]->eye[1], D_8009CFA0[0]->eye[2] );
    guMtxF2L( m, &D_800E6770[0][i] );
    func_80035220( m, x, y, z, D_8009CFA0[1]->eye[0], D_8009CFA0[1]->eye[1], D_8009CFA0[1]->eye[2] );
    guMtxF2L( m, &D_800E6770[1][i] );
}

/*
================
func_80035748

Ages the puffs, then draws the live ones: by kind, its texture, and for each camera that draws a puff, its square
turned to face that camera in the kind's colour and the puff's alpha.
================
*/
void func_80035748( void ) {
    u32 count[2];
    u32 i;
    u32 kind;   /* RULE-EXCEPTION: R30, also the "a puff is live" flag: retail keeps both in one register ($s2) */
    u32 view;

    for ( i = 0; i < 2; i++ ) {
        count[i] = 0;
    }
    kind = FALSE;
    for ( i = 0; i < NUM_PUFFS; i++ ) {
        if ( D_800E4D70[i].life != 0 ) {
            count[D_800E4D70[i].flags & PUFF_KIND]++;
            D_800E4D70[i].life--;
            D_800E4D70[i].size += D_800E4D70[i].sizeStep;
            kind = TRUE;
            if ( D_800E4D70[i].size >= NUM_PUFF_SIZES ) {
                D_800E4D70[i].size = NUM_PUFF_SIZES - 1;
            }
            D_800E4D70[i].alpha += D_800E4D70[i].alphaStep;
            if ( D_800E4D70[i].alpha < 0 ) {
                D_800E4D70[i].alpha = 0;
            }
        }
    }
    if ( !kind ) {
        return;
    }

    if ( D_80066C34 != 1 ) {
        D_800A3018 = 2;
        gSPLoadUcode( D_800F8094++, D_57F60, D_5F800 );
        gSPClipRatio( D_800F8094++, FRUSTRATIO_1 );
    }
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2 );
    /* (0 - 0) * K5 + PRIMITIVE: the colour is the primitive's, whatever the multiplier */
    gDPSetCombineLERP( D_800F8094++, 0, 0, K5, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, K5, PRIMITIVE, TEXEL0, 0,
                       PRIMITIVE, 0 );
    gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
    for ( kind = 0; kind < 2; kind++ ) {
        if ( count[kind] == 0 ) {
            continue;
        }
        gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
        gDPLoadTextureBlock_4b( D_800F8094++,
                                ( (fileImage_t *)D_800F80B0[D_8005C1E4[kind]].data )->data + CI4_PALETTE_SIZE, G_IM_FMT_I,
                                64, 64, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, 6, 6, G_TX_NOLOD,
                                G_TX_NOLOD );
        if ( kind == 1 ) {
            gDPSetAlphaDither( D_800F8094++, G_AD_NOISE );
            gDPSetAlphaCompare( D_800F8094++, G_AC_DITHER );
        }
        for ( view = 0; view < 2; view++ ) {
            if ( D_800A3018 != view ) {
                D_800A3018 = view;
                func_800316C0( D_8009CFA0[D_800A3018] );
            }
            gSPTexture( D_800F8094++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
            gSPClearGeometryMode( D_800F8094++, G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                                G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH );
            gSPSetGeometryMode( D_800F8094++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH );
            for ( i = 0; i < NUM_PUFFS; i++ ) {
                if ( ( D_800E4D70[i].flags & PUFF_KIND ) == kind && D_800E4D70[i].life != 0 &&
                     ( D_800E4D70[i].flags & PUFF_CAMERA( view ) ) ) {
                    gSPMatrix( D_800F8094++, &D_800E6770[view][i], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
                    gSPVertex( D_800F8094++, &D_8005C1F0[D_800E4D70[i].size * 4], 4, 0 );
                    gDPSetPrimColor( D_800F8094++, 0, 0, D_8005C970[kind][0], D_8005C970[kind][1], D_8005C970[kind][2],
                                     D_800E4D70[i].alpha );
                    gSP2Triangles( D_800F8094++, 0, 2, 1, 0, 2, 3, 1, 0 );
                    gDPPipeSync( D_800F8094++ );
                }
            }
        }
    }
}
