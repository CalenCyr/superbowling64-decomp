/* mathexc.c -- libm's error path: offers the error to matherr, then sets errno */

#include "libc.h"

/*
================
func_80056690

Reports a libm error of the given SVID type for the function name with its arguments, and returns the
function's result: retval, or whatever matherr put there. Unless matherr claims the error, errno becomes
ERANGE for OVERFLOW and UNDERFLOW and EDOM for the rest.
================
*/
double func_80056690( int type, char *name, double arg1, double arg2, double retval ) {
    struct exception exc;

    exc.type = type;
    exc.name = name;
    exc.arg1 = arg1;
    exc.arg2 = arg2;
    exc.retval = retval;
    if ( !matherr( &exc ) ) {
        errno = ( type == OVERFLOW || type == UNDERFLOW ) ? ERANGE : EDOM;
    }
    return exc.retval;
}
