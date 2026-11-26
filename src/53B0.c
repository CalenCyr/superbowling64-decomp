/* 53B0.c -- the object list, created from parameter records, and matrix helpers */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "heap.h"
#include "hostio.h"
#include "object.h"
#include "ground.h"
#include "boot.h"
#include "filecache.h"
#include "input.h"
#include "game.h"
#include "overlay.h"
#include "sprite.h"
#include "dbgtext.h"

/*
================
Obj_ClearList

Empties the object list, without freeing its objects.
================
*/
void Obj_ClearList( void ) {
    obj_list.next = NULL;
    obj_list.priority = 0;
    cam_count = 0;
    cam_manual = 0;
    obj_frameStep = 1;
    Scene_InitPuffs();
}

/*
================
Obj_Create

Creates the object of params with its class and inserts it into the list, after the objects of the same or
a lower priority.
================
*/
void Obj_Create( objParams_t *params ) {
    objNode_t *obj;
    objNode_t *prev;

    obj = obj_classes[(u32)params->type].create( params );
    obj->update = obj_classes[(u32)params->type].update;
    obj->priority = params->priority;
    obj->type = params->type;
    for ( prev = &obj_list; prev->next != NULL; prev = prev->next ) {
        if ( prev->next->priority > (u32)params->priority ) {
            break;
        }
    }
    obj->next = prev->next;
    prev->next = obj;
}

/*
================
Obj_CreateAll

Creates the objects of the records params with classes, up to the first record of a negative type.
================
*/
void Obj_CreateAll( objClass_t *classes, objParams_t *params ) {
    s32 i;

    obj_classes = classes;
    obj_params = params;
    for ( i = 0; !( obj_params[i].type <= OBJ_PARAMS_END ); i++ ) {
        if ( obj_params[i].type != OBJ_PARAMS_SKIP ) {
            Obj_Create( &obj_params[i] );
        }
    }
    if ( obj_params[i].type == OBJ_PARAMS_END ) {
        obj_paramsEnded = TRUE;
    } else {
        obj_paramsEnded = FALSE;
    }
}

/*
================
Obj_Frame

Runs a frame of the object list: in the pvcode scene, C-down and C-up switch the cameras to manual and back, and
while manual the animations step only with C-right (forward) and C-left (back). Then updates every object, draws
the scene, and draws the images of the records of type OBJ_PARAMS_SKIP over it.
================
*/
void Obj_Frame( void ) {
    objNode_t *obj;
    s32 i;

    if ( cam_manual == FALSE && ( cont_allPressed & D_CBUTTONS ) && game_scene == SCENE_PVCODE ) {
        cam_manual = TRUE;
        obj_frameStep = 0;
    }
    if ( cam_manual == TRUE && ( cont_allPressed & U_CBUTTONS ) && game_scene == SCENE_PVCODE ) {
        cam_manual = FALSE;
        obj_frameStep = 1;
    }
    obj_frameStep = 1;
    if ( cam_manual == TRUE ) {
        obj_frameStep = 0;
        if ( cont_allHeld & R_CBUTTONS ) {
            obj_frameStep = 1;
        }
        if ( cont_allHeld & L_CBUTTONS ) {
            obj_frameStep = -1;
        }
    }
    Scene_ClearDrawList();
    for ( obj = &obj_list; obj->next != NULL; obj = obj->next ) {
        obj->next->update( obj->next + 1 );
    }
    Scene_Draw();
    Scene_DrawPuffs();
    Scene_BeginFrame();
    gDPSetAlphaCompare( game_gfx++, G_AC_THRESHOLD );
    gDPSetTexturePersp( game_gfx++, G_TP_NONE );
    gDPSetTextureLUT( game_gfx++, G_TT_RGBA16 );
    gDPSetPrimColor( game_gfx++, 0, 0, 255, 255, 255, 255 );
    draw_rdpMode = DRAW2D_FILL;
    for ( i = 0; !( obj_params[i].type <= OBJ_PARAMS_END ); i++ ) {
        if ( obj_params[i].type == OBJ_PARAMS_SKIP ) {
            Draw_ImageScaled( (u32)obj_params[i].priority, obj_params[i].unk_08[0], obj_params[i].unk_08[1],
                              (u32)obj_params[i].unk_08[2], obj_params[i].unk_08[3], obj_params[i].unk_08[4] );
        }
    }
}

