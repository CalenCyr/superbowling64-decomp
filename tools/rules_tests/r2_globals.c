#include "common.h"
#include "variables.h"

extern s16 D_8006434C; // expect: R2
s32 snd_fxType; /* owner definition: fine, it has its header extern */
s32 gNoHeader; // expect: R2
static s32 sPrivate;

void func_80000010(void) {
    extern s32 gHidden; // expect: R2
    gHidden = sPrivate;
    gNoHeader = D_8006434C;
}
