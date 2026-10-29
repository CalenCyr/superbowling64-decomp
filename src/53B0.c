/* 53B0.c -- the object list, created from parameter records, and matrix helpers */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "heap.h"
#include "hostio.h"
#include "object.h"
#include "ground.h"

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

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002A98C);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AAD0);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AC84);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AEBC);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002AF20);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002B380);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002B610);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002B9CC);

INCLUDE_ASM("asm/nonmatchings/53B0", func_8002BE00);