/*
================
Obj_Free

Removes obj from the list and frees it.
================
*/
void Obj_Free( objNode_t *obj ) {
    objNode_t *prev;

    prev = &obj_list;
    if ( obj_list.next == obj ) {
        obj_list.next = obj->next;
    } else {
        do {
            prev = prev->next;
        } while ( prev->next != obj );
        prev->next = obj->next;
    }
    Mem_Free( obj );
}

/*
================
Obj_FreeAll

Frees every object, and empties the list.
================
*/
void Obj_FreeAll( void ) {
    objNode_t *head;

    head = &obj_list;
    while ( TRUE ) {
        if ( head->next == NULL ) {
            break;
        }
        Obj_Free( head->next );
    }
    Obj_ClearList();
}

/*
================
Str_ParseFloat

Returns the decimal number str, with its fraction after a '.'.
================
*/
f32 Str_ParseFloat( u8 *str ) {
    f32 whole;
    f32 fraction;
    u8 *point;
    s32 scale;
    s32 digits;
    s32 i;

    whole = atoi( str );
    fraction = 0.0f;
    point = strchr( str, '.' );
    if ( point != NULL ) {
        point++;
        digits = strlen( point );
        scale = 1;
        for ( i = 0; i < digits; i++ ) {
            scale *= 10;
        }
        fraction = (f32)atoi( point ) / scale;
    }
    return whole + fraction;
}

/*
================
Host_ReadFileToObjBuf

Reads host file name into obj_hostFile.
================
*/
void Host_ReadFileToObjBuf( u8 *name ) {
    /* BUG: the read allows 0x8000 bytes, more than the buffer; a longer file overruns the bss after it */
    Host_LoadFile( name, obj_hostFile, 0x8000 );
}

/*
================
Mtx_Translate

Translates m by (x, y, z) in its own axes.
================
*/
void Mtx_Translate( f32 m[4][4], f32 x, f32 y, f32 z ) {
    s32 i;

    for ( i = 0; i < 4; i++ ) {
        m[3][i] += m[0][i] * x + m[1][i] * y + m[2][i] * z;
    }
}

/*
================
Mtx_Scale

Scales the axes of m by x, y and z.
================
*/
void Mtx_Scale( f32 m[4][4], f32 x, f32 y, f32 z ) {
    s32 i;

    for ( i = 0; i < 3; i++ ) {
        m[0][i] *= x;
        m[1][i] *= y;
        m[2][i] *= z;
    }
}

/*
================
Mtx_Rotate

Rotates the axes of m by rot.
================
*/
void Mtx_Rotate( f32 m[4][4], f32 rot[3][3] ) {
    f32 result[3][3];
    s32 i;
    s32 j;
    s32 k;

    for ( i = 0; i < 3; i++ ) {
        for ( j = 0; j < 3; j++ ) {
            result[i][j] = 0.0f;
            for ( k = 0; k < 3; k++ ) {
                result[i][j] += rot[i][k] * m[k][j];
            }
        }
    }
    for ( i = 0; i < 3; i++ ) {
        for ( j = 0; j < 3; j++ ) {
            m[i][j] = result[i][j];
        }
    }
}

/*
================
Mtx_RotateXYZ

Rotates the axes of m by x, y and z radians.
================
*/
void Mtx_RotateXYZ( f32 m[4][4], f32 x, f32 y, f32 z ) {
    f32 rot[3][3];
    f32 sx, cx, sy, cy, sz, cz;
    f32 t;      // RULE-EXCEPTION: R30, one temporary for both shared products: two locals schedule differently

    sx = sinf( x );
    cx = cosf( x );
    sy = sinf( y );
    cy = cosf( y );
    sz = sinf( z );
    cz = cosf( z );
    t = sx * sz;
    rot[0][0] = cy * cz + sy * t;
    rot[0][1] = cx * sz;
    rot[0][2] = -cz * sy + cy * t;
    t = cz * sx;
    rot[1][0] = sy * t - cy * sz;
    rot[1][1] = cx * cz;
    rot[1][2] = cy * t + sy * sz;
    rot[2][0] = cx * sy;
    rot[2][1] = -sx;
    rot[2][2] = cx * cy;
    Mtx_Rotate( m, rot );
}

