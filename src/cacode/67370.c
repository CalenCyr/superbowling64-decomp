/* 67370.c -- cacode overlay */

#include "common.h"
#include "libultra.h"
#include "cacode/cacode.h"

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_80112350_cacode);

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

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_801128F4_cacode);

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_80112CDC_cacode);

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
