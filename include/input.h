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

/* per-port pad state, rebuilt each frame by func_80027BC0 */
extern OSContPad D_80066B58[MAXCONTROLLERS];
extern s8 D_800692D0[NUM_PLAYERS];  /* stick x, 0 inside the dead zone */
extern s8 D_800A2FD8[NUM_PLAYERS];  /* stick y, 0 inside the dead zone */
extern u16 D_800E57E4[NUM_PLAYERS]; /* buttons held */
extern u16 D_8009D198[NUM_PLAYERS]; /* buttons held last frame */
extern u16 D_800E44E0[NUM_PLAYERS]; /* buttons pressed this frame */
extern u16 D_800619B0[NUM_PLAYERS]; /* stick as D-pad bits */
extern u16 D_8005B1A0[NUM_PLAYERS]; /* the same, last frame */
extern u16 D_800E6760[NUM_PLAYERS]; /* D-pad presses, with auto-repeat */
extern s32 D_8005B1A4[NUM_PLAYERS]; /* frames a direction has been held */
extern s32 D_8005B194;              /* set when port 1 has no controller */
/* both ports merged: port 1's value, or port 2's when port 1's is 0 */
extern u16 D_800994BC;
extern u16 D_800E4D62;
extern u16 D_800E6758;
extern s8 D_8009D05C;
extern s8 D_800DF628;
extern s32 D_8005C0EC;
extern u32 D_800693B0;
extern s32 D_8005B0F4;

void func_800279F0( void );
void func_80027A7C( s32 chan );
void func_80027B18( s32 chan );
void func_80027B6C( s32 chan );
void func_80027BC0( void );
void func_80027FF0( void );

/* unit 12C70 */
void func_80037870( s32 arg0, s32 arg1 );

#endif
