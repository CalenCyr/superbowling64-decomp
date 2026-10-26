/* atoi.c -- decimal string to integer */

#include "libc.h"

/*
================
atoi
================
*/
int atoi( const char *s ) {
    return atol( s );
}

/*
================
atol

Skips leading white space, takes one optional sign and reads decimal digits up to the first non-digit. There is
no overflow check.
================
*/
long atol( const char *s ) {
    long n;
    char sign;
    char c;

    n = 0;
    while ( isspace( *s ) ) {
        s++;
    }
    if ( ( sign = *s ) == '-' || sign == '+' ) {
        s++;
    }
    while ( isdigit( c = *s++ ) ) {
        n = n * 10 + ( c - '0' );
    }
    if ( sign == '-' ) {
        n = -n;
    }
    return n;
}