/*
================
Mtx_SetRotScaleTrans

Sets m to a rotation by x, y and z radians, scaled by scale, then moved to (tx, ty, tz).
================
*/
void Mtx_SetRotScaleTrans( f32 m[4][4], f32 x, f32 y, f32 z, f32 scale, f32 tx, f32 ty, f32 tz ) {
    // R36: unused in the original: retail's frame keeps its 0x28 bytes untouched; the body is Mtx_RotateXYZ's, which builds rot
    f32 rot[3][3];
    f32 sx, cx, sy, cy, sz, cz;
    f32 t;      // RULE-EXCEPTION: R30, one temporary for both shared products, as in Mtx_RotateXYZ

    sx = sinf( x );
    cx = cosf( x );
    sy = sinf( y );
    cy = cosf( y );
    sz = sinf( z );
    cz = cosf( z );
    t = sx * sz;
    m[0][0] = ( cy * cz + sy * t ) * scale;
    m[0][1] = cx * sz * scale;
    m[0][2] = ( -cz * sy + cy * t ) * scale;
    m[0][3] = 0.0f;
    t = cz * sx;
    m[1][0] = ( sy * t - cy * sz ) * scale;
    m[1][1] = cx * cz * scale;
    m[1][2] = ( cy * t + sy * sz ) * scale;
    m[1][3] = 0.0f;
    m[2][0] = cx * sy * scale;
    m[2][1] = -sx * scale;
    m[2][2] = cx * cy * scale;
    m[2][3] = 0.0f;
    m[3][0] = tx;
    m[3][1] = ty;
    m[3][2] = tz;
    m[3][3] = 1.0f;
}

/*
================
Mtx_Place

Moves m by trans, then rotates it by rot (radians) and scales it by scale.
================
*/
void Mtx_Place( f32 m[4][4], f32 *trans, f32 *rot, f32 *scale ) {
    Mtx_Translate( m, trans[0], trans[1], trans[2] );
    if ( rot[0] != 0.0 || rot[1] != 0.0 || rot[2] != 0.0 ) {
        Mtx_RotateXYZ( m, rot[0], rot[1], rot[2] );
    }
    Mtx_Scale( m, scale[0], scale[1], scale[2] );
}

