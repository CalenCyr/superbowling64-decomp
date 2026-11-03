/* 7250.c -- 2D drawing into the display list: images of the file cache, and rectangles */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "sprite.h"
#include "object.h"

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

INCLUDE_ASM("asm/nonmatchings/7250", func_8002BFD8);

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

INCLUDE_ASM("asm/nonmatchings/7250", func_8002FDE4);

INCLUDE_ASM("asm/nonmatchings/7250", func_80030058);

INCLUDE_ASM("asm/nonmatchings/7250", func_8003068C);

INCLUDE_ASM("asm/nonmatchings/7250", func_800308C4);
