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

/*
================
func_8002BE50

Sets the ambient colour of camera index to color (red, green, blue).
================
*/
void func_8002BE50( s32 index, u8 *color ) {
    u32 i;

    for ( i = 0; i < 3; i++ ) {
        D_8009CFA0[index]->lights[D_8005C0B1].a.l.col[i] = D_8009CFA0[index]->lights[D_8005C0B1].a.l.colc[i] =
            color[i];
    }
}

/*
================
func_8002BED8

Reads frame of camera path file into eye and unk_2C, each component from the frame or the path's default.
================
*/
void func_8002BED8( s16 file, s16 frame, f32 *eye, f32 *unk_2C ) {
    fileType3_t *path;
    f32 *data;
    s32 i;

    path = func_80030C04( file, FILE_TYPE3 );
    data = path->frames + path->frameSize * frame;
    for ( i = 0; i < 3; i++ ) {
        if ( path->eyeMask & ( 4 >> i ) ) {
            *eye++ = *data++;
        } else {
            *eye++ = path->eye[i];
        }
    }
    for ( i = 0; i < 3; i++ ) {
        if ( path->unk_12 & ( 4 >> i ) ) {
            unk_2C[i] = *data++;
        } else {
            unk_2C[i] = path->unk_2C[i];
        }
    }
}

