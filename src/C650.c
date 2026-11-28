/* C650.c -- the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "sched.h"
#include "ground.h"
#include "filecache.h"
#include "object.h"
#include "sprite.h"

/* ===== Data ===== */

static u16 scene_powersOfTwo[9] = { 2, 4, 8, 16, 32, 64, 128, 256, 512 };
static u16 scene_puffImages[2] = { 0x093E, 0x093E };  /* image files of the two kinds of puff */
static s32 D_8005C1E8[2] = { 0 };           /* unreferenced */
static Vtx scene_puffSquares[NUM_PUFF_SIZES * 4] = {  /* squares of four vertices, by size; Scene_InitPuffs fills all but the first */
    { { { -1, 1, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { 1, 1, 0 }, 0, { 0x1000, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { -1, -1, 0 }, 0, { 0, 0x1000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { 1, -1, 0 }, 0, { 0x1000, 0x1000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
};
static u8 scene_puffColors[2][3] = {        /* rgb of the two kinds of puff */
    { 0x80, 0x80, 0x80 },
    { 0xFF, 0xFF, 0xFF },
};

/*
 * gDPLoadMultiBlock with the render tile at tmem 0: only the load tile takes tmem, so the render tile samples
 * whatever lies at the start of tmem.
 */
#define Scene_LoadMultiBlock( pkt, timg, tmem, rtile, fmt, siz, width, height, pal, cms, cmt, masks, maskt, shifts, \
                              shiftt )                                                                              \
    {                                                                                                               \
        gDPSetTextureImage( pkt, fmt, siz##_LOAD_BLOCK, 1, timg );                                                  \
        gDPSetTile( pkt, fmt, siz##_LOAD_BLOCK, 0, tmem, G_TX_LOADTILE, 0, cmt, maskt, shiftt, cms, masks, shifts ); \
        gDPLoadSync( pkt );                                                                                         \
        gDPLoadBlock( pkt, G_TX_LOADTILE, 0, 0, ( ( ( width ) * ( height ) + siz##_INCR ) >> siz##_SHIFT ) - 1,       \
                      CALC_DXT( width, siz##_BYTES ) );                                                             \
        gDPPipeSync( pkt );                                                                                         \
        gDPSetTile( pkt, fmt, siz, ( ( ( width ) * siz##_LINE_BYTES ) + 7 ) >> 3, 0, rtile, pal, cmt, maskt, shiftt, \
                    cms, masks, shifts );                                                                           \
        gDPSetTileSize( pkt, rtile, 0, 0, ( ( width ) - 1 ) << G_TEXTURE_IMAGE_FRAC,                                \
                        ( ( height ) - 1 ) << G_TEXTURE_IMAGE_FRAC );                                               \
    }

/*
================
Scene_ClearDrawList

Empties the draw list: no entries, and every bucket empty.
================
*/
void Scene_ClearDrawList( void ) {
    scene_numDraws = 0;
    scene_drawHead = DRAW_NONE;
    memset( scene_bucketHeads, 0xFF, sizeof( scene_bucketHeads ) );
}

#ifdef NON_MATCHING
// NON_MATCHING: block order now matches retail (empty arm, append arm, head test, head insert, search loop,
// insert); what differs is register allocation. Retail keeps the key and the helper's u16 key parameter in one register
// (a3), and scene_numDraws' HImode value and its sign-extended index in one (v1). Here integrate.c copies the
// argument (subreg, then a user-variable copy) and global-alloc gives the parameter its own register, and the HI load
// gets a second pseudo, so the loop needs one more register and every hoisted address moves up by one (t3-t9 for
// t2-t8). 100 differing lines with relocations compared by address (45 that are not register names: the two copies,
// the append arm's store schedule, and the shifted branch targets).
// Block order: the for (;;) whose first test is the key test with its insert and break is the loop exit test that
// expand_end_loop rotates to the loop end and duplicate_loop_exit_test copies before the loop (retail's head test and
// head insert, with a fresh pseudo for entry, so it is not cross-jumped); the append arm before the head test comes
// with this loop shape (checked by compiling). The reload of scene_numDraws after *link = ... (and not after scene_bucketHeads[...] = ...)
// shows the head insert stores through link. Hoisting scene_bucketHeads needs the empty test and arm written through
// the array. The layer local with a u16 bucket gives retail's andi on scene_addLayer (the bucket shift is narrowed to
// HImode and takes the unmasked load). The keys written as bits | (ucode << 2) | i keep retail's or order (the previous
// draft's (ucode << 2) | (i | 0x20) was reassociated).
// Ruled out: head test and walk on entry/next with the append inside the loop (the previous draft: the append stays in
// the loop), while (key < ...) with returns (one insert, not two), key read back from the entry in the helper (the
// loop reloads it), u16 key (the arms narrow to HImode and 0x18000 loses its bit), s16 key (signed compare),
// parameter order, a local n for scene_numDraws, chained bucket head/tail stores.
// Permuter (60 min, previous draft): 5445 -> 4965 only through generated inline wrappers.
// Equivalence: 251 random cases, no difference (49 skipped), tools/equiv.py.
/*
================
Scene_InsertEntry

Adds entry scene_numDraws of the draw list to bucket, after the entries of a lower key.
================
*/
static inline void Scene_InsertEntry( u16 key, u16 bucket ) {
    s16 *link;
    u16 entry;

    scene_drawList[scene_numDraws].key = key;
    if ( scene_bucketHeads[bucket] == DRAW_NONE ) {
        scene_bucketHeads[bucket] = scene_numDraws;
        scene_bucketTails[bucket] = scene_numDraws;
        scene_drawList[scene_numDraws].next = DRAW_NONE;
    } else {
        link = &scene_bucketHeads[bucket];
        for ( ;; ) {
            entry = *link;
            if ( scene_drawList[entry].key >= key ) {
                *link = scene_numDraws;
                scene_drawList[scene_numDraws].next = entry;
                break;
            }
            if ( scene_drawList[entry].next == DRAW_NONE ) {
                scene_drawList[entry].next = scene_numDraws;
                scene_drawList[scene_numDraws].next = DRAW_NONE;
                scene_bucketTails[bucket] = scene_numDraws;
                break;
            }
            link = &scene_drawList[entry].next;
        }
    }
    scene_numDraws++;
}

/*
================
Scene_AddToDrawList

Adds an entry to the draw list for each camera that draws, keeping its bucket in key order.
================
*/
// Idiom pass (unit: chained a = b = x, small inline helpers): a drawEntry_t pointer for the first three stores (further).
void Scene_AddToDrawList( void ) {
    u8 mask;
    u32 i;
    u32 key;
    u16 bucket;
    u16 layer;

    mask = scene_viewMask;
    for ( i = 0; i < cam_count; i++ ) {
        if ( ( mask & 1 ) && cam_cameras[i]->height != 0 ) {
            scene_drawList[scene_numDraws].materialBits = scene_addMaterial | ( scene_addSurface << 11 ) | ( i << 14 );
            scene_drawList[scene_numDraws].meshBits = scene_addMesh | ( scene_addCullBack << 11 ) | ( scene_addTwoCycle << 13 ) | ( scene_addUcode << 14 );
            if ( scene_addNoZBuffer == 0 ) {
                scene_drawList[scene_numDraws].meshBits |= 0x1000;
            }
            scene_drawList[scene_numDraws].matrix = scene_addMatrix;
            layer = scene_addLayer;
            if ( layer != 0 ) {
                key = ( layer << 15 ) | ( scene_addMaterial << 4 ) | ( scene_addUcode << 2 ) | i;
                bucket = ( layer << 3 ) | ( scene_addMaterial & 3 );
            } else if ( scene_addSurface >= 3 ) {
                if ( scene_addByMaterial != 0 ) {
                    key = 0x8000 | ( scene_addMaterial << 4 ) | ( scene_addUcode << 2 ) | i;
                    bucket = ( scene_addMaterial & 0x7F ) | 0x80;
                } else {
                    key = 0x20 | ( scene_addUcode << 2 ) | i;
                    bucket = key << 3;
                }
            } else {
                if ( scene_addByMaterial != 0 ) {
                    key = 0x18000 | ( scene_addMaterial << 4 ) | ( scene_addUcode << 2 ) | i;
                    bucket = ( scene_addMaterial & 0x7F ) | 0x180;
                } else {
                    key = 0x40 | ( scene_addUcode << 2 ) | i;
                    bucket = key << 3;
                }
            }
            Scene_InsertEntry( key, bucket );
        }
        mask >>= 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/C650", Scene_AddToDrawList);
#endif

/*
================
Scene_ChainDrawList

Chains the buckets of the draw list, in key order, into one list from scene_drawHead.
================
*/
inline void Scene_ChainDrawList( void ) {
    s16 *link;
    s32 i;

    link = &scene_drawHead;
    for ( i = 0; i < DRAW_BUCKETS; i++ ) {
        if ( scene_bucketHeads[i] != DRAW_NONE ) {
            *link = scene_bucketHeads[i];
            link = &scene_drawList[scene_bucketTails[i]].next;
        }
    }
}

/*
================
Scene_BeginView

Starts drawing the view of cam: the depth and colour images at its corner, its fog (turning the fog setting on or
off when it changes), its matrices, look-at, viewport, scissor and lights.
================
*/
void Scene_BeginView( sceneCamera_t *cam ) {
    gDPPipeSync( game_gfx++ );
    gDPSetDepthImage( game_gfx++,
                      osVirtualToPhysical( scene_depthBuffer + ( cam->left & 0xFFC ) + cam->top * SCREEN_WIDTH ) );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( sched_framebuffers[sched_drawFb] + ( cam->left & 0xFFC ) +
                                           cam->top * SCREEN_WIDTH ) );
    if ( cam->path->flags & FILE3_FOG ) {
        if ( scene_fog == 0 ) {
            scene_fog = 1;
            scene_geometryChanged = 1;
            scene_renderModeChanged = 1;
            Scene_PickRenderModes();
        }
        gSPFogPosition( game_gfx++, cam->path->fogNear, cam->path->fogFar );
        gDPSetFogColor( game_gfx++, cam->path->fogColor >> 24, cam->path->fogColor >> 16, cam->path->fogColor >> 8,
                        cam->path->fogColor );
    } else {
        if ( scene_fog == 1 ) {
            scene_fog = 0;
            scene_geometryChanged = 1;
            scene_renderModeChanged = 1;
            Scene_PickRenderModes();
        }
        gDPSetFogColor( game_gfx++, 0, 0, 0, 0 );
        gSPFogPosition( game_gfx++, 1000, 1200 );
    }
    gSPMatrix( game_gfx++, &cam->projL[sched_bufferIndex], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPMatrix( game_gfx++, &cam->viewL[sched_bufferIndex], G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH );
    gSPLookAt( game_gfx++, &cam->lookAt[sched_bufferIndex] );
    gSPViewport( game_gfx++, &cam->vp[sched_bufferIndex] );
    gDPSetScissor( game_gfx++, G_SC_NON_INTERLACE, 0, 0, cam->width, cam->height );
    gSPSetLights2( game_gfx++, cam->lights[sched_bufferIndex] );
}

/*
================
Scene_BeginFrame

Starts the frame's drawing: viewport and scissor of the screen, the framebuffer sched_drawFb and the depth buffer.
================
*/
void Scene_BeginFrame( void ) {
    gDPPipeSync( game_gfx++ );
    gSPViewport( game_gfx++, &scene_viewport );
    gDPSetScissor( game_gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( sched_framebuffers[sched_drawFb] ) );
    gDPSetDepthImage( game_gfx++, osVirtualToPhysical( scene_depthBuffer ) );
}

/*
================
Scene_TextureSizeLog2

Returns the n for which 1 << n, from 2 to 512 in scene_powersOfTwo, is the smallest power at least value; 10 if none is.
================
*/
inline s32 Scene_TextureSizeLog2( u32 value ) {
    s32 i;

    for ( i = 0; i < 9; i++ ) {
        if ( scene_powersOfTwo[i] >= value ) {
            break;
        }
    }
    return i + 1;
}

/*
================
Scene_PickRenderModes

Picks the render modes of the frame's two cycles, scene_renderMode1 and scene_renderMode2, from the fog setting and the
surface type scene_surface.
================
*/
void Scene_PickRenderModes( void ) {
    if ( scene_twoCycle == 0 ) {
        if ( scene_zBuffer != 0 ) {
            return;
        }
        switch ( scene_surface ) {
        case 2:
            scene_renderMode1 = G_RM_XLU_SURF;
            scene_renderMode2 = G_RM_XLU_SURF2;
            break;
        case 3:
            scene_renderMode1 = G_RM_AA_TEX_EDGE;
            scene_renderMode2 = G_RM_AA_TEX_EDGE2;
            break;
        case 6:
            if ( scene_fog != 0 ) {
                scene_renderMode1 = 0xC8000200;
                scene_renderMode2 = 0x32004200;
            } else {
                scene_renderMode1 = G_RM_OPA_SURF;
                scene_renderMode2 = G_RM_OPA_SURF2;
            }
            break;
        }
    } else {
        if ( scene_fog != 0 ) {
            scene_renderMode1 = G_RM_FOG_SHADE_A;
        } else {
            scene_renderMode1 = G_RM_PASS;
        }
        if ( scene_zBuffer == 0 ) {
            return;
        }
        switch ( scene_surface ) {
        case 0:
            scene_renderMode2 = G_RM_ZB_XLU_DECAL2;
            break;
        case 2:
            scene_renderMode2 = G_RM_ZB_XLU_SURF2;
            break;
        case 3:
            scene_renderMode2 = G_RM_AA_ZB_TEX_EDGE2;
            break;
        case 4:
            scene_renderMode2 = G_RM_ZB_XLU_DECAL2;
            break;
        case 6:
            scene_renderMode2 = G_RM_ZB_OPA_SURF2;
            break;
        }
    }
}

/*
================
Scene_Draw

Draws the frame's scene: clears the depth buffer and the background of camera 0, then walks the draw list,
setting up for each entry its microcode, camera, cycle and render modes, material (colours, combiner, geometry
mode and textures) and model matrix only where they change, and drawing its mesh.
================
*/
// RULE-EXCEPTION: s32 with no return value: retail keeps v0 live at the exit (the loop's last branch is likely)
s32 Scene_Draw( void ) {
    s16 *link;
    fileRefA_t *mesh;
    fileRefB_t *mat;
    u32 ucode;
    u32 view;
    u32 size;
    u32 cms, cmt;
    u32 clearColor;

    Scene_ChainDrawList();

    /* clear the depth buffer */
    gDPSetCycleType( game_gfx++, G_CYC_FILL );
    gDPSetRenderMode( game_gfx++, G_RM_NOOP, G_RM_NOOP2 );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical( scene_depthBuffer ) );
    gDPSetFillColor( game_gfx++, ( GPACK_ZDZ( G_MAXFBZ, 0 ) << 16 ) | GPACK_ZDZ( G_MAXFBZ, 0 ) );
    gDPFillRectangle( game_gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPPipeSync( game_gfx++ );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( sched_framebuffers[sched_drawFb] ) );

    /* the background of camera 0: its path's image, or its clear colour */
    if ( !( cam_cameras[0]->path->flags & 4 ) ) {
        if ( cam_cameras[0]->path->flags & 2 ) {
            gDPSetCycleType( game_gfx++, G_CYC_COPY );
            gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
            gDPSetTexturePersp( game_gfx++, G_TP_NONE );
            /* (0 - 0) * K5 + TEXEL0: the texel, whatever the multiplier */
            gDPSetCombineLERP( game_gfx++, 0, 0, K5, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, K5, TEXEL0, 0, 0, 0, TEXEL0 );
            gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
            Draw_CiImage( IMAGE_CI8, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           ( (fileImage_t *)cam_cameras[0]->path->unk_50 )->data,
                           ( (fileImage_t *)cam_cameras[0]->path->unk_50 )->data + CI8_PALETTE_SIZE );
        } else {
            gDPSetCycleType( game_gfx++, G_CYC_FILL );
            clearColor = cam_cameras[0]->path->clearColor;
            gDPSetFillColor( game_gfx++, ( GPACK_RGBA5551( clearColor >> 16, clearColor >> 8, clearColor, 1 ) << 16 ) |
                                             GPACK_RGBA5551( clearColor >> 16, clearColor >> 8, clearColor, 1 ) );
            gDPFillRectangle( game_gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
        }
        gDPPipeSync( game_gfx++ );
    }

    /* the state the entries change from */
    scene_twoCycle = 1;
    scene_renderMode1 = G_RM_FOG_PRIM_A;
    scene_texturing = 0;
    scene_surface = 0;
    scene_cullBack = 1;
    scene_zBuffer = 1;
    scene_smooth = 1;
    scene_lighting = 1;
    scene_texGen = 0;
    scene_fog = 0;
    scene_ucode = 0;
    scene_material = 0xFFFF;
    gDPSetCycleType( game_gfx++, G_CYC_2CYCLE );
    gDPSetTextureFilter( game_gfx++, G_TF_BILERP );
    gDPSetTexturePersp( game_gfx++, G_TP_PERSP );
    gSPPerspNormalize( game_gfx++, cam_perspNorm );
    gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
    gDPSetAlphaCompare( game_gfx++, G_AC_THRESHOLD );
    scene_renderMode2 = G_RM_ZB_OPA_SURF2;
    scene_combine.a0 = 0xFF;
    scene_primColor.r = scene_primColor.g = scene_primColor.b = 0x80;
    scene_primColor.a = 0xFF;
    gDPSetPrimColor( game_gfx++, 0, 0, scene_primColor.r, scene_primColor.g, scene_primColor.b, scene_primColor.a );
    scene_envColor.r = scene_envColor.g = scene_envColor.b = 0x80;
    scene_envColor.a = 0xFF;
    gDPSetEnvColor( game_gfx++, scene_envColor.r, scene_envColor.g, scene_envColor.b, scene_envColor.a );
    scene_view = 0;
    Scene_BeginView( cam_cameras[0] );
    scene_geometryChanged = 1;
    scene_renderModeChanged = 1;

    for ( link = &scene_drawHead; *link != DRAW_NONE; link = &scene_drawList[*link].next ) {
        mesh = cache_files[scene_drawList[*link].meshBits & 0x7FF].data;
        mat = cache_files[scene_drawList[*link].materialBits & 0x7FF].data;

        ucode = scene_drawList[*link].meshBits >> 14;
        if ( scene_ucode != ucode ) {
            scene_ucode = ucode & 3;
            scene_view = 0xFF;
            scene_geometryChanged = 1;
            switch ( scene_ucode ) {
            case 0:
                gSPLoadUcodeL( game_gfx++, gspF3DEX2_Rej_fifo );
                gSPClipRatio( game_gfx++, FRUSTRATIO_3 );
                break;
            case 1:
                gSPLoadUcodeL( game_gfx++, gspF3DEX2_fifo );
                gSPClipRatio( game_gfx++, FRUSTRATIO_1 );
                break;
            case 2:
                gSPLoadUcodeL( game_gfx++, gspL3DEX2_fifo );
                gSPClipRatio( game_gfx++, FRUSTRATIO_1 );
                break;
            }
            if ( scene_texturing ) {
                gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
            }
        }

        view = scene_drawList[*link].materialBits >> 14;
        if ( scene_view != view ) {
            scene_view = view;
            Scene_BeginView( cam_cameras[view] );
        }

        if ( scene_twoCycle != ( ( scene_drawList[*link].meshBits >> 13 ) & 1 ) ) {
            scene_twoCycle = ( scene_drawList[*link].meshBits >> 13 ) & 1;
            if ( scene_twoCycle == 0 ) {
                gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
            } else {
                gDPSetCycleType( game_gfx++, G_CYC_2CYCLE );
            }
            scene_renderModeChanged = 1;
            Scene_PickRenderModes();
            scene_combineChanged = 1;
        }
        if ( scene_zBuffer != ( ( scene_drawList[*link].meshBits >> 12 ) & 1 ) ) {
            scene_zBuffer = ( scene_drawList[*link].meshBits >> 12 ) & 1;
            scene_geometryChanged = 1;
            scene_renderModeChanged = 1;
            scene_combineChanged = 1;
            Scene_PickRenderModes();
        }
        if ( scene_cullBack != ( ( scene_drawList[*link].meshBits >> 11 ) & 1 ) ) {
            scene_cullBack = ( scene_drawList[*link].meshBits >> 11 ) & 1;
            scene_geometryChanged = 1;
        }
        if ( scene_surface != ( ( scene_drawList[*link].materialBits >> 11 ) & 7 ) ) {
            scene_surface = ( scene_drawList[*link].materialBits >> 11 ) & 7;
            scene_renderModeChanged = 1;
            Scene_PickRenderModes();
        }

        if ( scene_material != ( scene_drawList[*link].materialBits & 0x7FF ) ) {
            scene_material = scene_drawList[*link].materialBits & 0x7FF;
            if ( memcmp( &scene_primColor, &mat->prim, sizeof( rgba8_t ) ) ) {
                scene_primColor = mat->prim;
                gDPSetPrimColor( game_gfx++, 0, 0, scene_primColor.r, scene_primColor.g, scene_primColor.b, scene_primColor.a );
            }
            if ( memcmp( &scene_envColor, &mat->env, sizeof( rgba8_t ) ) ) {
                scene_envColor = mat->env;
                gDPSetEnvColor( game_gfx++, scene_envColor.r, scene_envColor.g, scene_envColor.b, scene_envColor.a );
            }
            if ( memcmp( &scene_combine, &mat->combine, sizeof( combine_t ) ) ) {
                scene_combine = mat->combine;
                scene_combineChanged = 1;
            }
            if ( ( ( mat->flags & MAT_SMOOTH ) >> 3 ) != scene_smooth ) {
                scene_smooth = ( mat->flags & MAT_SMOOTH ) >> 3;
                scene_geometryChanged = 1;
            }
            if ( ( ( mat->flags & MAT_LIGHTING ) >> 2 ) != scene_lighting ) {
                scene_lighting = ( mat->flags & MAT_LIGHTING ) >> 2;
                scene_geometryChanged = 1;
            }
            if ( ( ( mat->flags & MAT_TEXGEN ) >> 4 ) != scene_texGen ) {
                scene_texGen = ( mat->flags & MAT_TEXGEN ) >> 4;
                if ( scene_texGen ) {
                    scene_texturing = 1;
                    gSPTexture( game_gfx++, 0x07C0, 0x07C0, 0, G_TX_RENDERTILE, G_ON );
                } else if ( !scene_texturing ) {
                    gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
                } else {
                    gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
                }
                scene_geometryChanged = 1;
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
                if ( scene_texGen ) {
                    cms = cmt = G_TX_WRAP;
                }
                if ( scene_texturing != 1 ) {
                    scene_texturing = 1;
                    gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
                }
                switch ( mat->fmt ) {
                case MAT_I4:
                    gDPSetTextureLUT( game_gfx++, G_TT_NONE );
                    gDPLoadTextureBlock_4b( game_gfx++, mat->texels, G_IM_FMT_I, mat->width, mat->height, 0, cms,
                                            cmt, Scene_TextureSizeLog2( mat->width ), Scene_TextureSizeLog2( mat->height ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI4:
                    gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
                    gDPLoadTLUT_pal16( game_gfx++, 15, mat->palette );
                    gDPLoadTextureBlock_4b( game_gfx++, mat->texels, G_IM_FMT_CI, mat->width, mat->height, 15, cms,
                                            cmt, Scene_TextureSizeLog2( mat->width ), Scene_TextureSizeLog2( mat->height ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI8:
                    gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
                    gDPLoadTLUT_pal256( game_gfx++, mat->palette );
                    gDPLoadTextureBlock( game_gfx++, mat->texels, G_IM_FMT_CI, G_IM_SIZ_8b, mat->width, mat->height,
                                         0, cms, cmt, Scene_TextureSizeLog2( mat->width ), Scene_TextureSizeLog2( mat->height ),
                                         G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_RGBA16:
                    gDPSetTextureLUT( game_gfx++, G_TT_NONE );
                    gDPLoadTextureBlock( game_gfx++, mat->texels, G_IM_FMT_RGBA, G_IM_SIZ_16b, mat->width,
                                         mat->height, 0, cms, cmt, Scene_TextureSizeLog2( mat->width ),
                                         Scene_TextureSizeLog2( mat->height ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_I8:
                    gDPSetTextureLUT( game_gfx++, G_TT_NONE );
                    gDPLoadTextureBlock( game_gfx++, mat->texels, G_IM_FMT_I, G_IM_SIZ_8b, mat->width, mat->height,
                                         0, cms, cmt, Scene_TextureSizeLog2( mat->width ), Scene_TextureSizeLog2( mat->height ),
                                         G_TX_NOLOD, G_TX_NOLOD );
                    break;
                }
            } else if ( mat->flags & MAT_TEXTURE2 ) {
                size = mat->width * mat->height;
                scene_tmemNext = size;
                switch ( mat->fmt ) {
                case MAT_I4:
                case MAT_CI4:
                    scene_tmemNext = size >> 4;
                    break;
                case MAT_CI8:
                    scene_tmemNext = size >> 3;
                    break;
                case MAT_RGBA16:
                    scene_tmemNext = size >> 2;
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
                if ( scene_texGen ) {
                    cms = cmt = G_TX_WRAP;
                }
                switch ( mat->fmt2 ) {
                case MAT_I4:
                    /* loaded over the first texture, into tile 0 with palette 1: the tmem and tile of the others
                       went to the palette */
                    gDPLoadTextureBlock_4b( game_gfx++, mat->texels2, G_IM_FMT_I, mat->width2, mat->height2, 1, cms,
                                            cmt, Scene_TextureSizeLog2( mat->width2 ), Scene_TextureSizeLog2( mat->height2 ),
                                            G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI4:
                    gDPLoadTLUT_pal16( game_gfx++, 14, mat->palette2 );
                    gDPLoadMultiBlock_4b( game_gfx++, mat->texels2, scene_tmemNext, 1, G_IM_FMT_CI, mat->width2,
                                          mat->height2, 14, cms, cmt, Scene_TextureSizeLog2( mat->width2 ),
                                          Scene_TextureSizeLog2( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_CI8:
                    Scene_LoadMultiBlock( game_gfx++, mat->texels2, scene_tmemNext, 1, G_IM_FMT_CI, G_IM_SIZ_8b, mat->width2,
                                       mat->height2, 0, cms, cmt, Scene_TextureSizeLog2( mat->width2 ),
                                       Scene_TextureSizeLog2( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_RGBA16:
                    Scene_LoadMultiBlock( game_gfx++, mat->texels2, scene_tmemNext, 1, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                       mat->width2, mat->height2, 0, cms, cmt, Scene_TextureSizeLog2( mat->width2 ),
                                       Scene_TextureSizeLog2( mat->height2 ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                case MAT_I8:
                    /* the first texture again, not the second */
                    Scene_LoadMultiBlock( game_gfx++, mat->texels, scene_tmemNext, 1, G_IM_FMT_I, G_IM_SIZ_8b, mat->width,
                                       mat->height, 0, cms, cmt, Scene_TextureSizeLog2( mat->width ),
                                       Scene_TextureSizeLog2( mat->height ), G_TX_NOLOD, G_TX_NOLOD );
                    break;
                }
            } else if ( scene_texturing ) {
                scene_texturing = 0;
                gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF );
            }
        }

        if ( ( mat->flags & MAT_TEXTURE ) && ( mat->scroll[0] != 0 || mat->scroll[1] != 0 ) ) {
            gDPSetTileSize( game_gfx++, G_TX_RENDERTILE, mat->scroll[0], mat->scroll[1], mat->scroll[0] + 64,
                            mat->scroll[1] + 32 );
        }

        if ( scene_geometryChanged ) {
            scene_geometryChanged = 0;
            scene_geometryMode = G_SHADE;
            if ( scene_cullBack ) {
                scene_geometryMode = G_SHADE | G_CULL_BACK;
            } else {
                scene_geometryMode = G_SHADE | G_CULL_FRONT;
            }
            if ( scene_zBuffer ) {
                scene_geometryMode |= G_ZBUFFER;
            }
            if ( scene_smooth ) {
                scene_geometryMode |= G_SHADING_SMOOTH;
            }
            if ( scene_lighting ) {
                scene_geometryMode |= G_LIGHTING;
            }
            if ( scene_texGen ) {
                scene_geometryMode |= G_TEXTURE_GEN;
            }
            if ( scene_fog ) {
                scene_geometryMode |= G_FOG;
            }
            /* G_ZBUFFER is left out of the clear, so once set it stays set */
            gSPClearGeometryMode( game_gfx++, G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING |
                                                G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH );
            gSPSetGeometryMode( game_gfx++, scene_geometryMode );
        }

        if ( scene_combineChanged ) {
            scene_combineChanged = 0;
            if ( scene_zBuffer ) {
                gDPSetCombine( game_gfx++,
                               GCCc0w0( mat->combine.a0, mat->combine.c0, mat->combine.Aa0, mat->combine.Ac0 ) |
                                   GCCc1w0( mat->combine.a1, mat->combine.c1 ),
                               GCCc0w1( mat->combine.b0, mat->combine.d0, mat->combine.Ab0, mat->combine.Ad0 ) |
                                   GCCc1w1( mat->combine.b1, mat->combine.Aa1, mat->combine.Ac1, mat->combine.d1,
                                            mat->combine.Ab1, mat->combine.Ad1 ) );
            } else {
                /* without the z buffer, cycle 1 repeats cycle 0 */
                gDPSetCombine( game_gfx++,
                               GCCc0w0( mat->combine.a0, mat->combine.c0, mat->combine.Aa0, mat->combine.Ac0 ) |
                                   GCCc1w0( mat->combine.a0, mat->combine.c0 ),
                               GCCc0w1( mat->combine.b0, mat->combine.d0, mat->combine.Ab0, mat->combine.Ad0 ) |
                                   GCCc1w1( mat->combine.b0, mat->combine.Aa0, mat->combine.Ac0, mat->combine.d0,
                                            mat->combine.Ab0, mat->combine.Ad0 ) );
            }
        }

        if ( scene_renderModeChanged ) {
            scene_renderModeChanged = 0;
            gDPSetRenderMode( game_gfx++, scene_renderMode1, scene_renderMode2 );
        }

        if ( scene_modelMtx != scene_drawList[*link].matrix ) {
            scene_modelMtx = scene_drawList[*link].matrix;
            gSPMatrix( game_gfx++, scene_drawList[*link].matrix, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
        }

        gSPVertex( game_gfx++, mesh->verts, mesh->numVerts, 0 );
        gSPDisplayList( game_gfx++, mesh->cmds );
        gDPPipeSync( game_gfx++ );
    }
}

/*
================
Scene_InitPuffs

Frees every puff, loads the puff images, and makes the puff squares of each size from the first.
================
*/
void Scene_InitPuffs( void ) {
    s32 i;

    for ( i = 0; i < NUM_PUFFS; i++ ) {
        scene_puffs[i].life = 0;
    }
    for ( i = 0; i < 2; i++ ) {
        Cache_GetFile( scene_puffImages[i], FILE_RAW );
    }
    for ( i = 1; i < NUM_PUFF_SIZES; i++ ) {
        scene_puffSquares[i * 4] = scene_puffSquares[0];
        scene_puffSquares[i * 4].v.ob[0] = -( i + 1 );
        scene_puffSquares[i * 4].v.ob[1] = i + 1;
        scene_puffSquares[i * 4 + 1] = scene_puffSquares[1];
        scene_puffSquares[i * 4 + 1].v.ob[0] = i + 1;
        scene_puffSquares[i * 4 + 1].v.ob[1] = i + 1;
        scene_puffSquares[i * 4 + 2] = scene_puffSquares[2];
        scene_puffSquares[i * 4 + 2].v.ob[0] = -( i + 1 );
        scene_puffSquares[i * 4 + 2].v.ob[1] = -( i + 1 );
        scene_puffSquares[i * 4 + 3] = scene_puffSquares[3];
        scene_puffSquares[i * 4 + 3].v.ob[0] = i + 1;
        scene_puffSquares[i * 4 + 3].v.ob[1] = -( i + 1 );
    }
}

/*
================
Scene_FaceMatrix

Sets m to stand at (x, y, z) and face (tx, ty, tz), turned about y then tilted.
================
*/
inline void Scene_FaceMatrix( f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz ) {
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
Scene_StartPuff

Starts a puff of flags at (x, y, z), in the first free slot, facing each camera.
================
*/
void Scene_StartPuff( u16 flags, f32 x, f32 y, f32 z ) {
    f32 m[4][4];
    u32 i;

    for ( i = 0; i < NUM_PUFFS; i++ ) {
        if ( scene_puffs[i].life == 0 ) {
            break;
        }
    }
    scene_puffs[i].flags = flags;
    switch ( flags & PUFF_KIND ) {
    case 0:
        scene_puffs[i].alpha = 0xFF;
        scene_puffs[i].alphaStep = -8;
        scene_puffs[i].size = 15;
        scene_puffs[i].sizeStep = 2;
        scene_puffs[i].life = 15;
        break;
    case 1:
        scene_puffs[i].alpha = 0xFF;
        scene_puffs[i].alphaStep = -15;
        scene_puffs[i].size = 10;
        scene_puffs[i].sizeStep = 2;
        scene_puffs[i].life = 7;
        break;
    }
    Scene_FaceMatrix( m, x, y, z, cam_cameras[0]->eye[0], cam_cameras[0]->eye[1], cam_cameras[0]->eye[2] );
    guMtxF2L( m, &scene_puffMtx[0][i] );
    Scene_FaceMatrix( m, x, y, z, cam_cameras[1]->eye[0], cam_cameras[1]->eye[1], cam_cameras[1]->eye[2] );
    guMtxF2L( m, &scene_puffMtx[1][i] );
}

/*
================
Scene_DrawPuffs

Ages the puffs, then draws the live ones: by kind, its texture, and for each camera that draws a puff, its square
turned to face that camera in the kind's colour and the puff's alpha.
================
*/
void Scene_DrawPuffs( void ) {
    u32 count[2];
    u32 i;
    u32 kind;   /* RULE-EXCEPTION: R30, also the "a puff is live" flag: retail keeps both in one register ($s2) */
    u32 view;

    for ( i = 0; i < 2; i++ ) {
        count[i] = 0;
    }
    kind = FALSE;
    for ( i = 0; i < NUM_PUFFS; i++ ) {
        if ( scene_puffs[i].life != 0 ) {
            count[scene_puffs[i].flags & PUFF_KIND]++;
            scene_puffs[i].life--;
            scene_puffs[i].size += scene_puffs[i].sizeStep;
            kind = TRUE;
            if ( scene_puffs[i].size >= NUM_PUFF_SIZES ) {
                scene_puffs[i].size = NUM_PUFF_SIZES - 1;
            }
            scene_puffs[i].alpha += scene_puffs[i].alphaStep;
            if ( scene_puffs[i].alpha < 0 ) {
                scene_puffs[i].alpha = 0;
            }
        }
    }
    if ( !kind ) {
        return;
    }

    if ( scene_ucode != 1 ) {
        scene_view = 2;
        gSPLoadUcodeL( game_gfx++, gspF3DEX2_fifo );
        gSPClipRatio( game_gfx++, FRUSTRATIO_1 );
    }
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    gDPSetRenderMode( game_gfx++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2 );
    /* (0 - 0) * K5 + PRIMITIVE: the colour is the primitive's, whatever the multiplier */
    gDPSetCombineLERP( game_gfx++, 0, 0, K5, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, K5, PRIMITIVE, TEXEL0, 0,
                       PRIMITIVE, 0 );
    gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    for ( kind = 0; kind < 2; kind++ ) {
        if ( count[kind] == 0 ) {
            continue;
        }
        gDPSetTextureLUT( game_gfx++, G_TT_NONE );
        gDPLoadTextureBlock_4b( game_gfx++,
                                ( (fileImage_t *)cache_files[scene_puffImages[kind]].data )->data + CI4_PALETTE_SIZE, G_IM_FMT_I,
                                64, 64, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, 6, 6, G_TX_NOLOD,
                                G_TX_NOLOD );
        if ( kind == 1 ) {
            gDPSetAlphaDither( game_gfx++, G_AD_NOISE );
            gDPSetAlphaCompare( game_gfx++, G_AC_DITHER );
        }
        for ( view = 0; view < 2; view++ ) {
            if ( scene_view != view ) {
                scene_view = view;
                Scene_BeginView( cam_cameras[scene_view] );
            }
            gSPTexture( game_gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON );
            gSPClearGeometryMode( game_gfx++, G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                                G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH );
            gSPSetGeometryMode( game_gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH );
            for ( i = 0; i < NUM_PUFFS; i++ ) {
                if ( ( scene_puffs[i].flags & PUFF_KIND ) == kind && scene_puffs[i].life != 0 &&
                     ( scene_puffs[i].flags & PUFF_CAMERA( view ) ) ) {
                    gSPMatrix( game_gfx++, &scene_puffMtx[view][i], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
                    gSPVertex( game_gfx++, &scene_puffSquares[scene_puffs[i].size * 4], 4, 0 );
                    gDPSetPrimColor( game_gfx++, 0, 0, scene_puffColors[kind][0], scene_puffColors[kind][1], scene_puffColors[kind][2],
                                     scene_puffs[i].alpha );
                    gSP2Triangles( game_gfx++, 0, 2, 1, 0, 2, 3, 1, 0 );
                    gDPPipeSync( game_gfx++ );
                }
            }
        }
    }
}
