/* sqrt.c -- square root by Newton's method on the mantissa */

#include "libc.h"

/* A double's words, high (sign, exponent, top of the mantissa) first. */
typedef union {
    double d;
    struct {
        u32 hi;
        u32 lo;
    } w;
} DoubleBits;

typedef union {
    float f;
    u32   w;
} FloatBits;

#define DBL_SIGN_MANT 0x800FFFFF /* the high word without its exponent */
#define DBL_EXP       0x7FF00000
#define DBL_EXP_ONE   0x00100000
#define DBL_BIAS      0x3FF

/* the float without (and with only) exponent bits 23-29: KMC leaves bit 30, the top exponent bit, out of the field,
   which is harmless for the estimates here, all below 2 */
#define FLT_SIGN_MANT 0xC07FFFFF
#define FLT_EXP       0x3F800000
#define FLT_EXP_ONE   0x00800000

/*
================
sqrt

Splits x into an even power of two and a mantissa m in [0.25, 1), estimates sqrt( m ) in float from a linear
fit, refines it with one float and two double Newton steps ( r = ( r + m / r ) / 2, halving by decrementing the
exponent field) and gives the result half of x's exponent. A negative x is a DOMAIN error returning 0; a zero
or denormal x returns 0.
================
*/
double sqrt( double x ) {
    DoubleBits root;
    DoubleBits m;
    FloatBits  mf;
    FloatBits  est;
    float      y;
    int        e;
    int        rootExp;
    u32        expBits;
    int        i;

    m.d = x;
    root.d = x;
    if ( (int)root.w.hi < 0 ) {
        return func_80056690( DOMAIN, "sqrt", x, 0.0, 0.0 );
    }
    e = ( root.w.hi >> 20 ) & 0x7FF;
    if ( e == 0 ) {
        return 0.0;
    }
    e -= DBL_BIAS;
    root.w.hi = ( root.w.hi & DBL_SIGN_MANT ) | 0x3FD00000;
    y = root.d;
    est.f = y;
    est.w = ( est.w & FLT_SIGN_MANT ) | 0x3D000000;
    if ( e & 1 ) {
        e--;
        rootExp = e >> 1;
        est.f = y + est.f + 0.4375;
        m.w.hi = ( m.w.hi & DBL_SIGN_MANT ) | 0x3FE00000;
    } else {
        rootExp = e >> 1;
        est.f = y - est.f + 0.28125;
        m.w.hi = ( m.w.hi & DBL_SIGN_MANT ) | 0x3FD00000;
    }
    mf.f = m.d;
    est.f = est.f + mf.f / est.f;
    expBits = est.w & FLT_EXP;
    est.w = ( est.w & FLT_SIGN_MANT ) | ( ( expBits - FLT_EXP_ONE ) & FLT_EXP );
    root.d = est.f;
    for ( i = 0; i < 2; i++ ) {
        root.d = root.d + m.d / root.d;
        expBits = root.w.hi & DBL_EXP;
        root.w.hi = ( root.w.hi & DBL_SIGN_MANT ) | ( ( expBits - DBL_EXP_ONE ) & DBL_EXP );
    }
    root.w.hi = ( root.w.hi & DBL_SIGN_MANT ) | ( ( ( rootExp + DBL_BIAS ) & 0x7FF ) << 20 );
    return root.d;
}
