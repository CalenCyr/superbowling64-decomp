/* rand.c -- the linear congruential generator */

#include "libc.h"

static int next; /* the generator state; zero until srand */

/*
================
rand

Steps next = next * 1103515245 + 12345 and returns bits 16-30 of next + 1. The + 1 is KMC's.
================
*/
int rand( void ) {
    next = next * 1103515245 + 12345;
    return ( ( next + 1 ) >> 16 ) & RAND_MAX;
}

/*
================
srand
================
*/
void srand( unsigned int seed ) {
    next = seed;
}
