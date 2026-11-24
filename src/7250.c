/* 7250.c -- 2D drawing into the display list: images of the file cache, and rectangles */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "sprite.h"
#include "object.h"
#include "ground.h"
#include "heap.h"
#include "input.h"
#include "libc.h"

/*
================
Cam_SetAmbient

Sets the ambient colour of camera index to color (red, green, blue).
================
*/
void Cam_SetAmbient( s32 index, u8 *color ) {
    u32 i;

    for ( i = 0; i < 3; i++ ) {
        D_8009CFA0[index]->lights[sched_bufferIndex].a.l.col[i] =
            D_8009CFA0[index]->lights[sched_bufferIndex].a.l.colc[i] = color[i];
    }
}

/*
================
Cam_ReadPathFrame

Reads frame of camera path file into eye and upPoint, each component from the frame or the path's default.
================
*/
void Cam_ReadPathFrame( s16 file, s16 frame, f32 *eye, f32 *upPoint ) {
    fileType3_t *path;
    f32 *data;
    s32 i;

    path = Cache_GetFile( file, FILE_TYPE3 );
    data = path->frames + path->frameSize * frame;
    for ( i = 0; i < 3; i++ ) {
        if ( path->eyeMask & ( 4 >> i ) ) {
            *eye++ = *data++;
        } else {
            *eye++ = path->eye[i];
        }
    }
    for ( i = 0; i < 3; i++ ) {
        if ( path->upPointMask & ( 4 >> i ) ) {
            upPoint[i] = *data++;
        } else {
            upPoint[i] = path->upPoint[i];
        }
    }
}

/*
================
Cam_Create

Creates a camera from params: its path (unk_08[0]) with the starting eye, at and up point, its viewport size and
target (unk_08[1] to unk_08[6]), the path's light and a grey one from the eye, and lists it in D_8009CFA0 and,
by priority, in D_800EA120.
================
*/
objNode_t *Cam_Create( objParams_t *params ) {
    sceneCameraNode_t *node;
    sceneCamera_t *cam;
    s32 i;

    node = Mem_Alloc( sizeof( sceneCameraNode_t ) );
    cam = &node->cam;
    cam->path = Cache_GetFile( params->unk_08[0], FILE_TYPE3 );
    for ( i = 0; i < 3; i++ ) {
        cam->at[i] = cam->path->at[i];
        cam->eye[i] = cam->path->eye[i];
        cam->upPoint[i] = cam->path->upPoint[i];
    }
    D_8009CFA0[D_80099480++] = cam;
    cam->width = params->unk_08[1];
    cam->height = params->unk_08[2];
    cam->left = params->unk_08[3];
    cam->top = params->unk_08[4];
    cam->targetObj = params->unk_08[5];
    cam->targetJoint = params->unk_08[6];
    for ( i = 0; i < 2; i++ ) {
        /* the path's Lights1 is the ambient and first light of the Lights2: one 0x18-byte copy */
        *(Lights1 *)&cam->lights[i] = *cam->path->light;
        cam->lights[i].l[1].l.col[0] = cam->lights[i].l[1].l.colc[0] = 0x40;
        cam->lights[i].l[1].l.col[1] = cam->lights[i].l[1].l.colc[1] = 0x40;
        cam->lights[i].l[1].l.col[2] = cam->lights[i].l[1].l.colc[2] = 0x40;
    }
    for ( i = 0; i < 2; i++ ) {
        cam->vp[i].vp.vscale[2] = cam->vp[i].vp.vtrans[2] = G_MAXZ / 2;
        cam->vp[i].vp.vscale[3] = cam->vp[i].vp.vtrans[3] = 0;
    }
    D_800EA120[(s32)params->priority] = cam;
    return &node->node;
}

