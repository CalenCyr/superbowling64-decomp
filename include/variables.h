#ifndef VARIABLES_H
#define VARIABLES_H

#include "common.h"
#include "structs.h"

extern s32 D_8005CE4C;
extern s16 D_8006434C;
extern s16 D_8006434E;
extern s32 D_80064334;              /* number of entries in D_8006433C */
extern UnkChannel *D_8006433C;
extern UnkChannel *D_80064340;      /* &D_8006433C[4] */
extern s32 D_80064344;
extern s32 D_80064348;
extern s32 D_80064350;              /* next channel id */
extern s32 D_80064354;              /* random number state */
extern UnkSoundBank *D_80064358;
extern UnkSoundBank *D_8006435C;
extern s32 D_80064360;
extern s32 D_80064364;
extern UnkSeqBank *D_80064368;
extern UnkSeqBank *D_8006436C;          /* used once, by the next func_8003B930, instead of D_80064368 */
extern void (*D_80064370)( s32 id, s32 value );
/* ring buffer of D_8006437C messages: func_8003C3D4 writes at D_80064378 */
extern s32 D_80064374;
extern s32 D_80064378;
extern s32 D_8006437C;
extern UnkMsg8003C3D4 *D_80064380;

#endif
