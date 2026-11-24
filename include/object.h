#ifndef OBJECT_H
#define OBJECT_H

/* object.h -- the object list, created from parameter records, matrix helpers and scene objects (unit 53B0), and
   the scene's cameras (unit 7250) */

#include "common.h"
#include "libultra.h"
#include "filecache.h"

#define OBJ_PARAMS_SKIP     2.0f        /* objParams_t.type: no object */
#define OBJ_PARAMS_END      -1.0f       /* objParams_t.type: ends the records */

/* Parameter record of one object, all floats; Obj_CreateAll reads them in an array. */
typedef struct {
    /* 0x00 */ f32 type;            /* index into D_800E657C */
    /* 0x04 */ f32 priority;
    /* 0x08 */ f32 unk_08[14];
} objParams_t;                      /* size 0x40 */

/* Header of every object; the list from D_80066E00 is kept in priority order. */
typedef struct objNode_s {
    /* 0x00 */ void (*update)( void *obj ); /* objClass_t.update of its class; called each frame with the object
                                           after this header (func_8002A2F0) */
    /* 0x04 */ u32 priority;
    /* 0x08 */ u32 type;
    /* 0x0C */ struct objNode_s *next;
} objNode_t;

typedef struct {
    /* 0x00 */ objNode_t *(*create)( objParams_t *params );
    /* 0x04 */ void (*update)( void *obj ); /* each frame (Obj_Draw, Cam_Update, ...) */
} objClass_t;

extern objNode_t D_80066E00;        /* list head */
extern objClass_t *D_800E657C;      /* the classes, by objParams_t.type */
extern objParams_t *D_800A2FE0;     /* the records Obj_CreateAll creates objects from */
extern s32 D_800A2FDC;              /* TRUE when the records ended with OBJ_PARAMS_END */
extern s32 D_80099480;              /* cameras in D_8009CFA0 */
extern s32 D_800EA0C0;
extern s32 D_800F7BF8;              /* frames each update advances; 1 at start */
extern u8 D_800F30E0[0x4B00];       /* func_8002A7EC reads a host file here (scene_texGen follows) */

/* Model commands Obj_ConvertModel converts: the opcode byte, then its arguments. */
#define MDLCMD_END          0           /* end of the list */
#define MDLCMD_TRI1         2           /* vertices 1-3, flag 4 */
#define MDLCMD_VTX_ST       9           /* vertex 1, texture coordinates 2-5 */
#define MDLCMD_TRI2         13          /* vertices 1-3 and 4-6, flags in the nibbles of 7 */

/* Joint of a scene object: its matrix, the translation in row 3, and the fixed-point copies drawn from it. */
typedef struct {
    /* 0x000 */ f32 mtx[4][4];
    /* 0x040 */ Mtx mtxL[2];            /* by sched_bufferIndex */
    /* 0x0C0 */ Mtx mirrorL[2];         /* by sched_bufferIndex: mtx with its translation mirrored */
} sceneObjJoint_t;                         /* size 0x140 */

/* Object of the scene engine (D_800EA120 holds pointers to them). */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u8 unk_02[2];
    /* 0x04 */ f32 mtx[4][4];           /* placement */
    /* 0x44 */ f32 floorHeight;         /* drawn at by func_8002B9CC when flags & 0xC000; negative: none */
    /* 0x48 */ u16 frame;               /* of the animation */
    /* 0x4A */ u8 viewMask;             /* 0: not drawn */
    /* 0x4B */ u8 unk_4B;
    /* 0x4C */ fileType1_t *model;      /* a loaded FILE_TYPE1 file (Cache_GetFile) */
    /* 0x50 */ s32 *anim;               /* a loaded file (Cache_GetFile); its first word is its frame count */
    /* 0x54 */ u8 unk_54[0x04];
    /* 0x58 */ sceneObjJoint_t joints[2];  /* one per joint of the model (at least two) */
} sceneObject_t;

/*
Camera of the scene, following a camera path (D_8009CFA0 lists them; an object's viewMask has a bit for each,
by that index).
*/
typedef struct {
    /* 0x000 */ f32 eye[3];
    /* 0x00C */ f32 at[3];
    /* 0x018 */ f32 upPoint[3];         /* the up vector points from eye to it */
    /* 0x024 */ s32 frame;              /* of path; negative: the path is not followed */
    /* 0x028 */ u16 width;              /* of the viewport */
    /* 0x02A */ u16 height;
    /* 0x02C */ u16 left;               /* of the viewport, in the framebuffer */
    /* 0x02E */ u16 top;
    /* 0x030 */ u16 targetObj;          /* index in D_800EA120 of the object it looks at; 0: at stays */
    /* 0x032 */ u16 targetJoint;        /* the joint of that object */
    /* 0x034 */ f32 up[3];
    /* 0x040 */ f32 dir[3];             /* at - eye */
    /* 0x04C */ f32 mtx[4][4];          /* view */
    /* 0x08C */ u8 unk_8C[0x04];
    /* 0x090 */ Mtx viewL[2];           /* by sched_bufferIndex */
    /* 0x110 */ Mtx projL[2];
    /* 0x190 */ LookAt lookAt[2];
    /* 0x1D0 */ Vp vp[2];
    /* 0x1F0 */ Lights2 lights[2];      /* l[1] shines from the eye */
    /* 0x240 */ fileType3_t *path;
    /* 0x244 */ u8 unk_244[0x04];
} sceneCamera_t;                        /* size 0x248 */

extern sceneCamera_t *D_8009CFA0[];
extern u16 D_8009CF9E;              /* perspNorm of the cameras' projections (guPerspective) */

/* What the classes func_8002B610 and Cam_Create create: the list header, then the scene object or camera. */
typedef struct {
    /* 0x00 */ objNode_t node;
    /* 0x10 */ sceneObject_t obj;
} sceneObjectNode_t;

typedef struct {
    /* 0x00 */ objNode_t node;
    /* 0x10 */ sceneCamera_t cam;
} sceneCameraNode_t;                    /* size 0x258 */
extern void *D_800EA120[];          /* main's scene objects, of several kinds: cameras (sceneCamera_t, Cam_Create)
                                       and scene objects (sceneObject_t) */

void Obj_ClearList( void );
void Obj_Create( objParams_t *params );
void Obj_CreateAll( objClass_t *classes, objParams_t *params );
void func_8002A2F0( void );
void Obj_Free( objNode_t *obj );
void Obj_FreeAll( void );
f32 func_8002A724( u8 *str );
void func_8002A7EC( u8 *name );
void Mtx_Translate( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_Scale( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_Rotate( f32 m[4][4], f32 rot[3][3] );
void Mtx_RotateXYZ( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_SetRotScaleTrans( f32 m[4][4], f32 x, f32 y, f32 z, f32 scale, f32 tx, f32 ty, f32 tz );
void Mtx_Place( f32 m[4][4], f32 *trans, f32 *rot, f32 *scale );
void Obj_MakeMatrix( sceneObject_t *obj );
void Obj_ConvertModel( Gfx *gfx );
objNode_t *func_8002B610( objParams_t *params );
void func_8002B9CC( sceneObject_t *obj );
void Obj_Draw( sceneObject_t *obj );

/* the cameras (unit 7250) */
void Cam_SetAmbient( s32 index, u8 *color );
void Cam_ReadPathFrame( s16 file, s16 frame, f32 *eye, f32 *upPoint );
objNode_t *Cam_Create( objParams_t *params );
void Cam_Update( sceneCamera_t *cam );
void Cam_UpdateAndAdvance( sceneCamera_t *cam );

#endif
