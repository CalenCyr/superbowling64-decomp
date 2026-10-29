/* 3E350.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
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

/*
================
func_801144C4_gacode

Stops the sound of lanes 0 to lastLane and parks their unk_B10 handlers.
================
*/
void func_801144C4_gacode( u8 lastLane ) {
    s32 i;

    for ( i = 0; i <= lastLane; i++ ) {
        func_80120D7C_gacode( &D_8012DCF0_gacode[i].unk_B18 );
        D_8012DCF0_gacode[i].unk_B10 = func_80118F38_gacode;
    }
}

/*
================
func_80114550_gacode

Sets up a player: its controller port (-1: played by the game), character and lane.
================
*/
void func_80114550_gacode( s32 player, s8 port, u8 character, u8 lane ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->lane = &D_8012DCF0_gacode[lane];
    pl->laneIndex = lane;
    pl->port = port;
    memmove( pl->unk_48, pl->obj->mtx, sizeof( pl->unk_48 ) );
    memmove( &pl->character, &D_80123BE0_gacode[character], sizeof( gaCharacter_t ) );
    pl->unk_93 = D_80123BE0_gacode[character].unk_04;
    func_80116248_gacode( player );
    func_8011B8AC_gacode( player, pl->lane->camera );
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114640_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80114898_gacode);

/*
================
func_80114CB4_gacode

Places a body at its spot, offset by offset along x, and sets its object's unk_44 from the spot's height.
================
*/
void func_80114CB4_gacode( gaBody_t *body, gaSpot_t *spot, sceneObj_t *obj, f32 offset ) {
    f32 height;

    if ( obj->unk_4A == 0 ) {
        return;
    }
    body->pos[0] = spot->pos[0] + offset;
    body->pos[1] = spot->pos[1];
    body->pos[2] = spot->pos[2];
    func_8011D2C0_gacode( obj->mtx, body->pos, body->rot, body->scale );
    if ( spot->pos[2] <= -1186.0f ) {
        obj->unk_44 = -1.0f;
        return;
    }
    switch ( body->unk_5C ) {
    case 0:
        height = 11.25f;
        break;
    case 1:
        height = 6.25f;
        break;
    case 2:
        height = 0.0f;
        break;
    }
    obj->unk_44 = spot->pos[1] - height;
}

/*
================
func_80114DD4_gacode

Publishes the ball of the player's lane to main: its position, and in D_800DF595 its state (0: no ball,
1 or 2 (the lane in func_80118F68_gacode): unk_454 bit 1 set, 3: below -190).
================
*/
void func_80114DD4_gacode( gaPlayer_t *pl ) {
    gaLane_t *lane = pl->lane;
    gaSpot_t *spot = &lane->spots[0];
    u8 state;

    D_800E6560[pl->laneIndex][0] = lane->bodies[0].pos[0];
    D_800E6560[pl->laneIndex][1] = lane->bodies[0].pos[1];
    D_800E6560[pl->laneIndex][2] = lane->bodies[0].pos[2];
    if ( lane->bodies[0].unk_5D == 0 ) {
        D_800DF595[pl->laneIndex] = 0;
        return;
    }
    if ( ( lane->unk_454 >> 1 ) & 1 ) {
        if ( pl->lane->unk_B10 == func_80118F68_gacode ) {
            state = 2;
        } else {
            state = 1;
        }
    } else {
        state = 0;
    }
    if ( spot->pos[1] <= -190.0f ) {
        state = 3;
    }
    D_800DF595[pl->laneIndex] = state;
}

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

/*
================
func_80115054_gacode

Sets the camera's unk_24 to -1, clears the players' unk_92 and hides scene objects 120 to 133 and 141.
================
*/
void func_80115054_gacode( void ) {
    gaPlayer_t *pl;
    s32 player;
    s32 i;

    ( (gaCamera_t *)D_800EA120[1] )->unk_24 = -1;
    for ( player = MAX_PLAYERS - 1; player >= 0; player-- ) {
        pl = &D_8012D658_gacode[player];
        pl->unk_92 = 0;
    }
    for ( i = 0; i < 14; i++ ) {
        ( (sceneObj_t *)D_800EA120[120 + i] )->unk_4A = 0;
    }
    ( (sceneObj_t *)D_800EA120[141] )->unk_4A = 0;
}

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

