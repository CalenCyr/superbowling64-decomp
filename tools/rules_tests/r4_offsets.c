#include "common.h"

#define OBJ_FLAGS(obj) (*(s32 *)((u8 *)(obj) + 0x10)) // expect: R4

typedef struct Obj { s32 a; } Obj;

f32 func_80000020(Obj *obj) {
    *(s32 *)((u8 *)obj + 0x10) |= 1; // expect: R4
    ((Obj *)((char *)obj + 8))->a = 0; // expect: R4
    *((u8 *)obj + 3) = 0; // expect: R4
    return *(f32 *)((unsigned char *)obj + 0x58); // expect: R4
}

s32 func_80000024(Obj *obj) {
    return OBJ_FLAGS(obj); // expect-pending
}
