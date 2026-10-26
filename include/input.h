#ifndef INPUT_H
#define INPUT_H

/* input.h -- controllers and Rumble Paks (unit 2DF0) */

#include "common.h"
#include "libultra.h"

#define NUM_PLAYERS     2           /* controller ports the game reads */

extern OSMesgQueue D_80066B38;      /* serial interface events */
extern OSMesg D_800E4FCC;
extern u8 D_800619B8[];             /* its address is the serial event's message */
extern u8 D_8005B190;               /* bit per port with a standard controller */
extern OSContStatus D_800F7BE8[MAXCONTROLLERS];
extern OSPfs D_800E6400[NUM_PLAYERS];
extern u8 D_800E4FD0[NUM_PLAYERS];  /* TRUE when the port has a Rumble Pak */
extern s16 D_80066C24[NUM_PLAYERS];
extern u8 D_800DF680[NUM_PLAYERS];

void func_800279F0( void );
void func_80027A7C( s32 chan );
void func_80027B18( s32 chan );
void func_80027B6C( s32 chan );
void func_80027FF0( void );

#endif
