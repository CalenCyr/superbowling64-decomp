#ifndef LIBC_H
#define LIBC_H

/* libc.h -- KMC libkmc: the C runtime, libm and the SVID math-error hook linked after libultra */

#include "common.h"

/* ===== ctype ===== */

/* _ctype[c] class bits (the table is indexed by the character itself, with no EOF slot) */
#define _CTYPE_SPACE 0x01 /* space, \t \n \v \f \r */
#define _CTYPE_UPPER 0x02
#define _CTYPE_LOWER 0x04
#define _CTYPE_DIGIT 0x08
#define _CTYPE_HEX   0x10 /* A-F, a-f */
#define _CTYPE_CNTRL 0x20
#define _CTYPE_PUNCT 0x40
#define _CTYPE_KANJI 0x80 /* Shift-JIS lead byte: 0x81-0x9F, 0xE0-0xFC */

extern u8 _ctype[256];

#define isspace( c ) ( _ctype[(u8)( c )] & _CTYPE_SPACE )
#define isdigit( c ) ( _ctype[(u8)( c )] & _CTYPE_DIGIT )

/* ===== errno ===== */

#define EDOM   33
#define ERANGE 34

extern int errno;

/* ===== string and memory ===== */

typedef unsigned int size_t;

void *memcpy( void *dst, const void *src, size_t n ); /* libultra's */
int memcmp( const void *s1, const void *s2, size_t n );
void *memmove( void *dst, const void *src, size_t n );
void *memset( void *dst, int c, size_t n );
void *func_80055FE0( void *dst, const void *src, size_t n );
void func_8005612C( const void *src, void *dst, size_t n );
void *func_800561F0( void *dst, size_t n, int c );
void strcpy( char *dst, const char *src ); /* the game's own (unit 4890); returns nothing */
char *strcat( char *dst, const char *src );
int strncmp( const char *s1, const char *s2, size_t n );

/* ===== stdlib ===== */

#define RAND_MAX 0x7FFF

int atoi( const char *s );
long atol( const char *s );
int rand( void );
void srand( unsigned int seed );

/* ===== math ===== */

/* SVID exception types (struct exception.type) */
#define DOMAIN    1
#define SING      2
#define OVERFLOW  3
#define UNDERFLOW 4
#define TLOSS     5
#define PLOSS     6

struct exception {
    int    type;
    char  *name;
    double arg1;
    double arg2;
    double retval;
};

int matherr( struct exception *exc );
double func_80056690( int type, char *name, double arg1, double arg2, double retval );

double asin( double x );
double acos( double x );
double atan( double x );
double atan2( double y, double x );
double sqrt( double x );
void func_800559A0( long long x0, long long y0, double *angle );

#endif