/*
================
func_80115D70_gacode
================
*/
void func_80115D70_gacode( s32 player, u8 arg1 ) {
    gaLane_t *lane = D_8012D658_gacode[player].lane;
    s32 i;

    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->unk_4A = 0;
    }
    lane->unk_B04 = func_80116414_gacode;
    lane->unk_B08 = func_8011B564_gacode;
    lane->unk_B0C = func_8011BD34_gacode;
    if ( arg1 ) {
        lane->unk_B10 = func_80118F08_gacode;
    } else {
        lane->unk_B10 = func_80118F38_gacode;
    }
}

/*
================
func_80115DF4_gacode
================
*/
s32 func_80115DF4_gacode( s32 player ) {
    return D_8012D658_gacode[player].lane->unk_B10 == func_80118F08_gacode;
}

/*
================
func_80115E24_gacode

When the players of every lane are done (unk_B10 parked), stops their unk_B08 and unk_B0C handlers too.
================
*/
s32 func_80115E24_gacode( u8 twoLanes ) {
    if ( twoLanes ) {
        if ( D_8012DCF0_gacode[0].unk_B10 != func_80118F38_gacode ) {
            return 0;
        }
        if ( D_8012DCF0_gacode[1].unk_B10 != func_80118F38_gacode ) {
            return 0;
        }
        D_8012DCF0_gacode[1].unk_B08 = func_8011B400_gacode;
        D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
        D_8012DCF0_gacode[1].unk_B0C = func_8011C650_gacode;
        D_8012DCF0_gacode[0].unk_B0C = func_8011C650_gacode;
        return 1;
    }
    if ( D_8012DCF0_gacode[0].unk_B10 != func_80118F38_gacode ) {
        return 0;
    }
    D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
    D_8012DCF0_gacode[0].unk_B0C = func_8011C650_gacode;
    return 1;
}

/*
================
func_80115ED4_gacode

When the players of every lane are done, or wait in func_80118F08_gacode (whose lanes then restart at
func_80116248_gacode), stops their unk_B08 handlers; returns 1 then.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: control flow; retail shares the two-lane unk_B08 stores between both cases (the first jumps into
// the second's tail with the result already 1); no spelling tried yet produces that tail merge
s32 func_80115ED4_gacode( u8 twoLanes ) {
    if ( twoLanes ) {
        if ( D_8012DCF0_gacode[0].unk_B10 == func_80118F38_gacode && D_8012DCF0_gacode[1].unk_B10 == D_8012DCF0_gacode[0].unk_B10 ) {
            D_8012DCF0_gacode[1].unk_B08 = func_8011B400_gacode;
            D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
            return 1;
        }
        if ( D_8012DCF0_gacode[0].unk_B10 == func_80118F08_gacode && D_8012DCF0_gacode[1].unk_B10 == D_8012DCF0_gacode[0].unk_B10 ) {
            D_8012DCF0_gacode[1].unk_B04 = func_80116248_gacode;
            D_8012DCF0_gacode[0].unk_B04 = func_80116248_gacode;
            D_8012DCF0_gacode[1].unk_B08 = func_8011B400_gacode;
            D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
            return 1;
        }
        return 0;
    }
    if ( D_8012DCF0_gacode[0].unk_B10 == func_80118F38_gacode ) {
        D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
        return 1;
    }
    if ( D_8012DCF0_gacode[0].unk_B10 == func_80118F08_gacode ) {
        D_8012DCF0_gacode[0].unk_B04 = func_80116248_gacode;
        D_8012DCF0_gacode[0].unk_B08 = func_8011B400_gacode;
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115ED4_gacode);
#endif

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80115FCC_gacode);

/*
================
func_80116170_gacode

Once both lanes wait in func_80118EA0_gacode, ends the turn of players 0 and 1: with skip set, parks
their lanes' input; else first runs func_80121E34_gacode and sets the lanes' view handlers.
================
*/
void func_80116170_gacode( s32 skip ) {
    gaPlayer_t *pl;
    s32 i;

    if ( D_8012DCF0_gacode[0].unk_B10 != func_80118EA0_gacode || D_8012DCF0_gacode[1].unk_B10 != D_8012DCF0_gacode[0].unk_B10 ) {
        return;
    }
    D_8012D8D8_gacode = 0;
    if ( !skip ) {
        func_80121E34_gacode();
    }
    for ( i = 0; i < 2; i++ ) {
        pl = &D_8012D658_gacode[i];
        if ( skip ) {
            pl->lane->unk_B10 = func_80118F38_gacode;
        } else {
            pl->lane->unk_B28 = 2700.0f;
            pl->lane->unk_B0C = func_8011C650_gacode;
            pl->lane->unk_B04 = func_80116248_gacode;
        }
        pl->lane->unk_B08 = func_8011B400_gacode;
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116248_gacode);

/*
================
func_80116398_gacode

An unk_B04 handler that does nothing.
================
*/
void func_80116398_gacode( s32 player ) {
}

/*
================
func_801163A0_gacode

An unk_B04 handler: passes which pins stand to func_8011641C_gacode, then idles.
================
*/
void func_801163A0_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    u8 pins[NUM_PINS];
    gaBody_t *body = &pl->lane->bodies[1];
    s32 i;

    for ( i = 0; i < NUM_PINS; i++ ) {
        pins[i] = body->unk_5D;
        body++;
    }
    func_8011641C_gacode( player, pins );
    pl->lane->unk_B04 = func_80116414_gacode;
}

