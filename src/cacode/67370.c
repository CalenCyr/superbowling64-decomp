/* 67370.c -- cacode overlay */

#include "common.h"
#include "libultra.h"
#include "cacode/cacode.h"

/*
================
func_80112350_cacode

Draws frame frame of clip clip from the frame cache as 40 textured rectangles filling the 320 x 240 screen, with
alpha alpha; nothing when the frame is not cached.
================
*/
void func_80112350_cacode( s32 frame, s32 clip, u8 alpha ) {
    s32 slot;
    s32 row, col;

    for ( slot = 0; slot < D_801157D4_cacode; slot++ ) {
        if ( D_80115630_cacode[slot].frame == frame && D_80115630_cacode[slot].clip == clip ) {
            break;
        }
    }
    if ( slot == D_801157D4_cacode ) {
        return;
    }

    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
    gDPSetTexturePersp( D_800F8094++, G_TP_NONE );
    gDPSetCombineLERP( D_800F8094++, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE,
                       0 );
    gDPSetPrimColor( D_800F8094++, 0, 0, 255, 255, 255, alpha );
    for ( row = 0; row < 8; row++ ) {
        for ( col = 0; col < 5; col++ ) {
            gDPLoadTextureBlock( D_800F8094++, D_80115630_cacode[slot].tiles[col + row * 5], G_IM_FMT_RGBA,
                                 G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 6, 5,
                                 G_TX_NOLOD, G_TX_NOLOD );
            gSPTextureRectangle( D_800F8094++, ( col * 64 ) << 2, ( row * 32 ) << 2, ( col * 64 + 64 ) << 2,
                                 ( row * 32 + 32 ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
        }
    }
    gDPPipeSync( D_800F8094++ );
}

/*
================
func_8011268C_cacode

Makes sure frame frame of clip clip is in the frame cache. A miss reads the clip's BMP file into D_8013E880
(from the development host, or else a fixed file of the str segment) over the least recently used slot, and
cuts its bottom-up 320 x 240 picture into the slot's tiles.
================
*/
void func_8011268C_cacode( s32 frame, s32 clip ) {
    u8 name[24];
    u8 *pixels;
    u16 *tile;
    u32 oldest;
    s32 slot;
    s32 i;
    s32 row, col, y, x;

    for ( slot = 0; slot < D_801157D4_cacode; slot++ ) {
        if ( D_80115630_cacode[slot].frame == frame && D_80115630_cacode[slot].clip == clip ) {
            break;
        }
    }
    if ( slot != D_801157D4_cacode ) {
        return;
    }

    oldest = -1;
    for ( i = 0; i < D_801157D4_cacode; i++ ) {
        if ( D_80115630_cacode[i].stamp < oldest ) {
            oldest = D_80115630_cacode[i].stamp;
            slot = i;
        }
    }

    if ( D_8005C0BC ) {
        sprintf( name, "%1.1d - Frame %4.4d.bmp", clip, frame + D_80115374_cacode[clip] );
        func_80029728( name, D_8013E880, 54 + 320 * 240 * 2 );    /* the headers and the picture */
    } else {
        func_80029AF4( 0xA58, D_8013E880 );
    }

    D_80115630_cacode[slot].frame = frame;
    D_80115630_cacode[slot].clip = clip;
    D_80115630_cacode[slot].stamp = -2;

    /* the file header's offset of the pixel data (bfOffBits, little-endian) */
    pixels = D_8013E880 + ( D_8013E880[10] + ( D_8013E880[11] << 8 ) + ( D_8013E880[12] << 16 ) + ( D_8013E880[13] << 24 ) );
    /*
    BMP rows run bottom-up, 640 bytes of little-endian 5-5-5 texels each; the texel is shifted into RGBA16 with alpha
    set. Eight rows of 32-line tiles are 256 lines, so the last 16 lines of the bottom row of tiles read the bytes
    before the pixel data, an original quirk.
    */
    for ( row = 0; row < 8; row++ ) {
        for ( col = 0; col < 5; col++ ) {
            tile = D_80115630_cacode[slot].tiles[col + row * 5];
            for ( y = 0; y < 32; y++ ) {
                for ( x = 0; x < 64; x++ ) {
                    tile[y * 64 + x] = ( ( pixels[( 239 - ( y + row * 32 ) ) * 640 + ( col * 128 + x * 2 )]
                                           + ( pixels[( 239 - ( y + row * 32 ) ) * 640 + ( col * 128 + x * 2 ) + 1] << 8 ) ) << 1 ) | 1;
                }
            }
        }
    }
}

/*
================
func_801128F4_cacode

Entry of the overlay: Start switches to files from the development host; from the host, the scene's records come
from its "sce.dat". Creates the scene's three cameras and three objects, fills the per-frame tables from cameras 1
and 2 and the parts of object D_80115368_cacode, and allocates the frame cache (34 slots when D_800A2FE4 is set,
else 9) and the BMP buffer.
================
*/
void func_801128F4_cacode( void ) {
    fileType1_t *model;
    s32 i;
    u32 j;

    if ( D_800994BC & START_BUTTON ) {
        D_8005C0BC = 1;
        func_80029524();
    }
    if ( D_8005C0BC ) {
        func_80029728( "sce.dat", (u8 *)D_801153C0_cacode, sizeof( D_801153C0_cacode ) );
        D_8005C0BD = 1;
        for ( i = 0; i < 3; i++ ) {
            D_801157E4_cacode[i] = (sceneCameraNode_t *)func_8002BFD8( &D_801153C0_cacode[i] );
            D_801157D8_cacode[i] = (sceneObjectNode_t *)func_8002B610( &D_801153C0_cacode[i + 3] );
        }
        D_8005C0BD = 0;
    } else {
        for ( i = 0; i < 3; i++ ) {
            D_801157E4_cacode[i] = (sceneCameraNode_t *)func_8002BFD8( &D_801153C0_cacode[i] );
            D_801157D8_cacode[i] = (sceneObjectNode_t *)func_8002B610( &D_801153C0_cacode[i + 3] );
        }
    }
    model = D_801157D8_cacode[D_80115368_cacode]->obj.model;
    D_80115374_cacode[0] = D_801157D8_cacode[0]->obj.anim[0];
    for ( i = 0; i < 300; i++ ) {
        guMtxIdentF( D_8012FE10[0][i] );
        guMtxIdentF( D_8012FE10[1][i] );
        memcpy( D_80139410[0][i], D_801157E4_cacode[1]->cam.at, sizeof( D_80139410[0][i] ) );
        memcpy( D_8013B030[0][i], D_801157E4_cacode[1]->cam.eye, sizeof( D_8013B030[0][i] ) );
        memcpy( D_8013CC50[0][i], D_801157E4_cacode[1]->cam.unk_18, sizeof( D_8013CC50[0][i] ) );
        memcpy( D_80139410[1][i], D_801157E4_cacode[2]->cam.at, sizeof( D_80139410[1][i] ) );
        memcpy( D_8013B030[1][i], D_801157E4_cacode[2]->cam.eye, sizeof( D_8013B030[1][i] ) );
        memcpy( D_8013CC50[1][i], D_801157E4_cacode[2]->cam.unk_18, sizeof( D_8013CC50[1][i] ) );
        for ( j = 0; j < model->numParts; j++ ) {
            memcpy( D_80115830_cacode[j][i], model->parts[j]->unk_08, sizeof( D_80115830_cacode[j][i] ) );
        }
    }
    guMtxIdentF( D_801157F0_cacode );
    if ( D_800A2FE4 ) {
        D_801157D4_cacode = 34;
    } else {
        D_801157D4_cacode = 9;
    }
    for ( i = 0; i < D_801157D4_cacode; i++ ) {
        D_80115630_cacode[i].frame = -1;
        D_80115630_cacode[i].stamp = 0;
        D_80115630_cacode[i].tiles = func_80029E28( 40 * sizeof( *D_80115630_cacode[i].tiles ) );
    }
    D_8013E880 = func_80029E28( 54 + 320 * 240 * 2 );
}

/*
================
func_80112CDC_cacode

Splits the matrix src into its translation trans, its rotation angles rot (radians, wrapped into 0 to 2 pi) and
one uniform scale, written to all three components of scale.
================
*/
void func_80112CDC_cacode( caMatrix_t *src, f32 *trans, f32 *rot, f32 *scale ) {
    caMatrix_t mtx;
    f32 xx, yy, zz;
    f32 sinX;

    mtx = *src;
    trans[0] = mtx.m[3][0];
    trans[1] = mtx.m[3][1];
    trans[2] = mtx.m[3][2];

    rot[1] = atan( mtx.m[2][0] / mtx.m[2][2] );
    if ( rot[1] < 0.0f ) {
        rot[1] += 2 * M_PI;
    }
    rot[2] = atan( mtx.m[0][1] / mtx.m[1][1] );
    if ( rot[2] < 0.0f ) {
        rot[2] += 2 * M_PI;
    }
    if ( mtx.m[2][0] > 0.0f ) {
        if ( mtx.m[2][2] < 0.0f ) {
            rot[1] -= M_PI;
        }
    } else if ( mtx.m[2][2] < 0.0f ) {
        rot[1] += M_PI;
    }
    if ( mtx.m[0][1] > 0.0f ) {
        if ( mtx.m[1][1] < 0.0f ) {
            rot[2] -= M_PI;
        }
    } else if ( mtx.m[1][1] < 0.0f ) {
        rot[2] += M_PI;
    }

    sinX = -mtx.m[0][0] / mtx.m[2][1] - sinf( rot[1] ) * sinf( rot[2] );
    sinX = cosf( rot[1] ) * cosf( rot[2] ) / sinX;
    rot[0] = asin( sinX );
    if ( cosf( rot[0] ) < 0.0f ) {
        rot[0] = M_PI - rot[0];
    }
    if ( rot[0] < 0.0f ) {
        rot[0] += 2 * M_PI;
    }

    /* the scale comes from the largest component of the third row, the one least hurt by dividing */
    xx = mtx.m[2][0] * mtx.m[2][0];
    yy = mtx.m[2][1] * mtx.m[2][1];
    zz = mtx.m[2][2] * mtx.m[2][2];
    if ( yy < xx ) {
        if ( zz <= xx ) {
            scale[0] = scale[1] = scale[2] = mtx.m[2][0] / ( cosf( rot[0] ) * sinf( rot[1] ) );
            return;
        }
    } else if ( zz < yy ) {
        scale[0] = scale[1] = scale[2] = mtx.m[2][1] / -sinf( rot[0] );
        return;
    }
    scale[0] = scale[1] = scale[2] = mtx.m[2][2] / ( cosf( rot[0] ) * cosf( rot[1] ) );
}

/*
================
func_8011306C_cacode

Inverts the 4 x 4 matrix src into dst by Gauss-Jordan elimination on [src | I], without pivoting.
================
*/
void func_8011306C_cacode( float src[4][4], float dst[4][4] ) {
    float aug[4][8];
    float scale;
    int i, j, k;

    for ( i = 0; i < 4; i++ ) {
        for ( j = 0; j < 4; j++ ) {
            aug[i][j] = src[i][j];
            aug[i][j + 4] = 0;
        }
        aug[i][i + 4] = 1.0f;
    }

    for ( k = 0; k < 3; k++ ) {
        for ( i = k + 1; i < 4; i++ ) {
            scale = aug[i][k] / aug[k][k];
            for ( j = 0; j < 8; j++ ) {
                aug[i][j] -= scale * aug[k][j];
            }
        }
    }

    for ( k = 3; k >= 0; k-- ) {
        for ( j = 7; j >= k; j-- ) {
            aug[k][j] /= aug[k][k];
        }
        if ( k == 0 ) {
            break;  /* the top row has no rows above it to clear */
        }
        for ( i = k - 1; i >= 0; i-- ) {
            scale = aug[i][k];
            for ( j = 7; j >= k; j-- ) {
                aug[i][j] -= scale * aug[k][j];
            }
        }
    }

    for ( i = 0; i < 4; i++ ) {
        for ( j = 0; j < 4; j++ ) {
            dst[i][j] = aug[i][j + 4];
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_80113280_cacode);
