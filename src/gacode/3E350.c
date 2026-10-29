/* 3E350.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "gacode/gacode.h"

/*
================
func_80114130_gacode
================
*/
void func_80114130_gacode( void ) {
}

/*
================
func_80114138_gacode
================
*/
void func_80114138_gacode( void ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114140_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801144C4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114550_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114640_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114898_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114CB4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114DD4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114EC4_gacode);

/*
================
func_80114FC4_gacode

Runs the unk_B08 handler of the player's lane.
================
*/
void func_80114FC4_gacode( s32 player ) {
    D_8012D658_gacode[player].lane->unk_B08( player );
}

/*
================
func_80114FFC_gacode
================
*/
void func_80114FFC_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    sceneObj_t *obj = pl->obj;

    if ( obj->unk_4A != 0 && obj->unk_48 != 0 ) {
        obj->unk_48--;
    }
    func_80120D7C_gacode( &pl->lane->unk_B18 );
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115054_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801150B4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801151A0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801155BC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801156D8_gacode);

/*
================
func_801157F8_gacode

A game mode.
================
*/
s32 func_801157F8_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->lane->unk_B25 = 0;
    pl->lane->unk_B24 = 1;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011582C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115B9C_gacode);

/*
================
func_80115CDC_gacode
================
*/
s32 func_80115CDC_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    if ( pl->lane->unk_B24 != 0 ) {
        pl->lane->unk_B04 = func_80116248_gacode;
        return 1;
    }
    pl->lane->unk_B04 = func_801163A0_gacode;
    pl->lane->unk_B24 = 0;
    return 0;
}

/*
================
func_80115D38_gacode
================
*/
s32 func_80115D38_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->lane->unk_B04 = func_80116248_gacode;
    pl->lane->unk_B24 = 0;
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115D70_gacode);

/*
================
func_80115DF4_gacode
================
*/
s32 func_80115DF4_gacode( s32 player ) {
    return D_8012D658_gacode[player].lane->unk_B10 == func_80118F08_gacode;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115E24_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115ED4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115FCC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116170_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116248_gacode);

/*
================
func_80116398_gacode

An unk_B04 handler that does nothing.
================
*/
void func_80116398_gacode( s32 player ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801163A0_gacode);

/*
================
func_80116414_gacode

An unk_B04 handler that does nothing.
================
*/
void func_80116414_gacode( s32 player ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011641C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011669C_gacode);

/*
================
func_80116750_gacode
================
*/
void func_80116750_gacode( gaLane_t *lane ) {
    s32 i;

    lane->bodies[0].unk_5D = 0;
    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->unk_4A = ( lane->bodies[i].unk_5D == 1 ) ? 3 : 0;
    }
}

/*
================
func_80116798_gacode

Hides the pins.
================
*/
void func_80116798_gacode( gaLane_t *lane ) {
    s32 i;

    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->unk_4A = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801167C0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801168D0_gacode);

/*
================
func_80116998_gacode
================
*/
s32 func_80116998_gacode( gaPlayer_t *pl ) {
    if ( pl->lane->unk_428 >= 0.0f ) {
        func_80120D7C_gacode( &pl->lane->unk_B18 );
        pl->lane->unk_B10 = func_801189C4_gacode;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801169F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116A64_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116BA4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116DC8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116FF0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117124_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801178D8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801179FC_gacode);

/*
================
func_80117ABC_gacode
================
*/
s32 func_80117ABC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    D_8012D658_gacode[player].obj->unk_4A = 0;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117AE0_gacode);

INCLUDE_RODATA("asm/nonmatchings/gacode/3E350", D_8012CA20_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117BDC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117D20_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117EFC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118014_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118270_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011840C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118524_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801187D0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801189C4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118CCC_gacode);

/*
================
func_80118D9C_gacode
================
*/
s32 func_80118D9C_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->obj->unk_48 = 0;
    pl->obj->unk_4A = 0;
    pl->lane->unk_B08 = func_8011B400_gacode;
    pl->lane->unk_B0C = func_8011C0DC_gacode;
    return D_8012D23C_gacode( player );
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118E04_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118EA0_gacode);

/*
================
func_80118F08_gacode
================
*/
s32 func_80118F08_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->obj->unk_4A = 0;
    pl->obj->unk_48 = 0;
    return 0;
}

/*
================
func_80118F38_gacode
================
*/
s32 func_80118F38_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->obj->unk_4A = 0;
    pl->obj->unk_48 = 0;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80118F68_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011917C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119328_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801194D0_gacode);

INCLUDE_RODATA("asm/nonmatchings/gacode/3E350", D_8012CA78_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119638_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119800_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801198E4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119ACC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119C50_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119DD8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119F8C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A26C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A404_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A8AC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011AA78_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011AB0C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011AD5C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B07C_gacode);

INCLUDE_RODATA("asm/nonmatchings/gacode/3E350", D_8012CB38_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B1A0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B2E8_gacode);

/*
================
func_8011B400_gacode

An unk_B08 handler that does nothing.
================
*/
void func_8011B400_gacode( s32 player ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B408_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B490_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B564_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B5C8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B634_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B8AC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B974_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BA00_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BA8C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BB44_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BC10_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BCA8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BD34_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C0DC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C158_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C254_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C468_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C57C_gacode);

/*
================
func_8011C650_gacode

An unk_B0C handler that leaves the camera as it is.
================
*/
void func_8011C650_gacode( s32 player, gaCamera_t *camera ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C658_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C93C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011CEE4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011CF30_gacode);
