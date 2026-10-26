/* atan.c -- arctangent by CORDIC in 4.60 fixed point */

#include "libc.h"

#define CORDIC_STEPS 52                         /* one per bit of a double's mantissa */
#define FIX_ONE      1152921504606846976.0      /* 1.0 in 4.60 fixed point: 2^60 */

/* atan( 2^-i ) in 4.60 fixed point */
static long long atanTable[64] = {
    0x0C90FDAA22168C00LL, 0x076B19C1586ED3C0LL,
    0x03EB6EBF25901BA0LL, 0x01FD5BA9AAC2F6E0LL,
    0x00FFAADDB967EF50LL, 0x007FF556EEA5D894LL,
    0x003FFEAAB776E536LL, 0x001FFFD555BBBA97LL,
    0x000FFFFAAAADDDDBLL, 0x0007FFFF55556EEFLL,
    0x0003FFFFEAAAAB77LL, 0x0001FFFFFD55555BLL,
    0x0000FFFFFFAAAAAALL, 0x00007FFFFFF55555LL,
    0x00003FFFFFFEAAAALL, 0x00001FFFFFFFD555LL,
    0x00000FFFFFFFFAAALL, 0x000007FFFFFFFF55LL,
    0x000003FFFFFFFFEALL, 0x000001FFFFFFFFFDLL,
    0x000000FFFFFFFFFFLL, 0x0000007FFFFFFFFFLL,
    0x0000003FFFFFFFFFLL, 0x0000001FFFFFFFFFLL,
    0x0000000FFFFFFFFFLL, 0x00000007FFFFFFFFLL,
    0x00000003FFFFFFFFLL, 0x0000000200000000LL,
    0x0000000100000000LL, 0x0000000080000000LL,
    0x0000000040000000LL, 0x0000000020000000LL,
    0x0000000010000000LL, 0x0000000008000000LL,
    0x0000000004000000LL, 0x0000000002000000LL,
    0x0000000001000000LL, 0x0000000000800000LL,
    0x0000000000400000LL, 0x0000000000200000LL,
    0x0000000000100000LL, 0x0000000000080000LL,
    0x0000000000040000LL, 0x0000000000020000LL,
    0x0000000000010000LL, 0x0000000000008000LL,
    0x0000000000004000LL, 0x0000000000002000LL,
    0x0000000000001000LL, 0x0000000000000800LL,
    0x0000000000000400LL, 0x0000000000000200LL,
    0x0000000000000100LL, 0x0000000000000080LL,
    0x0000000000000040LL, 0x0000000000000020LL,
    0x0000000000000010LL, 0x0000000000000008LL,
    0x0000000000000004LL, 0x0000000000000002LL,
    0x0000000000000001LL, 0x0000000000000000LL,
    0x0000000000000000LL, 0x0000000000000000LL,
};

/*
================
func_800559A0

CORDIC vectoring: rotates the vector ( x0, y0 ), both 4.60 fixed point, onto the x axis and stores the angle
it turned through, atan( y0 / x0 ) in radians, at *angle.
================
*/
void func_800559A0( long long x0, long long y0, double *angle ) {
    long long x;
    long long y;
    long long z;
    long long nextX;
    int       i;

    x = x0;
    y = y0;
    z = 0;
    for ( i = 0; i < CORDIC_STEPS; i++ ) {
        if ( y >= 0 ) {
            nextX = x + ( y >> i );
            y -= x >> i;
            z += atanTable[i];
        } else {
            nextX = x - ( y >> i );
            y += x >> i;
            z -= atanTable[i];
        }
        x = nextX;
    }
    *angle = z / FIX_ONE;
}

/*
================
atan

Below 1e-7 atan( x ) is x. Above 1 the vector is ( 1 / x, 1 ), so both coordinates stay within 4.60.
================
*/
double atan( double x ) {
    int       sign;
    long long fixX;
    long long fixY;
    double    angle;

    if ( x < 0.0 ) {
        x = -x;
        sign = -1;
    } else {
        sign = 1;
    }
    if ( x > 1e-7 ) {
        if ( x >= 1.0 ) {
            fixY = 0x1000000000000000LL;
            fixX = 1.0 / x * FIX_ONE;
        } else {
            fixX = 0x1000000000000000LL;
            fixY = x * FIX_ONE;
        }
        func_800559A0( fixX, fixY, &angle );
    } else {
        angle = x;
    }
    return sign < 0 ? -angle : angle;
}

/*
================
atan2

atan( y / x ) placed in the quadrant of ( x, y ). atan2( 0, 0 ) is a DOMAIN error returning 0.
================
*/
double atan2( double y, double x ) {
    double angle;

    if ( x == 0.0 && y == 0.0 ) {
        return func_80056690( DOMAIN, "atan2", x, y, 0.0 );
    }
    if ( x == 0.0 ) {
        return y > 0.0 ? 1.57079632679489661923 : -1.57079632679489661923;
    }
    angle = atan( y / x );
    if ( x > 0.0 ) {
        return angle;
    }
    if ( y == 0.0 ) {
        return 3.14159265358979323846;
    }
    if ( y > 0.0 ) {
        return angle + 3.14159265358979323846;
    }
    return angle - 3.14159265358979323846;
}