/*
================
func_80116414_gacode

An unk_B04 handler that does nothing.
================
*/
void func_80116414_gacode( s32 player ) {
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011641C_gacode);

/*
================
func_8011669C_gacode

Sets the handlers of a player's turn for the game mode.
================
*/
void func_8011669C_gacode( gaPlayer_t *pl ) {
    if ( D_8012CDF4_gacode == func_801156D8_gacode || D_8012CDF4_gacode == func_801155BC_gacode ) {
        pl->lane->unk_B10 = func_801179FC_gacode;
        pl->lane->unk_B08 = func_8011AA78_gacode;
    } else if ( D_8012CDF4_gacode == func_801157F8_gacode ) {
        pl->lane->unk_B10 = func_80117ABC_gacode;
        pl->lane->unk_B08 = func_8011AA78_gacode;
    } else {
        pl->lane->unk_B10 = func_80117AE0_gacode;
        pl->lane->unk_B08 = func_8011AB0C_gacode;
    }
    pl->lane->unk_B0C = func_8011B974_gacode;
}

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

/*
================
func_801168D0_gacode
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: control flow; retail computes the height before the first test and gives that test its own
// return-0 block, and puts the 1 in v0 before the final store
s32 func_801168D0_gacode( gaPlayer_t *pl ) {
    gaLane_t *lane = pl->lane;

    if ( lane->spots[0].pos[2] > -1186.0f
         && lane->spots[0].pos[1] - lane->spots[0].unk_18 < D_8012D220_gacode->unk_14
         && ( lane->unk_454 & 1 ) ) {
        func_80120D7C_gacode( &lane->unk_B18 );
        func_80120D30_gacode( pl->laneIndex, 0x2F );
        pl->lane->bodies[0].rot[2] = pl->lane->unk_434 = 0.0f;
        pl->lane->unk_B25 = 3;
        pl->lane->unk_B10 = func_80118F68_gacode;
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801168D0_gacode);
#endif

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

/*
================
func_801169F8_gacode

Moves *value by delta / 50, within -50 to 50.
================
*/
void func_801169F8_gacode( s32 player, f32 *value, s16 delta ) {
    *value += delta / 50.0f;
    if ( *value < -50.0f ) {
        *value = -50.0f;
    } else if ( 50.0f < *value ) {
        *value = 50.0f;
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116A64_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116BA4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116DC8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80116FF0_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80117124_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801178D8_gacode);

/*
================
func_801179FC_gacode

Lane input handler: on button 1 of a human player, or after 30 frames for the game's own player, sounds
effect 1 and moves the lane on to func_80117AE0_gacode.
================
*/
s32 func_801179FC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    s8 port;

    pl->obj->unk_4A = 0;
    pl->lane->unk_B20++;
    port = pl->port;
    if ( ( port >= 0 && input == 1 ) || ( port < 0 && pl->lane->unk_B20 >= 30 ) ) {
        pl->lane->unk_B20 = 0;
        func_80120D30_gacode( pl->laneIndex, 1 );
        pl->lane->unk_B10 = func_80117AE0_gacode;
        pl->lane->unk_B08 = func_8011AB0C_gacode;
    }
    return 0;
}

