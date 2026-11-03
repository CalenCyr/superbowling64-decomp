/* 53B0.c -- the object list, created from parameter records, and matrix helpers */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "heap.h"
#include "hostio.h"
#include "object.h"
#include "ground.h"
#include "boot.h"

/*
================
func_80029FB0

Empties the object list, without freeing its objects.
================
*/
void func_80029FB0( void ) {
    D_80066E00.next = NULL;
    D_80066E00.priority = 0;
    D_80099480 = 0;
    D_800EA0C0 = 0;
    D_800F7BF8 = 1;
    func_80035074();
}

/*
================
func_80029FF4

Creates the object of params with its class and inserts it into the list, after the objects of the same or
a lower priority.
================
*/
void func_80029FF4( objParams_t *params ) {
    objNode_t *obj;
    objNode_t *prev;

    obj = D_800E657C[(u32)params->type].create( params );
    obj->unk_00 = D_800E657C[(u32)params->type].unk_04;
    obj->priority = params->priority;
    obj->type = params->type;
    for ( prev = &D_80066E00; prev->next != NULL; prev = prev->next ) {
        if ( prev->next->priority > (u32)params->priority ) {
            break;
        }
    }
    obj->next = prev->next;
    prev->next = obj;
}

/*
================
func_8002A1EC

Creates the objects of the records params with classes, up to the first record of a negative type.
================
*/
void func_8002A1EC( objClass_t *classes, objParams_t *params ) {
    s32 i;

    D_800E657C = classes;
    D_800A2FE0 = params;
    for ( i = 0; !( D_800A2FE0[i].type <= OBJ_PARAMS_END ); i++ ) {
        if ( D_800A2FE0[i].type != OBJ_PARAMS_SKIP ) {
            func_80029FF4( &D_800A2FE0[i] );
        }
    }
    if ( D_800A2FE0[i].type == OBJ_PARAMS_END ) {
        D_800A2FDC = TRUE;
    } else {
        D_800A2FDC = FALSE;
    }
}

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002A2F0);

/*
================
func_8002A624

Removes obj from the list and frees it.
================
*/
void func_8002A624( objNode_t *obj ) {
    objNode_t *prev;

    prev = &D_80066E00;
    if ( D_80066E00.next == obj ) {
        D_80066E00.next = obj->next;
    } else {
        do {
            prev = prev->next;
        } while ( prev->next != obj );
        prev->next = obj->next;
    }
    func_80029F18( obj );
}

/*
================
func_8002A67C

Frees every object, and empties the list.
================
*/
void func_8002A67C( void ) {
    objNode_t *head;

    head = &D_80066E00;
    while ( TRUE ) {
        if ( head->next == NULL ) {
            break;
        }
        func_8002A624( head->next );
    }
    func_80029FB0();
}

/*
================
func_8002A724

Returns the decimal number str, with its fraction after a '.'.
================
*/
f32 func_8002A724( u8 *str ) {
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
func_8002A7EC

Reads host file name into D_800F30E0.
================
*/
void func_8002A7EC( u8 *name ) {
    func_80029728( name, D_800F30E0, sizeof( D_800F30E0 ) );
}

/*
================
func_8002A810

Translates m by (x, y, z) in its own axes.
================
*/
void func_8002A810( f32 m[4][4], f32 x, f32 y, f32 z ) {
    s32 i;

    for ( i = 0; i < 4; i++ ) {
        m[3][i] += m[0][i] * x + m[1][i] * y + m[2][i] * z;
    }
}

/*
================
func_8002A864

Scales the axes of m by x, y and z.
================
*/
void func_8002A864( f32 m[4][4], f32 x, f32 y, f32 z ) {
    s32 i;

    for ( i = 0; i < 3; i++ ) {
        m[0][i] *= x;
        m[1][i] *= y;
        m[2][i] *= z;
    }
}

/*
================
func_8002A8B0

Rotates the axes of m by rot.
================
*/
void func_8002A8B0( f32 m[4][4], f32 rot[3][3] ) {
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
func_8002A98C

Rotates the axes of m by x, y and z radians.
================
*/
void func_8002A98C( f32 m[4][4], f32 x, f32 y, f32 z ) {
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
    func_8002A8B0( m, rot );
}

/*
================
func_8002AAD0

Sets m to a rotation by x, y and z radians, scaled by scale, then moved to (tx, ty, tz).
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: MATCH-PENDING (R17): byte-exact only with the unused local rot, which supplies retail's 0x28
// bytes of unused frame; without it the frame is 0x50 instead of 0x78
void func_8002AAD0( f32 m[4][4], f32 x, f32 y, f32 z, f32 scale, f32 tx, f32 ty, f32 tz ) {
    f32 rot[3][3];
    f32 sx, cx, sy, cy, sz, cz;
    f32 t;      // RULE-EXCEPTION: R30, one temporary for both shared products, as in func_8002A98C

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
#else
INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AAD0);
#endif

/*
================
func_8002AC84

Moves m by trans, then rotates it by rot (radians) and scales it by scale.
================
*/
void func_8002AC84( f32 m[4][4], f32 *trans, f32 *rot, f32 *scale ) {
    func_8002A810( m, trans[0], trans[1], trans[2] );
    if ( rot[0] != 0.0 || rot[1] != 0.0 || rot[2] != 0.0 ) {
        func_8002A98C( m, rot[0], rot[1], rot[2] );
    }
    func_8002A864( m, scale[0], scale[1], scale[2] );
}

/*
================
func_8002AEBC

Makes the drawn matrix of obj from its placement.
================
*/
void func_8002AEBC( sceneObject_t *obj ) {
    func_8002A864( obj->mtx, 1.0f, 1.0f, 1.0f );
    guMtxF2L( obj->mtx, &obj->joints[0].mtxL[D_8005C0B1] );
}

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AF20);

/*
================
func_8002B380

Converts the model commands at gfx, in place, into the display list they stand for: each command is 8 bytes,
like the Gfx it becomes.
================
*/
void func_8002B380( Gfx *gfx ) {
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

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002B610);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002B9CC);

/*
================
func_8002BE00

Draws obj, and advances its animation, from the start after its last frame.
================
*/
void func_8002BE00( sceneObject_t *obj ) {
    func_8002B9CC( obj );
    obj->frame += D_800F7BF8;
    if ( obj->frame == obj->anim[0] ) {
        obj->frame = 0;
    }
}
