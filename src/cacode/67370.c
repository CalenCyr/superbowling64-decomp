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

Makes sure frame frame of clip clip is in the frame cache. A miss reads the clip's BMP file into D_8013E880_cacode
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
        func_80029728( name, D_8013E880_cacode, 54 + 320 * 240 * 2 );    /* the headers and the picture */
    } else {
        func_80029AF4( 0xA58, D_8013E880_cacode );
    }

    D_80115630_cacode[slot].frame = frame;
    D_80115630_cacode[slot].clip = clip;
    D_80115630_cacode[slot].stamp = -2;

    /* the file header's offset of the pixel data (bfOffBits, little-endian) */
    pixels = D_8013E880_cacode + ( D_8013E880_cacode[10] + ( D_8013E880_cacode[11] << 8 ) + ( D_8013E880_cacode[12] << 16 ) + ( D_8013E880_cacode[13] << 24 ) );
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
        guMtxIdentF( D_8012FE10_cacode[0][i] );
        guMtxIdentF( D_8012FE10_cacode[1][i] );
        memcpy( D_80139410_cacode[0][i], D_801157E4_cacode[1]->cam.at, sizeof( D_80139410_cacode[0][i] ) );
        memcpy( D_8013B030_cacode[0][i], D_801157E4_cacode[1]->cam.eye, sizeof( D_8013B030_cacode[0][i] ) );
        memcpy( D_8013CC50_cacode[0][i], D_801157E4_cacode[1]->cam.unk_18, sizeof( D_8013CC50_cacode[0][i] ) );
        memcpy( D_80139410_cacode[1][i], D_801157E4_cacode[2]->cam.at, sizeof( D_80139410_cacode[1][i] ) );
        memcpy( D_8013B030_cacode[1][i], D_801157E4_cacode[2]->cam.eye, sizeof( D_8013B030_cacode[1][i] ) );
        memcpy( D_8013CC50_cacode[1][i], D_801157E4_cacode[2]->cam.unk_18, sizeof( D_8013CC50_cacode[1][i] ) );
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
    D_8013E880_cacode = func_80029E28( 54 + 320 * 240 * 2 );
}

/*
================
func_80112CDC_cacode

Splits the matrix src into its translation trans, its rotation angles rot (radians, wrapped into 0 to 2 pi) and
one uniform scale, written to all three components of scale.
================
*/
void func_80112CDC_cacode( f32 src[4][4], f32 *trans, f32 *rot, f32 *scale ) {
    f32 mtx[4][4];
    f32 xx, yy, zz;
    f32 sinX;

    memcpy( mtx, src, sizeof( mtx ) );
    trans[0] = mtx[3][0];
    trans[1] = mtx[3][1];
    trans[2] = mtx[3][2];

    rot[1] = atan( mtx[2][0] / mtx[2][2] );
    if ( rot[1] < 0.0f ) {
        rot[1] += 2 * M_PI;
    }
    rot[2] = atan( mtx[0][1] / mtx[1][1] );
    if ( rot[2] < 0.0f ) {
        rot[2] += 2 * M_PI;
    }
    if ( mtx[2][0] > 0.0f ) {
        if ( mtx[2][2] < 0.0f ) {
            rot[1] -= M_PI;
        }
    } else if ( mtx[2][2] < 0.0f ) {
        rot[1] += M_PI;
    }
    if ( mtx[0][1] > 0.0f ) {
        if ( mtx[1][1] < 0.0f ) {
            rot[2] -= M_PI;
        }
    } else if ( mtx[1][1] < 0.0f ) {
        rot[2] += M_PI;
    }

    sinX = -mtx[0][0] / mtx[2][1] - sinf( rot[1] ) * sinf( rot[2] );
    sinX = cosf( rot[1] ) * cosf( rot[2] ) / sinX;
    rot[0] = asin( sinX );
    if ( cosf( rot[0] ) < 0.0f ) {
        rot[0] = M_PI - rot[0];
    }
    if ( rot[0] < 0.0f ) {
        rot[0] += 2 * M_PI;
    }

    /* the scale comes from the largest component of the third row, the one least hurt by dividing */
    xx = mtx[2][0] * mtx[2][0];
    yy = mtx[2][1] * mtx[2][1];
    zz = mtx[2][2] * mtx[2][2];
    if ( yy < xx ) {
        if ( zz <= xx ) {
            scale[0] = scale[1] = scale[2] = mtx[2][0] / ( cosf( rot[0] ) * sinf( rot[1] ) );
            return;
        }
    } else if ( zz < yy ) {
        scale[0] = scale[1] = scale[2] = mtx[2][1] / -sinf( rot[0] );
        return;
    }
    scale[0] = scale[1] = scale[2] = mtx[2][2] / ( cosf( rot[0] ) * cosf( rot[1] ) );
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

/* the size of motion.dat: the motion tables, then D_80115374_cacode[1] and [2] */
#define MOTION_FILE_SIZE    ( sizeof( D_80115830_cacode ) + sizeof( D_8012FE10_cacode ) + sizeof( D_80139410_cacode ) \
                              + sizeof( D_8013B030_cacode ) + sizeof( D_8013CC50_cacode ) + 2 * sizeof( D_80115374_cacode[0] ) )