/*
================
func_80117ABC_gacode
================
*/
s32 func_80117ABC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    D_8012D658_gacode[player].obj->unk_4A = 0;
    return 0;
}

/*
================
func_80117AE0_gacode

Lane input handler: shows the standing pins (unk_4A 3) and hides the rest and the player, loads the
character's file 1 for it, shows objects unk_AF4 (2 on lane 1, else 1) and moves on to
func_80117BDC_gacode.
================
*/
s32 func_80117AE0_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    s16 *files = pl->character.unk_00->unk_00;
    gaLane_t *lane = pl->lane;
    s32 i;

    lane->bodies[0].unk_5D = 0;
    for ( i = 1; i < NUM_BODIES; i++ ) {
        lane->bodyObjs[i]->unk_4A = lane->bodies[i].unk_5D == 1 ? 3 : 0;
    }
    pl->obj->unk_4A = 0;
    pl->obj->unk_48 = 0;
    pl->obj->unk_50 = func_80030C04( files[1], FILE_TYPE2 );
    pl->lane->unk_AF4[0]->unk_4A = pl->lane->unk_AF4[1]->unk_4A = D_8012D658_gacode[player].laneIndex != 0 ? 2 : 1;
    pl->lane->unk_B10 = func_80117BDC_gacode;
    return 0;
}

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

/*
================
func_80118CCC_gacode
================
*/
s32 func_80118CCC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->lane->unk_B0C = func_8011BCA8_gacode;
    pl->lane->unk_B20++;
    if ( pl->obj->unk_48 == pl->character.unk_00->unk_1B[pl->lane->unk_B26] - 1
         || ( pl->lane->unk_B20 >= 30 && ( D_800E44E0[pl->laneIndex] & ( A_BUTTON | Z_TRIG ) ) ) ) {
        pl->lane->unk_B20 = 0;
        pl->lane->unk_B0C = func_8011C650_gacode;
        D_8012D8D8_gacode++;
        pl->lane->unk_B10 = func_80118D9C_gacode;
    }
    return 0;
}

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

/*
================
func_80118E04_gacode

Lane input handler: after the throw (40 frames of it when unk_B25 is set), starts the next turn.
================
*/
s32 func_80118E04_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    if ( pl->obj->unk_48 != 0 ) {
        pl->obj->unk_48--;
    }
    pl->lane->unk_B20++;
    if ( pl->lane->unk_B25 != 0 ) {
        if ( pl->lane->unk_B20 < 40 ) {
            return 0;
        }
        pl->lane->unk_B20 = 0;
    } else {
        pl->lane->unk_B20 = 0;
    }
    pl->obj->unk_48 = 0;
    D_8012D8D8_gacode++;
    return D_8012D23C_gacode( player );
}

/*
================
func_80118EA0_gacode
================
*/
s32 func_80118EA0_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    pl->obj->unk_4A = pl->laneIndex ? 2 : 1;
    if ( pl->obj->unk_48 == pl->character.unk_00->unk_1B[pl->lane->unk_B26] ) {
        pl->obj->unk_48--;
        return -1;
    }
    return 0;
}

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

