#ifndef OBJECT_H
#define OBJECT_H

/* object.h -- the object list, created from parameter records, matrix helpers and scene objects (unit 53B0), and
   the scene's cameras (unit 7250) */

#include "common.h"
#include "libultra.h"
#include "filecache.h"

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
extern s32 D_80099480;              /* cameras in D_8009CFA0 */
extern s32 D_800EA0C0;
extern s32 D_800F7BF8;              /* frames each update advances; 1 at start */
extern u8 D_800F30E0[0x8000];       /* func_8002A7EC reads a host file here */

/* Model commands func_8002B380 converts: the opcode byte, then its arguments. */
#define MDLCMD_END          0           /* end of the list */
#define MDLCMD_TRI1         2           /* vertices 1-3, flag 4 */
#define MDLCMD_VTX_ST       9           /* vertex 1, texture coordinates 2-5 */
#define MDLCMD_TRI2         13          /* vertices 1-3 and 4-6, flags in the nibbles of 7 */

/* Joint of a scene object: its matrix, the translation in row 3, and the fixed-point copies drawn from it. */
typedef struct {
    /* 0x000 */ f32 mtx[4][4];
    /* 0x040 */ Mtx mtxL[2];            /* by D_8005C0B1 */
    /* 0x0C0 */ Mtx mirrorL[2];         /* by D_8005C0B1: mtx with its translation mirrored */
} sceneObjJoint_t;                         /* size 0x140 */

/*
Object of the scene engine (gacode's D_800EA120 holds pointers to them). gacode.h keeps a partial copy of it,
sceneObj_t, until it includes this header.
*/
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u8 unk_02[2];
    /* 0x04 */ f32 mtx[4][4];           /* placement */
    /* 0x44 */ f32 unk_44;
    /* 0x48 */ u16 frame;               /* of the animation */
    /* 0x4A */ u8 viewMask;             /* 0: not drawn */
    /* 0x4B */ u8 unk_4B;
    /* 0x4C */ void *model;
    /* 0x50 */ s32 *anim;               /* a loaded file (func_80030C04); its first word is its frame count */
    /* 0x54 */ u8 unk_54[0x04];
    /* 0x58 */ sceneObjJoint_t joints[1];  /* one per joint of the model */
} sceneObject_t;

/*
Camera of the scene, following a camera path (D_8009CFA0 lists them; an object's viewMask has a bit for each,
by that index).
*/
typedef struct {
    /* 0x000 */ f32 eye[3];
    /* 0x00C */ f32 at[3];
    /* 0x018 */ f32 unk_18[3];          /* the up vector points from eye to it */
    /* 0x024 */ s32 frame;              /* of path; negative: the path is not followed */
    /* 0x028 */ u16 width;              /* of the viewport */
    /* 0x02A */ u16 height;
    /* 0x02C */ u16 unk_2C;
    /* 0x02E */ u16 unk_2E;
    /* 0x030 */ u16 targetObj;          /* index in D_800EA120 of the object it looks at; 0: at stays */
    /* 0x032 */ u16 targetJoint;        /* the joint of that object */
    /* 0x034 */ f32 up[3];
    /* 0x040 */ f32 dir[3];             /* at - eye */
    /* 0x04C */ f32 mtx[4][4];          /* view */
    /* 0x08C */ u8 unk_8C[0x04];
    /* 0x090 */ Mtx viewL[2];           /* by D_8005C0B1 */
    /* 0x110 */ Mtx projL[2];
    /* 0x190 */ LookAt lookAt[2];
    /* 0x1D0 */ Vp vp[2];
    /* 0x1F0 */ Lights2 lights[2];      /* l[1] shines from the eye */
    /* 0x240 */ fileType3_t *path;
    /* 0x244 */ u8 unk_244[0x04];
} sceneCamera_t;                        /* size 0x248 */

extern sceneCamera_t *D_8009CFA0[];

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
void func_8002A98C( f32 m[4][4], f32 x, f32 y, f32 z );
void func_8002AAD0( f32 m[4][4], f32 x, f32 y, f32 z, f32 scale, f32 tx, f32 ty, f32 tz );
void func_8002AC84( f32 m[4][4], f32 *trans, f32 *rot, f32 *scale );
void func_8002AEBC( sceneObject_t *obj );
void func_8002B380( Gfx *gfx );
void func_8002B9CC( sceneObject_t *obj );
void func_8002BE00( sceneObject_t *obj );

/* the cameras (unit 7250) */
void func_8002BE50( s32 index, u8 *color );
void func_8002BED8( s16 file, s16 frame, f32 *eye, f32 *unk_2C );
void func_8002C324( sceneCamera_t *cam );
void func_8002C908( sceneCamera_t *cam );

#endif