/*
================
func_8002BFD8

Creates a camera from params: its path (unk_08[0]) with the starting eye, at and up point, its viewport size and
target (unk_08[1] to unk_08[6]), the path's light and a grey one from the eye, and lists it in D_8009CFA0 and,
by priority, in D_800EA120.
================
*/
objNode_t *func_8002BFD8( objParams_t *params ) {
    sceneCameraNode_t *node;
    sceneCamera_t *cam;
    s32 i;

    node = func_80029E28( sizeof( sceneCameraNode_t ) );
    cam = &node->cam;
    cam->path = func_80030C04( params->unk_08[0], FILE_TYPE3 );
    for ( i = 0; i < 3; i++ ) {
        cam->at[i] = cam->path->at[i];
        cam->eye[i] = cam->path->eye[i];
        cam->unk_18[i] = cam->path->unk_2C[i];
    }
    D_8009CFA0[D_80099480++] = cam;
    cam->width = params->unk_08[1];
    cam->height = params->unk_08[2];
    cam->unk_2C = params->unk_08[3];
    cam->unk_2E = params->unk_08[4];
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

INCLUDE_ASM("asm/nonmatchings/7250", func_8002C324);

/*
================
func_8002C908

Updates cam, and advances it along its path, from the start after the last frame.
================
*/
void func_8002C908( sceneCamera_t *cam ) {
    func_8002C324( cam );
    cam->frame += D_800F7BF8;
    if ( cam->frame == cam->path->numFrames ) {
        cam->frame = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/7250", func_8002C950);

INCLUDE_ASM("asm/nonmatchings/7250", func_8002CE24);

/*
================
func_8002DF3C

Draws image id unscaled at (x, y), in copy mode.
================
*/
void func_8002DF3C( u16 id, s16 x, s16 y ) {
    fileImage_t *image;
    u32 type;
    u32 width;
    u32 height;
    u8 *data;

    if ( D_8005C190 != DRAW2D_COPY ) {
        D_8005C190 = DRAW2D_COPY;
        gDPSetCycleType( D_800F8094++, G_CYC_COPY );
        gDPSetRenderMode( D_800F8094++, G_RM_NOOP, G_RM_NOOP2 );
        gDPSetAlphaCompare( D_800F8094++, G_AC_THRESHOLD );
    }
    if ( D_800F80B0[id].refCount == 0 ) {
        func_80030C04( id, FILE_RAW );
    }
    image = D_800F80B0[id].data;
    type = image->type;
    width = image->width;
    height = image->height;
    data = image->data;
    if ( type == IMAGE_CI4 ) {
        func_8002CE24( IMAGE_CI4, x, y, width, height, data, data + CI4_PALETTE_SIZE );
    } else if ( type == IMAGE_CI8 ) {
        func_8002CE24( IMAGE_CI8, x, y, width, height, data, data + CI8_PALETTE_SIZE );
    } else {
        func_8002CE24( type, x, y, width, height, data, data );
    }
}

INCLUDE_ASM("asm/nonmatchings/7250", func_8002E0EC);

INCLUDE_ASM("asm/nonmatchings/7250", func_8002E4B0);

/*
================
func_8002FDE4

Fills the rectangle of width by height pixels at (x, y) with colour (r, g, b, a), clipped at the screen's top and
left edges; translucent unless a is 0xFF.
================
*/
void func_8002FDE4( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    if ( a == 0xFF ) {
        gDPSetRenderMode( D_800F8094++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    } else {
        gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
        gDPSetRenderMode( D_800F8094++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    }
    gDPSetCombineMode( D_800F8094++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( D_800F8094++, 0, 0, r, g, b, a );
    gSPScisTextureRectangle( D_800F8094++, x << 2, y << 2, ( x + width + 1 ) << 2, ( y + height + 1 ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gDPPipeSync( D_800F8094++ );
}

/*
================
func_80030058

Draws the one-pixel outline of the rectangle of width by height at (x, y) in colour (r, g, b, a), as func_8002FDE4
does its fill.
================
*/
void func_80030058( s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    if ( a == 0xFF ) {
        gDPSetRenderMode( D_800F8094++, G_RM_OPA_SURF, G_RM_OPA_SURF2 );
    } else {
        gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
        gDPSetRenderMode( D_800F8094++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    }
    gDPSetCombineMode( D_800F8094++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( D_800F8094++, 0, 0, r, g, b, a );
    gSPScisTextureRectangle( D_800F8094++, x << 2, y << 2, ( x + width ) << 2, ( y + 1 ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( D_800F8094++, x << 2, ( y + height - 1 ) << 2, ( x + width ) << 2,
                             ( y + height ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( D_800F8094++, x << 2, y << 2, ( x + 1 ) << 2, ( y + height ) << 2,
                             G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gSPScisTextureRectangle( D_800F8094++, ( x + width - 1 ) << 2, y << 2, ( x + width ) << 2,
                             ( y + height ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
    gDPPipeSync( D_800F8094++ );
}

/*
================
func_8003068C

Clears the depth buffer and fills the framebuffer being drawn with colour (r, g, b), leaving the RDP in fill mode.
================
*/
void func_8003068C( u32 r, u32 g, u32 b ) {
    gDPSetCycleType( D_800F8094++, G_CYC_FILL );
    gDPSetRenderMode( D_800F8094++, G_RM_NOOP, G_RM_NOOP2 );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical( D_80000400 ) );
    gDPSetFillColor( D_800F8094++, GPACK_ZDZ( G_MAXFBZ, 0 ) << 16 | GPACK_ZDZ( G_MAXFBZ, 0 ) );
    gDPFillRectangle( D_800F8094++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPPipeSync( D_800F8094++ );
    gDPSetColorImage( D_800F8094++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH,
                      osVirtualToPhysical( D_80066C38[D_8005B0F8] ) );
    D_8005C190 = DRAW2D_FILL;
    gDPSetCycleType( D_800F8094++, G_CYC_FILL );
    gDPSetFillColor( D_800F8094++, GPACK_RGBA5551( r, g, b, 1 ) << 16 | GPACK_RGBA5551( r, g, b, 1 ) );
    gDPFillRectangle( D_800F8094++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 );
    gDPSetTexturePersp( D_800F8094++, G_TP_NONE );
    gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
    gDPSetPrimColor( D_800F8094++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF );
}

/*
================
func_800308C4

Draws a line from start to end in the view of camera index, in colour (red, green, blue, alpha), color[4] pixels
wide, through the camera's two vertices D_80063A70[index].
================
*/
void func_800308C4( s32 index, f32 *start, f32 *end, u8 *color ) {
    u32 i;

    for ( i = 0; i < 3; i++ ) {
        D_80063A70[index][0].v.ob[i] = *start++;
        D_80063A70[index][1].v.ob[i] = *end++;
    }
    guMtxIdent( &D_80063AB0 );
    gSPLoadUcode( D_800F8094++, D_592F0, D_5FC20 );
    func_800316C0( D_8009CFA0[index] );
    gDPSetRenderMode( D_800F8094++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2 );
    gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
    gDPSetCombineMode( D_800F8094++, G_CC_PRIMITIVE, G_CC_PRIMITIVE );
    gDPSetPrimColor( D_800F8094++, 0, 0, color[0], color[1], color[2], color[3] );
    gSPMatrix( D_800F8094++, &D_80063AB0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPVertex( D_800F8094++, D_80063A70[index], 2, 0 );
    gSPLineW3D( D_800F8094++, 0, 1, color[4], 0 );
    func_80031BB0();
}