/*
================
func_80119800_gacode

Draws a human player's panel on its lane's side: the frame, its second part when full is set, and the
current image of the lane's D_801236B0_gacode list.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail sets y once after the lane test and reuses the loaded laneIndex for the first image id;
// this reloads laneIndex and swaps s2/s3 (y must stay a variable: retail adds 0x11 to it with ori)
void func_80119800_gacode( s32 player, u8 full ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    s32 x;
    s32 y;

    if ( pl->port < 0 ) {
        return;
    }
    if ( pl->laneIndex != 0 ) {
        x = 0xB0;
        y = 0xA8;
    } else {
        x = 0x18;
        y = 0xA8;
    }
    func_8002E0EC( pl->laneIndex != 0 ? 0xA46 : 0xA48, x, 0xA8 );
    if ( full ) {
        func_8002E0EC( pl->laneIndex != 0 ? 0xA47 : 0xA49, x + 0x1D, 0xA8 );
    }
    func_8003B1CC( D_801236B0_gacode[pl->laneIndex].ids[D_801236B0_gacode[pl->laneIndex].cur], x + 0xD, y + 0x11 );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119800_gacode);
#endif

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_801198E4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119ACC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119C50_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119DD8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_80119F8C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A26C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A404_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011A8AC_gacode);

/*
================
func_8011AA78_gacode

Runs the part of the game mode's ending that belongs to the mode.
================
*/
void func_8011AA78_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    if ( D_8012CDF4_gacode == func_801156D8_gacode ) {
        func_80119DD8_gacode( player, pl->laneIndex );
    } else if ( D_8012CDF4_gacode == func_801155BC_gacode ) {
        func_80119ACC_gacode( player, pl->laneIndex );
    } else if ( D_8012CDF4_gacode == func_801157F8_gacode ) {
        func_8011A404_gacode( pl );
    }
}

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

/*
================
func_8011B408_gacode
================
*/
void func_8011B408_gacode( s32 player ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    s32 x;
    s16 id;

    x = pl->laneIndex ? 0xA8 : 0x18;
    id = D_8012C784_gacode[pl->lane->unk_B25];
    if ( id >= 0 ) {
        func_8002E4B0( id, x, 0x18, D_80123601_gacode + 0x5F, 1.0f, 1.0f );
    }
}

/*
================
func_8011B490_gacode

Shows image 0x851 over the lane's side when its score leads, or 0xA6B when the two scores are level; then,
except in mode func_8011582C_gacode, draws the lane's panel.
================
*/
void func_8011B490_gacode( s32 lane ) {
    s32 rival = lane == 0;

    if ( D_800EA090[lane] > D_800EA090[rival] ) {
        func_8002E4B0( 0x851, lane == 0 ? 0x14 : 0xA4, 0x94, D_80123601_gacode + 0x7F, 1.0f, 1.0f );
    } else if ( D_800EA090[lane] == D_800EA090[rival] ) {
        func_8002E4B0( 0xA6B, lane != 0 ? 0xB4 : 0x24, 0x94, D_80123601_gacode + 0x7F, 1.0f, 1.0f );
    }
    if ( D_8012CDF4_gacode != func_8011582C_gacode ) {
        func_8003A658( lane, 0x18, lane == 0 ? 0x10 : 0xC0 );
    }
}

/*
================
func_8011B564_gacode
================
*/
void func_8011B564_gacode( s32 player ) {
    s32 x;

    x = D_8012D658_gacode[player].laneIndex == 0 ? 0x14 : 0xA4;
    func_8002E4B0( 0x891, x, 0xB4, D_80123601_gacode + 0x5F, 1.0f, 1.0f );
}

/*
================
func_8011B5C8_gacode
================
*/
void func_8011B5C8_gacode( s32 lane ) {
    if ( D_800EA090[lane] == 0 ) {
        func_8002FDE4( lane ? 0xA0 : 0, 0, 0xA0, 0xF0, 0xFF, 0x7F, 0, D_80123601_gacode + 0x4F );
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011B634_gacode);

/*
================
func_8011B8AC_gacode

An unk_B0C handler: the camera of func_8011C0DC_gacode, raised by unk_B28, which falls by 45 a frame to 0.
================
*/
void func_8011B8AC_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->lane->unk_B14;
    camera->eye[1] = pl->lane->unk_B28 + 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->lane->unk_B14;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
    pl->lane->unk_B28 -= 45.0f;
    if ( pl->lane->unk_B28 < 0.0f ) {
        pl->lane->unk_B28 = 0.0f;
    }
}

