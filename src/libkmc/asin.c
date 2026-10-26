/* asin.c -- arcsine and arccosine through atan2 */

#include "libc.h"

/*
================
asin

atan2( x, sqrt( 1 - x * x ) ); outside [-1, 1] a DOMAIN error returning 0.
================
*/
double asin( double x ) {
    if ( x > 1.0 || x < -1.0 ) {
        return func_80056690( DOMAIN, "asin", x, 0.0, 0.0 );
    }
    return atan2( x, sqrt( 1.0 - x * x ) );
}

/*
================
acos

pi / 2 - asin( x ); outside [-1, 1] a DOMAIN error returning 0.
================
*/
double acos( double x ) {
    if ( x > 1.0 || x < -1.0 ) {
        return func_80056690( DOMAIN, "acos", x, 0.0, 0.0 );
    }
    return 1.57079632679489661923 - asin( x );
}
