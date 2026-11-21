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
// takes &D_8009D1A0[entry] in a register for the insert) and in the key expressions' schedule
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
void func_80031650( void ) {
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
s32 func_80031C9C( u32 value ) {
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

INCLUDE_ASM("asm/nonmatchings/C650", func_80031E40);

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
#ifdef NON_MATCHING
// NON_MATCHING: register allocation: dx and dz * dz trade $f26 and $f24; the rest is exact. inline: func_80035380
// expands it twice
inline void func_80035220( f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz ) {
    f32 dx, dy, dz;
    f32 dist, distXZ;
    f32 cosPitch, sinPitch, cosYaw, sinYaw;

    dx = tx - x;
    dy = ty - y;
    dz = tz - z;
    dist = sqrtf( dx * dx + dy * dy + dz * dz );
    distXZ = sqrtf( dx * dx + dz * dz );
    cosPitch = distXZ / dist;
    sinYaw = dx / distXZ;
    cosYaw = dz / distXZ;
    sinPitch = ( y - ty ) / dist;
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
#else
INCLUDE_ASM("asm/nonmatchings/C650", func_80035220);
#endif

/*
================
func_80035380

Starts a puff of flags at (x, y, z), in the first free slot, facing each camera.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: exact but for the second func_80035220 expansion, where dx and dz * dz trade $f26 and $f24 as in
// func_80035220 itself; the first expansion is exact
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
#else
INCLUDE_ASM("asm/nonmatchings/C650", func_80035380);
#endif

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
