#include "common.h"
#include "functions.h"
#include "variables.h"

/* Genuine code the [match] rules must not flag. */

#define CLEAR(p, n) do { u8 *q_ = (u8 *)(p); s32 n_ = (n); while (n_--) *q_++ = 0; } while (0)
#define HW_REG(addr) (*(volatile u32 *)(addr)) // volatile: memory-mapped register

typedef struct Base { s32 kind; } Base;
typedef struct Derived { Base base; s32 extra; } Derived;   /* C inheritance idiom */

static s32 sCount;

/* Copies a string; the era's idioms. */
void func_80000060(u8 *dst, u8 *src, s32 n) {
    s32 i;

    while ((*dst++ = *src++)) {
    }
    for (i = 0; i < n; i++)
        ;
    while (--n) ;
    switch (n) {
        case 0: ;
            break;
        default:
            break;
    }
done: ;
    CLEAR(dst, 4);
    HW_REG(0xA4600010) = 3;
    sCount = sCount + 1;
}

/* Reads a header through its base; byte arithmetic on a parser buffer. */
s32 func_80000064(Derived *d, u8 *buf) {
    Base *b = &d->base;
    s32 tmp;
    s32 x = *(s32 *)(buf + 8);

    tmp = b->kind;
    return tmp + x + d->extra + D_80064364;
}
