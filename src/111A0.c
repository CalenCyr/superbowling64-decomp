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

/* ===== Data ===== */

u8 gridEd_active = 0;
static Vtx *gridEd_vtx = NULL;              /* a vertex per grid point, allocated on the first frame */
static s16 gridEd_cursorX = 0;              /* the cursor's point along x */
static s16 gridEd_cursorZ = 0;              /* the cursor's point along z */
static u16 gridEd_paint = 1;                /* the mark Z with the D-pad paints */
static u8 gridEd_heldFrames = 0;            /* frames the D-pad has been held, to 100; from 7 the cursor repeats */

/* ===== State ===== */

// volatile: retail loads it again, unused, right after each ++ and -- store, and re-reads and re-masks it before the switch
static volatile u8 gridEd_page;     /* the page, GRIDEDIT_* */
static Mtx gridEd_mtx;              /* identity, the model matrix of the grid */
static u8 gridEd_marked[GRIDEDIT_POINTS];  /* per point: marked */

/*
================
GridEd_Frame

Frame of the floor grid editor: draws the grid game_floorGrids[0] in lines through camera 0, marked points red and the
cursor flashing, then runs the page gridEd_page (R with up or down picks it) and prints the page, its values and the
height under the cursor.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register choices only: `resized` and the numZ address in page SIZE, and retail keeps &cont_allHeld
// in a register across the STEP and ORIGIN tests
// Permuter clue (1065 -> 960): `resized` reused as the clearing and marking loops' counter instead of i (R30).
// Flags (scratch compiles, -O1, -O3, -fno-inline-functions, 18 -f/-fno- options and their pairs): none moves
// it below its 132 differing lines at the unit's flags.
// Idiom pass: the permuter's shared flag/counter written out (resized folded into i, one local for both: 775 -> 758 diff
// lines, still not exact, left out as less readable); a floorGrid_t pointer for game_floorGrids[0] (1403).
// Equivalence: 141 random cases (9 skipped), no difference (tools/equiv.py).
void GridEd_Frame( void ) {
    u8 text[40];
    s32 x, z;
    s32 i;
    s32 resized;

    if ( gridEd_vtx == NULL ) {
        gridEd_page = GRIDEDIT_EXIT;
        gridEd_vtx = Mem_Alloc( GRIDEDIT_POINTS * sizeof( Vtx ) );
        memset( gridEd_marked, 0, sizeof( gridEd_marked ) );
        guMtxIdent( &gridEd_mtx );
    }

    gSPLoadUcodeL( game_gfx++, gspL3DEX2_fifo );
    Scene_BeginView( cam_cameras[0] );
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    gDPSetRenderMode( game_gfx++, G_RM_AA_ZB_XLU_LINE, G_RM_AA_ZB_XLU_LINE2 );
    gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    gDPSetCombineLERP( game_gfx++, 1, 0, SHADE, 0, 1, 0, PRIMITIVE, 0, 1, 0, SHADE, 0, 1, 0, PRIMITIVE, 0 );
    gDPSetPrimColor( game_gfx++, 0, 0, 0x80, 0x80, 0x80, 0x80 );
    gSPMatrix( game_gfx++, &gridEd_mtx, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPClearGeometryMode( game_gfx++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_TEXTURE_GEN | G_FOG );
    gSPSetGeometryMode( game_gfx++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH );

    for ( z = 0; z < game_floorGrids[0].numZ; z++ ) {
        for ( x = 0; x < game_floorGrids[0].numX; x++ ) {
            gridEd_vtx[x + z * game_floorGrids[0].numX].v.ob[0] = game_floorGrids[0].originX + x * game_floorGrids[0].stepX;
            gridEd_vtx[x + z * game_floorGrids[0].numX].v.ob[1] = game_floorGrids[0].heights[x + z * game_floorGrids[0].numX];
            gridEd_vtx[x + z * game_floorGrids[0].numX].v.ob[2] = game_floorGrids[0].originZ + z * game_floorGrids[0].stepZ;
            if ( gridEd_marked[x + z * game_floorGrids[0].numX] ) {
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[0] = 0xFF;
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[1] = 0;
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[2] = 0;
            } else {
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[0] = 0xFF;
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[1] = 0xFF;
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[2] = 0xFF;
            }
            if ( gridEd_cursorX == x && gridEd_cursorZ == z ) {
                gridEd_vtx[x + z * game_floorGrids[0].numX].v.cn[1] = sched_frameCount << 3;
            }
        }
    }

    for ( z = 0; z < game_floorGrids[0].numZ; z++ ) {
        for ( x = 0; x < game_floorGrids[0].numX; x++ ) {
            gSPVertex( game_gfx++, gridEd_vtx + x + z * game_floorGrids[0].numX, 1, 0 );
            if ( x + 1 < game_floorGrids[0].numX ) {
                gSPVertex( game_gfx++, gridEd_vtx + x + z * game_floorGrids[0].numX + 1, 1, 1 );
                gSPLine3D( game_gfx++, 0, 1, 0 );
            }
            if ( z + 1 < game_floorGrids[0].numZ ) {
                gSPVertex( game_gfx++, gridEd_vtx + x + ( z + 1 ) * game_floorGrids[0].numX, 1, 1 );
                gSPLine3D( game_gfx++, 0, 1, 0 );
            }
        }
    }

    gDPPipeSync( game_gfx++ );
    Scene_BeginFrame();
    Txt_LoadFont( &game_gfx );
    Txt_SetupFont( &game_gfx, 0 );
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );

    if ( ( cont_allPressed & U_JPAD ) && ( cont_allHeld & R_TRIG ) && gridEd_page != GRIDEDIT_EXIT ) {
        gridEd_page--;
    }
    if ( ( cont_allPressed & D_JPAD ) && ( cont_allHeld & R_TRIG ) && gridEd_page < GRIDEDIT_PAGES - 1 ) {
        gridEd_page++;
    }

    switch ( gridEd_page ) {
        case GRIDEDIT_EXIT:
            Txt_DrawString( &game_gfx, 32, 20, "EXIT" );
            if ( cont_allPressed & Z_TRIG ) {
                gridEd_active = 0;
            }
            break;

        case GRIDEDIT_LOAD:
            Txt_DrawString( &game_gfx, 32, 20, "FILE LOAD" );
            if ( cont_allPressed & Z_TRIG ) {
                Host_Sync();
                Host_GetFileSize( "ground.bin" );
                Host_ReadFile( (u8 *)&game_floorGrids[0] );
            }
            break;

        case GRIDEDIT_SAVE:
            Txt_DrawString( &game_gfx, 32, 20, "FILE SAVE" );
            if ( cont_allPressed & Z_TRIG ) {
                Host_WriteFile( "ground.bin", (u8 *)&game_floorGrids[0],
                               sizeof( floorGrid_t ) - ( GRIDEDIT_POINTS - game_floorGrids[0].numX * game_floorGrids[0].numZ ) * sizeof( s16 ) );
            }
            break;

        case GRIDEDIT_SIZE:
            Txt_DrawString( &game_gfx, 32, 20, "MTX FORM" );
            resized = FALSE;
            if ( ( cont_allPressed & U_JPAD ) && !( cont_allHeld & R_TRIG ) && game_floorGrids[0].numZ != 0 ) {
                game_floorGrids[0].numZ--;
                resized = TRUE;
            }
            if ( ( cont_allPressed & D_JPAD ) && !( cont_allHeld & R_TRIG ) ) {
                if ( ( game_floorGrids[0].numZ + 1 ) * game_floorGrids[0].numX < GRIDEDIT_POINTS ) {
                    game_floorGrids[0].numZ++;
                }
                resized = TRUE;
            }
            if ( ( cont_allPressed & L_JPAD ) && !( cont_allHeld & R_TRIG ) && game_floorGrids[0].numX > 2 ) {
                game_floorGrids[0].numX -= 2;
                if ( !( game_floorGrids[0].numX & 1 ) ) {
                    game_floorGrids[0].numX++;
                }
                resized = TRUE;
            }
            if ( ( cont_allPressed & R_JPAD ) && !( cont_allHeld & R_TRIG ) ) {
                if ( ( game_floorGrids[0].numX + 2 ) * game_floorGrids[0].numZ < GRIDEDIT_POINTS ) {
                    game_floorGrids[0].numX += 2;
                }
                resized = TRUE;
            }
            if ( resized ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    game_floorGrids[0].heights[i] = 0;
                }
            }
            sprintf( text, "X*Z=(%d,%d)", game_floorGrids[0].numX, game_floorGrids[0].numZ );
            Txt_DrawString( &game_gfx, 64, 30, text );
            break;

        case GRIDEDIT_STEP:
            Txt_DrawString( &game_gfx, 32, 20, "MTX WH" );
            if ( ( cont_allHeld & ( U_JPAD | R_TRIG ) ) == U_JPAD && game_floorGrids[0].stepZ != 0 ) {
                game_floorGrids[0].stepZ--;
            }
            if ( ( cont_allHeld & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                game_floorGrids[0].stepZ++;
            }
            if ( ( cont_allHeld & ( L_JPAD | R_TRIG ) ) == L_JPAD && game_floorGrids[0].stepX != 0 ) {
                game_floorGrids[0].stepX--;
            }
            if ( ( cont_allHeld & ( R_JPAD | R_TRIG ) ) == R_JPAD ) {
                game_floorGrids[0].stepX++;
            }
            sprintf( text, "WH=(%d,%d)", game_floorGrids[0].stepX, game_floorGrids[0].stepZ );
            Txt_DrawString( &game_gfx, 64, 30, text );
            break;

        case GRIDEDIT_ORIGIN:
            Txt_DrawString( &game_gfx, 32, 20, "MTX OFS" );
            if ( ( cont_allHeld & ( U_JPAD | R_TRIG ) ) == U_JPAD ) {
                game_floorGrids[0].originZ--;
            }
            if ( ( cont_allHeld & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                game_floorGrids[0].originZ++;
            }
            if ( ( cont_allHeld & ( L_JPAD | R_TRIG ) ) == L_JPAD ) {
                game_floorGrids[0].originX--;
            }
            if ( ( cont_allHeld & ( R_JPAD | R_TRIG ) ) == R_JPAD ) {
                game_floorGrids[0].originX++;
            }
            sprintf( text, "OFS=(%d,%d)", game_floorGrids[0].originX, game_floorGrids[0].originZ );
            Txt_DrawString( &game_gfx, 64, 30, text );
            break;

        case GRIDEDIT_SELECT:
            Txt_DrawString( &game_gfx, 32, 20, "VTX SEL" );
            if ( ( cont_allPressed & U_JPAD ) || ( gridEd_heldFrames >= 7 && ( cont_allHeld & U_JPAD ) ) ) {
                if ( gridEd_cursorZ > 0 ) {
                    gridEd_cursorZ--;
                }
                if ( cont_allHeld & Z_TRIG ) {
                    gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] = gridEd_paint;
                }
            }
            if ( ( cont_allPressed & D_JPAD ) || ( gridEd_heldFrames >= 7 && ( cont_allHeld & D_JPAD ) ) ) {
                if ( gridEd_cursorZ < game_floorGrids[0].numZ - 1 ) {
                    gridEd_cursorZ++;
                }
                if ( cont_allHeld & Z_TRIG ) {
                    gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] = gridEd_paint;
                }
            }
            if ( ( cont_allPressed & L_JPAD ) || ( gridEd_heldFrames >= 7 && ( cont_allHeld & L_JPAD ) ) ) {
                if ( gridEd_cursorX > 0 ) {
                    gridEd_cursorX--;
                }
                if ( cont_allHeld & Z_TRIG ) {
                    gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] = gridEd_paint;
                }
            }
            if ( ( cont_allPressed & R_JPAD ) || ( gridEd_heldFrames >= 7 && ( cont_allHeld & R_JPAD ) ) ) {
                if ( gridEd_cursorX < game_floorGrids[0].numX - 1 ) {
                    gridEd_cursorX++;
                }
                if ( cont_allHeld & Z_TRIG ) {
                    gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] = gridEd_paint;
                }
            }
            if ( cont_allHeld & ( U_JPAD | D_JPAD | L_JPAD | R_JPAD ) ) {
                if ( gridEd_heldFrames < 100 ) {
                    gridEd_heldFrames++;
                }
            } else {
                gridEd_heldFrames = 0;
            }
            if ( cont_allPressed & Z_TRIG ) {
                gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] = gridEd_paint =
                    gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] ^ 1;
            }
            if ( ( cont_allPressed & START_BUTTON ) && !( cont_allHeld & R_TRIG ) ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    gridEd_marked[i] = 0;
                }
            }
            if ( ( cont_allPressed & START_BUTTON ) && ( cont_allHeld & R_TRIG ) ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    gridEd_marked[i] ^= 1;
                }
            }
            sprintf( text, "SEL=%d", gridEd_marked[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] );
            Txt_DrawString( &game_gfx, 64, 30, text );
            break;

        case GRIDEDIT_HEIGHT:
            Txt_DrawString( &game_gfx, 32, 20, "VTX Y" );
            if ( ( cont_allHeld & ( U_JPAD | R_TRIG ) ) == U_JPAD ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    if ( gridEd_marked[i] ) {
                        game_floorGrids[0].heights[i]++;
                    }
                }
            }
            if ( ( cont_allHeld & ( D_JPAD | R_TRIG ) ) == D_JPAD ) {
                for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                    if ( gridEd_marked[i] ) {
                        game_floorGrids[0].heights[i]--;
                    }
                }
            }
            for ( i = 0; i < GRIDEDIT_POINTS; i++ ) {
                if ( gridEd_marked[i] ) {
                    sprintf( text, "Y=%d", game_floorGrids[0].heights[i] );
                    Txt_DrawString( &game_gfx, 64, 30, text );
                    break;
                }
            }
            break;
    }

    sprintf( text, "(%d,%d)=%d", gridEd_cursorX, gridEd_cursorZ,
             game_floorGrids[0].heights[gridEd_cursorX + gridEd_cursorZ * game_floorGrids[0].numX] );
    Txt_DrawString( &game_gfx, 120, 20, text );
}
#else
INCLUDE_ASM("asm/nonmatchings/111A0", GridEd_Frame);
#endif
