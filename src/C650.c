/* C650.c -- the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "ground.h"
#include "filecache.h"
#include "object.h"

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

INCLUDE_ASM("asm/nonmatchings/C650", func_8003128C);

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

INCLUDE_ASM("asm/nonmatchings/C650", func_800316C0);

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

INCLUDE_ASM("asm/nonmatchings/C650", func_80035748);