#define STICK_DEAD          8           /* stick values within this of 0 are ignored */
#define RAD_TO_DEG          57.2957795130823229

/*
================
func_80113280_cacode

Per-frame update and drawing of the overlay, a motion editor for the scene. L cycles the mode; Start selects the
next camera and object (camera 2 only once clip 2 has a first frame); left and right on the D-pad step the frame
(on their own once held), up copies the next or previous frame (with Z: the next) into this one, and down caches
the selected camera's BMP frame. In CAMODE_ROTATE, A and B select the model part, the stick and R move the camera
or turn the object, Z with A or B scales it, and the C buttons turn the selected part, whose rotation is stored
for the frame; Z with Start swaps that rotation with the one kept in D_8013E870_cacode. CAMODE_FILE loads and saves the
tables as motion.dat; CAMODE_IMAGE moves the clips' first BMP frames. Cameras 1 and 2 draw their clip's frame behind
the scene (Z with the down and up C buttons sets its alpha), and the selected part's axes and the mode's text are
drawn over it.
================
*/
void func_80113280_cacode( void ) {
    u8 str[80];
    f32 inv[4][4];
    f32 m[4][4];
    f32 rotM[4][4];
    f32 move[3];
    f32 rot[3];
    f32 scale[3];
    f32 swap[3];
    sceneObject_t *obj;
    sceneCamera_t *cam;
    fileType1_t *model;
    u8 *buf;
    s32 size;
    s32 i, j;

    obj = &D_801157D8_cacode[D_80115368_cacode]->obj;
    model = obj->model;
    D_801157E4_cacode[D_80115368_cacode]->cam.frame = -1;
    for ( i = 0; i < D_801157D4_cacode; i++ ) {
        if ( D_80115630_cacode[i].stamp ) {
            D_80115630_cacode[i].stamp--;
        }
    }

    if ( ( D_800E4D62 & L_TRIG ) && ( D_800994BC & Z_TRIG ) ) {
        if ( ++D_80115364_cacode == CAMODE_COUNT ) {
            D_80115364_cacode = CAMODE_ROTATE;
        }
    }
    if ( D_800994BC == 0 ) {
        D_80115360_cacode = 0;
    }

    if ( D_80115364_cacode == CAMODE_FILE ) {
        size = MOTION_FILE_SIZE;
        if ( D_800E4D62 & A_BUTTON ) {
            buf = func_80029E28( MOTION_FILE_SIZE );
            func_80029728( "motion.dat", buf, MOTION_FILE_SIZE );
            size = 0;
            memcpy( D_80115830_cacode, buf + size, sizeof( D_80115830_cacode ) );
            size += sizeof( D_80115830_cacode );
            memcpy( D_8012FE10_cacode, buf + size, sizeof( D_8012FE10_cacode ) );
            size += sizeof( D_8012FE10_cacode );
            memcpy( D_80139410_cacode, buf + size, sizeof( D_80139410_cacode ) );
            size += sizeof( D_80139410_cacode );
            memcpy( D_8013B030_cacode, buf + size, sizeof( D_8013B030_cacode ) );
            size += sizeof( D_8013B030_cacode );
            memcpy( D_8013CC50_cacode, buf + size, sizeof( D_8013CC50_cacode ) );
            size += sizeof( D_8013CC50_cacode );
            memcpy( &D_80115374_cacode[1], buf + size, 2 * sizeof( D_80115374_cacode[0] ) );
            size += 2 * sizeof( D_80115374_cacode[0] );
            func_80029F18( buf );
        }
        if ( D_800E4D62 & B_BUTTON ) {
            buf = func_80029E28( size );
            size = 0;
            memcpy( buf + size, D_80115830_cacode, sizeof( D_80115830_cacode ) );
            size += sizeof( D_80115830_cacode );
            memcpy( buf + size, D_8012FE10_cacode, sizeof( D_8012FE10_cacode ) );
            size += sizeof( D_8012FE10_cacode );
            memcpy( buf + size, D_80139410_cacode, sizeof( D_80139410_cacode ) );
            size += sizeof( D_80139410_cacode );
            memcpy( buf + size, D_8013B030_cacode, sizeof( D_8013B030_cacode ) );
            size += sizeof( D_8013B030_cacode );
            memcpy( buf + size, D_8013CC50_cacode, sizeof( D_8013CC50_cacode ) );
            size += sizeof( D_8013CC50_cacode );
            memcpy( buf + size, &D_80115374_cacode[1], 2 * sizeof( D_80115374_cacode[0] ) );
            size += 2 * sizeof( D_80115374_cacode[0] );
            func_80029814( "motion.dat", buf, size );
            func_80029F18( buf );
        }
    }

    if ( ( D_800E4D62 & START_BUTTON ) && !( D_800994BC & Z_TRIG ) ) {
        if ( ++D_80115368_cacode == 3 ) {
            D_80115368_cacode = 0;
        }
        if ( D_80115368_cacode == 2 && D_80115374_cacode[2] < 0 ) {
            D_80115368_cacode = 0;
        }
        obj = &D_801157D8_cacode[D_80115368_cacode]->obj;
        /* the selected camera fills the screen; the others are drawn small */
        switch ( D_80115368_cacode ) {
        case 0:
            cam = &D_801157E4_cacode[0]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->unk_2C = 0;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[1]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 200;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[2]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 200;
            cam->unk_2E = 120;
            break;
        case 1:
            cam = &D_801157E4_cacode[0]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 0;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[1]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->unk_2C = 0;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[2]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 200;
            cam->unk_2E = 120;
            break;
        case 2:
            cam = &D_801157E4_cacode[0]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 0;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[1]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->unk_2C = 200;
            cam->unk_2E = 0;
            cam = &D_801157E4_cacode[2]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->unk_2C = 0;
            cam->unk_2E = 0;
            break;
        }
    }

    if ( D_800994BC & L_JPAD ) {
        if ( ( D_800E4D62 & L_JPAD ) || D_80115360_cacode >= 7 ) {
            --D_80115370_cacode;
            if ( D_80115370_cacode < 0 ) {
                D_80115370_cacode = D_80115374_cacode[0] - 1;
            }
            memcpy( D_8013E870_cacode, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], sizeof( D_8013E870_cacode ) );
        }
        D_80115360_cacode++;
    }
    if ( D_800994BC & R_JPAD ) {
        if ( ( D_800E4D62 & R_JPAD ) || D_80115360_cacode >= 7 ) {
            ++D_80115370_cacode;
            if ( D_80115370_cacode >= D_80115374_cacode[0] ) {
                D_80115370_cacode = 0;
            }
            memcpy( D_8013E870_cacode, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], sizeof( D_8013E870_cacode ) );
        }
        D_80115360_cacode++;
    }

    if ( D_800E4D62 & U_JPAD ) {
        if ( D_800994BC & Z_TRIG ) {
            if ( D_80115370_cacode == D_80115374_cacode[0] - 1 ) {
                i = 0;
            } else {
                i = D_80115370_cacode + 1;
            }
        } else if ( D_80115370_cacode == 0 ) {
            i = D_80115374_cacode[0] - 1;
        } else {
            i = D_80115370_cacode - 1;
        }
        for ( j = 0; j < 2; j++ ) {
            memcpy( D_8012FE10_cacode[j][D_80115370_cacode], D_8012FE10_cacode[j][i], sizeof( D_8012FE10_cacode[j][i] ) );
            memcpy( D_80139410_cacode[j][D_80115370_cacode], D_80139410_cacode[j][i], sizeof( D_80139410_cacode[j][i] ) );
            memcpy( D_8013B030_cacode[j][D_80115370_cacode], D_8013B030_cacode[j][i], sizeof( D_8013B030_cacode[j][i] ) );
            memcpy( D_8013CC50_cacode[j][D_80115370_cacode], D_8013CC50_cacode[j][i], sizeof( D_8013CC50_cacode[j][i] ) );
        }
        for ( j = 0; j < 30; j++ ) {
            memcpy( D_80115830_cacode[j][D_80115370_cacode], D_80115830_cacode[j][i],
                    sizeof( D_80115830_cacode[j][i] ) );
        }
    }

    if ( D_80115368_cacode != 0 && ( D_800E4D62 & D_JPAD ) ) {
        func_8011268C_cacode( D_80115370_cacode, D_80115368_cacode );
    }

    if ( D_80115364_cacode <= CAMODE_ROTATE && !( D_800994BC & Z_TRIG ) ) {
        if ( D_800E4D62 & A_BUTTON ) {
            if ( ++D_8011536C_cacode == model->numParts ) {
                D_8011536C_cacode = 0;
            }
            memcpy( D_8013E870_cacode, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], sizeof( D_8013E870_cacode ) );
        }
        if ( D_800E4D62 & B_BUTTON ) {
            if ( --D_8011536C_cacode == -1 ) {
                D_8011536C_cacode = model->numParts - 1;
            }
            memcpy( D_8013E870_cacode, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], sizeof( D_8013E870_cacode ) );
        }
    }

    /* pose the object for the frame */
    for ( i = 0; i < model->numParts; i++ ) {
        memcpy( model->parts[i]->unk_08, D_80115830_cacode[i][D_80115370_cacode], sizeof( model->parts[i]->unk_08 ) );
    }
    if ( D_80115368_cacode == 0 ) {
        memcpy( obj->mtx, D_801157F0_cacode, sizeof( obj->mtx ) );
    } else {
        memcpy( obj->mtx, D_8012FE10_cacode[D_80115368_cacode - 1][D_80115370_cacode], sizeof( obj->mtx ) );
    }
    for ( i = 1; i < 3; i++ ) {
        memcpy( D_801157D8_cacode[i]->obj.mtx, D_8012FE10_cacode[i - 1][D_80115370_cacode], sizeof( D_801157D8_cacode[i]->obj.mtx ) );
        memcpy( D_801157E4_cacode[i]->cam.at, D_80139410_cacode[i - 1][D_80115370_cacode], sizeof( D_801157E4_cacode[i]->cam.at ) );
        memcpy( D_801157E4_cacode[i]->cam.eye, D_8013B030_cacode[i - 1][D_80115370_cacode], sizeof( D_801157E4_cacode[i]->cam.eye ) );
        memcpy( D_801157E4_cacode[i]->cam.unk_18, D_8013CC50_cacode[i - 1][D_80115370_cacode],
                sizeof( D_801157E4_cacode[i]->cam.unk_18 ) );
    }

    func_80031250();
    func_80035074();
    for ( i = 0; i < 3; i++ ) {
        func_8002C324( &D_801157E4_cacode[i]->cam );
        func_8002B9CC( &D_801157D8_cacode[i]->obj );
    }
    func_80031E40();

    switch ( D_80115364_cacode ) {
    case CAMODE_ROTATE:
        cam = &D_801157E4_cacode[D_80115368_cacode]->cam;
        cam->frame = -1;
        if ( D_800994BC & R_TRIG ) {
            /* R: the stick moves the camera, with Z in depth */
            move[0] = move[1] = move[2] = 0.0f;
            if ( D_800994BC & Z_TRIG ) {
                if ( D_8009D05C > STICK_DEAD || D_8009D05C < -STICK_DEAD ) {
                    move[2] = D_8009D05C / 8.0f;
                }
            } else {
                if ( D_8009D05C > STICK_DEAD || D_8009D05C < -STICK_DEAD ) {
                    move[0] = -D_8009D05C / 8.0f;
                }
                if ( D_800DF628 > STICK_DEAD || D_800DF628 < -STICK_DEAD ) {
                    move[1] = -D_800DF628 / 8.0f;
                }
            }
            if ( move[0] != 0.0f || move[1] != 0.0f || move[2] != 0.0f ) {
                cam->at[0] += move[0];
                cam->at[1] += move[1];
                cam->at[2] += move[2];
                cam->eye[0] += move[0];
                cam->eye[1] += move[1];
                cam->eye[2] += move[2];
                cam->unk_18[0] += move[0];
                cam->unk_18[1] += move[1];
                cam->unk_18[2] += move[2];
            }
        }
        if ( D_800994BC & Z_TRIG ) {
            /* Z with A or B scales the object */
            scale[0] = 1.0f;
            if ( D_800994BC & A_BUTTON ) {
                scale[0] = 1.01f;
            }
            if ( D_800994BC & B_BUTTON ) {
                scale[0] = 0.99f;
            }
            if ( scale[0] != 1.0 ) {
                guScaleF( m, scale[0], scale[0], scale[0] );
                guMtxCatF( obj->mtx, m, obj->mtx );
            }
        }
        if ( !( D_800994BC & R_TRIG ) ) {
            /* the stick turns the object, with Z about its roll */
            rot[0] = rot[1] = rot[2] = 0.0f;
            if ( D_800994BC & Z_TRIG ) {
                if ( D_8009D05C > STICK_DEAD || D_8009D05C < -STICK_DEAD ) {
                    rot[2] = -D_8009D05C / 8.0f;
                }
            } else {
                if ( D_8009D05C > STICK_DEAD || D_8009D05C < -STICK_DEAD ) {
                    rot[1] = D_8009D05C / 8.0f;
                }
                if ( D_800DF628 > STICK_DEAD || D_800DF628 < -STICK_DEAD ) {
                    rot[0] = -D_800DF628 / 8.0f;
                }
            }
            if ( rot[0] != 0.0f || rot[1] != 0.0f || rot[2] != 0.0f ) {
                guRotateRPYF( m, rot[0], rot[1], rot[2] );
                guMtxCatF( obj->mtx, m, obj->mtx );
            }
        }
        /* the C buttons turn the selected part a degree a frame, with Z about its roll */
        rot[0] = rot[1] = rot[2] = 0.0f;
        if ( D_800994BC & Z_TRIG ) {
            if ( D_800994BC & R_CBUTTONS ) {
                rot[2] = -1.0f;
            }
            if ( D_800994BC & L_CBUTTONS ) {
                rot[2] = 1.0f;
            }
        } else {
            if ( D_800994BC & R_CBUTTONS ) {
                rot[1] = 1.0f;
            }
            if ( D_800994BC & L_CBUTTONS ) {
                rot[1] = -1.0f;
            }
            if ( D_800994BC & U_CBUTTONS ) {
                rot[0] = -1.0f;
            }
            if ( D_800994BC & D_CBUTTONS ) {
                rot[0] = 1.0f;
            }
        }
        if ( rot[0] != 0.0f || rot[1] != 0.0f || rot[2] != 0.0f ) {
            guRotateRPYF( rotM, rot[0], rot[1], rot[2] );
            guMtxCatF( obj->joints[D_8011536C_cacode].mtx, rotM, m );
            if ( D_8011536C_cacode > 0 ) {
                /* back to the parent's frame, and store the part's new rotation */
                func_8011306C_cacode( obj->joints[model->parts[D_8011536C_cacode]->unk_04].mtx, inv );
                guMtxCatF( m, inv, m );
                func_80112CDC_cacode( m, move, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], scale );
            }
        }
        if ( ( D_800E4D62 & START_BUTTON ) && ( D_800994BC & Z_TRIG ) ) {
            memcpy( swap, D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], sizeof( swap ) );
            memcpy( D_80115830_cacode[D_8011536C_cacode][D_80115370_cacode], D_8013E870_cacode, sizeof( D_8013E870_cacode ) );
            memcpy( D_8013E870_cacode, swap, sizeof( D_8013E870_cacode ) );
        }
        break;
    case CAMODE_IMAGE:
        if ( D_800994BC & Z_TRIG ) {
            if ( D_800994BC & A_BUTTON ) {
                D_80115374_cacode[2]++;
            }
            if ( D_800994BC & B_BUTTON ) {
                D_80115374_cacode[2]--;
            }
        } else {
            if ( D_800994BC & A_BUTTON ) {
                D_80115374_cacode[1]++;
            }
            if ( D_800994BC & B_BUTTON ) {
                D_80115374_cacode[1]--;
            }
        }
        break;
    }

    /* keep what was edited */
    if ( D_80115368_cacode == 0 ) {
        memcpy( D_801157F0_cacode, obj->mtx, sizeof( D_801157F0_cacode ) );
    } else {
        memcpy( D_8012FE10_cacode[D_80115368_cacode - 1][D_80115370_cacode], obj->mtx, sizeof( obj->mtx ) );
        memcpy( D_80139410_cacode[D_80115368_cacode - 1][D_80115370_cacode], D_801157E4_cacode[D_80115368_cacode]->cam.at,
                sizeof( D_80139410_cacode[0][0] ) );
        memcpy( D_8013B030_cacode[D_80115368_cacode - 1][D_80115370_cacode], D_801157E4_cacode[D_80115368_cacode]->cam.eye,
                sizeof( D_8013B030_cacode[0][0] ) );
        memcpy( D_8013CC50_cacode[D_80115368_cacode - 1][D_80115370_cacode], D_801157E4_cacode[D_80115368_cacode]->cam.unk_18,
                sizeof( D_8013CC50_cacode[0][0] ) );
    }

    if ( D_80115368_cacode != 0 ) {
        if ( D_800994BC & Z_TRIG ) {
            if ( D_800994BC & D_CBUTTONS ) {
                D_80115540_cacode += 8;
                if ( D_80115540_cacode > 247 ) {
                    D_80115540_cacode = 247;
                }
            }
            if ( D_800994BC & U_CBUTTONS ) {
                D_80115540_cacode -= 8;
                if ( D_80115540_cacode < 10 ) {
                    D_80115540_cacode = 10;
                }
            }
        }
        func_80112350_cacode( D_80115370_cacode, D_80115368_cacode, D_80115540_cacode );
    }

    gSPLoadUcode( D_800F8094++, D_592F0, D_5FC20 );
    func_800316C0( &D_801157E4_cacode[D_80115368_cacode]->cam );

    /* the selected part's axes */
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2 );
    gDPSetAlphaCompare( D_800F8094++, G_AC_NONE );
    gDPSetCombineLERP( D_800F8094++, 1, 0, SHADE, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 1, 0, SHADE, ENVIRONMENT, 0, 0, 0,
                       ENVIRONMENT );
    gDPSetPrimColor( D_800F8094++, 0, 0, 128, 128, 128, 128 );
    gDPSetEnvColor( D_800F8094++, 0, 0, 0, 128 );
    gSPMatrix( D_800F8094++, &obj->joints[D_8011536C_cacode].mtxL[D_8005C0B1],
               G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPVertex( D_800F8094++, D_80115380_cacode, 4, 0 );
    gSPLine3D( D_800F8094++, 0, 1, 0 );
    gSPLine3D( D_800F8094++, 0, 2, 0 );
    gSPLine3D( D_800F8094++, 0, 3, 0 );
    gDPPipeSync( D_800F8094++ );
    func_80031BB0();

    func_80028A0C( &D_800F8094 );
    func_80028950( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    switch ( D_80115364_cacode ) {
    case CAMODE_ROTATE:
        func_80028DA4( &D_800F8094, 20, 20, "ROTATE" );
        sprintf( str, "H=%3.1f P=%3.1f B=%3.1f", model->parts[D_8011536C_cacode]->unk_08[0] * RAD_TO_DEG,
                 model->parts[D_8011536C_cacode]->unk_08[1] * RAD_TO_DEG,
                 model->parts[D_8011536C_cacode]->unk_08[2] * RAD_TO_DEG );
        func_80028DA4( &D_800F8094, 64, 215, str );
        break;
    case CAMODE_FILE:
        func_80028DA4( &D_800F8094, 20, 20, "FILE" );
        break;
    case CAMODE_IMAGE:
        func_80028DA4( &D_800F8094, 20, 20, "IMAGE" );
        sprintf( str, "IMG1=%d IMG2=%d", D_80115374_cacode[1], D_80115374_cacode[2] );
        func_80028DA4( &D_800F8094, 84, 30, str );
        break;
    }
    sprintf( str, "CAM=%d OBJ=%d FRAME=%d", D_80115368_cacode, D_8011536C_cacode, D_80115370_cacode );
    func_80028DA4( &D_800F8094, 84, 20, str );
}
