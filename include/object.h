#ifndef OBJECT_H
#define OBJECT_H

/* object.h -- the object list, created from parameter records, and matrix helpers (unit 53B0) */

#include "common.h"
#include "libultra.h"

#define OBJ_PARAMS_SKIP     2.0f        /* objParams_t.type: no object */
#define OBJ_PARAMS_END      -1.0f       /* objParams_t.type: ends the records */

/* Parameter record of one object, all floats; func_8002A1EC reads them in an array. */
typedef struct {
    /* 0x00 */ f32 type;            /* index into D_800E657C */
    /* 0x04 */ f32 priority;
    /* 0x08 */ f32 unk_08[14];
} objParams_t;                      /* size 0x40 */

/* Header of every object; the list from D_80066E00 is kept in priority order. */
typedef struct objNode_s {
    /* 0x00 */ u32 unk_00;          /* objClass_t.unk_04 of its class */
    /* 0x04 */ u32 priority;
    /* 0x08 */ u32 type;
    /* 0x0C */ struct objNode_s *next;
} objNode_t;

typedef struct {
    /* 0x00 */ objNode_t *(*create)( objParams_t *params );
    /* 0x04 */ u32 unk_04;
} objClass_t;

extern objNode_t D_80066E00;        /* list head */
extern objClass_t *D_800E657C;      /* the classes, by objParams_t.type */
extern objParams_t *D_800A2FE0;     /* the records func_8002A1EC creates objects from */
extern s32 D_800A2FDC;              /* TRUE when the records ended with OBJ_PARAMS_END */
extern s32 D_80099480;
extern s32 D_800EA0C0;
extern s32 D_800F7BF8;              /* frames each update advances; 1 at start */
extern u8 D_800F30E0[0x8000];       /* func_8002A7EC reads a host file here */

void func_80029FB0( void );
void func_80029FF4( objParams_t *params );
void func_8002A1EC( objClass_t *classes, objParams_t *params );
void func_8002A624( objNode_t *obj );
void func_8002A67C( void );
f32 func_8002A724( u8 *str );
void func_8002A7EC( u8 *name );
void func_8002A810( f32 m[4][4], f32 x, f32 y, f32 z );
void func_8002A864( f32 m[4][4], f32 x, f32 y, f32 z );
void func_8002A8B0( f32 m[4][4], f32 rot[3][3] );
void func_8002B380( void *arg );
void func_8002B9CC( void *obj );
void func_8002BE00( void *obj );

#endif
