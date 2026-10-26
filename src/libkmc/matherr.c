/* matherr.c -- the default SVID math-error hook */

#include "libc.h"

/*
================
matherr

Called with each libm error before errno is set; a nonzero return claims the error and leaves errno alone. The
default claims none.
================
*/
int matherr( struct exception *exc ) {
    return 0;
}
