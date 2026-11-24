#ifndef GROUND_H
#define GROUND_H

/* ground.h -- unit C650: the draw list, sorted in buckets by key, and the frame's render state */

#include "common.h"
#include "libultra.h"
#include "object.h"

#define DRAW_BUCKETS        0x400       /* keys of the draw list */
#define DRAW_NONE           -1          /* end of a chain of draw entries; an empty bucket */

/* One entry of the draw list (scene_drawList), chained by index. */
typedef struct {
    /* 0x00 */ s16 next;            /* index of the next entry, or DRAW_NONE */
    /* 0x02 */ u16 key;             /* entries of a bucket are kept in key order */
    /* 0x04 */ u16 unk_04;
    /* 0x06 */ u16 unk_06;
    /* 0x08 */ void *unk_08;
} drawEntry_t;                      /* size 0xC */

#define NUM_PUFFS           50
#define NUM_PUFF_SIZES      30          /* squares of scene_puffSquares: half-width 1 to 30 */

/* puff_t.flags */
#define PUFF_KIND           0xFF        /* 0 or 1: its texture (scene_puffImages) and colour (scene_puffColors) */
#define PUFF_CAMERA( n )    ( 0x100 << ( n ) )  /* drawn by camera n */

/* A puff: a square that faces the cameras, grows and fades, for life frames. */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u16 life;            /* frames left; 0: free */
    /* 0x04 */ s16 alpha;
    /* 0x06 */ s16 alphaStep;
    /* 0x08 */ s16 size;            /* its square in scene_puffSquares */
    /* 0x0A */ s16 sizeStep;
} puff_t;                           /* size 0xC */

extern s16 scene_drawHead;          /* first entry of the draw list, once Scene_ChainDrawList has chained the buckets */
extern s16 scene_numDraws;          /* entries in scene_drawList */
extern s16 scene_bucketHeads[DRAW_BUCKETS];  /* first entry of each bucket, or DRAW_NONE */
extern s16 scene_bucketTails[DRAW_BUCKETS];  /* last entry of each bucket */
extern drawEntry_t scene_drawList[];
extern u8 scene_surface;            /* surface type, 0 to 6 */
extern u8 scene_twoCycle;           /* 1: G_CYC_2CYCLE set, 0: G_CYC_1CYCLE */
extern u8 scene_zBuffer;            /* z-buffer on */
extern u8 scene_fog;                /* fog on */
extern u32 scene_renderMode1;       /* render mode of the first cycle */
extern u32 scene_renderMode2;       /* render mode of the second cycle */
extern u16 scene_powersOfTwo[];     /* 2, 4, ... 512 */
extern u8 scene_viewMask;           /* cameras that draw, a bit per index in D_8009CFA0 */
extern u8 scene_addSurface;         /* surface type of the entries Scene_AddToDrawList adds */
extern u16 scene_addMaterial;       /* their material (file index) */
extern u8 scene_addTwoCycle;        /* their cycle type (scene_twoCycle) */
extern u8 scene_addCullBack;        /* their face culling (scene_cullBack) */
extern u16 scene_addMesh;           /* their mesh (file index) */
extern u8 scene_addUcode;           /* their microcode (scene_ucode) */
extern u8 scene_addNoZBuffer;       /* 0: they use the z-buffer */
extern void *scene_addMatrix;       /* their model matrix, drawEntry_t.unk_08 */
extern u16 scene_addLayer;          /* nonzero: the layer of buckets they go in */
extern u8 scene_addByMaterial;      /* nonzero: they are keyed by material, else by microcode */
extern u8 scene_geometryChanged;    /* set: the geometry mode is set again (as when scene_fog changes) */
extern u8 scene_renderModeChanged;  /* set: the render mode is set again (as when scene_fog changes) */
extern Vp scene_viewport;           /* the screen's viewport, in the KNSTATICSYS file */
extern u16 scene_depthBuffer[];     /* depth buffer, in the low RAM */


extern rgba8_t scene_primColor;     /* the primitive colour set */
extern rgba8_t scene_envColor;      /* the environment colour set */
extern combine_t scene_combine;     /* the combiner of the last material */
extern u8 scene_combineChanged;     /* set: the combiner is set again */
extern u8 scene_texturing;          /* texturing on */
extern u8 scene_texGen;             /* texture generation on (MAT_TEXGEN) */
extern u8 scene_smooth;             /* MAT_SMOOTH of the last material */
extern u8 scene_lighting;           /* MAT_LIGHTING of the last material */
extern u8 scene_cullBack;           /* 1: cull back faces, 0: front faces */
extern u32 scene_geometryMode;      /* the geometry mode set when scene_geometryChanged is */
extern u16 scene_tmemNext;          /* tmem words of the first texture: where the second is loaded */
extern u16 scene_material;          /* the material (file index) last set up; 0xFFFF: none */
extern void *scene_modelMtx;        /* the model matrix last loaded */

void Scene_ClearDrawList( void );
void Scene_AddToDrawList( void );
void Scene_BeginView( sceneCamera_t *cam );
void Scene_Draw( void );
void Scene_ChainDrawList( void );
void Scene_BeginFrame( void );
void Scene_PickRenderModes( void );
s32 Scene_TextureSizeLog2( u32 value );
extern puff_t scene_puffs[NUM_PUFFS];
extern u16 scene_puffImages[2];     /* image files of the two kinds of puff */
extern Vtx scene_puffSquares[NUM_PUFF_SIZES * 4];  /* squares of four vertices, by size */
extern Mtx scene_puffMtx[2][NUM_PUFFS];  /* each puff turned to face camera 0, then camera 1 */
extern u8 scene_puffColors[2][3];   /* rgb of the two kinds of puff */
extern u8 scene_view;               /* index in D_8009CFA0 of the camera Scene_BeginView last set up; 2: none */
extern u8 scene_ucode;              /* microcode loaded: 0 gspF3DEX2_Rej_fifo, 1 gspF3DEX2_fifo, 2 gspL3DEX2_fifo */

void Scene_InitPuffs( void );
void Scene_FaceMatrix( f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz );
void Scene_StartPuff( u16 flags, f32 x, f32 y, f32 z );
void Scene_DrawPuffs( void );

#endif
