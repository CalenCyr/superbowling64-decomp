#include "common.h"
#include "structs.h"

/* A local copy of a header type. */
typedef struct {
    u8 cmd;
    s32 arg;
} UnkMsg8003C3D4; // expect: R1

typedef struct Whole {
    s32 count;
    f32 scale;
    s32 flags;
} Whole;

/* A layout view: the leading members of Whole, read through a cast. */
typedef struct WholeHead {
    s32 count;
    f32 scale;
} WholeHead;

static Whole sWhole;

s32 func_80000000(void) {
    return ((WholeHead *)&sWhole)->count; // expect: R1
}

union Tagged { s32 i; };
struct Tagged *func_80000004(void); // expect: R1
