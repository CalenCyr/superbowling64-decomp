#include "common.h"

typedef struct Padded {
    s32 a;
    u8 pad[4]; // expect: R17
    s32 _pad2; // expect: R17
    s32 b;
} Padded;

volatile s32 gCarrier; // expect: R17 R2

s32 func_80000040(Padded *p, s32 x) {
    volatile s32 v = x; // expect: R17
    s32 unused; // expect: R17
    s32 dummy; // expect: R17
    s32 i;

    dummy = x;
    if (x) {} // expect: R17
    if (x) { // expect: R17
    } else { // expect: R17
    }
    ; // expect: R17
    if (x)
        ; // expect: R17
    {} // expect: R17
    do { x++; } while (0); // expect: R17
    x = x; // expect: R17
    p->a = p->a; // expect: R17
    x = x + 0; // expect: R17
    x = p->b * 1; // expect: R17
    for (i = 0; i < 4; i++) {
    }
    return v + p->b;
}

s32 func_80000044(Padded *p) {
    return p->a; // expect-pending
}
