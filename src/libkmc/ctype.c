/* ctype.c -- the character class table behind the ctype macros of libc.h */

#include "libc.h"

#define _S _CTYPE_SPACE
#define _U _CTYPE_UPPER
#define _L _CTYPE_LOWER
#define _N _CTYPE_DIGIT
#define _X _CTYPE_HEX
#define _C _CTYPE_CNTRL
#define _P _CTYPE_PUNCT
#define _K _CTYPE_KANJI

/*
================
_ctype

One set of _CTYPE_* bits per character code. Codes 0x80-0xFF are not ASCII: the Shift-JIS lead bytes carry
_CTYPE_KANJI. The name is invented.
================
*/
u8 _ctype[256] = {
    _C, _C, _C, _C, _C, _C, _C, _C,   /* 00 */
    _C, _C|_S, _C|_S, _C|_S, _C|_S, _C|_S, _C, _C,   /* 08 */
    _C, _C, _C, _C, _C, _C, _C, _C,   /* 10 */
    _C, _C, _C, _C, _C, _C, _C, _C,   /* 18 */
    _S, _P, _P, _P, _P, _P, _P, _P,   /* 20 */
    _P, _P, _P, _P, _P, _P, _P, _P,   /* 28 */
    _N, _N, _N, _N, _N, _N, _N, _N,   /* 30 */
    _N, _N, _P, _P, _P, _P, _P, _P,   /* 38 */
    _P, _U|_X, _U|_X, _U|_X, _U|_X, _U|_X, _U|_X, _U,   /* 40 */
    _U, _U, _U, _U, _U, _U, _U, _U,   /* 48 */
    _U, _U, _U, _U, _U, _U, _U, _U,   /* 50 */
    _U, _U, _U, _P, _P, _P, _P, _P,   /* 58 */
    _P, _L|_X, _L|_X, _L|_X, _L|_X, _L|_X, _L|_X, _L,   /* 60 */
    _L, _L, _L, _L, _L, _L, _L, _L,   /* 68 */
    _L, _L, _L, _L, _L, _L, _L, _L,   /* 70 */
    _L, _L, _L, _P, _P, _P, _P, _C,   /* 78 */
    0, _K, _K, _K, _K, _K, _K, _K,   /* 80 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* 88 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* 90 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* 98 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* A0 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* A8 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* B0 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* B8 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* C0 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* C8 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* D0 */
    0, 0, 0, 0, 0, 0, 0, 0,   /* D8 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* E0 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* E8 */
    _K, _K, _K, _K, _K, _K, _K, _K,   /* F0 */
    _K, _K, _K, _K, _K, 0, 0, 0,   /* F8 */
};
