#ifndef CACODE_CACODE_H
#define CACODE_CACODE_H

/* cacode.h -- the cacode overlay (file CACODE  SYS, ROM 0x67370, loaded at 0x80112350) */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "filecache.h"
#include "hostio.h"
#include "archive.h"
#include "heap.h"
#include "ground.h"

/*
One slot of the cache of 320 x 240 BMP frames. The image is kept as 40 tiles of 64 x 32 RGBA16 texels, 5 across
and 8 down. The names of the type and its members are invented.
*/
typedef struct {
    /* 0x00 */ s16 frame;
    /* 0x02 */ s16 clip;
    /* 0x04 */ u32 stamp;               /* the slot with the smallest is reused; -1 is never smaller */
    /* 0x08 */ u16 ( *tiles )[32 * 64];
} frameSlot_t;                          /* size 0xC */

extern s32 D_80115360_cacode;           /* frames the D-pad has been held; frames step on their own from 7 */
extern s32 D_80115364_cacode;           /* the viewer's mode, a CAMODE_ value */
extern s32 D_80115368_cacode;           /* the selected camera and object (0 to 2) */
extern s32 D_8011536C_cacode;           /* the selected model part */
extern s32 D_80115370_cacode;           /* the frame shown */
extern s32 D_80115374_cacode[3];        /* [0]: frames of the animation; [1], [2]: first BMP frame of clips 1, 2 */
extern Vtx D_80115380_cacode[4];        /* the origin and the ends of the three axes, drawn as lines */

extern objParams_t D_801153C0_cacode[6];     /* the scene: three cameras, then three objects; or the host's sce.dat */
extern u8 D_80115540_cacode;            /* alpha of the frame drawn behind cameras 1 and 2 */

extern frameSlot_t D_80115630_cacode[35];
extern u8 D_801157D4_cacode;            /* slots in use */
extern sceneObjectNode_t *D_801157D8_cacode[3];     /* the scene's objects */
extern sceneCameraNode_t *D_801157E4_cacode[3];     /* the scene's cameras */
extern f32 D_801157F0_cacode[4][4];
extern f32 D_80115830_cacode[30][300][3];   /* by model part, per frame: the part's rotation (radians) */

/*
The motion tables, saved to and loaded from the host's "motion.dat" in this order, followed by D_80115374_cacode[1]
and [2]. Cameras and objects 1 and 2 follow the tables of index 0 and 1; camera 0 has the one matrix
D_801157F0_cacode. They lie contiguously after D_80115830_cacode, so they are presumably the overlay's own bss
(identity TODO: splat names them without the segment suffix, and the bss size is still an estimate).
*/
extern f32 D_8012FE10[2][300][4][4];    /* per frame, for objects 1 and 2 */
extern f32 D_80139410[2][300][3];       /* per frame: the at of cameras 1 and 2 */
extern f32 D_8013B030[2][300][3];       /* the eye */
extern f32 D_8013CC50[2][300][3];       /* the up point */
extern f32 D_8013E870[3];               /* the rotation of the selected part, kept while frames are stepped */
extern u8 *D_8013E880;                  /* buffer one BMP file is read into */

/* D_80115364_cacode */
#define CAMODE_ROTATE       0           /* the pad moves the camera, the object and the selected part */
#define CAMODE_FILE         1           /* A loads motion.dat, B saves it */
#define CAMODE_IMAGE        2           /* A and B move the first BMP frame of clip 1 (with Z: clip 2) */
#define CAMODE_COUNT        3

void func_8011268C_cacode( s32 frame, s32 clip );
void func_80112CDC_cacode( f32 src[4][4], f32 *trans, f32 *rot, f32 *scale );
void func_8011306C_cacode( float src[4][4], float dst[4][4] );

#endif
