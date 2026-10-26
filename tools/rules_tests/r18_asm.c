#include "common.h"

#define GET_SR(x) __asm__("mfc0 %0, $12" : "=r"(x)) // expect: R18

__asm__(".word 0"); // expect: R18

u32 func_80000050(void) {
    u32 sr;

    __asm__ volatile("mfc0 %0, $12" : "=r"(sr)); // expect: R18
    return sr;
}

u32 func_80000054(void) {
    u32 sr;

    GET_SR(sr); // expect-pending
    return sr;
}