/*
================
func_8011B974_gacode
================
*/
void func_8011B974_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->character.unk_14 + pl->lane->unk_B14;
    camera->eye[1] = 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->character.unk_14 + pl->lane->unk_B14;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011BA00_gacode
================
*/
void func_8011BA00_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = 1130.0f;
    camera->eye[0] = pl->character.unk_20 + pl->lane->unk_B14;
    camera->eye[1] = 200.0f;
    camera->at[2] = D_8012C158_gacode[0];
    camera->at[0] = pl->character.unk_20 + pl->lane->unk_B14;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011BA8C_gacode
================
*/
void func_8011BA8C_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];
    s32 side = pl->laneIndex ? -1 : 1;

    camera->eye[2] = 730.0f;
    camera->eye[0] = pl->obj->mtx[3][0] + side * 250.0;
    camera->eye[1] = 40.0f;
    camera->at[2] = pl->obj->unk_1C8[2];
    camera->at[0] = pl->obj->unk_1C8[0];
    camera->at[1] = 60.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011BB44_gacode

An unk_B0C handler: R and L step unk_92 through 0 to 4, wrapping; the camera is func_8011B634_gacode's.
================
*/
void func_8011BB44_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    if ( D_800E44E0[pl->laneIndex] & R_TRIG ) {
        func_80120D30_gacode( pl->laneIndex, 0x33 );
        if ( pl->unk_92 == 4 ) {
            pl->unk_92 = 0;
        } else {
            pl->unk_92++;
        }
    }
    if ( D_800E44E0[pl->laneIndex] & L_TRIG ) {
        func_80120D30_gacode( pl->laneIndex, 0x33 );
        if ( pl->unk_92 == 0 ) {
            pl->unk_92 = 4;
        } else {
            pl->unk_92--;
        }
    }
    func_8011B634_gacode( pl, pl->unk_92 );
}

/*
================
func_8011BC10_gacode
================
*/
void func_8011BC10_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = -896.0f;
    camera->eye[0] = pl->lane->unk_B14;
    camera->eye[1] = D_8012D220_gacode->unk_14 + 80.0f;
    camera->at[2] = -1096.0f;
    camera->at[0] = pl->lane->unk_B14;
    camera->at[1] = D_8012D220_gacode->unk_14 + 24.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011BCA8_gacode
================
*/
void func_8011BCA8_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = pl->obj->unk_1C8[2] - 256.0f;
    camera->eye[0] = pl->obj->unk_1C8[0];
    camera->eye[1] = 50.0f;
    camera->at[2] = pl->obj->unk_1C8[2];
    camera->at[0] = pl->obj->unk_1C8[0];
    camera->at[1] = pl->obj->unk_1C8[1];
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011BD34_gacode);

/*
================
func_8011C0DC_gacode
================
*/
void func_8011C0DC_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->lane->unk_B14;
    camera->eye[1] = 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->lane->unk_B14;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C158_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C254_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011C468_gacode);

/*
================
func_8011C57C_gacode

Camera handler: puts the eye on frame D_8012D8D8_gacode % 5 of the D_8012C8E8_gacode paths, looking down the
lane.
================
*/
void func_8011C57C_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &D_8012D658_gacode[player];

    func_8002BED8( D_8012C8E8_gacode[D_8012D8D8_gacode % 5], 0, camera->eye, camera->unk_18 );
    camera->at[2] = -1096.0f;
    camera->at[0] = pl->lane->unk_B14;
    camera->at[1] = D_8012D220_gacode->unk_14;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

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

/*
================
func_8011CEE4_gacode

Gives the player the four values of its character's D_80123850_gacode row.
================
*/
void func_8011CEE4_gacode( gaPlayer_t *pl, s32 player ) {
    f32 *values = D_80123850_gacode[D_800DF59C.unk_04[D_8009D19C[player]]];

    pl->unk_98 = values[0];
    pl->unk_9C = values[1];
    pl->character.unk_2C = values[2];
    pl->character.unk_30 = values[3];
}

INCLUDE_ASM("asm/nonmatchings/gacode/3E350", func_8011CF30_gacode);
