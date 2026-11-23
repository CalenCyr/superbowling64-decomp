/* 111A0.c -- the debug editor of the floor grid */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "ground.h"
#include "game.h"
#include "heap.h"
#include "hostio.h"
#include "dbgtext.h"
#include "gridedit.h"

/*
================
func_80035DA0

Frame of the floor grid editor: draws the grid D_80100100[0] in lines through camera 0, marked points red and the
cursor flashing, then runs the page D_80063AF0 (R with up or down picks it) and prints the page, its values and the
height under the cursor.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: with D_80063AF0 volatile (retail reloads it after each store and before the switch, and orders the
// first block's stores that way) everything but register choices matches: `resized` and the numZ address in page
// SIZE, and retail keeps &D_800994BC in a register across the STEP and ORIGIN tests
void func_80035DA0( void ) {
    u8 text[40];
    s32 x, z;
    s32 i;
    s32 resized;

    if ( D_8005C984 == NULL ) {
        D_80063AF0 = GRIDEDIT_EXIT;
        D_8005C984 = func_80029E28( GRIDEDIT_POINTS * sizeof( Vtx ) );
        memset( D_80063B40, 0, sizeof( D_80063B40 ) );
        guMtxIdent( &D_80063AF8 );
    }

    gSPLoadUcodeL( D_800F8094++, gspL3DEX2_fifo );
    func_800316C0( D_8009CFA0[0] );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_AA_ZB_XLU_LINE, G_RM_AA_ZB_XLU_LINE2 );
    gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
    gDPSetCombineLERP( D_800F8094++, 1, 0, SHADE, 0, 1, 0, PRIMITIVE, 0, 1, 0, SHADE, 0, 1, 0, PRIMITIVE, 0 );
    gDPSetPrimColor( D_800F8094++, 0, 0, 0x80, 0x80, 0x80, 0x80 );
    gSPMatrix( D_800F8094++, &D_80063AF8, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPClearGeometryMode( D_800F8094++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_TEXTURE_GEN | G_FOG );
    gSPSetGeometryMode( D_800F8094++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH );

    for ( z = 0; z < D_80100100[0].numZ; z++ ) {
        for ( x = 0; x < D_80100100[0].numX; x++ ) {
            D_8005C984[x + z * D_80100100[0].numX].v.ob[0] = D_80100100[0].originX + x * D_80100100[0].stepX;
            D_8005C984[x + z * D_80100100[0].numX].v.ob[1] = D_80100100[0].heights[x + z * D_80100100[0].numX];
            D_8005C984[x + z * D_80100100[0].numX].v.ob[2] = D_80100100[0].originZ + z * D_80100100[0].stepZ;
            if ( D_80063B40[x + z * D_80100100[0].numX] ) {
                D_8005C984[x + z * D_80100100[0].numX].v.cn[0] = 0xFF;
                D_8005C984[x + z * D_80100100[0].numX].v.cn[1] = 0;
                D_8005C984[x + z * D_80100100[0].numX].v.cn[2] = 0;
            } else {
                D_8005C984[x + z * D_80100100[0].numX].v.cn[0] = 0xFF;
                D_8005C984[x + z * D_80100100[0].numX].v.cn[1] = 0xFF;
                D_8005C984[x + z * D_80100100[0].numX].v.cn[2] = 0xFF;
            }
            if ( D_8005C988 == x && D_8005C98A == z ) {
                D_8005C984[x + z * D_80100100[0].numX].v.cn[1] = D_8005B0F4 << 3;
            }
        }
    }

    for ( z = 0; z < D_80100100[0].numZ; z++ ) {
        for ( x = 0; x < D_80100100[0].numX; x++ ) {
            gSPVertex( D_800F8094++, D_8005C984 + x + z * D_80100100[0].numX, 1, 0 );
            if ( x + 1 < D_80100100[0].numX ) {
                gSPVertex( D_800F8094++, D_8005C984 + x + z * D_80100100[0].numX + 1, 1, 1 );
                gSPLine3D( D_800F8094++, 0, 1, 0 );
            }
            if ( z + 1 < D_80100100[0].numZ ) {
                gSPVertex( D_800F8094++, D_8005C984 + x + ( z + 1 ) * D_80100100[0].numX, 1, 1 );
                gSPLine3D( D_800F8094++, 0, 1, 0 );
            }
        }
    }

    gDPPipeSync( D_800F8094++ );
    func_80031BB0();
    func_80028A0C( &D_800F8094 );
    func_80028950( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );

    if ( ( D_800E4D62 & U_JPAD ) && ( D_800994BC & R_TRIG ) && D_80063AF0 != GRIDEDIT_EXIT ) {
        D_80063AF0--;
    }
    if ( ( D_800E4D62 & D_JPAD ) && ( D_800994BC & R_TRIG ) && D_80063AF0 < GRIDEDIT_PAGES - 1 ) {
        D_80063AF0++;
    }

    switch ( D_80063AF0 ) {
        case GRIDEDIT_EXIT:
            func_80028DA4( &D_800F8094, 32, 20, "EXIT" );
            if ( D_800E4D62 & Z_TRIG ) {
                D_8005C980 = 0;
            }
            break;

        case GRIDEDIT_LOAD:
            func_80028DA4( &D_800F8094, 32, 20, "FILE LOAD" );
            if ( D_800E4D62 & Z_TRIG ) {
                func_80029524();
                func_8002961C( "ground.bin" );
                func_800296D4( (u8 *)&D_80100100[0] );
            }
            break;

        case GRIDEDIT_SAVE:
            func_80028DA4( &D_800F8094, 32, 20, "FILE SAVE" );
            if ( D_800E4D62 & Z_TRIG ) {
                func_80029814( "ground.bin", (u8 *)&D_80100100[0],
                               sizeof( floorGrid_t ) - ( GRIDEDIT_POINTS - D_80100100[0].numX * D_80100100[0].numZ ) * sizeof( s16 ) );
            }
            break;

        case GRIDEDIT_SIZE:
            func_80028DA4( &D_800F8094, 32, 20, "MTX FORM" );
            resized = FALSE;
            if ( ( D_800E4D62 & U_JPAD ) && !( D_800994BC & R_TRIG ) && D_80100100[0].numZ != 0 ) {
                D_80100100[0].numZ--;
                resized = TRUE;
            }
            if ( ( D_800E4D62 & D_JPAD ) && !( D_800994BC & R_TRIG ) ) {
                if ( ( D_80100100[0].numZ + 1 ) * D_80100100[0].numX < GRIDEDIT_POINTS ) {
                    D_80100100[0].numZ++;
                }
                resized = TRUE;
            }
            if ( ( D_800E4D62 & L_JPAD ) && !( D_800994BC & R_TRIG ) && D_80100100[0].numX > 2 ) {
                D_80100100[0].numX -= 2;
                if ( !( D_80100100[0].numX & 1 ) ) {
                    D_80100100[0].numX++;
                }
                resized = TRUE;
            }
            if ( ( D_800E4D62 & R_JPAD ) && !( D_800994BC & R_TRIG ) ) {
                if ( ( D_80100100[0].numX + 2 ) * D_80100100[0].numZ < GRIDEDIT_POINTS ) {
                    D_80100100[0].numX += 2;
                }
                resized = TRUE;
            }
            if ( resized ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    D_80100100[0].heights[i] = 0;
                }
            }
            sprintf( text, "X*Z=(%d,%d)", D_80100100[0].numX, D_80100100[0].numZ );
            func_80028DA4( &D_800F8094, 64, 30, text );
            break;

        case GRIDEDIT_STEP:
            func_80028DA4( &D_800F8094, 32, 20, "MTX WH" );
            if ( ( D_800994BC & ( U_JPAD | R_TRIG ) ) == U_JPAD && D_80100100[0].stepZ != 0 ) {
                D_80100100[0].stepZ--;
            }
            if ( ( D_800994BC & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                D_80100100[0].stepZ++;
            }
            if ( ( D_800994BC & ( L_JPAD | R_TRIG ) ) == L_JPAD && D_80100100[0].stepX != 0 ) {
                D_80100100[0].stepX--;
            }
            if ( ( D_800994BC & ( R_JPAD | R_TRIG ) ) == R_JPAD ) {
                D_80100100[0].stepX++;
            }
            sprintf( text, "WH=(%d,%d)", D_80100100[0].stepX, D_80100100[0].stepZ );
            func_80028DA4( &D_800F8094, 64, 30, text );
            break;

        case GRIDEDIT_ORIGIN:
            func_80028DA4( &D_800F8094, 32, 20, "MTX OFS" );
            if ( ( D_800994BC & ( U_JPAD | R_TRIG ) ) == U_JPAD ) {
                D_80100100[0].originZ--;
            }
            if ( ( D_800994BC & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                D_80100100[0].originZ++;
            }
            if ( ( D_800994BC & ( L_JPAD | R_TRIG ) ) == L_JPAD ) {
                D_80100100[0].originX--;
            }
            if ( ( D_800994BC & ( R_JPAD | R_TRIG ) ) == R_JPAD ) {
                D_80100100[0].originX++;
            }
            sprintf( text, "OFS=(%d,%d)", D_80100100[0].originX, D_80100100[0].originZ );
            func_80028DA4( &D_800F8094, 64, 30, text );
            break;

        case GRIDEDIT_SELECT:
            func_80028DA4( &D_800F8094, 32, 20, "VTX SEL" );
            if ( ( D_800E4D62 & U_JPAD ) || ( D_8005C98E >= 7 && ( D_800994BC & U_JPAD ) ) ) {
                if ( D_8005C98A > 0 ) {
                    D_8005C98A--;
                }
                if ( D_800994BC & Z_TRIG ) {
                    D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] = D_8005C98C;
                }
            }
            if ( ( D_800E4D62 & D_JPAD ) || ( D_8005C98E >= 7 && ( D_800994BC & D_JPAD ) ) ) {
                if ( D_8005C98A < D_80100100[0].numZ - 1 ) {
                    D_8005C98A++;
                }
                if ( D_800994BC & Z_TRIG ) {
                    D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] = D_8005C98C;
                }
            }
            if ( ( D_800E4D62 & L_JPAD ) || ( D_8005C98E >= 7 && ( D_800994BC & L_JPAD ) ) ) {
                if ( D_8005C988 > 0 ) {
                    D_8005C988--;
                }
                if ( D_800994BC & Z_TRIG ) {
                    D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] = D_8005C98C;
                }
            }
            if ( ( D_800E4D62 & R_JPAD ) || ( D_8005C98E >= 7 && ( D_800994BC & R_JPAD ) ) ) {
                if ( D_8005C988 < D_80100100[0].numX - 1 ) {
                    D_8005C988++;
                }
                if ( D_800994BC & Z_TRIG ) {
                    D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] = D_8005C98C;
                }
            }
            if ( D_800994BC & ( U_JPAD | D_JPAD | L_JPAD | R_JPAD ) ) {
                if ( D_8005C98E < 100 ) {
                    D_8005C98E++;
                }
            } else {
                D_8005C98E = 0;
            }
            if ( D_800E4D62 & Z_TRIG ) {
                D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] = D_8005C98C =
                    D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] ^ 1;
            }
            if ( ( D_800E4D62 & START_BUTTON ) && !( D_800994BC & R_TRIG ) ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    D_80063B40[i] = 0;
                }
            }
            if ( ( D_800E4D62 & START_BUTTON ) && ( D_800994BC & R_TRIG ) ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    D_80063B40[i] ^= 1;
                }
            }
            sprintf( text, "SEL=%d", D_80063B40[D_8005C988 + D_8005C98A * D_80100100[0].numX] );
            func_80028DA4( &D_800F8094, 64, 30, text );
            break;

        case GRIDEDIT_HEIGHT:
            func_80028DA4( &D_800F8094, 32, 20, "VTX Y" );
            if ( ( D_800994BC & ( U_JPAD | R_TRIG ) ) == U_JPAD ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    if ( D_80063B40[i] ) {
                        D_80100100[0].heights[i]++;
                    }
                }
            }
            if ( ( D_800994BC & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    if ( D_80063B40[i] ) {
                        D_80100100[0].heights[i]--;
                    }
                }
            }
            for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                if ( D_80063B40[i] ) {
                    sprintf( text, "Y=%d", D_80100100[0].heights[i] );
                    func_80028DA4( &D_800F8094, 64, 30, text );
                    break;
                }
            }
            break;
    }

    sprintf( text, "(%d,%d)=%d", D_8005C988, D_8005C98A,
             D_80100100[0].heights[D_8005C988 + D_8005C98A * D_80100100[0].numX] );
    func_80028DA4( &D_800F8094, 120, 20, text );
}
#else
INCLUDE_ASM("asm/nonmatchings/111A0", func_80035DA0);
#endif
