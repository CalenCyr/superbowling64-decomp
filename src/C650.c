/* C650.c -- the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "ground.h"

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

INCLUDE_ASM("asm/nonmatchings/C650", func_80035074);

INCLUDE_ASM("asm/nonmatchings/C650", func_80035220);

INCLUDE_ASM("asm/nonmatchings/C650", func_80035380);

INCLUDE_ASM("asm/nonmatchings/C650", func_80035748);