/*
================
Cam_Update

Updates cam: at from its target joint, eye and up point from the path's current frame, the viewport, the
projection, and the view (with the eye's light) or, while D_800EA0C0 is set, the controller's zoom, pan and
rotation of the last view.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with one local doing two jobs (marked R30 below); separate locals
// change the register allocation, and a reused local is not counted as a match
void Cam_Update( sceneCamera_t *cam ) {
    f32 *data;
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    // RULE-EXCEPTION: R30, one local for both normalisations: retail keeps the two in one register
    f32 norm;
    f32 m[4][4];

    if ( cam->frame >= 0 ) {
        data = cam->path->frames + cam->path->frameSize * cam->frame;
        if ( cam->targetObj != 0 ) {
            guMtxXFMF( ( (sceneObject_t *)D_800EA120[cam->targetObj] )->joints[cam->targetJoint].mtx, 0.0f, 0.0f,
                       0.0f, &cam->at[0], &cam->at[1], &cam->at[2] );
        }
        for ( i = 0; i < 3; i++ ) {
            if ( cam->path->eyeMask & ( 4 >> i ) ) {
                cam->eye[i] = *data++;
            }
        }
        for ( i = 0; i < 3; i++ ) {
            if ( cam->path->upPointMask & ( 4 >> i ) ) {
                cam->upPoint[i] = *data++;
            }
        }
    }
    cam->vp[sched_bufferIndex].vp.vscale[0] = cam->vp[sched_bufferIndex].vp.vtrans[0] = cam->width * 2;
    cam->vp[sched_bufferIndex].vp.vscale[1] = cam->vp[sched_bufferIndex].vp.vtrans[1] = cam->height * 2;
    guPerspective( &cam->projL[sched_bufferIndex], &D_8009CF9E, cam->path->fovy, (f32)cam->width / (f32)cam->height,
                   20.0f, 16000.0f, 1.0f );
    /* the up vector is divided by the sum of its components, not by its length */
    norm = ( cam->upPoint[0] - cam->eye[0] ) + ( cam->upPoint[1] - cam->eye[1] ) + ( cam->upPoint[2] - cam->eye[2] );
    cam->up[0] = ( cam->upPoint[0] - cam->eye[0] ) / norm;
    cam->up[1] = ( cam->upPoint[1] - cam->eye[1] ) / norm;
    cam->up[2] = ( cam->upPoint[2] - cam->eye[2] ) / norm;
    if ( D_800EA0C0 == 0 ) {
        guLookAtReflectF( cam->mtx, &cam->lookAt[sched_bufferIndex], cam->eye[0], cam->eye[1], cam->eye[2], cam->at[0],
                          cam->at[1], cam->at[2], cam->up[0], cam->up[1], cam->up[2] );
        dx = cam->eye[0] - cam->at[0];
        dy = cam->eye[1] - cam->at[1];
        dz = cam->eye[2] - cam->at[2];
        norm = 127.0f / sqrtf( dx * dx + dy * dy + dz * dz );
        cam->lights[sched_bufferIndex].l[1].l.dir[0] = dx * norm;
        cam->lights[sched_bufferIndex].l[1].l.dir[1] = dy * norm;
        cam->lights[sched_bufferIndex].l[1].l.dir[2] = dz * norm;
    } else {
        if ( cont_allHeld & 0x8000 ) {
            guScaleF( m, 1.01f, 1.01f, 1.01f );
            guMtxCatF( m, cam->mtx, cam->mtx );
        }
        if ( cont_allHeld & 0x4000 ) {
            guScaleF( m, 0.99f, 0.99f, 0.99f );
            guMtxCatF( m, cam->mtx, cam->mtx );
        }
        if ( cont_allHeld & 0x2000 ) {
            if ( cont_allStickX * cont_allStickX + cont_allStickY * cont_allStickY > 20 ) {
                guTranslateF( m, cont_allStickX / 4, cont_allStickY / 4, 0.0f );
                guMtxCatF( cam->mtx, m, cam->mtx );
            }
        } else {
            if ( cont_allStickX * cont_allStickX + cont_allStickY * cont_allStickY > 20 ) {
                guRotateRPYF( m, -cont_allStickY / 10.0f, cont_allStickX / 10.0f, 0.0f );
                guMtxCatF( m, cam->mtx, cam->mtx );
            }
        }
    }
    guMtxF2L( cam->mtx, &cam->viewL[sched_bufferIndex] );
    for ( i = 0; i < 3; i++ ) {
        cam->dir[i] = cam->at[i] - cam->eye[i];
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/7250", Cam_Update);
#endif

/*
================
Cam_UpdateAndAdvance

Updates cam, and advances it along its path, from the start after the last frame.
================
*/
void Cam_UpdateAndAdvance( sceneCamera_t *cam ) {
    Cam_Update( cam );
    cam->frame += D_800F7BF8;
    if ( cam->frame == cam->path->numFrames ) {
        cam->frame = 0;
    }
}

/*
================
Draw_RoundedBox

Draws a translucent box of w by h pixels at (x, y) in colour (r, g, b, a), with rounded corners 8 pixels
outside it: one 16 by 16 intensity image (file 0xB33) drawn in four quadrants, each mirrored away from the box.
================
*/
void Draw_RoundedBox( s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL;
    if ( D_800F80B0[0xB33].refCount == 0 ) {
        Cache_GetFile( 0xB33, FILE_RAW );
    }
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    gDPSetRenderMode( game_gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    gDPSetCombineLERP( game_gfx++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                       PRIMITIVE, 0 );
    gDPSetTextureLUT( game_gfx++, G_TT_NONE );
    gDPSetPrimColor( game_gfx++, 0, 0, r, g, b, a );
    gDPLoadTextureBlock_4b( game_gfx++, ( (fileImage_t *)D_800F80B0[0xB33].data )->data + CI4_PALETTE_SIZE,
                            G_IM_FMT_I, 16, 16, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( game_gfx++, ( x - 8 ) << 2, ( y - 8 ) << 2, ( x + w ) << 2, ( y + h ) << 2,
                         G_TX_RENDERTILE, 8 << 5, 8 << 5, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
    gDPSetTile( game_gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD,
                G_TX_MIRROR, 4, G_TX_NOLOD );
    gSPTextureRectangle( game_gfx++, ( x + w ) << 2, ( y - 8 ) << 2, ( x + w + 8 ) << 2, ( y + h ) << 2,
                         G_TX_RENDERTILE, 16 << 5, 8 << 5, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
    gDPSetTile( game_gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0, G_TX_RENDERTILE, 0, G_TX_MIRROR, 4, G_TX_NOLOD,
                G_TX_CLAMP, 4, G_TX_NOLOD );
    gSPTextureRectangle( game_gfx++, ( x - 8 ) << 2, ( y + h ) << 2, ( x + w ) << 2, ( y + h + 8 ) << 2,
                         G_TX_RENDERTILE, 8 << 5, 16 << 5, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
    gDPSetTile( game_gfx++, G_IM_FMT_I, G_IM_SIZ_4b, 1, 0, G_TX_RENDERTILE, 0, G_TX_MIRROR, 4, G_TX_NOLOD,
                G_TX_MIRROR, 4, G_TX_NOLOD );
    gSPTextureRectangle( game_gfx++, ( x + w ) << 2, ( y + h ) << 2, ( x + w + 8 ) << 2, ( y + h + 8 ) << 2,
                         G_TX_RENDERTILE, 16 << 5, 16 << 5, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
    gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
}

/*
================
Draw_CiImage

Draws a colour-indexed image of width by height pixels at (x, y), in copy mode: loads its palette, then draws
it in strips of whole rows that fit texture memory, first in the widest power-of-two columns that fit the
padded width, then in the power-of-two columns that cover the rest. The pixels are stored strip after strip.
Any type other than IMAGE_CI4 draws as IMAGE_CI8 (without loading a palette, and with the strip sizes unset).
================
*/
void Draw_CiImage( s16 type, s16 x, s16 y, s16 width, s16 height, void *palette, void *pixels ) {
    s32 texels;         /* pixels in texture memory */
    s32 pad;
    s32 i;
    s32 stripWidth;
    s32 restWidth;
    s32 remaining;
    s32 rows;
    s32 strip;
    s32 offset;

    offset = 0;
    switch ( type ) {
    case IMAGE_CI4:
        texels = 0x1000;
        pad = width % 16;
        gDPLoadTLUT_pal16( game_gfx++, 0, palette );
        break;
    case IMAGE_CI8:
        texels = 0x800;
        pad = width % 8;
        gDPLoadTLUT_pal256( game_gfx++, palette );
        break;
    }
    for ( i = 1; i < 10; i++ ) {
        if ( ( 1 << i ) > width + pad ) {
            break;
        }
    }
    stripWidth = 1 << ( i - 1 );
    if ( width + pad == stripWidth ) {
        restWidth = 0;
    } else {
        for ( i = 1; i < 10; i++ ) {
            if ( ( 1 << i ) >= width + pad - stripWidth ) {
                break;
            }
        }
        restWidth = 1 << i;
    }
    if ( type == IMAGE_CI4 ) {
        for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
            rows = texels / stripWidth;
            if ( remaining < rows ) {
                rows = remaining;
                remaining = 0;
            } else {
                remaining -= rows;
            }
            gDPLoadTextureBlock_4b( game_gfx++, (u16 *)pixels + offset, G_IM_FMT_CI, stripWidth, rows, 0,
                                    G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
            offset += rows * stripWidth / 4;
            gSPScisTextureRectangle( game_gfx++, x << 2, ( y + strip * texels / stripWidth ) << 2,
                                     ( x + stripWidth - 1 ) << 2, ( y + strip * texels / stripWidth + rows - 1 ) << 2,
                                     G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10 );
        }
        if ( restWidth != 0 ) {
            for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
                rows = texels / restWidth;
                if ( remaining < rows ) {
                    rows = remaining;
                    remaining = 0;
                } else {
                    remaining -= rows;
                }
                gDPLoadTextureBlock_4b( game_gfx++, (u16 *)pixels + offset, G_IM_FMT_CI, restWidth, rows, 0,
                                        G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
                offset += rows * restWidth / 4;
                gSPScisTextureRectangle( game_gfx++, ( x + stripWidth ) << 2, ( y + strip * texels / restWidth ) << 2,
                                         ( x + stripWidth + restWidth - 1 ) << 2,
                                         ( y + strip * texels / restWidth + rows - 1 ) << 2, G_TX_RENDERTILE, 0, 0, 4 << 10,
                                         1 << 10 );
            }
        }
    } else {
        for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
            rows = texels / stripWidth;
            if ( remaining < rows ) {
                rows = remaining;
                remaining = 0;
            } else {
                remaining -= rows;
            }
            gDPLoadTextureBlock( game_gfx++, (u16 *)pixels + offset, G_IM_FMT_CI, G_IM_SIZ_8b, stripWidth, rows,
                                 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
            offset += rows * stripWidth / 2;
            gSPScisTextureRectangle( game_gfx++, x << 2, ( y + strip * texels / stripWidth ) << 2,
                                     ( x + stripWidth - 1 ) << 2, ( y + strip * texels / stripWidth + rows - 1 ) << 2,
                                     G_TX_RENDERTILE, 0, 0, 4 << 10, 1 << 10 );
        }
        if ( restWidth != 0 ) {
            for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
                rows = texels / restWidth;
                if ( remaining < rows ) {
                    rows = remaining;
                    remaining = 0;
                } else {
                    remaining -= rows;
                }
                gDPLoadTextureBlock( game_gfx++, (u16 *)pixels + offset, G_IM_FMT_CI, G_IM_SIZ_8b, restWidth,
                                     rows, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                     G_TX_NOLOD );
                offset += rows * restWidth / 2;
                gSPScisTextureRectangle( game_gfx++, ( x + stripWidth ) << 2, ( y + strip * texels / restWidth ) << 2,
                                         ( x + stripWidth + restWidth - 1 ) << 2,
                                         ( y + strip * texels / restWidth + rows - 1 ) << 2, G_TX_RENDERTILE, 0, 0, 4 << 10,
                                         1 << 10 );
            }
        }
    }
    gDPPipeSync( game_gfx++ );
}

/*
================
Draw_Image

Draws image id unscaled at (x, y), in copy mode.
================
*/
void Draw_Image( u16 id, s16 x, s16 y ) {
    fileImage_t *image;
    u32 type;
    u32 width;
    u32 height;
    u8 *data;

    if ( D_8005C190 != DRAW2D_COPY ) {
        D_8005C190 = DRAW2D_COPY;
        gDPSetCycleType( game_gfx++, G_CYC_COPY );
        gDPSetRenderMode( game_gfx++, G_RM_NOOP, G_RM_NOOP2 );
        gDPSetAlphaCompare( game_gfx++, G_AC_THRESHOLD );
    }
    if ( D_800F80B0[id].refCount == 0 ) {
        Cache_GetFile( id, FILE_RAW );
    }
    image = D_800F80B0[id].data;
    type = image->type;
    width = image->width;
    height = image->height;
    data = image->data;
    if ( type == IMAGE_CI4 ) {
        Draw_CiImage( IMAGE_CI4, x, y, width, height, data, data + CI4_PALETTE_SIZE );
    } else if ( type == IMAGE_CI8 ) {
        Draw_CiImage( IMAGE_CI8, x, y, width, height, data, data + CI8_PALETTE_SIZE );
    } else {
        Draw_CiImage( type, x, y, width, height, data, data );
    }
}

/*
================
Draw_ImageCopyPalette

Draws image id unscaled at (x, y), in copy mode, from a copy of its palette: the RDP loads the palette after
the file cache may have moved the image, so a colour-indexed image's palette goes to the next of the frame's
four palette slots.
================
*/
void Draw_ImageCopyPalette( u16 id, s16 x, s16 y ) {
    fileImage_t *image;
    u32 type;
    u32 width;
    u32 height;
    u8 *data;

    if ( D_8005C190 != DRAW2D_COPY ) {
        D_8005C190 = DRAW2D_COPY;
        gDPSetCycleType( game_gfx++, G_CYC_COPY );
        gDPSetRenderMode( game_gfx++, G_RM_NOOP, G_RM_NOOP2 );
        gDPSetAlphaCompare( game_gfx++, G_AC_THRESHOLD );
    }
    if ( D_800F80B0[id].refCount == 0 ) {
        Cache_GetFile( id, FILE_RAW );
    }
    image = D_800F80B0[id].data;
    type = image->type;
    width = image->width;
    height = image->height;
    data = image->data;
    if ( type == IMAGE_CI4 ) {
        memcpy( D_80062A70[sched_bufferIndex][D_8005C194], data, CI4_PALETTE_SIZE );
        Draw_CiImage( type, x, y, width, height, D_80062A70[sched_bufferIndex][D_8005C194],
                       data + CI4_PALETTE_SIZE );
    } else if ( type == IMAGE_CI8 ) {
        memcpy( D_80062A70[sched_bufferIndex][D_8005C194], data, CI8_PALETTE_SIZE );
        Draw_CiImage( type, x, y, width, height, D_80062A70[sched_bufferIndex][D_8005C194],
                       data + CI8_PALETTE_SIZE );
    } else {
        Draw_CiImage( type, x, y, width, height, D_80062A70[sched_bufferIndex][D_8005C194], data );
    }
    if ( ++D_8005C194 == 4 ) {
        D_8005C194 = 0;
    }
}

/*
================
Draw_ImageScaled

Draws image id at (x, y) scaled by (scaleX, scaleY), in one-cycle mode with its alpha multiplied by alpha:
loads its palette, then draws it in strips of whole rows that fit texture memory, as Draw_CiImage does.
Any type other than IMAGE_CI4 draws as IMAGE_CI8 (without loading a palette, and with the strip sizes unset).
The strips' screen positions are scaled in floating point and truncated to whole pixels.
================
*/
void Draw_ImageScaled( u16 id, s16 x, s16 y, u8 alpha, f32 scaleX, f32 scaleY ) {
    fileImage_t *image;
    s32 type;
    s32 width;
    s32 height;
    u8 *data;
    s32 texels;         /* pixels in texture memory */
    s32 pad;
    s32 i;
    s32 strip;
    s32 stripWidth;
    s32 restWidth;
    s32 remaining;
    s32 rows;
    s32 offset;

    if ( D_8005C190 != DRAW2D_1CYCLE ) {
        D_8005C190 = DRAW2D_1CYCLE;
        gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
        gDPSetRenderMode( game_gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
        gDPSetCombineLERP( game_gfx++, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, TEXEL0, TEXEL0, 0,
                           PRIMITIVE, 0 );
        gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    }
    gDPSetTextureFilter( game_gfx++, G_TF_POINT );
    gDPSetPrimColor( game_gfx++, 0, 0, 0xFF, 0xFF, 0xFF, alpha );
    if ( D_800F80B0[id].refCount == 0 ) {
        Cache_GetFile( id, FILE_RAW );
    }
    image = D_800F80B0[id].data;
    type = image->type;
    width = image->width;
    height = image->height;
    data = image->data;
    offset = 0;
    switch ( type ) {
    case IMAGE_CI4:
        texels = 0x1000;
        pad = width % 16;
        gDPLoadTLUT_pal16( game_gfx++, 0, data );
        offset = CI4_PALETTE_SIZE / sizeof( u16 );
        break;
    case IMAGE_CI8:
        texels = 0x800;
        pad = width % 8;
        gDPLoadTLUT_pal256( game_gfx++, data );
        offset = CI8_PALETTE_SIZE / sizeof( u16 );
        break;
    }
    /* the original's bug: any other type leaves texels and pad unset */
    for ( i = 1; i < 10; i++ ) {
        if ( ( 1 << i ) > width + pad ) {
            break;
        }
    }
    stripWidth = 1 << ( i - 1 );
    if ( width + pad == stripWidth ) {
        restWidth = 0;
    } else {
        for ( i = 1; i < 10; i++ ) {
            if ( ( 1 << i ) >= width + pad - stripWidth ) {
                break;
            }
        }
        restWidth = 1 << i;
    }
    if ( type == IMAGE_CI4 ) {
        for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
            rows = texels / stripWidth;
            if ( remaining < rows ) {
                rows = remaining;
                remaining = 0;
            } else {
                remaining -= rows;
            }
            gDPLoadTextureBlock_4b( game_gfx++, (u16 *)data + offset, G_IM_FMT_CI, stripWidth, rows, 0,
                                    G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
            offset += rows * stripWidth / 4;
            /* the original's bug: the top edge is scaled by scaleX */
            gSPScisTextureRectangle( game_gfx++, x << 2, (s16)( y + scaleX * strip * texels / stripWidth ) << 2,
                                     (s16)( x + scaleX * stripWidth ) << 2,
                                     (s16)( y + scaleY * strip * texels / stripWidth + scaleY * rows ) << 2,
                                     G_TX_RENDERTILE, 0, 0, 1024.0f / scaleX, 1024.0f / scaleY );
        }
        if ( restWidth != 0 ) {
            for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
                rows = texels / restWidth;
                if ( remaining < rows ) {
                    rows = remaining;
                    remaining = 0;
                } else {
                    remaining -= rows;
                }
                gDPLoadTextureBlock_4b( game_gfx++, (u16 *)data + offset, G_IM_FMT_CI, restWidth, rows, 0,
                                        G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
                offset += rows * restWidth / 4;
                gSPScisTextureRectangle( game_gfx++, (s16)( x + scaleX * stripWidth ) << 2,
                                         (s16)( y + scaleY * strip * texels / restWidth ) << 2,
                                         (s16)( x + scaleX * ( stripWidth + restWidth ) ) << 2,
                                         (s16)( y + scaleY * ( strip * texels / restWidth + rows ) ) << 2, G_TX_RENDERTILE,
                                         0, 0, 1024.0f / scaleX, 1024.0f / scaleY );
            }
        }
    } else {
        for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
            rows = texels / stripWidth;
            if ( remaining < rows ) {
                rows = remaining;
                remaining = 0;
            } else {
                remaining -= rows;
            }
            gDPLoadTextureBlock( game_gfx++, (u16 *)data + offset, G_IM_FMT_CI, G_IM_SIZ_8b, stripWidth, rows,
                                 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD );
            offset += rows * stripWidth / 2;
            gSPScisTextureRectangle( game_gfx++, x << 2, (s16)( y + scaleY * strip * texels / stripWidth ) << 2,
                                     (s16)( x + scaleX * stripWidth ) << 2,
                                     (s16)( y + scaleY * ( strip * texels / stripWidth + rows ) ) << 2, G_TX_RENDERTILE,
                                     0, 0, 1024.0f / scaleX, 1024.0f / scaleY );
        }
        if ( restWidth != 0 ) {
            for ( remaining = height, strip = 0; remaining > 0; strip++ ) {
                rows = texels / restWidth;
                if ( remaining < rows ) {
                    rows = remaining;
                    remaining = 0;
                } else {
                    remaining -= rows;
                }
                gDPLoadTextureBlock( game_gfx++, (u16 *)data + offset, G_IM_FMT_CI, G_IM_SIZ_8b, restWidth,
                                     rows, 0, G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                     G_TX_NOLOD );
                offset += rows * restWidth / 2;
                gSPScisTextureRectangle( game_gfx++, (s16)( x + scaleX * stripWidth ) << 2,
                                         (s16)( y + scaleY * strip * texels / restWidth ) << 2,
                                         (s16)( x + scaleX * ( stripWidth + restWidth ) ) << 2,
                                         (s16)( y + scaleY * ( strip * texels / restWidth + rows ) ) << 2, G_TX_RENDERTILE,
                                         0, 0, 1024.0f / scaleX, 1024.0f / scaleY );
            }
        }
    }
    gDPPipeSync( game_gfx++ );
}

/*
================
Draw_FillRect

Fills the rectangle of width by height pixels at (x, y) with colour (r, g, b, a), clipped at the screen's top and
left edges; translucent unless a is 0xFF.
================
*/
void Draw_FillRect( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    if ( a == 0xFF ) {
        gDPSetRenderMode( game_gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    } else {
        gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
        gDPSetRenderMode( game_gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    }
    gDPSetCombineMode( game_gfx++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( game_gfx++, 0, 0, r, g, b, a );
    gSPScisTextureRectangle( game_gfx++, x << 2, y << 2, ( x + width + 1 ) << 2, ( y + height + 1 ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
}

/*
================
Draw_RectOutline

Draws the one-pixel outline of the rectangle of width by height at (x, y) in colour (r, g, b, a), as Draw_FillRect
does its fill.
================
*/
void Draw_RectOutline( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    if ( a == 0xFF ) {
        gDPSetRenderMode( game_gfx++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    } else {
        gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
        gDPSetRenderMode( game_gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    }
    gDPSetCombineMode( game_gfx++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( game_gfx++, 0, 0, r, g, b, a );
    gSPScisTextureRectangle( game_gfx++, x << 2, y << 2, ( x + width ) << 2, ( y + 1 ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( game_gfx++, x << 2, ( y + height - 1 ) << 2, ( x + width ) << 2,
                             ( y + height ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( game_gfx++, x << 2, y << 2, ( x + 1 ) << 2, ( y + height ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( game_gfx++, ( x + width - 1 ) << 2, y << 2, ( x + width ) << 2,
                             ( y + height ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gDPPipeSync( game_gfx++ );
}

/*
================
Draw_ClearScreen

Clears the depth buffer and fills the framebuffer being drawn with colour (r, g, b), leaving the RDP in fill mode.
================
*/
void Draw_ClearScreen( u32 r, u32 g, u32 b ) {
    gDPSetCycleType( game_gfx++, G_CYC_FILL );
    gDPSetRenderMode( game_gfx++, G_RM_NOOP, G_RM_NOOP2 );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical( D_80000400 ) );
    gDPSetFillColor( game_gfx++, GPACK_ZDZ( G_MAXFBZ, 0 ) << 16 | GPACK_ZDZ( G_MAXFBZ, 0 ) );
    gDPFillRectangle( game_gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPPipeSync( game_gfx++ );
    gDPSetColorImage( game_gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( sched_framebuffers[sched_drawFb] ) );
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( game_gfx++, G_CYC_FILL );
    gDPSetFillColor( game_gfx++, GPACK_RGBA5551( r, g, b, 1 ) << 16 | GPACK_RGBA5551( r, g, b, 1 ) );
    gDPFillRectangle( game_gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPSetTexturePersp( game_gfx++, G_TP_NONE );
    gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
    gDPSetPrimColor( game_gfx++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF );
}

/*
================
Draw_Line3D

Draws a line from start to end in the view of camera index, in colour (red, green, blue, alpha), color[4] pixels
wide, through the camera's two vertices D_80063A70[index].
================
*/
void Draw_Line3D( s32 index, f32 *start, f32 *end, u8 *color ) {
    u32 i;

    for ( i = 0; i < 3; i++ ) {
        D_80063A70[index][0].v.ob[i] = *start++;
        D_80063A70[index][1].v.ob[i] = *end++;
    }
    guMtxIdent( &D_80063AB0 );
    gSPLoadUcodeL( game_gfx++, gspL3DEX2_fifo );
    Scene_BeginView( D_8009CFA0[index] );
    gDPSetRenderMode( game_gfx++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2 );
    gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    gDPSetCombineMode( game_gfx++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( game_gfx++, 0, 0, color[0], color[1], color[2], color[3] );
    gSPMatrix( game_gfx++, &D_80063AB0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPVertex( game_gfx++, D_80063A70[index], 2, 0 );
    gSPLineW3D( game_gfx++, 0, 1, color[4], 0 );
    Scene_BeginFrame();
}
