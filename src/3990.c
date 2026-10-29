/* 3990.c -- references to library routines that only the code overlays call, so that the link keeps them */

#include "common.h"

/*
RULE-EXCEPTION: R25, the calls below pass no arguments (retail sets up no argument registers), which C allows
only without prototypes; so this unit declares these routines old-style instead of including their headers.
sqrtf alone is called with an argument (0.0, in $f12) and so with its prototype.
*/
u32 guRandom();
s16 sins();
s16 coss();
float sqrtf( float value );
void guPerspective();
void guLookAt();
void guMtxXFML();
void guPosition();
void guMtxIdent();
void guRotate();
s32 sprintf();
void guScale();
void spScissor();
void *spDraw();
void spMove();
void spSetAttribute();
void spFinish();
void guOrtho();
void guRotateRPY();
void *memset();
void spColor();
void guTranslateF();
int memcmp();
void *memmove();
void osViSetMode();
void osViSetSpecialFeatures();
u32 osGetMemSize();
int rand();
void guLookAtReflect();
double atan();
double asin();
char *strcat();

/*
================
func_80028590

Never called: it only refers to each routine once, for the overlays.
================
*/
void func_80028590( void ) {
    guRandom();
    sins();
    coss();
    sqrtf( 0.0f );
    guPerspective();
    guLookAt();
    guMtxXFML();
    guPosition();
    guMtxIdent();
    guRotate();
    sprintf();
    guScale();
    spScissor();
    spDraw();
    spMove();
    spSetAttribute();
    spMove();
    spFinish();
    guOrtho();
    guRotateRPY();
    memset();
    spColor();
    guTranslateF();
    memcmp( 0, 0, 0 );
    memmove();
    osViSetMode();
    osViSetSpecialFeatures();
    osGetMemSize();
    rand();
    guLookAtReflect();
    atan();
    asin();
    strcat();
}
