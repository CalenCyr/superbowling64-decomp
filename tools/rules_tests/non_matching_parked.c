/* A parked draft inside #ifdef NON_MATCHING is exempt from every rule; the INCLUDE_ASM branch is what builds. */
#include "include_asm.h"

#ifdef NON_MATCHING
int parked_draft( int x ) {
    int unused;
    volatile int carrier = x;
    if ( x ) {
    }
    return *( int * )( ( char * )&carrier + 0 );
}
#else
INCLUDE_ASM("asm/nonmatchings/fixture", parked_draft);
#endif

int after_draft( int x ) {
    return x + 1;
}