/*
================
Obj_MakeMatrix

Makes the drawn matrix of obj from its placement.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R2): exact only with sched_bufferIndex declared a plain s8 (lb); boot.h declares it
// volatile, as units 1060, 7250 and C650 read it, which loads it with lbu and a sign extension
void Obj_MakeMatrix( sceneObject_t *obj ) {
    Mtx_Scale( obj->mtx, 1.0f, 1.0f, 1.0f );
    guMtxF2L( obj->mtx, &obj->joints[0].mtxL[sched_bufferIndex] );
}
#else
INCLUDE_ASM("asm/nonmatchings/53B0", Obj_MakeMatrix);
#endif

/*
================
Obj_PoseJoints

Sets the joint matrices of obj for its frame: each joint is its parent's matrix (the object's for a root), moved,
rotated and scaled by the components its animation mask selects from the frame, the others from the base pose or
the model's part. Makes the drawn matrices, and with flags & 0xC000 the mirrored ones.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R2): exact only with sched_bufferIndex declared a plain s8 (lb), as for
// Obj_MakeMatrix; boot.h declares it volatile, which loads it with lbu and a sign extension
void Obj_PoseJoints( sceneObject_t *obj ) {
    fileType1_t *model;
    fileType2_t *anim;
    f32 *base;
    f32 *values;
    s32 hasBase;
    u32 j;
    s32 i;
    u32 mask;
    f32 trans[3];
    f32 rot[3];
    f32 scale[3];

    anim = obj->anim;
    model = obj->model;
    if ( anim->masks[0] & ANIM_BASE_POSE ) {
        hasBase = TRUE;
        base = anim->data;
        values = base + anim->frameSize * obj->frame + anim->numJoints * 9;
    } else {
        hasBase = FALSE;
        values = anim->data + anim->frameSize * obj->frame;
    }
    for ( j = 0; j < model->numParts; j++ ) {
        if ( model->parts[j]->parent != -1 ) {
            memcpy( obj->joints[j].mtx, obj->joints[model->parts[j]->parent].mtx, sizeof( obj->mtx ) );
        } else {
            memcpy( obj->joints[j].mtx, obj->mtx, sizeof( obj->mtx ) );
        }
        if ( anim->masks[j] == 0 ) {
            if ( hasBase ) {
                Mtx_Place( obj->joints[j].mtx, &base[j * 9], &base[j * 9 + 3], &base[j * 9 + 6] );
            } else {
                Mtx_Place( obj->joints[j].mtx, model->parts[j]->trans, model->parts[j]->rot, model->parts[j]->scale );
            }
        } else {
            mask = anim->masks[j];
            for ( i = 0; i < 3; i++ ) {
                if ( mask & ANIM_MASK_FIRST ) {
                    trans[i] = *values++;
                } else if ( hasBase ) {
                    trans[i] = base[j * 9 + i];
                } else {
                    trans[i] = model->parts[j]->trans[i];
                }
                mask <<= 1;
            }
            for ( i = 0; i < 3; i++ ) {
                if ( mask & ANIM_MASK_FIRST ) {
                    rot[i] = *values++;
                } else if ( hasBase ) {
                    rot[i] = base[j * 9 + 3 + i];
                } else {
                    rot[i] = model->parts[j]->rot[i];
                }
                mask <<= 1;
            }
            for ( i = 0; i < 3; i++ ) {
                if ( mask & ANIM_MASK_FIRST ) {
                    scale[i] = *values++;
                } else if ( hasBase ) {
                    scale[i] = base[j * 9 + 6 + i];
                } else {
                    scale[i] = model->parts[j]->scale[i];
                }
                mask <<= 1;
            }
            Mtx_Place( obj->joints[j].mtx, trans, rot, scale );
        }
        guMtxF2L( obj->joints[j].mtx, &obj->joints[j].mtxL[sched_bufferIndex] );
        if ( obj->flags & 0xC000 ) {
            for ( i = 0; i < 4; i++ ) {
                obj->joints[j].mtx[i][1] = -obj->joints[j].mtx[i][1];
            }
            guMtxF2L( obj->joints[j].mtx, &obj->joints[j].mirrorL[sched_bufferIndex] );
            for ( i = 0; i < 4; i++ ) {
                obj->joints[j].mtx[i][1] = -obj->joints[j].mtx[i][1];
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53B0", Obj_PoseJoints);
#endif

/*
================
Obj_ConvertModel

Converts the model commands at gfx, in place, into the display list they stand for: each command is 8 bytes,
like the Gfx it becomes.
================
*/
void Obj_ConvertModel( Gfx *gfx ) {
    u8 cmd[0x18];   /* a command, read with more bytes than it has */
    s32 end;

    end = 0;
    do {
        memmove( cmd, gfx, sizeof( cmd ) );
        switch ( cmd[0] ) {
        case MDLCMD_END:
            gSPEndDisplayList( gfx++ );
            end = -1;
            break;
        case MDLCMD_TRI1:
            gSP1Triangle( gfx++, cmd[1], cmd[2], cmd[3], cmd[4] );
            break;
        case MDLCMD_TRI2:
            gSP2Triangles( gfx++, cmd[1], cmd[2], cmd[3], cmd[7] & 0xF, cmd[4], cmd[5], cmd[6], cmd[7] >> 4 );
            break;
        case MDLCMD_VTX_ST:
            gSPModifyVertex( gfx++, cmd[1], G_MWO_POINT_ST, ( cmd[2] << 24 ) + ( cmd[3] << 16 ) + ( cmd[4] << 8 ) + cmd[5] );
            break;
        }
    } while ( end != -1 );
}

/*
================
Obj_CreateSceneObject

The create of the scene object class: loads the model and animation of params, sizes the object for the model's
parts, places it, and lists it in obj_scene.
================
*/
objNode_t *Obj_CreateSceneObject( objParams_t *params ) {
    fileType1_t *model;
    sceneObjectNode_t *node;
    sceneObject_t *obj;

    model = Cache_GetFile( params->unk_08[0], FILE_TYPE1 );
    node = Mem_Alloc( sizeof( sceneObjectNode_t ) - ( OBJ_MAX_JOINTS - model->numParts ) * sizeof( sceneObjJoint_t ) );
    obj = &node->obj;
    obj->model = model;
    obj->anim = Cache_GetFile( params->unk_08[1], FILE_TYPE2 );
    Mtx_SetRotScaleTrans( obj->mtx, params->unk_08[8] * DEG_TO_RAD, params->unk_08[9] * DEG_TO_RAD,
                          params->unk_08[10] * DEG_TO_RAD, 1.0 - params->unk_08[11], params->unk_08[2],
                          params->unk_08[3], params->unk_08[4] );
    obj->frame = 0;
    obj->viewMask = (u32)params->unk_08[5];
    obj->flags = (u32)params->unk_08[6];
    if ( params->unk_08[13] != 0.0f ) {
        obj->flags |= 0xC000;
    }
    obj->unk_02 = (u32)params->unk_08[7];
    obj_scene[(s32)params->priority] = obj;
    return &node->node;
}

