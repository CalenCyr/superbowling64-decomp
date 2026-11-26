/* 67370.c -- cacode overlay */

#include "common.h"
#include "libultra.h"
#include "cacode/cacode.h"

/*
================
CA_DrawFrame

Draws frame frame of clip clip from the frame cache as 40 textured rectangles filling the 320 x 240 screen, with
alpha alpha; nothing when the frame is not cached.
================
*/
void CA_DrawFrame( s32 frame, s32 clip, u8 alpha ) {
    s32 slot;
    s32 row, col;

    for ( slot = 0; slot < ca_numSlots; slot++ ) {
        if ( ca_frameCache[slot].frame == frame && ca_frameCache[slot].clip == clip ) {
            break;
        }
    }
    if ( slot == ca_numSlots ) {
        return;
    }

    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    gDPSetRenderMode( game_gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    gDPSetTexturePersp( game_gfx++, G_TP_NONE );
    gDPSetCombineLERP( game_gfx++, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE,
                       0 );
    gDPSetPrimColor( game_gfx++, 0, 0, 255, 255, 255, alpha );
    for ( row = 0; row < 8; row++ ) {
        for ( col = 0; col < 5; col++ ) {
            gDPLoadTextureBlock( game_gfx++, ca_frameCache[slot].tiles[col + row * 5], G_IM_FMT_RGBA,
                                 G_IM_SIZ_16b, 64, 32, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 6, 5,
                                 G_TX_NOLOD, G_TX_NOLOD );
            gSPTextureRectangle( game_gfx++, ( col * 64 ) << 2, ( row * 32 ) << 2, ( col * 64 + 64 ) << 2,
                                 ( row * 32 + 32 ) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );
        }
    }
    gDPPipeSync( game_gfx++ );
}

/*
================
CA_CacheFrame

Makes sure frame frame of clip clip is in the frame cache. A miss reads the clip's BMP file into ca_bmpBuffer
(from the development host, or else a fixed file of the str segment) over the least recently used slot, and
cuts its bottom-up 320 x 240 picture into the slot's tiles.
================
*/
void CA_CacheFrame( s32 frame, s32 clip ) {
    u8 name[24];
    u8 *pixels;
    u16 *tile;
    u32 oldest;
    s32 slot;
    s32 i;
    s32 row, col, y, x;

    for ( slot = 0; slot < ca_numSlots; slot++ ) {
        if ( ca_frameCache[slot].frame == frame && ca_frameCache[slot].clip == clip ) {
            break;
        }
    }
    if ( slot != ca_numSlots ) {
        return;
    }

    oldest = -1;
    for ( i = 0; i < ca_numSlots; i++ ) {
        if ( ca_frameCache[i].stamp < oldest ) {
            oldest = ca_frameCache[i].stamp;
            slot = i;
        }
    }

    if ( cache_hostMode ) {
        sprintf( name, "%1.1d - Frame %4.4d.bmp", clip, frame + ca_clipFrameBase[clip] );
        Host_LoadFile( name, ca_bmpBuffer, 54 + 320 * 240 * 2 );    /* the headers and the picture */
    } else {
        Str_ReadFile( 0xA58, ca_bmpBuffer );
    }

    ca_frameCache[slot].frame = frame;
    ca_frameCache[slot].clip = clip;
    ca_frameCache[slot].stamp = -2;

    /* the file header's offset of the pixel data (bfOffBits, little-endian) */
    pixels = ca_bmpBuffer + ( ca_bmpBuffer[10] + ( ca_bmpBuffer[11] << 8 ) + ( ca_bmpBuffer[12] << 16 ) + ( ca_bmpBuffer[13] << 24 ) );
    /*
    BMP rows run bottom-up, 640 bytes of little-endian 5-5-5 texels each; the texel is shifted into RGBA16 with alpha
    set. Eight rows of 32-line tiles are 256 lines, so the last 16 lines of the bottom row of tiles read the bytes
    before the pixel data, an original quirk.
    */
    for ( row = 0; row < 8; row++ ) {
        for ( col = 0; col < 5; col++ ) {
            tile = ca_frameCache[slot].tiles[col + row * 5];
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
CA_Init

Entry of the overlay: Start switches to files from the development host; from the host, the scene's records come
from its "sce.dat". Creates the scene's three cameras and three objects, fills the per-frame tables from cameras 1
and 2 and the parts of object ca_selected, and allocates the frame cache (34 slots when D_800A2FE4 is set,
else 9) and the BMP buffer.
================
*/
void CA_Init( void ) {
    fileType1_t *model;
    s32 i;
    u32 j;

    if ( cont_allHeld & START_BUTTON ) {
        cache_hostMode = 1;
        Host_Sync();
    }
    if ( cache_hostMode ) {
        Host_LoadFile( "sce.dat", (u8 *)ca_sceneParams, sizeof( ca_sceneParams ) );
        cache_hostFiles = 1;
        for ( i = 0; i < 3; i++ ) {
            ca_cameras[i] = (sceneCameraNode_t *)Cam_Create( &ca_sceneParams[i] );
            ca_objects[i] = (sceneObjectNode_t *)Obj_CreateSceneObject( &ca_sceneParams[i + 3] );
        }
        cache_hostFiles = 0;
    } else {
        for ( i = 0; i < 3; i++ ) {
            ca_cameras[i] = (sceneCameraNode_t *)Cam_Create( &ca_sceneParams[i] );
            ca_objects[i] = (sceneObjectNode_t *)Obj_CreateSceneObject( &ca_sceneParams[i + 3] );
        }
    }
    model = ca_objects[ca_selected]->obj.model;
    ca_clipFrameBase[0] = ca_objects[0]->obj.anim->numFrames;
    for ( i = 0; i < 300; i++ ) {
        guMtxIdentF( ca_objectMatrices[0][i] );
        guMtxIdentF( ca_objectMatrices[1][i] );
        memcpy( ca_cameraAt[0][i], ca_cameras[1]->cam.at, sizeof( ca_cameraAt[0][i] ) );
        memcpy( ca_cameraEye[0][i], ca_cameras[1]->cam.eye, sizeof( ca_cameraEye[0][i] ) );
        memcpy( ca_cameraUp[0][i], ca_cameras[1]->cam.upPoint, sizeof( ca_cameraUp[0][i] ) );
        memcpy( ca_cameraAt[1][i], ca_cameras[2]->cam.at, sizeof( ca_cameraAt[1][i] ) );
        memcpy( ca_cameraEye[1][i], ca_cameras[2]->cam.eye, sizeof( ca_cameraEye[1][i] ) );
        memcpy( ca_cameraUp[1][i], ca_cameras[2]->cam.upPoint, sizeof( ca_cameraUp[1][i] ) );
        for ( j = 0; j < model->numParts; j++ ) {
            memcpy( ca_partRotations[j][i], model->parts[j]->rot, sizeof( ca_partRotations[j][i] ) );
        }
    }
    guMtxIdentF( ca_camera0Matrix );
    if ( D_800A2FE4 ) {
        ca_numSlots = 34;
    } else {
        ca_numSlots = 9;
    }
    for ( i = 0; i < ca_numSlots; i++ ) {
        ca_frameCache[i].frame = -1;
        ca_frameCache[i].stamp = 0;
        ca_frameCache[i].tiles = Mem_Alloc( 40 * sizeof( *ca_frameCache[i].tiles ) );
    }
    ca_bmpBuffer = Mem_Alloc( 54 + 320 * 240 * 2 );
}

/*
================
CA_DecomposeMatrix

Splits the matrix src into its translation trans, its rotation angles rot (radians, wrapped into 0 to 2 pi) and
one uniform scale, written to all three components of scale.
================
*/
void CA_DecomposeMatrix( f32 src[4][4], f32 *trans, f32 *rot, f32 *scale ) {
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
CA_InvertMatrix

Inverts the 4 x 4 matrix src into dst by Gauss-Jordan elimination on [src | I], without pivoting.
================
*/
void CA_InvertMatrix( float src[4][4], float dst[4][4] ) {
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

/* the size of motion.dat: the motion tables, then ca_clipFrameBase[1] and [2] */
#define MOTION_FILE_SIZE    ( sizeof( ca_partRotations ) + sizeof( ca_objectMatrices ) + sizeof( ca_cameraAt ) \
                              + sizeof( ca_cameraEye ) + sizeof( ca_cameraUp ) + 2 * sizeof( ca_clipFrameBase[0] ) )
#define STICK_DEAD          8           /* stick values within this of 0 are ignored */
#define RAD_TO_DEG          57.2957795130823229

/*
================
CA_Frame

Per-frame update and drawing of the overlay, a motion editor for the scene. L cycles the mode; Start selects the
next camera and object (camera 2 only once clip 2 has a first frame); left and right on the D-pad step the frame
(on their own once held), up copies the next or previous frame (with Z: the next) into this one, and down caches
the selected camera's BMP frame. In CAMODE_ROTATE, A and B select the model part, the stick and R move the camera
or turn the object, Z with A or B scales it, and the C buttons turn the selected part, whose rotation is stored
for the frame; Z with Start swaps that rotation with the one kept in ca_keptRotation. CAMODE_FILE loads and saves the
tables as motion.dat; CAMODE_IMAGE moves the clips' first BMP frames. Cameras 1 and 2 draw their clip's frame behind
the scene (Z with the down and up C buttons sets its alpha), and the selected part's axes and the mode's text are
drawn over it.
================
*/
void CA_Frame( void ) {
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

    obj = &ca_objects[ca_selected]->obj;
    model = obj->model;
    ca_cameras[ca_selected]->cam.frame = -1;
    for ( i = 0; i < ca_numSlots; i++ ) {
        if ( ca_frameCache[i].stamp ) {
            ca_frameCache[i].stamp--;
        }
    }

    if ( ( cont_allPressed & L_TRIG ) && ( cont_allHeld & Z_TRIG ) ) {
        if ( ++ca_mode == CAMODE_COUNT ) {
            ca_mode = CAMODE_ROTATE;
        }
    }
    if ( cont_allHeld == 0 ) {
        ca_dpadHeld = 0;
    }

    if ( ca_mode == CAMODE_FILE ) {
        size = MOTION_FILE_SIZE;
        if ( cont_allPressed & A_BUTTON ) {
            buf = Mem_Alloc( MOTION_FILE_SIZE );
            Host_LoadFile( "motion.dat", buf, MOTION_FILE_SIZE );
            size = 0;
            memcpy( ca_partRotations, buf + size, sizeof( ca_partRotations ) );
            size += sizeof( ca_partRotations );
            memcpy( ca_objectMatrices, buf + size, sizeof( ca_objectMatrices ) );
            size += sizeof( ca_objectMatrices );
            memcpy( ca_cameraAt, buf + size, sizeof( ca_cameraAt ) );
            size += sizeof( ca_cameraAt );
            memcpy( ca_cameraEye, buf + size, sizeof( ca_cameraEye ) );
            size += sizeof( ca_cameraEye );
            memcpy( ca_cameraUp, buf + size, sizeof( ca_cameraUp ) );
            size += sizeof( ca_cameraUp );
            memcpy( &ca_clipFrameBase[1], buf + size, 2 * sizeof( ca_clipFrameBase[0] ) );
            size += 2 * sizeof( ca_clipFrameBase[0] );
            Mem_Free( buf );
        }
        if ( cont_allPressed & B_BUTTON ) {
            buf = Mem_Alloc( size );
            size = 0;
            memcpy( buf + size, ca_partRotations, sizeof( ca_partRotations ) );
            size += sizeof( ca_partRotations );
            memcpy( buf + size, ca_objectMatrices, sizeof( ca_objectMatrices ) );
            size += sizeof( ca_objectMatrices );
            memcpy( buf + size, ca_cameraAt, sizeof( ca_cameraAt ) );
            size += sizeof( ca_cameraAt );
            memcpy( buf + size, ca_cameraEye, sizeof( ca_cameraEye ) );
            size += sizeof( ca_cameraEye );
            memcpy( buf + size, ca_cameraUp, sizeof( ca_cameraUp ) );
            size += sizeof( ca_cameraUp );
            memcpy( buf + size, &ca_clipFrameBase[1], 2 * sizeof( ca_clipFrameBase[0] ) );
            size += 2 * sizeof( ca_clipFrameBase[0] );
            Host_WriteFile( "motion.dat", buf, size );
            Mem_Free( buf );
        }
    }

    if ( ( cont_allPressed & START_BUTTON ) && !( cont_allHeld & Z_TRIG ) ) {
        if ( ++ca_selected == 3 ) {
            ca_selected = 0;
        }
        if ( ca_selected == 2 && ca_clipFrameBase[2] < 0 ) {
            ca_selected = 0;
        }
        obj = &ca_objects[ca_selected]->obj;
        /* the selected camera fills the screen; the others are drawn small */
        switch ( ca_selected ) {
        case 0:
            cam = &ca_cameras[0]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->left = 0;
            cam->top = 0;
            cam = &ca_cameras[1]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 200;
            cam->top = 0;
            cam = &ca_cameras[2]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 200;
            cam->top = 120;
            break;
        case 1:
            cam = &ca_cameras[0]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 0;
            cam->top = 0;
            cam = &ca_cameras[1]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->left = 0;
            cam->top = 0;
            cam = &ca_cameras[2]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 200;
            cam->top = 120;
            break;
        case 2:
            cam = &ca_cameras[0]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 0;
            cam->top = 0;
            cam = &ca_cameras[1]->cam;
            cam->width = 120;
            cam->height = 120;
            cam->left = 200;
            cam->top = 0;
            cam = &ca_cameras[2]->cam;
            cam->width = 320;
            cam->height = 240;
            cam->left = 0;
            cam->top = 0;
            break;
        }
    }

    if ( cont_allHeld & L_JPAD ) {
        if ( ( cont_allPressed & L_JPAD ) || ca_dpadHeld >= 7 ) {
            --ca_frame;
            if ( ca_frame < 0 ) {
                ca_frame = ca_clipFrameBase[0] - 1;
            }
            memcpy( ca_keptRotation, ca_partRotations[ca_part][ca_frame], sizeof( ca_keptRotation ) );
        }
        ca_dpadHeld++;
    }
    if ( cont_allHeld & R_JPAD ) {
        if ( ( cont_allPressed & R_JPAD ) || ca_dpadHeld >= 7 ) {
            ++ca_frame;
            if ( ca_frame >= ca_clipFrameBase[0] ) {
                ca_frame = 0;
            }
            memcpy( ca_keptRotation, ca_partRotations[ca_part][ca_frame], sizeof( ca_keptRotation ) );
        }
        ca_dpadHeld++;
    }

    if ( cont_allPressed & U_JPAD ) {
        if ( cont_allHeld & Z_TRIG ) {
            if ( ca_frame == ca_clipFrameBase[0] - 1 ) {
                i = 0;
            } else {
                i = ca_frame + 1;
            }
        } else if ( ca_frame == 0 ) {
            i = ca_clipFrameBase[0] - 1;
        } else {
            i = ca_frame - 1;
        }
        for ( j = 0; j < 2; j++ ) {
            memcpy( ca_objectMatrices[j][ca_frame], ca_objectMatrices[j][i], sizeof( ca_objectMatrices[j][i] ) );
            memcpy( ca_cameraAt[j][ca_frame], ca_cameraAt[j][i], sizeof( ca_cameraAt[j][i] ) );
            memcpy( ca_cameraEye[j][ca_frame], ca_cameraEye[j][i], sizeof( ca_cameraEye[j][i] ) );
            memcpy( ca_cameraUp[j][ca_frame], ca_cameraUp[j][i], sizeof( ca_cameraUp[j][i] ) );
        }
        for ( j = 0; j < 30; j++ ) {
            memcpy( ca_partRotations[j][ca_frame], ca_partRotations[j][i],
                    sizeof( ca_partRotations[j][i] ) );
        }
    }

    if ( ca_selected != 0 && ( cont_allPressed & D_JPAD ) ) {
        CA_CacheFrame( ca_frame, ca_selected );
    }

    if ( ca_mode <= CAMODE_ROTATE && !( cont_allHeld & Z_TRIG ) ) {
        if ( cont_allPressed & A_BUTTON ) {
            if ( ++ca_part == model->numParts ) {
                ca_part = 0;
            }
            memcpy( ca_keptRotation, ca_partRotations[ca_part][ca_frame], sizeof( ca_keptRotation ) );
        }
        if ( cont_allPressed & B_BUTTON ) {
            if ( --ca_part == -1 ) {
                ca_part = model->numParts - 1;
            }
            memcpy( ca_keptRotation, ca_partRotations[ca_part][ca_frame], sizeof( ca_keptRotation ) );
        }
    }

    /* pose the object for the frame */
    for ( i = 0; i < model->numParts; i++ ) {
        memcpy( model->parts[i]->rot, ca_partRotations[i][ca_frame], sizeof( model->parts[i]->rot ) );
    }
    if ( ca_selected == 0 ) {
        memcpy( obj->mtx, ca_camera0Matrix, sizeof( obj->mtx ) );
    } else {
        memcpy( obj->mtx, ca_objectMatrices[ca_selected - 1][ca_frame], sizeof( obj->mtx ) );
    }
    for ( i = 1; i < 3; i++ ) {
        memcpy( ca_objects[i]->obj.mtx, ca_objectMatrices[i - 1][ca_frame], sizeof( ca_objects[i]->obj.mtx ) );
        memcpy( ca_cameras[i]->cam.at, ca_cameraAt[i - 1][ca_frame], sizeof( ca_cameras[i]->cam.at ) );
        memcpy( ca_cameras[i]->cam.eye, ca_cameraEye[i - 1][ca_frame], sizeof( ca_cameras[i]->cam.eye ) );
        memcpy( ca_cameras[i]->cam.upPoint, ca_cameraUp[i - 1][ca_frame],
                sizeof( ca_cameras[i]->cam.upPoint ) );
    }

    Scene_ClearDrawList();
    Scene_InitPuffs();
    for ( i = 0; i < 3; i++ ) {
        Cam_Update( &ca_cameras[i]->cam );
        Obj_AddToDrawList( &ca_objects[i]->obj );
    }
    Scene_Draw();

    switch ( ca_mode ) {
    case CAMODE_ROTATE:
        cam = &ca_cameras[ca_selected]->cam;
        cam->frame = -1;
        if ( cont_allHeld & R_TRIG ) {
            /* R: the stick moves the camera, with Z in depth */
            move[0] = move[1] = move[2] = 0.0f;
            if ( cont_allHeld & Z_TRIG ) {
                if ( cont_allStickX > STICK_DEAD || cont_allStickX < -STICK_DEAD ) {
                    move[2] = cont_allStickX / 8.0f;
                }
            } else {
                if ( cont_allStickX > STICK_DEAD || cont_allStickX < -STICK_DEAD ) {
                    move[0] = -cont_allStickX / 8.0f;
                }
                if ( cont_allStickY > STICK_DEAD || cont_allStickY < -STICK_DEAD ) {
                    move[1] = -cont_allStickY / 8.0f;
                }
            }
            if ( move[0] != 0.0f || move[1] != 0.0f || move[2] != 0.0f ) {
                cam->at[0] += move[0];
                cam->at[1] += move[1];
                cam->at[2] += move[2];
                cam->eye[0] += move[0];
                cam->eye[1] += move[1];
                cam->eye[2] += move[2];
                cam->upPoint[0] += move[0];
                cam->upPoint[1] += move[1];
                cam->upPoint[2] += move[2];
            }
        }
        if ( cont_allHeld & Z_TRIG ) {
            /* Z with A or B scales the object */
            scale[0] = 1.0f;
            if ( cont_allHeld & A_BUTTON ) {
                scale[0] = 1.01f;
            }
            if ( cont_allHeld & B_BUTTON ) {
                scale[0] = 0.99f;
            }
            if ( scale[0] != 1.0 ) {
                guScaleF( m, scale[0], scale[0], scale[0] );
                guMtxCatF( obj->mtx, m, obj->mtx );
            }
        }
        if ( !( cont_allHeld & R_TRIG ) ) {
            /* the stick turns the object, with Z about its roll */
            rot[0] = rot[1] = rot[2] = 0.0f;
            if ( cont_allHeld & Z_TRIG ) {
                if ( cont_allStickX > STICK_DEAD || cont_allStickX < -STICK_DEAD ) {
                    rot[2] = -cont_allStickX / 8.0f;
                }
            } else {
                if ( cont_allStickX > STICK_DEAD || cont_allStickX < -STICK_DEAD ) {
                    rot[1] = cont_allStickX / 8.0f;
                }
                if ( cont_allStickY > STICK_DEAD || cont_allStickY < -STICK_DEAD ) {
                    rot[0] = -cont_allStickY / 8.0f;
                }
            }
            if ( rot[0] != 0.0f || rot[1] != 0.0f || rot[2] != 0.0f ) {
                guRotateRPYF( m, rot[0], rot[1], rot[2] );
                guMtxCatF( obj->mtx, m, obj->mtx );
            }
        }
        /* the C buttons turn the selected part a degree a frame, with Z about its roll */
        rot[0] = rot[1] = rot[2] = 0.0f;
        if ( cont_allHeld & Z_TRIG ) {
            if ( cont_allHeld & R_CBUTTONS ) {
                rot[2] = -1.0f;
            }
            if ( cont_allHeld & L_CBUTTONS ) {
                rot[2] = 1.0f;
            }
        } else {
            if ( cont_allHeld & R_CBUTTONS ) {
                rot[1] = 1.0f;
            }
            if ( cont_allHeld & L_CBUTTONS ) {
                rot[1] = -1.0f;
            }
            if ( cont_allHeld & U_CBUTTONS ) {
                rot[0] = -1.0f;
            }
            if ( cont_allHeld & D_CBUTTONS ) {
                rot[0] = 1.0f;
            }
        }
        if ( rot[0] != 0.0f || rot[1] != 0.0f || rot[2] != 0.0f ) {
            guRotateRPYF( rotM, rot[0], rot[1], rot[2] );
            guMtxCatF( obj->joints[ca_part].mtx, rotM, m );
            if ( ca_part > 0 ) {
                /* back to the parent's frame, and store the part's new rotation */
                CA_InvertMatrix( obj->joints[model->parts[ca_part]->parent].mtx, inv );
                guMtxCatF( m, inv, m );
                CA_DecomposeMatrix( m, move, ca_partRotations[ca_part][ca_frame], scale );
            }
        }
        if ( ( cont_allPressed & START_BUTTON ) && ( cont_allHeld & Z_TRIG ) ) {
            memcpy( swap, ca_partRotations[ca_part][ca_frame], sizeof( swap ) );
            memcpy( ca_partRotations[ca_part][ca_frame], ca_keptRotation, sizeof( ca_keptRotation ) );
            memcpy( ca_keptRotation, swap, sizeof( ca_keptRotation ) );
        }
        break;
    case CAMODE_IMAGE:
        if ( cont_allHeld & Z_TRIG ) {
            if ( cont_allHeld & A_BUTTON ) {
                ca_clipFrameBase[2]++;
            }
            if ( cont_allHeld & B_BUTTON ) {
                ca_clipFrameBase[2]--;
            }
        } else {
            if ( cont_allHeld & A_BUTTON ) {
                ca_clipFrameBase[1]++;
            }
            if ( cont_allHeld & B_BUTTON ) {
                ca_clipFrameBase[1]--;
            }
        }
        break;
    }

    /* keep what was edited */
    if ( ca_selected == 0 ) {
        memcpy( ca_camera0Matrix, obj->mtx, sizeof( ca_camera0Matrix ) );
    } else {
        memcpy( ca_objectMatrices[ca_selected - 1][ca_frame], obj->mtx, sizeof( obj->mtx ) );
        memcpy( ca_cameraAt[ca_selected - 1][ca_frame], ca_cameras[ca_selected]->cam.at,
                sizeof( ca_cameraAt[0][0] ) );
        memcpy( ca_cameraEye[ca_selected - 1][ca_frame], ca_cameras[ca_selected]->cam.eye,
                sizeof( ca_cameraEye[0][0] ) );
        memcpy( ca_cameraUp[ca_selected - 1][ca_frame], ca_cameras[ca_selected]->cam.upPoint,
                sizeof( ca_cameraUp[0][0] ) );
    }

    if ( ca_selected != 0 ) {
        if ( cont_allHeld & Z_TRIG ) {
            if ( cont_allHeld & D_CBUTTONS ) {
                ca_clipAlpha += 8;
                if ( ca_clipAlpha > 247 ) {
                    ca_clipAlpha = 247;
                }
            }
            if ( cont_allHeld & U_CBUTTONS ) {
                ca_clipAlpha -= 8;
                if ( ca_clipAlpha < 10 ) {
                    ca_clipAlpha = 10;
                }
            }
        }
        CA_DrawFrame( ca_frame, ca_selected, ca_clipAlpha );
    }

    gSPLoadUcodeL( game_gfx++, gspL3DEX2_fifo );
    Scene_BeginView( &ca_cameras[ca_selected]->cam );

    /* the selected part's axes */
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    gDPSetRenderMode( game_gfx++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2 );
    gDPSetAlphaCompare( game_gfx++, G_AC_NONE );
    gDPSetCombineLERP( game_gfx++, 1, 0, SHADE, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 1, 0, SHADE, ENVIRONMENT, 0, 0, 0,
                       ENVIRONMENT );
    gDPSetPrimColor( game_gfx++, 0, 0, 128, 128, 128, 128 );
    gDPSetEnvColor( game_gfx++, 0, 0, 0, 128 );
    gSPMatrix( game_gfx++, &obj->joints[ca_part].mtxL[sched_bufferIndex],
               G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH );
    gSPVertex( game_gfx++, ca_axes, 4, 0 );
    gSPLine3D( game_gfx++, 0, 1, 0 );
    gSPLine3D( game_gfx++, 0, 2, 0 );
    gSPLine3D( game_gfx++, 0, 3, 0 );
    gDPPipeSync( game_gfx++ );
    Scene_BeginFrame();

    Txt_LoadFont( &game_gfx );
    Txt_SetupFont( &game_gfx, 0 );
    gDPSetCycleType( game_gfx++, G_CYC_1CYCLE );
    switch ( ca_mode ) {
    case CAMODE_ROTATE:
        Txt_DrawString( &game_gfx, 20, 20, "ROTATE" );
        sprintf( str, "H=%3.1f P=%3.1f B=%3.1f", model->parts[ca_part]->rot[0] * RAD_TO_DEG,
                 model->parts[ca_part]->rot[1] * RAD_TO_DEG,
                 model->parts[ca_part]->rot[2] * RAD_TO_DEG );
        Txt_DrawString( &game_gfx, 64, 215, str );
        break;
    case CAMODE_FILE:
        Txt_DrawString( &game_gfx, 20, 20, "FILE" );
        break;
    case CAMODE_IMAGE:
        Txt_DrawString( &game_gfx, 20, 20, "IMAGE" );
        sprintf( str, "IMG1=%d IMG2=%d", ca_clipFrameBase[1], ca_clipFrameBase[2] );
        Txt_DrawString( &game_gfx, 84, 30, str );
        break;
    }
    sprintf( str, "CAM=%d OBJ=%d FRAME=%d", ca_selected, ca_part, ca_frame );
    Txt_DrawString( &game_gfx, 84, 20, str );
}
