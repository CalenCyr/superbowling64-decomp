#include "common.h"

/* R13 is n/a for C: a struct of function pointers is not a vtable and never fires. */
typedef struct Ops {
    void (*init)(void);
    void (*update)(s32 dt);
} Ops;

void func_80000030(Ops *ops) {
    ops->init();
}