/*
================
Obj_AddToDrawList

Poses obj and adds its parts to the draw list, for the cameras of its viewMask that face it (all of them while the
cameras are manual, or for an object of another microcode); with flags & 0xC000 and a floor height, adds their
mirror images too, and with flags & 0x3000 its shadow, on the joint flags & 0xE00 picks.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R2): exact only with sched_bufferIndex declared a plain s8 (lb), as for
// Obj_MakeMatrix; boot.h declares it volatile, which loads it with lbu and a sign extension
void Obj_AddToDrawList( sceneObject_t *obj ) {
    fileType1_t *model;
    fileType1Part_t *part;
    fileRefB_t *material;
    sceneCamera_t *cam;
    u8 viewMask;
    u32 i;
    u32 j;
    s32 k;
    Mtx *shadowMtx;
    f32 mtx[4][4];      // R36: unused in the original: retail's frame (0xA0) holds 0x40 bytes of locals below its
                        // two spill slots that no instruction addresses

    model = obj->model;
    Obj_PoseJoints( obj );
    viewMask = obj->viewMask;
    scene_addUcode = obj->flags & 3;
    if ( scene_addUcode == 0 && cam_manual == FALSE ) {
        for ( i = 0; i < cam_count; i++ ) {
            if ( ( viewMask >> i ) & 1 ) {
                cam = cam_cameras[i];
                if ( cam->dir[0] * ( obj->mtx[3][0] - cam->eye[0] ) + cam->dir[1] * ( obj->mtx[3][1] - cam->eye[1] )
                     + cam->dir[2] * ( obj->mtx[3][2] - cam->eye[2] ) < 0.0f ) {
                    viewMask &= ~( 1 << i );
                }
            }
        }
    }
    if ( viewMask == 0 ) {
        return;
    }
    for ( j = 0; j < model->numParts; j++ ) {
        part = model->parts[j];
        scene_addSurface = part->flags & 7;
        for ( k = 0; k < part->numRefs; k++ ) {
            material = cache_files[part->refs[k].material].data;
            scene_viewMask = viewMask;
            scene_addLayer = part->layer & 0xF;
            if ( scene_addLayer < 2 ) {
                scene_addNoZBuffer = FALSE;
                scene_addTwoCycle = TRUE;
            } else {
                scene_addNoZBuffer = TRUE;
                scene_addTwoCycle = FALSE;
            }
            scene_addCullBack = TRUE;
            scene_addByMaterial = material->flags & MAT_TEXTURE;
            scene_addMesh = part->refs[k].mesh;
            scene_addMaterial = part->refs[k].material;
            scene_addMatrix = &obj->joints[j].mtxL[sched_bufferIndex];
            Scene_AddToDrawList();
            if ( ( obj->flags & 0xC000 ) && obj->floorHeight >= 0.0f ) {
                scene_viewMask &= obj->flags >> 14;
                scene_addLayer = 3;
                scene_addNoZBuffer = TRUE;
                scene_addTwoCycle = FALSE;
                scene_addCullBack = FALSE;
                scene_addMatrix = &obj->joints[j].mirrorL[sched_bufferIndex];
                Scene_AddToDrawList();
            }
        }
    }
    if ( obj->flags & 0x3000 ) {
        Cache_GetFile( 0x6E1, FILE_TYPE1 );
        scene_addSurface = 2;
        scene_addLayer = 0;
        scene_addNoZBuffer = FALSE;
        scene_addTwoCycle = TRUE;
        scene_addMaterial = 0x6E0;
        scene_addByMaterial = TRUE;
        scene_addMesh = ( obj->flags >> 12 ) | 0x6DC;
        switch ( obj->flags & 0xE00 ) {
        case 0x400:
            shadowMtx = &obj->joints[20].mtxL[sched_bufferIndex];
            break;
        case 0x800:
            shadowMtx = &obj->joints[17].mtxL[sched_bufferIndex];
            break;
        case 0xC00:
            shadowMtx = &obj->joints[24].mtxL[sched_bufferIndex];
            break;
        case 0xE00:
            shadowMtx = &obj->joints[15].mtxL[sched_bufferIndex];
            break;
        default:
            shadowMtx = &obj->joints[0].mtxL[sched_bufferIndex];
            break;
        }
        scene_addMatrix = shadowMtx;
        Scene_AddToDrawList();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53B0", Obj_AddToDrawList);
#endif

/*
================
Obj_Draw

Draws obj, and advances its animation, from the start after its last frame.
================
*/
void Obj_Draw( sceneObject_t *obj ) {
    Obj_AddToDrawList( obj );
    obj->frame += obj_frameStep;
    if ( obj->frame == obj->anim->numFrames ) {
        obj->frame = 0;
    }
}
