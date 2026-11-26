#ifndef OBJECT_H
#define OBJECT_H

/* object.h -- the object list, created from parameter records, matrix helpers and scene objects (unit 53B0), and
   the scene's cameras (unit 7250) */

#include "common.h"
#include "libultra.h"
#include "filecache.h"

#define OBJ_PARAMS_SKIP     2.0f        /* objParams_t.type: no object; Obj_Frame draws it as an image: priority is
                                           its id, args[0..4] its x, y, alpha and x and y scales */
#define OBJ_PARAMS_END      -1.0f       /* objParams_t.type: ends the records */

/* Parameter record of one object, all floats; Obj_CreateAll reads them in an array. */
typedef struct {
    /* 0x00 */ f32 type;            /* index into obj_classes */
    /* 0x04 */ f32 priority;
    /* 0x08 */ f32 args[14];
} objParams_t;                      /* size 0x40 */

/* Header of every object; the list from obj_list is kept in priority order. */
typedef struct objNode_s {
    /* 0x00 */ void (*update)( void *obj ); /* objClass_t.update of its class; called each frame with the object
                                           after this header (Obj_Frame) */
    /* 0x04 */ u32 priority;
    /* 0x08 */ u32 type;
    /* 0x0C */ struct objNode_s *next;
} objNode_t;

typedef objNode_t *(*objCreateFunc_t)( objParams_t *params );
typedef void (*objUpdateFunc_t)( void *obj );

typedef struct {
    /* 0x00 */ objCreateFunc_t create;
    /* 0x04 */ objUpdateFunc_t update;  /* each frame (Obj_Draw, Cam_Update, ...) */
} objClass_t;

extern objNode_t obj_list;          /* list head */
extern objClass_t *obj_classes;     /* the classes, by objParams_t.type */
extern objParams_t *obj_params;     /* the records Obj_CreateAll creates objects from */
extern s32 obj_paramsEnded;         /* TRUE when the records ended with OBJ_PARAMS_END */
extern s32 cam_count;               /* cameras in cam_cameras */
extern s32 cam_manual;               /* set: Cam_Update applies the controller's zoom, pan and rotation */
extern s32 obj_frameStep;           /* frames each update advances; 1 at start */
extern u8 obj_hostFile[0x4B00];     /* Host_ReadFileToObjBuf reads a host file here (scene_texGen follows) */

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

#define OBJ_MAX_JOINTS      100

/* Object of the scene engine (obj_scene holds pointers to them). */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u16 unk_02;              /* set from its parameter record (Obj_CreateSceneObject) */
    /* 0x04 */ f32 mtx[4][4];           /* placement */
    /* 0x44 */ f32 floorHeight;         /* drawn at by Obj_AddToDrawList when flags & 0xC000; negative: none */
    /* 0x48 */ u16 frame;               /* of the animation */
    /* 0x4A */ u8 viewMask;             /* 0: not drawn */
    /* 0x4B */ u8 unk_4B;
    /* 0x4C */ fileType1_t *model;      /* a loaded FILE_TYPE1 file (Cache_GetFile) */
    /* 0x50 */ fileType2_t *anim;       /* a loaded FILE_TYPE2 file (Cache_GetFile) */
    /* 0x54 */ u8 unk_54[0x04];
    /* 0x58 */ sceneObjJoint_t joints[OBJ_MAX_JOINTS];  /* one per part of the model; Obj_CreateSceneObject allocates
                                           only those */
} sceneObject_t;

/*
Camera of the scene, following a camera path (cam_cameras lists them; an object's viewMask has a bit for each,
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
    /* 0x030 */ u16 targetObj;          /* index in obj_scene of the object it looks at; 0: at stays */
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

extern sceneCamera_t *cam_cameras[];
extern u16 cam_perspNorm;           /* perspNorm of the cameras' projections (guPerspective) */

/* What the classes Obj_CreateSceneObject and Cam_Create create: the list header, then the scene object or camera. */
typedef struct {
    /* 0x00 */ objNode_t node;
    /* 0x10 */ sceneObject_t obj;
} sceneObjectNode_t;

typedef struct {
    /* 0x00 */ objNode_t node;
    /* 0x10 */ sceneCamera_t cam;
} sceneCameraNode_t;                    /* size 0x258 */
extern void *obj_scene[];           /* main's scene objects, of several kinds: cameras (sceneCamera_t, Cam_Create)
                                        and scene objects (sceneObject_t) */

#define DEG_TO_RAD          ( 3.14159265358979323846 / 180.0 )

void Obj_ClearList( void );
void Obj_Create( objParams_t *params );
void Obj_CreateAll( objClass_t *classes, objParams_t *params );
void Obj_Frame( void );
void Obj_Free( objNode_t *obj );
void Obj_FreeAll( void );
f32 Str_ParseFloat( u8 *str );
void Host_ReadFileToObjBuf( u8 *name );
void Mtx_Translate( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_Scale( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_Rotate( f32 m[4][4], f32 rot[3][3] );
void Mtx_RotateXYZ( f32 m[4][4], f32 x, f32 y, f32 z );
void Mtx_SetRotScaleTrans( f32 m[4][4], f32 x, f32 y, f32 z, f32 scale, f32 tx, f32 ty, f32 tz );
void Mtx_Place( f32 m[4][4], f32 *trans, f32 *rot, f32 *scale );
void Obj_MakeMatrix( sceneObject_t *obj );
void Obj_ConvertModel( Gfx *gfx );
objNode_t *Obj_CreateSceneObject( objParams_t *params );
void Obj_AddToDrawList( sceneObject_t *obj );
void Obj_Draw( sceneObject_t *obj );

/* the cameras (unit 7250) */
void Cam_SetAmbient( s32 index, u8 *color );
void Cam_ReadPathFrame( s16 file, s16 frame, f32 *eye, f32 *upPoint );
objNode_t *Cam_Create( objParams_t *params );
void Cam_Update( sceneCamera_t *cam );
void Cam_UpdateAndAdvance( sceneCamera_t *cam );

#endif
