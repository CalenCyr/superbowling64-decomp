#include "t.h"

#define TWICE( x ) ( ( x ) + ( x ) )

int Twice( Foo_t *foo ) {
    return TWICE( foo->unk_04 );                        /* want 1 keep 0 */
}

int Sum( Bar_t *bar, Foo_t *foo ) {
    return foo->unk_04 + bar->unk_04;                   /* want 1 keep 1 */
}

int Mixed( Bar_t *bar ) {
    return bar->unk_04 + bar->foo[1].unk_04 + bars[0].unk_04 + bars[1].foo[0] . unk_04; /* want 2 keep 2 */
}

#ifdef NON_MATCHING
int Parked( Foo_t *foo ) {
    return foo[1].unk_04;                               /* want 1 keep 0 */
}
#else
int Other( Bar_t *bar ) {
    return bar->unk_04;                                 /* want 0 keep 1 */
}
#endif
