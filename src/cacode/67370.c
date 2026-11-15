/* 67370.c -- cacode overlay */

#include "common.h"
#include "libultra.h"
#include "cacode/cacode.h"

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_80112350_cacode);

INCLUDE_ASM("asm/nonmatchings/cacode/67370", func_8011268C_cacode);

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
