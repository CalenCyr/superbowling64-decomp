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

/*
================
GA_SetupLanes

Sets up both lanes: their sprites, scene objects, camera and state; lane 1 is shifted by offset. With
arg2 set, also clears a flag of the lanes' body objects and shows their markerObjs[2] objects.
================
*/
void GA_SetupLanes( u8 arg0, f32 offset, u8 arg2 ) {
    s32 i;
    s32 j;
    s32 base;
    s32 player;

    GA_SetupSprite( &D_8012D240_gacode[0], 0xA49, 0x10, 0x41 );
    GA_SetupSprite( &D_8012D240_gacode[1], 0xA47, 0x10, 0x41 );
    GA_SetupSprite( &D_8012CE10_gacode[0], 0xA49, 0x50, 0x7D );
    GA_SetupSprite( &D_8012CE10_gacode[1], 0xA47, 0x50, 0x7D );
    GA_SetupSprite( &D_8012D8E0_gacode[0], 0xA48, 0x10, 0x58 );
    GA_SetupSprite( &D_8012D8E0_gacode[1], 0xA46, 0x10, 0x58 );
    for ( i = 0; i < 2; i++ ) {
        base = i != 0 ? 120 : 100;
        ga_lanes[i].offsetX = i != 0 ? offset : 0.0f;
        ga_lanes[i].camera = D_800EA120[i];
        ga_lanes[i].camera->unk_24 = -1;
        for ( j = 0; j < NUM_BODIES; j++ ) {
            ga_lanes[i].bodyObjs[j] = D_800EA120[base + j];
            if ( arg0 ) {
                ga_lanes[i].bodyObjs[j]->flags &= i == 0 ? ~0x8000 : ~0x4000;
            }
        }
        ga_lanes[i].markerObjs[0] = D_800EA120[base + j++];
        ga_lanes[i].markerObjs[1] = D_800EA120[base + j++];
        ga_lanes[i].markerObjs[2] = D_800EA120[base + j];
        if ( arg2 ) {
            ga_lanes[i].markerObjs[2]->viewMask = i + 1;
            D_8012C110_gacode[2].pos[1] = ga_stage->unk_14;
        }
        ga_lanes[i].cameraRaise = 2700.0f;
        GA_RestorePalette( &D_8012D240_gacode[i] );
        GA_RestorePalette( &D_8012CE10_gacode[i] );
        GA_RestorePalette( &D_8012D8E0_gacode[i] );
        ga_lanes[i].pinsDown = 0;
        ga_lanes[i].pinsHit = 0;
        ga_lanes[i].frameOver = 0;
        ga_lanes[i].soundHandle = 0;
        ga_lanes[i].timer = 0;
        ga_lanes[i].bounceSound = ga_stage->unk_1E[0];
        ga_laneAim[i].unk_00 = -1096.0f;
        ga_laneAim[i].unk_04 = 0;
        ga_laneAim[i].unk_06 = 6;
        ga_laneAim[i].unk_07 = 0;
    }
    D_8012D8D8_gacode = 0;
    for ( player = 0; player < MAX_PLAYERS; player++ ) {
        ga_players[player].obj = D_800EA120[140 + player];
    }
}

/*
================
GA_StopLanes

Stops the sound of lanes 0 to lastLane and parks their inputFunc handlers.
================
*/
void GA_StopLanes( u8 lastLane ) {
    s32 i;

    for ( i = 0; i <= lastLane; i++ ) {
        GA_StopLaneSound( &ga_lanes[i].soundHandle );
        ga_lanes[i].inputFunc = func_80118F38_gacode;
    }
}

/*
================
GA_SetupPlayer

Sets up a player: its controller port (-1: played by the game), character and lane.
================
*/
void GA_SetupPlayer( s32 player, s32 port, u8 character, u8 lane ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->lane = &ga_lanes[lane];
    pl->laneIndex = lane;
    pl->port = port;
    memmove( pl->homeMtx, pl->obj->mtx, sizeof( pl->homeMtx ) );
    memmove( &pl->character, &ga_characters[character], sizeof( gaCharacter_t ) );
    pl->ballWeight = ga_characters[character].ballWeight;
    GA_StartTurn( player );
    func_8011B8AC_gacode( player, pl->lane->camera );
}

/*
================
GA_RunPlayer

Runs a player's frame: the lane's turnFunc handler, then its input handler with the controller's buttons
(1: A or B, -1: the other button) and the stick (only its dominant axis), or the game's own input for a
player without a port; then places the lane's bodies and sets its camera. Returns the input handler's
result.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register only: before the body loop retail loads pl->lane into a0 (this into v0); the
// loops' pointer order (spot set and stepped first) was found by A/B, and indexed &pl->lane->bodies[i] or a
// lane local miss the loop-invariant base; the permuter (target-only scoring, 1.3k iterations from 15) found nothing better.
// Also tried: the increments' order (i first, body first), the pointers set in the for's init, i as
// s16/u8/u32, body declared before spot.
s32 GA_RunPlayer( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 port;
    s32 input;
    s16 stick[2];
    s32 result;
    gaSpot_t *spot;
    gaBody_t *body;
    s32 i;

    pl->lane->turnFunc( player );
    port = pl->port;
    input = 0;
    stick[1] = 0;
    stick[0] = 0;
    if ( D_8005C0BF == 0 ) {
        if ( port >= 0 ) {
            stick[1] = D_800A2FD8[port];
            stick[0] = D_800692D0[port];
            if ( ( stick[1] < 0 ? -stick[1] : stick[1] ) * 2 <= ( stick[0] < 0 ? -stick[0] : stick[0] ) ) {
                stick[1] = 0;
            } else if ( ( stick[0] < 0 ? -stick[0] : stick[0] ) * 2 <= ( stick[1] < 0 ? -stick[1] : stick[1] ) ) {
                stick[0] = 0;
            }
            if ( D_800E44E0[port] & 0x4000 ) {
                input = -1;
            } else {
                input = ( D_800E44E0[port] & 0xA000 ) != 0;
            }
        } else {
            input = GA_CpuInput( pl, player );
        }
    }
    result = pl->lane->inputFunc( player, input, stick[0], stick[1] );
    GA_PhysicsStep( player, port );
    spot = pl->lane->spots;
    body = pl->lane->bodies;
    for ( i = 0; i < NUM_BODIES; spot++, body++, i++ ) {
        GA_PlaceBody( body, spot, pl->lane->bodyObjs[i], pl->lane->offsetX );
    }
    spot = pl->lane->markerSpots;
    body = pl->lane->markers;
    for ( i = 0; i < 3; spot++, body++, i++ ) {
        GA_PlaceBody( body, spot, pl->lane->markerObjs[i], pl->lane->offsetX );
    }
    pl->lane->cameraFunc( player, pl->lane->camera );
    GA_PublishBall( pl );
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", GA_RunPlayer);
#endif

/*
================
GA_PhysicsStep

The per-frame physics step of the player's lane: turns each standing body by its spin (keeping its angles
within -180..180) and moves its spot by its velocity, then lets GA_KeepOnFloor collide each one, playing
the sounds and rumbling the port for the ball's hits, and finally GA_CollideLane and
GA_SettleBodies.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: calls and constants as retail (CFG not yet compared past the prologue); retail keeps pointers into the lane (bodies in s7,
// spots in fp, &lane->motion spilled to the stack and walked by both loops, and loop 2 reaches secondSpots as
// spots + 0x160), which suggests pointer locals walked per body instead of lane->x[i]; frame 0x68 here 0x70.
// bounceSound is read with lh (s16 in retail?) against gaStage_t.unk_1E[1].
/* An angle in degrees brought back within -180..180. */
#define GA_WRAP_DEGREES( a ) ( ( a ) < -180.0f ? ( a ) + 360.0f : ( a ) >= 180.0f ? ( a ) - 360.0f : ( a ) )

void GA_PhysicsStep( s32 player, s32 port ) {
    gaPlayer_t *pl = &ga_players[player];
    gaLane_t *lane = pl->lane;
    s32 i;

    for ( i = 0; i < NUM_BODIES; i++ ) {
        if ( lane->bodies[i].standState != 0 ) {
            if ( lane->bodies[i].kind == 1 ) {
                lane->bodies[i].rot[1] += lane->motion[i].spin[1];
            }
            lane->bodies[i].rot[1] = GA_WRAP_DEGREES( lane->bodies[i].rot[1] );
            lane->bodies[i].rot[2] += lane->motion[i].spin[2];
            lane->bodies[i].rot[2] = GA_WRAP_DEGREES( lane->bodies[i].rot[2] );
            lane->bodies[i].rot[0] += lane->motion[i].spin[0];
            lane->bodies[i].rot[0] = GA_WRAP_DEGREES( lane->bodies[i].rot[0] );
            lane->spots[i].pos[0] += lane->motion[i].vel[0];
            lane->spots[i].pos[1] += lane->motion[i].vel[1];
            lane->spots[i].pos[2] += lane->motion[i].vel[2];
            if ( lane->secondSpots[i].radius != 0.0f ) {
                GA_TransformPoint( lane->secondSpots[i].pos, D_8012C758_gacode, lane->bodies[i].rot, lane->spots[i].pos );
            }
        }
    }
    for ( i = 0; i < NUM_BODIES; i++ ) {
        if ( lane->bodies[i].standState != 0 ) {
            s32 hit = GA_KeepOnFloor( &lane->motion[i], &lane->spots[i], &lane->secondSpots[i], 180.0f,
                                            ga_stage->unk_14 );

            if ( lane->bodies[i].kind == 0 ) {
                if ( hit && lane->spots[i].pos[2] > -1050.0 ) {
                    if ( lane->spots[i].pos[1] <= -190.0f ) {
                        GA_PlayLaneSound( pl->laneIndex, 0x48 );
                    } else {
                        GA_PlayLaneSound( pl->laneIndex, pl->lane->bounceSound );
                    }
                    if ( port >= 0 ) {
                        D_800DF680[port] = pl->lane->bounceSound == ga_stage->unk_1E[1] ? 6 : 4;
                        D_8006940D[port] = 10;
                    }
                }
                if ( ( ( lane->motion[i].flags >> 2 ) & 1 ) || ( ( lane->motion[i].flags >> 6 ) & 1 ) ) {
                    GA_PlayLaneSound( pl->laneIndex, lane->spots[i].pos[1] <= -190.0f ? 0x4C : 0x2F );
                    if ( port >= 0 && ( ( lane->motion[i].flags >> 6 ) & 1 ) ) {
                        D_800DF680[port] = 5;
                        D_8006940D[port] = 10;
                    }
                }
            }
        }
    }
    if ( GA_CollideLane( player, lane->bodies, lane->motion, lane->spots, pl->laneIndex ) && port >= 0 ) {
        D_800DF680[port] = 5;
        D_8006940D[port] = 15;
    }
    GA_SettleBodies( lane->bodies, lane->motion, lane->spots );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", GA_PhysicsStep);
#endif

/*
================
GA_PlaceBody

Places a body at its spot, offset by offset along x, and sets its object's unk_44 from the spot's height.
================
*/
void GA_PlaceBody( gaBody_t *body, gaSpot_t *spot, sceneObject_t *obj, f32 offset ) {
    f32 height;

    if ( obj->viewMask == 0 ) {
        return;
    }
    body->pos[0] = spot->pos[0] + offset;
    body->pos[1] = spot->pos[1];
    body->pos[2] = spot->pos[2];
    GA_BodyMatrix( obj->mtx, body->pos, body->rot, body->scale );
    if ( spot->pos[2] <= -1186.0f ) {
        obj->unk_44 = -1.0f;
        return;
    }
    switch ( body->kind ) {
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
GA_PublishBall

Publishes the ball of the player's lane to main: its position, and in D_800DF595 its state (0: no ball,
1 or 2 (the lane in func_80118F68_gacode): motion[0].flags bit 1 set, 3: below -190).
================
*/
void GA_PublishBall( gaPlayer_t *pl ) {
    gaLane_t *lane = pl->lane;
    gaSpot_t *spot = &lane->spots[0];
    u8 state;

    D_800E6560[pl->laneIndex][0] = lane->bodies[0].pos[0];
    D_800E6560[pl->laneIndex][1] = lane->bodies[0].pos[1];
    D_800E6560[pl->laneIndex][2] = lane->bodies[0].pos[2];
    if ( lane->bodies[0].standState == 0 ) {
        D_800DF595[pl->laneIndex] = 0;
        return;
    }
    if ( ( lane->motion[0].flags >> 1 ) & 1 ) {
        if ( pl->lane->inputFunc == func_80118F68_gacode ) {
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

/*
================
GA_HoldBall

While the ball of the player's lane is held, places it in the player's hand: at the character's offset
from the joint it names, and sets the ball's spot from there (relative to the lane's offsetX).
================
*/
void GA_HoldBall( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    gaCharacterData_t *data;
    f32 mf[4][4];
    u8 joint;

    if ( pl->lane->bodies[0].standState != 0 ) {
        return;
    }
    data = pl->character.data;
    joint = data->unk_08 < 0 ? -data->unk_08 : data->unk_08;
    guTranslateF( mf, data->unk_0C[0], data->unk_0C[1], data->unk_0C[2] );
    guMtxCatF( mf, pl->obj->joints[joint].mtx, pl->lane->bodyObjs[0]->mtx );
    guMtxXFMF( pl->lane->bodyObjs[0]->mtx, 0.0f, 0.0f, 0.0f, &pl->lane->spots[0].pos[0], &pl->lane->spots[0].pos[1],
               &pl->lane->spots[0].pos[2] );
    pl->lane->spots[0].pos[0] -= pl->lane->offsetX;
    Obj_MakeMatrix( pl->lane->bodyObjs[0] );
}

/*
================
func_80114FC4_gacode

Runs the panelFunc handler of the player's lane.
================
*/
void func_80114FC4_gacode( s32 player ) {
    ga_players[player].lane->panelFunc( player );
}

/*
================
func_80114FFC_gacode
================
*/
void func_80114FFC_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    sceneObject_t *obj = pl->obj;

    if ( obj->viewMask != 0 && obj->frame != 0 ) {
        obj->frame--;
    }
    GA_StopLaneSound( &pl->lane->soundHandle );
}

/*
================
func_80115054_gacode

Sets the camera's unk_24 to -1, clears the players' ballView and hides scene objects 120 to 133 and 141.
================
*/
void func_80115054_gacode( void ) {
    gaPlayer_t *pl;
    s32 player;
    s32 i;

    ( (gaCamera_t *)D_800EA120[1] )->unk_24 = -1;
    for ( player = MAX_PLAYERS - 1; player >= 0; player-- ) {
        pl = &ga_players[player];
        pl->ballView = 0;
    }
    for ( i = 0; i < 14; i++ ) {
        ( (sceneObject_t *)D_800EA120[120 + i] )->viewMask = 0;
    }
    ( (sceneObject_t *)D_800EA120[141] )->viewMask = 0;
}

/*
================
GA_UpdateLaneCamera

Moves main's camera on from the view the player's lane shows: each cameraFunc handler hands over to the next
camera.
================
*/
void GA_UpdateLaneCamera( s32 player ) {
    void (*view)( s32 player, gaCamera_t *camera ) = ga_players[player].lane->cameraFunc;
    gaCamera_t *camera = D_800EA120[1];

    if ( view == func_8011BB44_gacode ) {
        func_8011C468_gacode( player, camera );
        D_8012C7C8_gacode = 0;
    } else if ( view == func_8011BC10_gacode ) {
        if ( D_8012C7C8_gacode == 0 ) {
            func_8011C57C_gacode( player, camera );
            D_8012C7C8_gacode = 1;
        } else {
            func_8011C650_gacode( player, camera );
        }
    } else if ( view == func_8011BA8C_gacode || view == func_8011BCA8_gacode ) {
        func_8011C254_gacode( player, camera );
    } else if ( view == func_8011C650_gacode ) {
        func_8011C650_gacode( player, camera );
    } else {
        func_8011C158_gacode( player, camera );
    }
}

/*
================
func_801151A0_gacode

A game mode. The player scores for their team when the sub-mode's argument is 1 (players 2 and 3 for player
1). After a func_80116398_gacode turn (unless D_80068A50 is 2), records GA_IsSplit's result in the
frame's flags. When the throw ends the frame, counts the run D_800A304C on after a func_80116398_gacode turn
(returns 2), else ends it (returns 1); otherwise plays the throw's sound and ends the run, returning 3 when
the lane's callout was 3 or the run had reached 3, else 0.
================
*/
s32 func_801151A0_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 strike = 0;
    s32 scorer;
    s32 frame;
    s32 result;

    if ( ( D_8005C160 >> 6 ) == 1 ) {
        scorer = player >= 2;
    } else {
        scorer = player;
    }
    frame = D_8009D19C[scorer];
    if ( pl->lane->turnFunc == func_80116398_gacode && D_80068A50[scorer] != 2 ) {
        strike = GA_IsSplit( pl->lane );
        if ( strike != 0 ) {
            D_8009CF80[scorer]++;
        }
        D_80099458[scorer][frame] = strike | ( D_80099458[scorer][frame] & 0xF0 );
        if ( frame == 9 && strike != 0 ) {
            D_80099458[scorer][frame] |= 0x10 << D_80068A50[scorer];
        }
    }
    func_80121FD0_gacode( scorer, pl->lane->pinsHit );
    if ( pl->lane->pinsDown == 10 ) {
        if ( pl->lane->turnFunc == func_80116398_gacode ) {
            GA_PlayLaneSound( pl->laneIndex, ga_runSounds[D_800A304C[scorer]] );
            pl->lane->callout = D_800A304C[scorer] + 6;
            D_800A304C[scorer]++;
            D_800DF6B4[scorer]++;
            if ( D_800A304C[scorer] >= 3 ) {
                GA_PlayLaneSound( pl->laneIndex, 0x36 );
            } else {
                GA_PlayLaneSound( pl->laneIndex, 0x37 );
            }
            result = 2;
        } else {
            if ( frame != 9 ? D_80099458[scorer][frame] != 0
                            : ( D_80099458[scorer][9] & 1 )
                              && ( ( D_80099458[scorer][9] & 0x10 ) || ( D_80099458[scorer][9] & 0x20 ) ) ) {
                GA_PlayLaneSound( pl->laneIndex, 0x7E );
            } else {
                GA_PlayLaneSound( pl->laneIndex, 0x5A );
            }
            pl->lane->callout = D_80099458[scorer][frame] ? 5 : 1;
            D_800A304C[scorer] = 0;
            D_8009D088[scorer]++;
            GA_PlayLaneSound( pl->laneIndex, 0x38 );
            result = 1;
        }
        pl->lane->frameOver = 1;
    } else {
        result = 0;
        if ( pl->lane->callout == 3 ) {
            if ( pl->lane->turnFunc == func_80116414_gacode ) {
                pl->lane->callout = 4;
            }
            GA_PlayLaneSound( pl->laneIndex, 0x62 );
            result = 3;
        } else if ( D_800A304C[scorer] >= 3 ) {
            if ( strike != 0 ) {
                GA_PlayLaneSound( pl->laneIndex, 0x7C );
                pl->lane->callout = 2;
            } else {
                GA_PlayLaneSound( pl->laneIndex, 0x63 );
            }
            result = 3;
        } else if ( pl->lane->pinsHit == 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x62 );
            pl->lane->callout = 4;
        } else if ( strike != 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x7C );
            pl->lane->callout = 2;
        }
        pl->lane->frameOver = pl->lane->turnFunc == func_80116414_gacode;
        D_800A304C[scorer] = 0;
    }
    return result;
}

/*
================
GA_ScoreResult

Result of a game mode for a frame's score, given as 2 + the score against par: 1 for par, 2 for one under, else
3. Retail has no out-of-line copy; func_801155BC_gacode expands it (the result reaches the caller's variable
through a copy, which a plain ternary there does not give).
================
*/
static inline s32 GA_ScoreResult( s32 score ) {
    return score == 2 ? 1 : score == 1 ? 2 : 3;
}

/*
================
func_801155BC_gacode

A game mode: when GA_CountParThrow ends the player's frame, plays the sound for its score against par,
sets the lane's callout from the score and returns 1 for par, 2 for one under and 3 otherwise; else 0.
================
*/
s32 func_801155BC_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 result;

    if ( ( pl->lane->frameOver = GA_CountParThrow( player, pl->lane->pinsDown == 10 ) ) != 0 ) {
        GA_PlayLaneSound( pl->laneIndex, D_8012C7E6_gacode[D_800DF630[player][D_8009D19C[player] - 1]] );
        pl->lane->callout = D_800DF630[player][D_8009D19C[player] - 1] + 0x1A;
        result = GA_ScoreResult( pl->lane->callout - 0x18 );
    } else {
        pl->lane->callout = 0;
        result = 0;
    }
    return result;
}

/*
================
func_801156D8_gacode

A game mode: when GA_EndFrame ends the player's frame, plays the sounds and sets the lane's callout
when the game is over (returns 2) or not (returns 1); else sets callout by D_80068A50 and returns 0.
================
*/
s32 func_801156D8_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( ( pl->lane->frameOver = GA_EndFrame( player, pl->lane->pinsDown == 10 ) ) != 0 ) {
        if ( D_8009D19C[player] == 10 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x77 );
            pl->lane->callout = 0x14;
            GA_PlayLaneSound( pl->laneIndex, 0x36 );
            return 2;
        }
        GA_PlayLaneSound( pl->laneIndex, 0x7D );
        pl->lane->callout = 0x12;
        GA_PlayLaneSound( pl->laneIndex, 0x37 );
        return 1;
    }
    if ( D_80068A50[player] == 0 ) {
        GA_PlayLaneSound( pl->laneIndex, 0x6E );
        pl->lane->callout = player + 0x15;
    } else {
        GA_PlayLaneSound( pl->laneIndex, 0x6D );
        pl->lane->callout = 0x13;
    }
    return 0;
}

/*
================
func_801157F8_gacode

A game mode.
================
*/
s32 func_801157F8_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->lane->callout = 0;
    pl->lane->frameOver = 1;
    return 0;
}

/*
================
func_8011582C_gacode

A game mode. When the throw ends the frame (pinsDown is 10): after a func_80116398_gacode turn plays the sound
for the player's run D_800A304C, adds to the other player's ga_laneAim unk_04 and counts the run on;
otherwise scores by D_80068A50 and D_80099458 and ends the run. Else scores the throw by GA_IsSplit.
Returns 0.
================
*/
s32 func_8011582C_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 other = player == 0;
    u8 run;
    u8 result;

    if ( ( pl->lane->frameOver = pl->lane->pinsDown == 10 ) ) {
        if ( pl->lane->turnFunc == func_80116398_gacode ) {
            run = D_800A304C[player];
            if ( run > 10 ) {
                run = 10;
            }
            GA_PlayLaneSound( pl->laneIndex, ga_runSounds[run] );
            pl->lane->callout = run < 10 ? run + 6 : 0;
            ga_laneAim[other].unk_04 += 25 + D_800A304C[player] * 10;
            if ( D_800A304C[player] < 255 ) {
                D_800A304C[player]++;
            }
        } else {
            if ( D_80068A50[player] == 1 ) {
                GA_PlayLaneSound( pl->laneIndex, D_80099458[player][0] ? 0x7E : 0x5A );
                pl->lane->callout = D_80099458[player][0] ? 5 : 1;
                ga_laneAim[other].unk_04 += D_80099458[player][0] ? 30 : 0;
                ga_laneAim[player].unk_04 -= 15;
            } else {
                ga_laneAim[other].unk_04 += D_80099458[player][0] ? 10 : 0;
            }
            ga_laneAim[player].unk_04 -= 10;
            D_800A304C[player] = 0;
        }
        D_80068A50[player] = 0;
    } else {
        result = GA_IsSplit( pl->lane );
        D_80099458[player][0] = result;
        if ( pl->lane->callout == 3 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x62 );
            ga_laneAim[player].unk_04 += 30;
        } else if ( pl->lane->pinsHit == 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x62 );
            pl->lane->callout = 4;
            ga_laneAim[player].unk_04 += 20;
        } else if ( result != 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x7C );
            pl->lane->callout = 2;
        }
        D_80068A50[player]++;
    }
    ga_laneAim[player].unk_07 = 0;
    return 0;
}

/*
================
GA_EndTurn

Ends a player's turn: picks the lane's next turnFunc handler and returns 1 when the game goes on, 0 when it
does not, or -1 instead of 0 when the sub-mode's argument is 1 (in which players 2 and 3 play for player 1, the
others for player 0).
================
*/
s32 GA_EndTurn( s32 player ) {
    gaLane_t *lane = ga_players[player].lane;
    s32 result;
    s32 frame;

    if ( ( D_8005C160 >> 6 ) == 1 ) {
        player = player < 2 ? 0 : 1;
    }
    frame = D_8009D19C[player];
    if ( D_800994BE[player] != 0 ) {
        lane->turnFunc = GA_StartTurn;
        D_80066C00.unk_20[player]++;
        lane->frameOver = 0;
        return 1;
    }
    result = 0;
    if ( lane->frameOver != 0 ) {
        lane->turnFunc = GA_StartTurn;
        D_80066C00.unk_20[player]++;
        if ( frame != 9 ) {
            result = 1;
        } else if ( D_80066E18[player][9] != -1 && D_800A3020[player][9] != -1 ) {
            result = 1;
        }
    } else {
        lane->turnFunc = func_801163A0_gacode;
    }
    if ( result == 0 && ( D_8005C160 >> 6 ) == 1 ) {
        result = -1;
    }
    lane->frameOver = 0;
    return result;
}

/*
================
func_80115CDC_gacode
================
*/
s32 func_80115CDC_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( pl->lane->frameOver != 0 ) {
        pl->lane->turnFunc = GA_StartTurn;
        return 1;
    }
    pl->lane->turnFunc = func_801163A0_gacode;
    pl->lane->frameOver = 0;
    return 0;
}

/*
================
func_80115D38_gacode
================
*/
s32 func_80115D38_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->lane->turnFunc = GA_StartTurn;
    pl->lane->frameOver = 0;
    return 1;
}

/*
================
func_80115D70_gacode
================
*/
void func_80115D70_gacode( s32 player, u8 arg1 ) {
    gaLane_t *lane = ga_players[player].lane;
    s32 i;

    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->viewMask = 0;
    }
    lane->turnFunc = func_80116414_gacode;
    lane->panelFunc = func_8011B564_gacode;
    lane->cameraFunc = GA_FreeCamera;
    if ( arg1 ) {
        lane->inputFunc = func_80118F08_gacode;
    } else {
        lane->inputFunc = func_80118F38_gacode;
    }
}

/*
================
func_80115DF4_gacode
================
*/
s32 func_80115DF4_gacode( s32 player ) {
    return ga_players[player].lane->inputFunc == func_80118F08_gacode;
}

/*
================
func_80115E24_gacode

When the players of every lane are done (inputFunc parked), stops their panelFunc and cameraFunc handlers too.
================
*/
s32 func_80115E24_gacode( u8 twoLanes ) {
    if ( twoLanes ) {
        if ( ga_lanes[0].inputFunc != func_80118F38_gacode ) {
            return 0;
        }
        if ( ga_lanes[1].inputFunc != func_80118F38_gacode ) {
            return 0;
        }
        ga_lanes[1].panelFunc = func_8011B400_gacode;
        ga_lanes[0].panelFunc = func_8011B400_gacode;
        ga_lanes[1].cameraFunc = func_8011C650_gacode;
        ga_lanes[0].cameraFunc = func_8011C650_gacode;
        return 1;
    }
    if ( ga_lanes[0].inputFunc != func_80118F38_gacode ) {
        return 0;
    }
    ga_lanes[0].panelFunc = func_8011B400_gacode;
    ga_lanes[0].cameraFunc = func_8011C650_gacode;
    return 1;
}

/*
================
func_80115ED4_gacode

When the players of every lane are done, or wait in func_80118F08_gacode (whose lanes then restart at
GA_StartTurn), stops their panelFunc handlers; returns 1 then.
================
*/
s32 func_80115ED4_gacode( u8 twoLanes ) {
    if ( twoLanes ) {
        if ( ga_lanes[0].inputFunc == func_80118F38_gacode && ga_lanes[1].inputFunc == ga_lanes[0].inputFunc ) {
            ga_lanes[1].panelFunc = func_8011B400_gacode;
            ga_lanes[0].panelFunc = func_8011B400_gacode;
            return 1;
        }
        if ( ga_lanes[0].inputFunc == func_80118F08_gacode && ga_lanes[1].inputFunc == ga_lanes[0].inputFunc ) {
            ga_lanes[1].turnFunc = GA_StartTurn;
            ga_lanes[0].turnFunc = GA_StartTurn;
            ga_lanes[1].panelFunc = func_8011B400_gacode;
            ga_lanes[0].panelFunc = func_8011B400_gacode;
            return 1;
        }
    } else {
        if ( ga_lanes[0].inputFunc == func_80118F38_gacode ) {
            ga_lanes[0].panelFunc = func_8011B400_gacode;
            return 1;
        }
        if ( ga_lanes[0].inputFunc == func_80118F08_gacode ) {
            ga_lanes[0].turnFunc = GA_StartTurn;
            ga_lanes[0].panelFunc = func_8011B400_gacode;
            return 1;
        }
    }
    return 0;
}

/*
================
GA_EndDuelGame

Ends a game of a duel: records both scores, sets both lanes to the result handlers with each player's win (3)
or loss (4) animation, and plays SE 0x7A for the winner, or sound 0x63 on a tie. Returns the winning lane, or
-1 on a tie.
================
*/
s32 GA_EndDuelGame( void ) {
    s32 winner;
    s32 i;

    if ( D_800EA090[0] > D_800EA090[1] ) {
        winner = 0;
    } else {
        winner = D_800EA090[0] < D_800EA090[1] ? 1 : -1;
    }
    for ( i = 0; i < 2; i++ ) {
        gaPlayer_t *pl;
        s16 *files;
        s32 rival;

        D_8009D030[i][D_800693DC[0] + D_800693DC[1]] = D_800EA090[i];
        pl = &ga_players[i];
        files = pl->character.data->unk_00;
        pl->lane->inputFunc = func_80118EA0_gacode;
        pl->lane->panelFunc = func_8011B490_gacode;
        pl->lane->cameraFunc = func_8011BCA8_gacode;
        rival = i != 1;
        pl->lane->reaction = D_800EA090[i] > D_800EA090[rival] ? 3 : 4;
        pl->obj->frame = 0;
        pl->obj->anim = Cache_GetFile( files[pl->lane->reaction + 2], FILE_TYPE2 );
    }
    if ( winner >= 0 ) {
        GA_PlayLaneSound( winner, 0x7A );
    } else {
        Sound_PlaySe( 0x63, 0 );
    }
    return winner;
}

/*
================
func_80116170_gacode

Once both lanes wait in func_80118EA0_gacode, ends the turn of players 0 and 1: with skip set, parks
their lanes' input; else first runs GA_ClearScores and sets the lanes' view handlers.
================
*/
void func_80116170_gacode( s32 skip ) {
    gaPlayer_t *pl;
    s32 i;

    if ( ga_lanes[0].inputFunc != func_80118EA0_gacode || ga_lanes[1].inputFunc != ga_lanes[0].inputFunc ) {
        return;
    }
    D_8012D8D8_gacode = 0;
    if ( !skip ) {
        GA_ClearScores();
    }
    for ( i = 0; i < 2; i++ ) {
        pl = &ga_players[i];
        if ( skip ) {
            pl->lane->inputFunc = func_80118F38_gacode;
        } else {
            pl->lane->cameraRaise = 2700.0f;
            pl->lane->cameraFunc = func_8011C650_gacode;
            pl->lane->turnFunc = GA_StartTurn;
        }
        pl->lane->panelFunc = func_8011B400_gacode;
    }
}

/*
================
GA_StartTurn

Sets up the pins of the player's lane for the game mode and frame, then starts the turn: with
D_8012D8D8_gacode at 0 through the lane's opening handlers, else through GA_SetTurnHandlers.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: stack frame only; retail's is 0x50 bytes, 0x20 more than this body needs (as GA_BodyMatrix:
// as if a 32-byte local were declared and never used); every instruction else is the same. A 32-byte unused local
// after pins (or two more u8[NUM_PINS]) gives the exact bytes; not adopted (R17) without evidence of what it was.
void GA_StartTurn( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    u8 pins[NUM_PINS];
    u16 mask;

    if ( ga_gameMode == func_801157F8_gacode ) {
        mask = ga_editorPins;
    } else if ( ga_gameMode == func_801156D8_gacode ) {
        mask = D_801237F0_gacode[D_800DF59C.unk_04[D_8009D19C[player]]].pins;
    } else if ( ga_gameMode == func_801155BC_gacode ) {
        mask = D_80123950_gacode[D_800DF59C.unk_04[D_8009D19C[player]]].pins;
    } else {
        mask = 0x3FF;
    }
    GA_UnpackPinMask( pins, mask );
    GA_ResetLane( player, pins );
    pl->lane->turnFunc = func_80116398_gacode;
    if ( D_8012D8D8_gacode == 0 ) {
        pl->lane->inputFunc = func_801178D8_gacode;
        pl->lane->panelFunc = func_8011B400_gacode;
        pl->lane->cameraFunc = func_8011B8AC_gacode;
    } else {
        GA_SetTurnHandlers( pl );
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", GA_StartTurn);
#endif

/*
================
func_80116398_gacode

A turnFunc handler that does nothing.
================
*/
void func_80116398_gacode( s32 player ) {
}

/*
================
func_801163A0_gacode

A turnFunc handler: passes which pins stand to GA_ResetLane, then idles.
================
*/
void func_801163A0_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    u8 pins[NUM_PINS];
    gaBody_t *body = &pl->lane->bodies[1];
    s32 i;

    for ( i = 0; i < NUM_PINS; i++ ) {
        pins[i] = body->standState;
        body++;
    }
    GA_ResetLane( player, pins );
    pl->lane->turnFunc = func_80116414_gacode;
}

/*
================
func_80116414_gacode

A turnFunc handler that does nothing.
================
*/
void func_80116414_gacode( s32 player ) {
}

/*
================
GA_ResetLane

Resets a player's lane for a new turn: puts the player's object back, resets the character's aim, copies the
lane's initial bodies and spots with the given pins standing, and starts the turn's handlers.
================
*/
void GA_ResetLane( s32 player, u8 *pins ) {
    gaPlayer_t *pl = &ga_players[player];
    gaLane_t *lane = pl->lane;
    gaBody_t *body;
    gaSpot_t *spot;
    gaSpot_t *spot2;
    s32 i;

    memmove( pl->obj->mtx, pl->homeMtx, sizeof( pl->homeMtx ) );
    pl->character.unk_44 = ga_stage->unk_0C;
    pl->character.unk_34 = 1.5707964f;
    pl->character.spin = pl->character.aim[0] = pl->character.stance[0] = 0.0f;
    pl->character.speed = pl->character.minSpeed;
    memmove( lane->bodies, ga_initBodies, sizeof( lane->bodies ) );
    memmove( lane->motion, ga_initMotion, sizeof( ga_initMotion ) );
    spot = lane->spots;
    memmove( spot, ga_initSpots, sizeof( ga_initSpots ) );
    body = lane->bodies;
    spot2 = lane->secondSpots;
    for ( i = 0; i < NUM_BODIES; i++, spot2++ ) {
        if ( i != 0 ) {
            body[i].standState = pins[i - 1];
            body[i].pos[1] += ga_stage->unk_14;
        }
        spot[i].pos[0] = body[i].pos[0];
        spot[i].pos[1] = body[i].pos[1];
        spot[i].pos[2] = body[i].pos[2];
        spot2->contacts = 0;
        spot[i].contacts = 0;
    }
    body = lane->markers;
    spot = lane->markerSpots;
    for ( i = 0; i < 3; i++ ) {
        if ( i == 2 && D_8012D8D8_gacode ) {
            ga_laneAim[pl->laneIndex].unk_00 = body[2].pos[2];
        }
        memmove( &body[i], &D_8012C110_gacode[i], sizeof( gaBody_t ) );
        if ( i == 2 && D_8012D8D8_gacode ) {
            body[2].pos[2] = ga_laneAim[pl->laneIndex].unk_00;
        }
        spot[i].pos[0] = body[i].pos[0];
        spot[i].pos[1] = body[i].pos[1];
        spot[i].pos[2] = body[i].pos[2];
    }
    lane->bodyObjs[0]->viewMask = 0;
    lane->panelFunc = func_8011AB0C_gacode;
    lane->cameraFunc = func_8011B974_gacode;
    lane->callout = 0;
    lane->inputFunc = func_80117AE0_gacode;
    if ( pl->port >= 0 ) {
        D_800DF680[pl->port] = 8;
        D_8006940D[pl->port] = 0;
    }
}

/*
================
GA_SetTurnHandlers

Sets the handlers of a player's turn for the game mode.
================
*/
void GA_SetTurnHandlers( gaPlayer_t *pl ) {
    if ( ga_gameMode == func_801156D8_gacode || ga_gameMode == func_801155BC_gacode ) {
        pl->lane->inputFunc = func_801179FC_gacode;
        pl->lane->panelFunc = func_8011AA78_gacode;
    } else if ( ga_gameMode == func_801157F8_gacode ) {
        pl->lane->inputFunc = func_80117ABC_gacode;
        pl->lane->panelFunc = func_8011AA78_gacode;
    } else {
        pl->lane->inputFunc = func_80117AE0_gacode;
        pl->lane->panelFunc = func_8011AB0C_gacode;
    }
    pl->lane->cameraFunc = func_8011B974_gacode;
}

/*
================
func_80116750_gacode
================
*/
void func_80116750_gacode( gaLane_t *lane ) {
    s32 i;

    lane->bodies[0].standState = 0;
    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->viewMask = ( lane->bodies[i].standState == 1 ) ? 3 : 0;
    }
}

/*
================
GA_HidePins

Hides the pins.
================
*/
void GA_HidePins( gaLane_t *lane ) {
    s32 i;

    for ( i = 1; i <= NUM_PINS; i++ ) {
        lane->bodyObjs[i]->viewMask = 0;
    }
}

/*
================
GA_SoundThrowResult

Sounds the result of a throw from the stage's list: 0x49 for a ball in the gutter, else by how far the
lane's motion[0].vel[2] got, the second list when strong is set; keeps the sound in the lane's soundHandle.

Declared inline: nothing calls this copy; GA_RollBall and GA_RollBallSpin expand it.
================
*/
inline void GA_SoundThrowResult( s32 player, gaPlayer_t *pl, u8 strong ) {
    s32 id;

    if ( pl->lane->spots[0].pos[1] <= -190.0f ) {
        id = 0x49;
    } else if ( pl->lane->motion[0].vel[2] >= -13.0f ) {
        id = strong ? ga_stage->unk_1E[5] : ga_stage->unk_1E[2];
    } else if ( pl->lane->motion[0].vel[2] <= -16.0f ) {
        id = strong ? ga_stage->unk_1E[7] : ga_stage->unk_1E[4];
    } else {
        id = strong ? ga_stage->unk_1E[6] : ga_stage->unk_1E[3];
    }
    pl->lane->soundHandle = GA_PlayLaneSound( pl->laneIndex, id );
}

/*
================
func_801168D0_gacode
Declared inline: nothing calls this copy; GA_RollBall and GA_RollBallSpin expand it.
================
*/
inline s32 func_801168D0_gacode( gaPlayer_t *pl ) {
    gaLane_t *lane = pl->lane;
    f32 height = lane->spots[0].pos[1] - lane->spots[0].radius;

    if ( lane->spots[0].pos[2] > -1186.0f
         && height < ga_stage->unk_14
         && ( lane->motion[0].flags & 1 ) ) {
        GA_StopLaneSound( &lane->soundHandle );
        GA_PlayLaneSound( pl->laneIndex, 0x2F );
        pl->lane->bodies[0].rot[2] = pl->lane->motion[0].spin[2] = 0.0f;
        pl->lane->callout = 3;
        pl->lane->inputFunc = func_80118F68_gacode;
        return 1;
    }
    return 0;
}

/*
================
func_80116998_gacode
Declared inline: nothing calls this copy; GA_RollBall and GA_RollBallSpin expand it.
================
*/
inline s32 func_80116998_gacode( gaPlayer_t *pl ) {
    if ( pl->lane->motion[0].vel[2] >= 0.0f ) {
        GA_StopLaneSound( &pl->lane->soundHandle );
        pl->lane->inputFunc = func_801189C4_gacode;
        return 1;
    }
    return 0;
}

/*
================
GA_StepClamped

Moves *value by delta / 50, within -50 to 50.
================
*/
void GA_StepClamped( s32 player, f32 *value, s16 delta ) {
    *value += delta / 50.0f;
    if ( *value < -50.0f ) {
        *value = -50.0f;
    } else if ( 50.0f < *value ) {
        *value = 50.0f;
    }
}

/*
================
func_80116A64_gacode

Moves the character's speed by the stick's y and spin against its x, each kept in its range.

Declared inline: nothing calls this copy, and func_80118014_gacode and func_80118270_gacode repeat its body.
================
*/
inline void func_80116A64_gacode( gaPlayer_t *pl, s16 stickX, s16 stickY ) {
    if ( stickX == 0 && stickY == 0 ) {
        return;
    }
    pl->character.speed += stickY / 200.0f;
    pl->character.speed = pl->character.speed < pl->character.minSpeed ? pl->character.minSpeed
                         : ( pl->character.speed > pl->character.maxSpeed ? pl->character.maxSpeed : pl->character.speed );
    pl->character.spin -= stickX / 120.0f;
    if ( pl->character.reverseSpin != 0 ) {
        pl->character.spin = pl->character.spin < -pl->character.maxSpin ? -pl->character.maxSpin
                             : ( pl->character.spin > 0.0 ? 0.0f : pl->character.spin );
    } else {
        pl->character.spin = pl->character.spin < 0.0 ? 0.0f
                             : ( pl->character.spin > pl->character.maxSpin ? pl->character.maxSpin : pl->character.spin );
    }
}

/*
================
GA_RollBall

Rolls the ball of a player's throw, unless func_801168D0_gacode or func_80116998_gacode ends it: while the
lane's motion[0].flags bit 1 is set, adds the character's unk_44 to its motion[0].vel[0] and starts the roll's sound;
else stops it.
================
*/
void GA_RollBall( s32 player, gaPlayer_t *pl ) {
    if ( func_801168D0_gacode( pl ) ) {
        return;
    }
    if ( func_80116998_gacode( pl ) ) {
        return;
    }
    if ( ( pl->lane->motion[0].flags >> 1 ) & 1 ) {
        pl->lane->motion[0].vel[0] += pl->character.unk_44;
        if ( pl->lane->soundHandle == 0 ) {
            GA_SoundThrowResult( player, pl, 0 );
        }
    } else {
        GA_StopLaneSound( &pl->lane->soundHandle );
    }
    pl->lane->motion[0].spin[0] = 2.0f;
    pl->lane->motion[0].spin[2] = pl->lane->motion[0].spin[1] * 2.0f;
}

/*
================
GA_RollBallSpin

As GA_RollBall, scaling the character's unk_44 by the stage's unk_10, and turns the lane's
motion[0].vel into the ball's spin.
================
*/
void GA_RollBallSpin( s32 player, gaPlayer_t *pl ) {
    if ( func_801168D0_gacode( pl ) ) {
        return;
    }
    if ( func_80116998_gacode( pl ) ) {
        return;
    }
    if ( ( pl->lane->motion[0].flags >> 1 ) & 1 ) {
        pl->lane->motion[0].vel[0] += pl->character.unk_44 * ga_stage->unk_10;
        if ( pl->lane->soundHandle == 0 ) {
            GA_SoundThrowResult( player, pl, 0 );
        }
    } else {
        GA_StopLaneSound( &pl->lane->soundHandle );
    }
    pl->lane->motion[0].spin[0] = pl->lane->motion[0].vel[2] * 1.5;
    GA_SetBallHeading( pl->lane->motion[0].vel, &pl->lane->bodies[0] );
}

/*
================
func_80116FF0_gacode

Steps the lane's aim to step once x passes its mark (730 - 60 per step): from the step below it the lane just
takes it; from the step above it also plays the step's SE and rumbles the port. Returns 1 when the lane is at
step.

Declared inline: nothing calls this copy, and func_80117124_gacode repeats its body.
================
*/
inline s32 func_80116FF0_gacode( s32 lane, s8 port, f32 x, u8 step ) {
    s16 seIds[5] = { 0x69, 0x68, 0x67, 0x66, 0x65 };

    if ( 730 - step * 60 < x ) {
        if ( ga_laneAim[lane].unk_06 == step ) {
            return 1;
        } else if ( ga_laneAim[lane].unk_06 == step - 1 ) {
            ga_laneAim[lane].unk_06 = step;
            return 1;
        } else if ( ga_laneAim[lane].unk_06 == step + 1 ) {
            GA_PlayLaneSound( lane, seIds[step - 1] );
            if ( port >= 0 ) {
                D_800DF680[port] = 8 - step;
                D_8006940D[port] = 10;
            }
            ga_laneAim[lane].unk_06 = step;
            return 1;
        }
    }
    return 0;
}

/*
================
func_80117124_gacode

Moves the lane's aim marker (markerSpots[2]) while its object is shown: ga_laneAim unk_04 (kept within
999 either way) pushes it on 5 per frame and counts down (rumbling the port on the first push), a negative
one pulls it back 5; else it creeps on 1. Past 730 every player is done and both lanes hand over to the
func_8011C650_gacode view (returns 1); otherwise steps the aim by func_80116FF0_gacode, step 6 once below
every mark. Returns 0.
================
*/
s32 func_80117124_gacode( s32 player, gaPlayer_t *pl ) {
    f32 move;
    s32 i;

    if ( pl->lane->markerObjs[2]->viewMask != 0 ) {
        ga_laneAim[player].unk_04 = ga_laneAim[player].unk_04 < -999 ? -999
                                         : ga_laneAim[player].unk_04 > 999  ? 999
                                                                                   : ga_laneAim[player].unk_04;
        if ( ga_laneAim[player].unk_04 > 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x4E );
            if ( ga_laneAim[player].unk_07 == 0 ) {
                ga_laneAim[player].unk_07 = 1;
                if ( pl->port >= 0 ) {
                    D_800DF680[pl->port] = 3;
                    D_8006940D[pl->port] = ga_laneAim[player].unk_04;
                }
            }
            ga_laneAim[player].unk_04--;
            move = 5.0f;
        } else if ( ga_laneAim[player].unk_04 < 0 ) {
            GA_PlayLaneSound( pl->laneIndex, 0x4F );
            move = -5.0f;
            ga_laneAim[player].unk_07 = 0;
            ga_laneAim[player].unk_04++;
        } else {
            move = 1.0f;
            ga_laneAim[player].unk_07 = 0;
        }
        if ( ( pl->lane->markerSpots[2].pos[2] += move ) < -1096.0f ) {
            pl->lane->markerSpots[2].pos[2] = -1096.0f;
        }
        pl->lane->markerSpots[2].pos[1] = GA_FloorHeight( pl->lane->markerSpots[2].pos[0], pl->lane->markerSpots[2].pos[2], 0 );
        if ( pl->lane->markerSpots[2].pos[2] >= 730.0f ) {
            pl->lane->markerSpots[2].pos[2] = 730.0f;
            for ( i = 0; i != 2; i++ ) {
                D_800994BE[i] = 1;
                if ( i != player ) {
                    D_800EA090[i] = 1;
                } else {
                    GA_PlayLaneSound( player, 0x6A );
                    if ( pl->port >= 0 ) {
                        D_800DF680[pl->port] = 7;
                        D_8006940D[pl->port] = 20;
                    }
                }
                GA_StopLaneSound( &ga_lanes[i].soundHandle );
                ga_lanes[i].frameOver = 1;
                ga_lanes[i].callout = 0x20;
                ga_lanes[i].timer = 0;
                ga_lanes[i].bodies[0].standState = 0;
                ga_lanes[i].bodyObjs[0]->viewMask = ga_lanes[i].markerObjs[0]->viewMask =
                    ga_lanes[i].markerObjs[1]->viewMask = 0;
                ga_lanes[i].cameraFunc = func_8011C650_gacode;
                ga_lanes[i].panelFunc = func_8011B5C8_gacode;
                ga_lanes[i].inputFunc = func_80118E04_gacode;
            }
            return 1;
        }
        if ( !func_80116FF0_gacode( player, pl->port, pl->lane->markerSpots[2].pos[2], 1 )
             && !func_80116FF0_gacode( player, pl->port, pl->lane->markerSpots[2].pos[2], 2 )
             && !func_80116FF0_gacode( player, pl->port, pl->lane->markerSpots[2].pos[2], 3 )
             && !func_80116FF0_gacode( player, pl->port, pl->lane->markerSpots[2].pos[2], 4 )
             && !func_80116FF0_gacode( player, pl->port, pl->lane->markerSpots[2].pos[2], 5 ) ) {
            ga_laneAim[player].unk_06 = 6;
        }
    }
    return 0;
}

/*
================
func_801178D8_gacode

Lane input handler: hides the player and, once the lane's cameraRaise camera offset is down to 0, plays SE 100
(for player 0) and picks the lane's next handlers by the game mode.
================
*/
s32 func_801178D8_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->viewMask = 0;
    if ( pl->lane->cameraRaise > 0.0f ) {
        return 0;
    }
    if ( player == 0 ) {
        Sound_PlaySe( 100, 0 );
    }
    if ( ga_gameMode == func_801156D8_gacode || ga_gameMode == func_801155BC_gacode ) {
        pl->lane->inputFunc = func_801179FC_gacode;
        pl->lane->panelFunc = func_8011AA78_gacode;
    } else if ( ga_gameMode == func_801157F8_gacode ) {
        pl->lane->inputFunc = func_80117ABC_gacode;
        pl->lane->panelFunc = func_8011AA78_gacode;
    } else {
        pl->lane->inputFunc = func_80117AE0_gacode;
        pl->lane->panelFunc = func_8011AB0C_gacode;
    }
    pl->lane->cameraFunc = func_8011B974_gacode;
    return 0;
}

/*
================
func_801179FC_gacode

Lane input handler: on button 1 of a human player, or after 30 frames for the game's own player, sounds
effect 1 and moves the lane on to func_80117AE0_gacode.
================
*/
s32 func_801179FC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];
    s8 port;

    pl->obj->viewMask = 0;
    pl->lane->timer++;
    port = pl->port;
    if ( ( port >= 0 && input == 1 ) || ( port < 0 && pl->lane->timer >= 30 ) ) {
        pl->lane->timer = 0;
        GA_PlayLaneSound( pl->laneIndex, 1 );
        pl->lane->inputFunc = func_80117AE0_gacode;
        pl->lane->panelFunc = func_8011AB0C_gacode;
    }
    return 0;
}

/*
================
func_80117ABC_gacode
================
*/
s32 func_80117ABC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    ga_players[player].obj->viewMask = 0;
    return 0;
}

/*
================
func_80117AE0_gacode

Lane input handler: shows the standing pins (unk_4A 3) and hides the rest and the player, loads the
character's file 1 for it, shows objects markerObjs (2 on lane 1, else 1) and moves on to
func_80117BDC_gacode.
================
*/
s32 func_80117AE0_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];
    s16 *files = pl->character.data->unk_00;
    gaLane_t *lane = pl->lane;
    s32 i;

    lane->bodies[0].standState = 0;
    for ( i = 1; i < NUM_BODIES; i++ ) {
        lane->bodyObjs[i]->viewMask = lane->bodies[i].standState == 1 ? 3 : 0;
    }
    pl->obj->viewMask = 0;
    pl->obj->frame = 0;
    pl->obj->anim = Cache_GetFile( files[1], FILE_TYPE2 );
    pl->lane->markerObjs[0]->viewMask = pl->lane->markerObjs[1]->viewMask = ga_players[player].laneIndex != 0 ? 2 : 1;
    pl->lane->inputFunc = func_80117BDC_gacode;
    return 0;
}


/*
================
func_80117BDC_gacode

Lane input handler: unless func_80117124_gacode handles the frame, turns the lane's markers[0] and on 1 goes
on to func_80117D20_gacode; else the stick moves the character's aim (within 50) and the lane's
markerSpots[0] with it.
================
*/
s32 func_80117BDC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( func_80117124_gacode( player, pl ) ) {
        return 0;
    }
    pl->obj->frame = pl->character.data->unk_19;
    pl->lane->markers[0].rot[1] += 8.0;
    pl->lane->markers[1].rot[1] = 0.0f;
    if ( input == 1 ) {
        GA_PlayLaneSound( pl->laneIndex, 0xB );
        pl->lane->cameraFunc = func_8011BA00_gacode;
        pl->lane->inputFunc = func_80117D20_gacode;
        return 0;
    }
    pl->character.aim[0] += stickX / 50.0f;
    if ( pl->character.aim[0] < -50.0f ) {
        pl->character.aim[0] = -50.0f;
    } else if ( pl->character.aim[0] > 50.0f ) {
        pl->character.aim[0] = 50.0f;
    }
    pl->lane->markerSpots[0].pos[0] = pl->character.aim[0];
    return 0;
}

/*
================
func_80117D20_gacode

Lane input handler: unless func_80117124_gacode handles the frame, turns the lane's markers[1]; 1 moves the
player's object across to the character's stance, sets its direction from stance and aim and goes on to
func_80117EFC_gacode, -1 goes back to func_80117BDC_gacode; else the stick moves the character's stance[0]
(within 50) and the lane's markerSpots[1] with it.
================
*/
s32 func_80117D20_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];
    f32 mf[4][4];

    if ( func_80117124_gacode( player, pl ) ) {
        return 0;
    }
    pl->obj->frame = pl->character.data->unk_19;
    pl->lane->markers[0].rot[1] = 0.0f;
    pl->lane->markers[1].rot[1] += 8.0;
    if ( input == 1 ) {
        GA_PlayLaneSound( pl->laneIndex, 0xB );
        pl->obj->frame = 0;
        pl->lane->markerObjs[0]->viewMask = pl->lane->markerObjs[1]->viewMask = 0;
        guTranslateF( mf, pl->character.stance[0] - pl->lane->spots[0].pos[0], 0.0f, 0.0f );
        guMtxCatF( pl->obj->mtx, mf, pl->obj->mtx );
        GA_VecDirection( pl->character.direction, pl->character.stance, pl->character.aim );
        pl->lane->inputFunc = func_80117EFC_gacode;
        return 0;
    }
    if ( input == -1 ) {
        GA_PlayLaneSound( pl->laneIndex, 0xC );
        pl->lane->cameraFunc = func_8011B974_gacode;
        pl->lane->inputFunc = func_80117BDC_gacode;
        return 0;
    }
    pl->character.stance[0] += stickX / 50.0f;
    if ( pl->character.stance[0] < -50.0f ) {
        pl->character.stance[0] = -50.0f;
    } else if ( pl->character.stance[0] > 50.0f ) {
        pl->character.stance[0] = 50.0f;
    }
    pl->lane->markerSpots[1].pos[0] = pl->character.stance[0];
    return 0;
}

/*
================
func_80117EFC_gacode

inputFunc handler: shows the player and the ball and picks the lane's next handlers by whether its markerObjs[2]
object is shown.
================
*/
s32 func_80117EFC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->frame = 0;
    GA_LoadLanePalettes( pl->laneIndex, 0, pl->ballWeight );
    pl->lane->motion[0].mass = D_801239E8_gacode[pl->ballWeight];
    pl->lane->bodyObjs[0]->viewMask = pl->obj->viewMask = pl->laneIndex != 0 ? 2 : 1;
    if ( pl->port >= 0 ) {
        D_800DF680[pl->port] = 8;
        D_8006940D[pl->port] = 0;
    }
    pl->lane->cameraFunc = func_8011BA8C_gacode;
    if ( pl->lane->markerObjs[2]->viewMask != 0 ) {
        pl->lane->panelFunc = func_8011B07C_gacode;
        pl->lane->inputFunc = func_80118270_gacode;
    } else {
        pl->lane->panelFunc = func_8011AD5C_gacode;
        pl->lane->inputFunc = func_80118014_gacode;
    }
    return 0;
}

/*
================
func_80118014_gacode

Lane input handler: 1 goes on to func_80118270_gacode; -1 backs out (hides the player and the ball, restores
the player's placement and goes back to func_80117D20_gacode); else the stick moves the character's speed and
spin.
================
*/
s32 func_80118014_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->frame = 0;
    if ( input == 1 ) {
        GA_PlayLaneSound( pl->laneIndex, 0xB );
        pl->lane->panelFunc = func_8011B07C_gacode;
        pl->lane->inputFunc = func_80118270_gacode;
        return 0;
    }
    if ( input == -1 ) {
        GA_PlayLaneSound( pl->laneIndex, 0xC );
        pl->obj->frame = pl->character.data->unk_19;
        pl->lane->bodyObjs[0]->viewMask = pl->obj->viewMask = 0;
        pl->lane->markerObjs[0]->viewMask = pl->lane->markerObjs[1]->viewMask = pl->laneIndex != 0 ? 2 : 1;
        memmove( pl->obj->mtx, pl->homeMtx, sizeof( pl->homeMtx ) );
        pl->lane->panelFunc = func_8011AB0C_gacode;
        pl->lane->cameraFunc = func_8011BA00_gacode;
        pl->lane->inputFunc = func_80117D20_gacode;
        return 0;
    }
    func_80116A64_gacode( pl, stickX, stickY );
    return 0;
}

/*
================
func_80118270_gacode

Lane input handler: the stick moves the character's speed and spin until the player object's unk_48 passes
the character's unk_18, then goes on to func_8011840C_gacode.
================
*/
s32 func_80118270_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( pl->obj->frame > pl->character.data->unk_18 ) {
        pl->lane->panelFunc = func_8011B1A0_gacode;
        pl->lane->inputFunc = func_8011840C_gacode;
        return 0;
    }
    func_80116A64_gacode( pl, stickX, stickY );
    return 0;
}

/*
================
func_8011840C_gacode

Lane input handler: commits the throw (keeps the player object's unk_48 in releaseFrame and goes on to
func_80118524_gacode) once the sine of the character's unk_34 is down to -0.3, on B at the last frame when
ga_earlyCommit allows it, or on input 1 while the sine is at most 0.5.
================
*/
s32 func_8011840C_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( sinf( pl->character.unk_34 ) <= -0.3 ||
         ( ga_earlyCommit && pl->port >= 0 && ( D_800E57E4[pl->port] & B_BUTTON ) &&
           pl->obj->frame == pl->character.data->unk_19 ) ||
         ( sinf( pl->character.unk_34 ) <= 0.5 && input == 1 ) ) {
        pl->releaseFrame = pl->obj->frame;
        pl->lane->inputFunc = func_80118524_gacode;
        pl->lane->panelFunc = func_8011B2E8_gacode;
        return 0;
    }
    return 0;
}

/*
================
func_80118524_gacode

Lane input handler at the release: sets the ball's standState, aims from the ball's spot towards the character's
aim (into direction), sets the ball's velocity from speed (turned by the sine and cosine of unk_34, the sideways
part curved by its square), its spin from spin, scales unk_44 by the spin, picks the stage's unk_1E[1] or
unk_1E[0] by how far unk_34 turns the throw, and hands over to func_801187D0_gacode.
================
*/
s32 func_80118524_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];
    vec3_t from;
    s32 side;
    f32 bend;
    f32 dir;

    GA_LoadLanePalettes( pl->laneIndex, 1, pl->ballWeight );
    if ( pl->obj->frame == pl->character.data->unk_1A ) {
        pl->obj->frame--;
    }
    pl->lane->bodies[0].standState = 1;
    from[0] = pl->lane->spots[0].pos[0];
    from[1] = pl->character.stance[1];
    from[2] = pl->character.stance[2];
    GA_VecDirection( pl->character.direction, from, pl->character.aim );
    pl->lane->motion[0].vel[1] = pl->character.speed * pl->character.direction[2];
    pl->lane->motion[0].vel[2] = pl->character.speed * pl->character.direction[2];
    pl->lane->motion[0].vel[0] = pl->character.speed * pl->character.direction[0];
    side = pl->character.data->unk_08 < 0 ? -1 : 1;
    pl->lane->motion[0].vel[1] *= sinf( pl->character.unk_34 ) * side;
    pl->lane->motion[0].vel[2] *= cosf( pl->character.unk_34 );
    side = pl->character.reverseSpin ? -1 : 1;
    bend = sinf( pl->character.unk_34 ) * 10.0f;
    dir = bend < 0.0f ? -1.0f : 1.0f;
    pl->lane->motion[0].vel[0] += pl->character.speed / 2.0f * ( dir * bend * bend / 100.0f ) * side;
    pl->lane->motion[0].spin[1] = pl->character.spin;
    pl->character.unk_44 = pl->character.unk_44 * ( -pl->lane->motion[0].spin[1] / 15.0 );
    if ( FABS( sinf( pl->character.unk_34 ) ) >= 0.15 ) {
        pl->lane->bounceSound = ga_stage->unk_1E[1];
    } else {
        pl->lane->bounceSound = ga_stage->unk_1E[0];
    }
    pl->lane->inputFunc = func_801187D0_gacode;
    return 0;
}

/*
================
func_801187D0_gacode

Lane input handler while the ball rolls: halves the character's unk_44 when the ball's motion flag bit 6 is set,
hands the view to the ball at the character's unk_1A frame, rolls the ball (GA_RollBallSpin while bit 1
is set and the ball is short of the stage's unk_08, else GA_RollBall), switches the camera at -1050,
lets Z steer it by the stick when ga_steerBall is set, and at -1186 stops the roll's sound and hands
over to func_801189C4_gacode.
================
*/
s32 func_801187D0_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( ( pl->lane->motion[0].flags >> 6 ) & 1 ) {
        pl->character.unk_44 /= 2.0f;
    }
    if ( pl->obj->frame == pl->character.data->unk_1A ) {
        pl->obj->frame--;
        pl->lane->bodyObjs[0]->viewMask = 3;
        pl->obj->viewMask = 0;
        pl->lane->panelFunc = func_8011B400_gacode;
        pl->lane->cameraFunc = func_8011BB44_gacode;
    }
    if ( ( ( pl->lane->motion[0].flags >> 1 ) & 1 ) && pl->lane->spots[0].pos[2] < ga_stage->unk_08 ) {
        pl->lane->bodies[0].rot[2] = pl->lane->motion[0].spin[2] = 0.0f;
        GA_RollBallSpin( player, pl );
    } else {
        GA_RollBall( player, pl );
    }
    if ( pl->lane->spots[0].pos[2] <= -1050.0 ) {
        pl->lane->cameraFunc = func_8011BC10_gacode;
    }
    if ( ga_steerBall && pl->port >= 0 && ( D_800E57E4[pl->port] & Z_TRIG ) ) {
        pl->lane->motion[0].vel[0] += stickX / 1800.0f;
    }
    if ( pl->lane->spots[0].pos[2] <= -1186.0f ) {
        GA_StopLaneSound( &pl->lane->soundHandle );
        pl->lane->inputFunc = func_801189C4_gacode;
    }
    return 0;
}

/*
================
func_801189C4_gacode

Lane input handler after the roll: waits 45 frames, then (outside modes 2 and sub-mode 2) raises the stage's
unk_10 by 0.05, hides the ball, counts the pins knocked down this throw (standState 2: marked in the frame's
first pin mask after a func_80116398_gacode turn, else in the second) into pinsHit and the pins down into
pinsDown, and asks the game mode for the result: nonzero shows the player again, hides the fallen pins, picks
the character's reaction (1 to 3) and hands over to func_80118CCC_gacode, else to func_80118E04_gacode.
================
*/
s32 func_801189C4_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];
    s16 *files = pl->character.data->unk_00;
    s32 scorer;
    s32 result;
    s32 reaction;
    s32 i;

    pl->obj->frame = 0;
    if ( ++pl->lane->timer != 45 ) {
        return 0;
    }
    pl->lane->timer = 0;
    if ( ( D_8005C160 & 0xF ) != 2 && ( ( D_8005C160 >> 4 ) & 3 ) != 2 ) {
        ga_stage->unk_10 += 0.05;
    }
    pl->lane->bodies[0].standState = 0;
    pl->lane->bodyObjs[0]->viewMask = 0;
    if ( ( D_8005C160 >> 6 ) == 1 ) {
        scorer = player >= 2;
    } else {
        scorer = player;
    }
    pl->lane->pinsHit = pl->lane->pinsDown = 0;
    for ( i = 1; i < NUM_BODIES; i++ ) {
        if ( pl->lane->bodies[i].standState == 2 ) {
            pl->lane->bodies[i].standState = 0;
            pl->lane->pinsHit++;
            if ( pl->lane->turnFunc == func_80116398_gacode ) {
                D_800A3050[scorer][D_80066C00.unk_20[scorer]] |= 1 << ( 10 - i );
            } else {
                D_800F7C18[scorer][D_80066C00.unk_20[scorer]] |= 1 << ( 10 - i );
            }
        }
        if ( pl->lane->bodies[i].standState == 0 ) {
            pl->lane->pinsDown++;
        }
    }
    if ( pl->lane->callout == 3 && pl->lane->pinsHit != 0 ) {
        pl->lane->callout = 0;
    }
    result = ga_gameMode( player );
    if ( result != 0 ) {
        pl->obj->viewMask = pl->laneIndex ? 2 : 1;
        for ( i = 1; i < NUM_BODIES; i++ ) {
            if ( pl->lane->bodies[i].standState == 0 ) {
                pl->lane->bodyObjs[i]->viewMask = 0;
            }
        }
        if ( result > 0 ) {
            if ( result < 4 ) {
                reaction = result;
            } else {
                reaction = 3;
            }
        } else {
            reaction = 1;
        }
        pl->lane->reaction = reaction - 1;
        pl->obj->frame = 0;
        pl->obj->anim = Cache_GetFile( files[pl->lane->reaction + 2], 2 );
        pl->lane->inputFunc = func_80118CCC_gacode;
    } else {
        pl->lane->inputFunc = func_80118E04_gacode;
    }
    if ( pl->lane->callout != 0 ) {
        pl->lane->panelFunc = func_8011B408_gacode;
    }
    return 0;
}

/*
================
func_80118CCC_gacode
================
*/
s32 func_80118CCC_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->lane->cameraFunc = func_8011BCA8_gacode;
    pl->lane->timer++;
    if ( pl->obj->frame == pl->character.data->unk_1B[pl->lane->reaction] - 1
         || ( pl->lane->timer >= 30 && ( D_800E44E0[pl->laneIndex] & ( A_BUTTON | Z_TRIG ) ) ) ) {
        pl->lane->timer = 0;
        pl->lane->cameraFunc = func_8011C650_gacode;
        D_8012D8D8_gacode++;
        pl->lane->inputFunc = func_80118D9C_gacode;
    }
    return 0;
}

/*
================
func_80118D9C_gacode
================
*/
s32 func_80118D9C_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->frame = 0;
    pl->obj->viewMask = 0;
    pl->lane->panelFunc = func_8011B400_gacode;
    pl->lane->cameraFunc = func_8011C0DC_gacode;
    return D_8012D23C_gacode( player );
}

/*
================
func_80118E04_gacode

Lane input handler: after the throw (40 frames of it when callout is set), starts the next turn.
================
*/
s32 func_80118E04_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( pl->obj->frame != 0 ) {
        pl->obj->frame--;
    }
    pl->lane->timer++;
    if ( pl->lane->callout != 0 ) {
        if ( pl->lane->timer < 40 ) {
            return 0;
        }
        pl->lane->timer = 0;
    } else {
        pl->lane->timer = 0;
    }
    pl->obj->frame = 0;
    D_8012D8D8_gacode++;
    return D_8012D23C_gacode( player );
}

/*
================
func_80118EA0_gacode
================
*/
s32 func_80118EA0_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->viewMask = pl->laneIndex ? 2 : 1;
    if ( pl->obj->frame == pl->character.data->unk_1B[pl->lane->reaction] ) {
        pl->obj->frame--;
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
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->viewMask = 0;
    pl->obj->frame = 0;
    return 0;
}

/*
================
func_80118F38_gacode
================
*/
s32 func_80118F38_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    pl->obj->viewMask = 0;
    pl->obj->frame = 0;
    return 0;
}

/*
================
func_80118F68_gacode

Lane input handler while the ball rolls, as func_801187D0_gacode with the strong sounds: hands the view to the ball once the player's animation is past
the character's unk_18 frame, spins the ball by its speed down the lane, sounds the roll from the stage's
second list while motion[0].flags bit 1 is set (else stops it), switches the camera at -1050, and at -1186
or once the ball stops going down the lane stops the sound and hands over to func_801189C4_gacode.
================
*/
s32 func_80118F68_gacode( s32 player, s32 input, s16 stickX, s16 stickY ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( pl->obj->frame <= pl->character.data->unk_18 ) {
        pl->obj->frame = pl->character.data->unk_1A;
        pl->lane->bodyObjs[0]->viewMask = 3;
        pl->obj->viewMask = 0;
        pl->lane->cameraFunc = func_8011BB44_gacode;
    }
    pl->lane->motion[0].spin[0] = pl->lane->motion[0].vel[2];
    if ( ( pl->lane->motion[0].flags >> 1 ) & 1 ) {
        if ( pl->lane->soundHandle == 0 ) {
            GA_SoundThrowResult( player, pl, 1 );
        }
    } else {
        GA_StopLaneSound( &pl->lane->soundHandle );
    }
    if ( pl->lane->spots[0].pos[2] <= -1050.0 ) {
        pl->lane->cameraFunc = func_8011BC10_gacode;
    }
    if ( pl->lane->spots[0].pos[2] <= -1186.0f ) {
        GA_StopLaneSound( &pl->lane->soundHandle );
        pl->lane->inputFunc = func_801189C4_gacode;
    }
    if ( pl->lane->motion[0].vel[2] >= 0.0f ) {
        GA_StopLaneSound( &pl->lane->soundHandle );
        pl->lane->inputFunc = func_801189C4_gacode;
    }
    return 0;
}

/*
================
GA_DrawScorePanel

Draws a player's score panel on the lane's side: the name, the row of three frames ending at the current
one (from the scores of the lane's team player in a team game), and the games won.
================
*/
void GA_DrawScorePanel( s32 player, u8 lane ) {
    s32 x;
    s16 y;
    s32 scorer;
    s32 first;
    s32 i;

    x = 0xAC;
    if ( lane == 0 ) {
        x = 0x18;
    }
    Menu_DrawNamePlate( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, x, 0x19 );
    x += 0x38;
    y = 0x10;
    if ( ( D_8005C160 >> 6 ) == 1 ) {
        scorer = player >= 2;
    } else {
        scorer = player;
    }
    if ( D_8009D19C[scorer] < 2 ) {
        first = 0;
    } else {
        first = D_8009D19C[scorer] - 2;
    }
    Score_DrawFrameHeaders( 3, first, x, y );
    for ( i = 0; i < 3; i++ ) {
        Score_DrawFrame( scorer, first + i, D_8009D19C[scorer], D_80068A50[scorer], D_800994BE[scorer], x, y + 9 );
        x += 0x14;
    }
    x = 0xAC;
    if ( lane == 0 ) {
        x = 0x78;
    }
    Menu_DrawBox( player, D_800693DC[player], 0, x, y + 0x20 );
}

/*
================
GA_DrawPinDiagram

Draws the pin diagram of the player's lane on its side of the screen: the frame (0x92A), then image 0x929 for
each pin still standing. Declared inline: func_8011AB0C_gacode and func_8011AD5C_gacode repeat its body.
================
*/
inline void GA_DrawPinDiagram( gaPlayer_t *pl ) {
    s16 offsets[NUM_BODIES][2] = {
        { 0x00, 0x00 },                 /* the ball: unused */
        { 0x10, 0x1C },
        { 0x0C, 0x14 }, { 0x14, 0x14 },
        { 0x08, 0x0C }, { 0x10, 0x0C }, { 0x18, 0x0C },
        { 0x04, 0x04 }, { 0x0C, 0x04 }, { 0x14, 0x04 }, { 0x1C, 0x04 },
    };
    s16 x = pl->laneIndex != 0 ? 0xFC : 0x18;
    s16 y = 0x30;
    s32 i;

    Draw_Image( 0x92A, x, y );
    for ( i = 1; i < NUM_BODIES; i++ ) {
        if ( pl->lane->bodies[i].standState == 1 ) {
            Draw_Image( 0x929, x + offsets[i][0], y + offsets[i][1] );
        }
    }
}

/*
================
func_801194D0_gacode

Draws a human player's ball weight selector on its lane's side, and steps ballWeight (0 to 10, wrapping) down on C-left
and up on C-right.
================
*/
void func_801194D0_gacode( gaPlayer_t *pl ) {
    s32 x;
    s16 y;

    if ( pl->port < 0 ) {
        return;
    }
    x = pl->laneIndex == 0 ? 0x6C : 0xA4;
    y = 0xC8;
    Draw_Image( 0xAF7, x, y );
    Draw_Image( D_8012C7F4_gacode[pl->ballWeight], x + 0x10, y );
    Draw_Image( 0xAF5, x + 0x20, y );
    Score_DrawNumber( pl->ballWeight + 6, x - 6, 0xD9 );
    Draw_Image( 0xAF3, x + 0x10, 0xD8 );
    if ( D_800E44E0[pl->port] & L_CBUTTONS ) {
        Draw_Image( 0xAF6, x, y );
        GA_PlayLaneSound( pl->laneIndex, 0 );
        pl->ballWeight = pl->ballWeight == 0 ? 10 : pl->ballWeight - 1;
    }
    if ( D_800E44E0[pl->port] & R_CBUTTONS ) {
        Draw_Image( 0xAF4, x + 0x20, y );
        GA_PlayLaneSound( pl->laneIndex, 0 );
        if ( pl->ballWeight == 10 ) {
            pl->ballWeight = 0;
        } else {
            pl->ballWeight++;
        }
    }
}

/*
================
func_80119638_gacode

Sets the lane's two gauge sprites from the player's speed (between its limits) and spin (against its
range), and picks the lane's D_801236B0_gacode image by the size of spin (full at 15), from the second
half of the list when reverseSpin is set.
================
*/
void func_80119638_gacode( gaPlayer_t *pl ) {
    f32 t;

    GA_FillPaletteFraction( &D_8012D240_gacode[pl->laneIndex], 0,
                          ( pl->character.speed - pl->character.minSpeed ) / ( pl->character.maxSpeed - pl->character.minSpeed ) );
    t = FABS( pl->character.spin ) / FABS( pl->character.maxSpin );
    GA_FillPaletteFraction( &D_8012CE10_gacode[pl->laneIndex], 0, t );
    t = FABS( pl->character.spin ) / 15.0;
    if ( t < 0.0 ) {
        t = 0.0f;
    } else if ( t > 1.0 ) {
        t = 1.0f;
    }
    D_801236B0_gacode[pl->laneIndex].cur = (u16)( D_801236B0_gacode[pl->laneIndex].count * t ) + ( pl->character.reverseSpin ? 8 : 0 );
}

/*
================
func_80119800_gacode

Draws a human player's panel on its lane's side: the frame, its second part when full is set, and the
current image of the lane's D_801236B0_gacode list. Declared inline: func_8011AD5C_gacode repeats its body
with full set.
================
*/
inline void func_80119800_gacode( s32 player, u8 full ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 x;
    s16 y;

    if ( pl->port < 0 ) {
        return;
    }
    if ( pl->laneIndex != 0 ) {
        x = 0xB0;
    } else {
        x = 0x18;
    }
    y = 0xA8;
    Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA46 : 0xA48, x, y );
    if ( full ) {
        Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA47 : 0xA49, x + 0x1D, y );
    }
    Menu_DrawImage( D_801236B0_gacode[pl->laneIndex].ids[D_801236B0_gacode[pl->laneIndex].cur], x + 0xD, y + 0x11 );
}

/*
================
GA_DrawParPanel

Draws the par game's panel for a player on the lane's side: the name, the frame and its par, the par
mark, and a mark per throw of the frame, the last one blended.
================
*/
void GA_DrawParPanel( s32 player, u8 lane ) {
    s16 x;
    s16 y;
    s32 i;

    x = lane != 0 ? 0xA8 : 0x18;
    Menu_DrawNamePlate( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, x, 0x18 );
    x += 0x38;
    Draw_Image( 0xA3B, x, 0x10 );
    Score_DrawNumber( ga_parFrame + 1, x + 0x24, 0x15 );
    Score_DrawNumber( D_80099458[0][ga_parFrame], x + 0x24, 0x21 );
    y = 0x30;
    if ( lane != 0 ) {
        x -= 0x38;
    }
    Draw_Image( D_80099458[0][ga_parFrame] == 1 ? 0xA1B : 0xA1A, x, y );
    x += 2;
    for ( i = 0; i < D_80066E18[player][D_8009D19C[player]]; i++ ) {
        Draw_Image( 0xA19, x, y );
        x += 0xA;
    }
    Draw_ImageScaled( 0xA19, x, y, (u8)ga_blinkLevel + 0x1F, 1.0f, 1.0f );
}

/*
================
GA_DrawParFramePanel

Draws the par game's frame panel on the lane's side: the frame number, its par, and a chart of the pins
the frame starts with.
================
*/
void GA_DrawParFramePanel( s32 player, u8 lane ) {
    u8 pins[NUM_PINS];
    s16 x;
    s16 y;
    s32 i;

    x = lane != 0 ? 0xB0 : 0x20;
    y = 0x30;
    Draw_Image( 0xA1D, x, y );
    Draw_Image( ga_numberImages[ga_parFrame + 1], x + 0x48, 0x36 );
    Draw_Image( ga_numberImages[D_80099458[0][ga_parFrame]], x + 0x48, 0x46 );
    GA_UnpackPinMask( pins, D_80123950_gacode[D_800DF59C.unk_04[ga_parFrame]].pins );
    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( pins[i] == 1 ) {
            Draw_Image( 0xA2B, x + ga_parChartPins[i][0], y + ga_parChartPins[i][1] );
        }
    }
    Draw_Image( D_8012C848_gacode[D_8005C163], x + 0xC, y + 0x80 );
}

/*
================
func_80119C50_gacode

Draws a player's frame panel on the lane's side: the name, the frame number and a mark per D_80068A50
count, the last one blended.
================
*/
void func_80119C50_gacode( s32 player, u8 lane ) {
    s16 digits[4];
    s16 x;
    s16 y;
    s32 i;

    x = lane != 0 ? 0xA8 : 0x18;
    Menu_DrawNamePlate( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, x, 0x18 );
    x += 0x38;
    Draw_Image( 0xAB7, x, 0x10 );
    Score_GetDigits( digits, D_8009D19C[player] + 1, 0, 0 );
    Menu_DrawImage( digits[2], x + 0x30, 0x11 );
    Menu_DrawImage( digits[3], x + 0x38, 0x11 );
    x += 0xF;
    y = 0x20;
    for ( i = 0; i < D_80068A50[player] - 1; i++ ) {
        Draw_Image( 0xADB, x, y );
        x += 0xA;
    }
    Draw_ImageScaled( 0xADB, x, y, (u8)ga_blinkLevel + 0x40, 1.0f, 1.0f );
}

/*
================
func_80119DD8_gacode

Draws the pin panel of a player's frame on the lane's side: the frame, a chart of the pins the frame's
setup starts with, the frame number, the setup's image and a mark per unk_00 count.
================
*/
void func_80119DD8_gacode( s32 player, u8 lane ) {
    u8 pins[NUM_PINS];
    s16 x;
    s16 y;
    s32 setup;
    s32 i;

    x = lane != 0 ? 0xBC : 0x24;
    y = 0x40;
    setup = D_800DF59C.unk_04[D_8009D19C[player]];
    Draw_Image( 0xABA, x, y );
    GA_UnpackPinMask( pins, D_801237F0_gacode[setup].pins );
    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( pins[i] == 1 ) {
            Draw_Image( 0xAB9, x + ga_pinPanelPins[i][0], y + ga_pinPanelPins[i][1] );
        }
    }
    Score_DrawNumber( D_8009D19C[player] + 1, x + 0x3E, y + 6 );
    Draw_Image( D_801237F0_gacode[setup].unk_04, x + 7, y + 0x11 );
    for ( i = 0; i < D_801237F0_gacode[setup].unk_00; i++ ) {
        Draw_Image( 0xAB8, x + 0x26 + i * 0xA, y + 0x68 );
    }
}

/*
================
func_80119F8C_gacode

Draws a player's aim panel on the lane's side (over a dark band while the lane's aim step is below 6): the
frame, the name, the aim value of ga_laneAim with its sign mark (blended when positive), and the
distance of the lane's markerSpots[2] from 730 units, in four digits.
================
*/
void func_80119F8C_gacode( s32 player, u8 lane ) {
    gaPlayer_t *pl = &ga_players[player];
    s16 digits[4];
    s16 x;
    s16 y;
    s16 aim;

    if ( ga_laneAim[lane].unk_06 < 6 ) {
        Draw_FillRect( lane ? 0xA0 : 0, 0, 0xA0, 0xF0, 0xFF, 0x7F, 0, (u8)ga_blinkLevel );
    }
    x = lane ? 0xA8 : 0x18;
    Draw_Image( 0x9E0, x + 0x38, 0x10 );
    Draw_Image( 0x9DF, x + 0x58, 0x10 );
    Menu_DrawNamePlate( player, D_8005C0E4[player] >> 7, D_80068218[player].chars, x, 0x19 );
    x += 0x38;
    y = 0x19;
    Draw_Image( 0x86D, x, y );
    aim = ga_laneAim[player].unk_04;
    Score_DrawNumber( aim < 0 ? -aim : aim, x + 0xB, 0x20 );
    if ( ga_laneAim[player].unk_04 != 0 ) {
        Draw_Image( ga_laneAim[player].unk_04 < 0 ? 0x9F0 : 0x9EF, x + 3, 0x1D );
        if ( ga_laneAim[player].unk_04 > 0 ) {
            Draw_ImageScaled( 0x86C, x, y, (u8)ga_blinkLevel + 0x40, 1.0f, 1.0f );
        }
    }
    x += 0x20;
    Draw_Image( 0x86B, x, y );
    Score_GetDigits( digits, FABS( 730.0f - pl->lane->markerSpots[2].pos[2] ), 1, 0 );
    y += 4;
    Menu_DrawImage( digits[0], x + 0x5, y );
    Menu_DrawImage( digits[1], x + 0xD, y );
    Menu_DrawImage( digits[2], x + 0x15, y );
    Menu_DrawImage( digits[3], x + 0x1D, y );
}

/*
================
GA_DrawDistancePanels

Draws both players' distance panels: for each lane side the frame, the player's name, and the distance of
the lane's markerSpots[2] from 730 units, in four digits.
================
*/
void GA_DrawDistancePanels( void ) {
    gaPlayer_t *pl;
    s16 digits[4];
    s32 i;
    s16 x;
    s16 y;

    for ( i = 0; i < 2; i++ ) {
        x = i != 0 ? 0xB8 : 0x28;
        y = 0x60;
        pl = &ga_players[i];
        Draw_Image( 0x9DF, x + 0x38, y );
        y += 9;
        Menu_DrawNamePlate( i, D_8005C0E4[i] >> 7, D_80068218[i].chars, x, y );
        x += 0x38;
        Draw_Image( 0x86B, x, y );
        Score_GetDigits( digits, FABS( 730.0f - pl->lane->markerSpots[2].pos[2] ), 1, 0 );
        y += 4;
        Menu_DrawImage( digits[0], x + 5, y );
        Menu_DrawImage( digits[1], x + 0xD, y );
        Menu_DrawImage( digits[2], x + 0x15, y );
        Menu_DrawImage( digits[3], x + 0x1D, y );
    }
}

/*
================
GA_TogglePin

Stands body i of a lane's bodies up if it is not standing, else knocks it down (standState 1 or 0), and returns the
new state. Retail has no out-of-line copy; GA_PinEditor expands it (the call gives the cursor index and
the body address their own registers, which the open-coded toggle does not).
================
*/
static inline u8 GA_TogglePin( gaBody_t *bodies, s32 i ) {
    return bodies[i].standState = bodies[i].standState != 1;
}

/*
================
GA_PinEditor

Pin editor of func_801157F8_gacode's mode: draws the pin chart of ga_editorPins with the cursor (on a pin,
or on the button below the chart), and unless the overlay is leaving reads the first controller: up enters
the chart or moves up a row, down moves down a row (off the top row back to the button), left and right move
along a row, and A or Z on the button starts the throw (func_80117AE0_gacode), on a pin stands it up or knocks
it down.
================
*/
void GA_PinEditor( gaPlayer_t *pl ) {
    u8 pins[NUM_PINS];
    s16 x;
    s16 y;
    s32 i;

    x = 0x20;
    y = 0x30;
    Draw_Image( 0x881, x, y );
    GA_UnpackPinMask( pins, ga_editorPins );
    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( pins[i] == 1 ) {
            Draw_Image( 0x883, x + ga_editorPinPos[i][0], y + ga_editorPinPos[i][1] );
        }
    }
    if ( ga_editorOnPins ) {
        Draw_ImageScaled( 0x887, x + ga_editorPinPos[ga_editorCursor][0] - 1,
                       y + ga_editorPinPos[ga_editorCursor][1] - 1, (u8)ga_blinkLevel + 0x7F, 1.0f, 1.0f );
    }
    if ( !ga_editorOnPins ) {
        Draw_ImageScaled( 0x886, x + 8, y + 0x7A, (u8)ga_blinkLevel + 0x30, 1.0f, 1.0f );
    }
    if ( D_8005C0BF ) {
        return;
    }
    if ( D_800E6760[0] & U_JPAD ) {
        if ( !ga_editorOnPins ) {
            Sound_PlaySe( 0, 0 );
            ga_editorOnPins = 1;
            ga_editorCursor = 0;
        } else {
            switch ( ga_editorCursor ) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    Sound_PlaySe( 0, 0 );
                    break;
            }
            switch ( ga_editorCursor ) {
                case 0:
                    ga_editorCursor = 1;
                    break;
                case 1:
                case 2:
                    ga_editorCursor = 4;
                    break;
                case 3:
                    ga_editorCursor = 6;
                    break;
                case 4:
                    ga_editorCursor = 7;
                    break;
                case 5:
                    ga_editorCursor = 8;
                    break;
            }
        }
    }
    if ( ga_editorOnPins == 1 ) {
        if ( D_800E6760[0] & D_JPAD ) {
            Sound_PlaySe( 0, 0 );
            switch ( ga_editorCursor ) {
                case 0:
                    ga_editorOnPins = 0;
                    break;
                case 1:
                case 2:
                    ga_editorCursor = 0;
                    break;
                case 3:
                case 4:
                    ga_editorCursor = 1;
                    break;
                case 5:
                    ga_editorCursor = 2;
                    break;
                case 6:
                    ga_editorCursor = 3;
                    break;
                case 7:
                case 8:
                    ga_editorCursor = 4;
                    break;
                case 9:
                    ga_editorCursor = 5;
                    break;
            }
        }
        if ( D_800E6760[0] & L_JPAD ) {
            switch ( ga_editorCursor ) {
                case 2:
                case 4:
                case 5:
                case 7:
                case 8:
                case 9:
                    Sound_PlaySe( 0, 0 );
                    ga_editorCursor--;
                    break;
            }
        }
        if ( D_800E6760[0] & R_JPAD ) {
            switch ( ga_editorCursor ) {
                case 1:
                case 3:
                case 4:
                case 6:
                case 7:
                case 8:
                    Sound_PlaySe( 0, 0 );
                    ga_editorCursor++;
                    break;
            }
        }
    }
    if ( D_800E44E0[0] & ( A_BUTTON | Z_TRIG ) ) {
        if ( !ga_editorOnPins ) {
            Sound_PlaySe( 1, 0 );
            pl->lane->inputFunc = func_80117AE0_gacode;
            pl->lane->panelFunc = func_8011AB0C_gacode;
        } else {
            Sound_PlaySe( 0xB, 0 );
            if ( GA_TogglePin( pl->lane->bodies, ga_editorCursor + 1 ) == 1 ) {
                ga_editorPins |= 1 << ( 9 - ga_editorCursor );
            } else {
                ga_editorPins &= ~( 1 << ( 9 - ga_editorCursor ) );
            }
        }
    }
}

/*
================
GA_DrawAimLine

Draws the lane's aim line between its markers[0] and markers[1] bodies, in a colour that cycles: two
channels follow the cosine of ga_aimColorAngle, the third is full, and the channels rotate (ga_aimChannelOrders)
each time the angle passes half a turn.
================
*/
void GA_DrawAimLine( gaPlayer_t *pl ) {
    u8 color[5] = { 0x7F, 0x7F, 0x7F, 0xC0, 0x01 };
    u8 channel;

    channel = ga_aimChannelOrders[ga_aimChannelOrder % 3][0];
    color[channel] = -coss( ga_aimColorAngle ) / 256 + 0x7F;
    channel = ga_aimChannelOrders[ga_aimChannelOrder % 3][1];
    color[channel] = coss( ga_aimColorAngle ) / 256 + 0x7F;
    channel = ga_aimChannelOrders[ga_aimChannelOrder % 3][2];
    color[channel] = 0xFF;
    Draw_Line3D( pl->laneIndex, pl->lane->markers[0].pos, pl->lane->markers[1].pos, color );
    ga_aimColorAngle += 0x5B0;
    if ( ga_aimColorAngle >= 0x8000 ) {
        ga_aimColorAngle = 0;
        ga_aimChannelOrder++;
    }
}

/*
================
func_8011AA78_gacode

Runs the part of the game mode's ending that belongs to the mode.
================
*/
void func_8011AA78_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( ga_gameMode == func_801156D8_gacode ) {
        func_80119DD8_gacode( player, pl->laneIndex );
    } else if ( ga_gameMode == func_801155BC_gacode ) {
        GA_DrawParFramePanel( player, pl->laneIndex );
    } else if ( ga_gameMode == func_801157F8_gacode ) {
        GA_PinEditor( pl );
    }
}

/*
================
func_8011AB0C_gacode

panelFunc handler: runs the game mode's part of the frame, draws the lane's pin diagram and the player's
ball weight selector.
================
*/
void func_8011AB0C_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( ga_gameMode == func_801157F8_gacode ) {
        GA_DrawAimLine( pl );
    } else if ( ga_gameMode == func_801156D8_gacode ) {
        func_80119C50_gacode( player, pl->laneIndex );
    } else if ( ga_gameMode == func_801155BC_gacode ) {
        GA_DrawParPanel( player, pl->laneIndex );
    } else if ( ga_gameMode == func_8011582C_gacode ) {
        func_80119F8C_gacode( player, pl->laneIndex );
    } else {
        GA_DrawScorePanel( player, pl->laneIndex );
    }
    GA_DrawPinDiagram( pl );
    func_801194D0_gacode( pl );
}

/*
================
func_8011AD5C_gacode

panelFunc handler: runs the game mode's part of the frame (none for func_801157F8_gacode), draws the lane's pin
diagram, runs func_80119638_gacode, clears the lane's D_8012D8E0_gacode sprite and draws a human player's
full panel.
================
*/
void func_8011AD5C_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( ga_gameMode != func_801157F8_gacode ) {
        if ( ga_gameMode == func_801156D8_gacode ) {
            func_80119C50_gacode( player, pl->laneIndex );
        } else if ( ga_gameMode == func_801155BC_gacode ) {
            GA_DrawParPanel( player, pl->laneIndex );
        } else if ( ga_gameMode == func_8011582C_gacode ) {
            func_80119F8C_gacode( player, pl->laneIndex );
        } else {
            GA_DrawScorePanel( player, pl->laneIndex );
        }
    }
    GA_DrawPinDiagram( pl );
    func_80119638_gacode( pl );
    GA_FillPaletteFraction( &D_8012D8E0_gacode[pl->laneIndex], 0, 0.0f );
    func_80119800_gacode( player, 1 );
}

/*
================
func_8011B07C_gacode

panelFunc handler: runs func_80119638_gacode, fades the lane's D_8012D8E0_gacode sprite by the player object's
unk_48 against the character's unk_18, and draws a human player's full panel.
================
*/
void func_8011B07C_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 x;

    func_80119638_gacode( pl );
    GA_FillPaletteFraction( &D_8012D8E0_gacode[pl->laneIndex], 0,
                          (f32)pl->obj->frame / (f32)pl->character.data->unk_18 );
    if ( pl->port < 0 ) {
        return;
    }
    x = pl->laneIndex != 0 ? 0xB0 : 0x18;
    Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA46 : 0xA48, x, 0xA8 );
    Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA47 : 0xA49, x + 0x1D, 0xA8 );
    Menu_DrawImage( D_801236B0_gacode[pl->laneIndex].ids[D_801236B0_gacode[pl->laneIndex].cur], x + 0xD, 0xA8 + 0x11 );
}


/*
================
func_8011B1A0_gacode

panelFunc handler: as func_8011B2E8_gacode, by the player object's unk_48, and sets the character's unk_34 angle
from the same fraction (-pi/2 to pi/2).
================
*/
void func_8011B1A0_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    gaCharacterData_t *data = pl->character.data;
    s32 last = data->unk_19 - 1;
    s32 range = ( last - data->unk_18 ) * 2;
    s32 cur = pl->obj->frame - 1;
    f32 t = (f32)( range - ( cur - data->unk_18 ) ) / (f32)range;
    s32 x;

    GA_FillPaletteFraction( &D_8012D8E0_gacode[pl->laneIndex], 0x7C1, t );
    if ( pl->port >= 0 ) {
        x = pl->laneIndex != 0 ? 0xB0 : 0x18;
        Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA46 : 0xA48, x, 0xA8 );
        Menu_DrawImage( D_801236B0_gacode[pl->laneIndex].ids[D_801236B0_gacode[pl->laneIndex].cur], x + 0xD,
                       0xA8 + 0x11 );
    }
    pl->character.unk_34 = ( t - 0.5 ) * M_PI;
}

/*
================
func_8011B2E8_gacode

panelFunc handler: fades the lane's D_8012D8E0_gacode sprite by how far releaseFrame has come in the character's
unk_18 to unk_19 range, and draws a human player's panel.
================
*/
void func_8011B2E8_gacode( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    gaCharacterData_t *data = pl->character.data;
    s32 last = data->unk_19 - 1;
    s32 range = ( last - data->unk_18 ) * 2;
    s32 cur = pl->releaseFrame - 1;
    s32 x;

    GA_FillPaletteFraction( &D_8012D8E0_gacode[pl->laneIndex], 0x7C1,
                          (f32)( range - ( cur - data->unk_18 ) ) / (f32)range );
    if ( pl->port < 0 ) {
        return;
    }
    x = pl->laneIndex != 0 ? 0xB0 : 0x18;
    Draw_ImageCopyPalette( pl->laneIndex != 0 ? 0xA46 : 0xA48, x, 0xA8 );
    Menu_DrawImage( D_801236B0_gacode[pl->laneIndex].ids[D_801236B0_gacode[pl->laneIndex].cur], x + 0xD, 0xA8 + 0x11 );
}

/*
================
func_8011B400_gacode

A panelFunc handler that does nothing.
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
    gaPlayer_t *pl = &ga_players[player];
    s32 x;
    s16 id;

    x = pl->laneIndex ? 0xA8 : 0x18;
    id = D_8012C784_gacode[pl->lane->callout];
    if ( id >= 0 ) {
        Draw_ImageScaled( id, x, 0x18, (u8)ga_blinkLevel + 0x5F, 1.0f, 1.0f );
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
        Draw_ImageScaled( 0x851, lane == 0 ? 0x14 : 0xA4, 0x94, (u8)ga_blinkLevel + 0x7F, 1.0f, 1.0f );
    } else if ( D_800EA090[lane] == D_800EA090[rival] ) {
        Draw_ImageScaled( 0xA6B, lane != 0 ? 0xB4 : 0x24, 0x94, (u8)ga_blinkLevel + 0x7F, 1.0f, 1.0f );
    }
    if ( ga_gameMode != func_8011582C_gacode ) {
        Score_DrawPlayerBoard( lane, 0x18, lane == 0 ? 0x10 : 0xC0 );
    }
}

/*
================
func_8011B564_gacode
================
*/
void func_8011B564_gacode( s32 player ) {
    s32 x;

    x = ga_players[player].laneIndex == 0 ? 0x14 : 0xA4;
    Draw_ImageScaled( 0x891, x, 0xB4, (u8)ga_blinkLevel + 0x5F, 1.0f, 1.0f );
}

/*
================
func_8011B5C8_gacode
================
*/
void func_8011B5C8_gacode( s32 lane ) {
    if ( D_800EA090[lane] == 0 ) {
        Draw_FillRect( lane ? 0xA0 : 0, 0, 0xA0, 0xF0, 0xFF, 0x7F, 0, (u8)ga_blinkLevel + 0x4F );
    }
}

/*
================
GA_SetBallView

Sets the lane's camera to one of five views of the ball (view, clamped to 0 to 4): behind it, high behind
it, behind it looking up the lane's middle, in front of it, and high to the lane's outer side.
================
*/
void GA_SetBallView( gaPlayer_t *pl, s32 view ) {
    gaCamera_t *camera = pl->lane->camera;
    gaLane_t *lane = pl->lane;
    s32 side;

    view = view < 0 ? 0 : ( view > 4 ? 4 : view );
    side = pl->laneIndex != 0 ? -1 : 1;
    switch ( view ) {
    case 0:
        // identity: view 0's eye offset is tuned to zero, and retail still adds it (mtc1 zero / add.s)
        camera->eye[2] = lane->bodies[0].pos[2] + 0.0f;
        camera->eye[0] = lane->bodies[0].pos[0];
        camera->eye[1] = lane->bodies[0].pos[1] + 20.0f;
        camera->at[2] = lane->bodies[0].pos[2] - 1000.0f;
        camera->at[0] = lane->bodies[0].pos[0];
        camera->at[1] = lane->bodies[0].pos[1];
        camera->unk_18[2] = camera->eye[2];
        camera->unk_18[0] = camera->eye[0];
        camera->unk_18[1] = 512.0f;
        break;
    case 1:
        camera->eye[2] = lane->bodies[0].pos[2] + 100.0f;
        camera->eye[0] = lane->bodies[0].pos[0];
        camera->eye[1] = lane->bodies[0].pos[1] + 75.0f;
        camera->at[2] = lane->bodies[0].pos[2] - 50.0f;
        camera->at[0] = lane->bodies[0].pos[0];
        camera->at[1] = lane->bodies[0].pos[1];
        camera->unk_18[2] = camera->eye[2];
        camera->unk_18[0] = camera->eye[0];
        camera->unk_18[1] = 512.0f;
        break;
    case 2:
        camera->eye[2] = lane->bodies[0].pos[2] + 100.0f;
        camera->eye[0] = lane->bodies[0].pos[0];
        camera->eye[1] = lane->bodies[0].pos[1] + 20.0f;
        camera->at[2] = lane->bodies[0].pos[2] - 1000.0f;
        camera->at[0] = lane->bodies[0].pos[0];
        camera->at[1] = lane->bodies[0].pos[1];
        camera->unk_18[2] = camera->eye[2];
        camera->unk_18[0] = -pl->lane->spots[0].pos[0] + pl->lane->offsetX;
        camera->unk_18[1] = 512.0f;
        break;
    case 3:
        camera->eye[2] = lane->bodies[0].pos[2] - 125.0f;
        camera->eye[0] = lane->bodies[0].pos[0];
        camera->eye[1] = lane->bodies[0].pos[1] + 50.0f;
        camera->at[2] = lane->bodies[0].pos[2] + 25.0f;
        camera->at[0] = lane->bodies[0].pos[0];
        camera->at[1] = lane->bodies[0].pos[1];
        camera->unk_18[2] = camera->eye[2];
        camera->unk_18[0] = camera->eye[0];
        camera->unk_18[1] = 512.0f;
        break;
    case 4:
        camera->eye[2] = lane->bodies[0].pos[2] + 128.0f;
        camera->eye[0] = lane->bodies[0].pos[0] + side * 128;
        camera->eye[1] = lane->bodies[0].pos[1] + 256.0f;
        camera->at[2] = lane->bodies[0].pos[2];
        camera->at[0] = lane->bodies[0].pos[0];
        camera->at[1] = lane->bodies[0].pos[1];
        camera->unk_18[2] = camera->eye[2];
        camera->unk_18[0] = camera->eye[0];
        camera->unk_18[1] = 512.0f;
        break;
    }
}

/*
================
func_8011B8AC_gacode

A cameraFunc handler: the camera of func_8011C0DC_gacode, raised by cameraRaise, which falls by 45 a frame to 0.
================
*/
void func_8011B8AC_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->lane->offsetX;
    camera->eye[1] = pl->lane->cameraRaise + 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->lane->offsetX;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
    pl->lane->cameraRaise -= 45.0f;
    if ( pl->lane->cameraRaise < 0.0f ) {
        pl->lane->cameraRaise = 0.0f;
    }
}

/*
================
func_8011B974_gacode
================
*/
void func_8011B974_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->character.aim[0] + pl->lane->offsetX;
    camera->eye[1] = 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->character.aim[0] + pl->lane->offsetX;
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
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = 1130.0f;
    camera->eye[0] = pl->character.stance[0] + pl->lane->offsetX;
    camera->eye[1] = 200.0f;
    camera->at[2] = D_8012C158_gacode[0];
    camera->at[0] = pl->character.stance[0] + pl->lane->offsetX;
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
    gaPlayer_t *pl = &ga_players[player];
    s32 side = pl->laneIndex ? -1 : 1;

    camera->eye[2] = 730.0f;
    camera->eye[0] = pl->obj->mtx[3][0] + side * 250.0;
    camera->eye[1] = 40.0f;
    camera->at[2] = pl->obj->joints[1].mtx[3][2];
    camera->at[0] = pl->obj->joints[1].mtx[3][0];
    camera->at[1] = 60.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011BB44_gacode

A cameraFunc handler: R and L step ballView through 0 to 4, wrapping; the camera is GA_SetBallView's.
================
*/
void func_8011BB44_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    if ( D_800E44E0[pl->laneIndex] & R_TRIG ) {
        GA_PlayLaneSound( pl->laneIndex, 0x33 );
        if ( pl->ballView == 4 ) {
            pl->ballView = 0;
        } else {
            pl->ballView++;
        }
    }
    if ( D_800E44E0[pl->laneIndex] & L_TRIG ) {
        GA_PlayLaneSound( pl->laneIndex, 0x33 );
        if ( pl->ballView == 0 ) {
            pl->ballView = 4;
        } else {
            pl->ballView--;
        }
    }
    GA_SetBallView( pl, pl->ballView );
}

/*
================
func_8011BC10_gacode
================
*/
void func_8011BC10_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = -896.0f;
    camera->eye[0] = pl->lane->offsetX;
    camera->eye[1] = ga_stage->unk_14 + 80.0f;
    camera->at[2] = -1096.0f;
    camera->at[0] = pl->lane->offsetX;
    camera->at[1] = ga_stage->unk_14 + 24.0f;
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
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = pl->obj->joints[1].mtx[3][2] - 256.0f;
    camera->eye[0] = pl->obj->joints[1].mtx[3][0];
    camera->eye[1] = 50.0f;
    camera->at[2] = pl->obj->joints[1].mtx[3][2];
    camera->at[0] = pl->obj->joints[1].mtx[3][0];
    camera->at[1] = pl->obj->joints[1].mtx[3][1];
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
GA_FreeCamera

A cameraFunc handler: a free camera. The stick turns the lane's ga_spinCameraAngle angle, or with Z held
moves ga_freeCameraPos sideways; stick y moves it along the angle. ga_freeCameraPos stays within -50 to 50
across and -1000 to 0 along the lane, and the eye looks 1024 out along the angle.
================
*/
void GA_FreeCamera( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];
    f32 angle = ga_spinCameraAngle[pl->laneIndex] + M_PI;
    f32 speed;

    if ( D_800E57E4[pl->laneIndex] & Z_TRIG ) {
        speed = D_800692D0[pl->laneIndex] / 16.0f;
        ga_freeCameraPos[pl->laneIndex][2] -= speed * sinf( -angle );
        ga_freeCameraPos[pl->laneIndex][0] -= speed * cosf( -angle );
    } else {
        ga_spinCameraAngle[pl->laneIndex] -= D_800692D0[pl->laneIndex] * 0.01 * ( M_PI / 180.0 );
    }
    speed = D_800A2FD8[pl->laneIndex] / 8.0f;
    ga_freeCameraPos[pl->laneIndex][2] += speed * cosf( angle );
    ga_freeCameraPos[pl->laneIndex][0] += speed * sinf( angle );
    ga_freeCameraPos[pl->laneIndex][2] = ga_freeCameraPos[pl->laneIndex][2] < -1000.0f ? -1000.0f
        : ( ga_freeCameraPos[pl->laneIndex][2] > 0.0f ? 0.0f : ga_freeCameraPos[pl->laneIndex][2] );
    ga_freeCameraPos[pl->laneIndex][0] = ga_freeCameraPos[pl->laneIndex][0] < -50.0f ? -50.0f
        : ( ga_freeCameraPos[pl->laneIndex][0] > 50.0f ? 50.0f : ga_freeCameraPos[pl->laneIndex][0] );
    camera->eye[2] = ga_freeCameraPos[pl->laneIndex][2] + 730.0f;
    camera->eye[0] = pl->lane->offsetX + ga_freeCameraPos[pl->laneIndex][0];
    camera->eye[1] = 150.0f;
    camera->at[2] = camera->eye[2] + cosf( angle ) * 1024.0f;
    camera->at[0] = camera->eye[0] + sinf( angle ) * 1024.0f;
    camera->at[1] = 20.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011C0DC_gacode
================
*/
void func_8011C0DC_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = 960.0f;
    camera->eye[0] = pl->lane->offsetX;
    camera->eye[1] = 150.0f;
    camera->at[2] = 335.0f;
    camera->at[0] = pl->lane->offsetX;
    camera->at[1] = 0.0f;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011C158_gacode

Camera handler: eye above the origin, looking out at the lane's ga_spinCameraAngle angle, which turns by
0.3 degrees a frame.
================
*/
void func_8011C158_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    camera->eye[2] = 0.0f;
    camera->eye[0] = 0.0f;
    camera->eye[1] = 80.0f;
    camera->at[2] = cosf( ga_spinCameraAngle[pl->laneIndex] ) * 256.0f;
    camera->at[0] = sinf( ga_spinCameraAngle[pl->laneIndex] ) * 256.0f;
    camera->at[1] = camera->eye[1];
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
    ga_spinCameraAngle[pl->laneIndex] += M_PI / 600.0;
}

/*
================
func_8011C254_gacode

Camera handler during the throw: shows the ball and the player in both views, puts the eye on one of five
paths (ga_throwCamPathsLate while the lane's inputFunc is func_80118CCC_gacode or func_80118EA0_gacode, else
ga_throwCamPaths; by D_8012D8D8_gacode) at the frame that follows the player's animation, and looks at the
player object's second joint.
================
*/
void func_8011C254_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];
    s32 joint;
    u16 file;
    u8 frames;
    s16 frame;

    if ( pl->lane->bodyObjs[0]->viewMask ) {
        pl->lane->bodyObjs[0]->viewMask |= 2;
    }
    if ( pl->obj->viewMask ) {
        pl->obj->viewMask |= 2;
    }
    joint = 1;
    if ( pl->lane->inputFunc == func_80118CCC_gacode || pl->lane->inputFunc == func_80118EA0_gacode ) {
        file = ga_throwCamPathsLate[D_8012D8D8_gacode % 5];
        frames = pl->character.data->unk_1B[pl->lane->reaction];
    } else {
        file = ga_throwCamPaths[D_8012D8D8_gacode % 5];
        frames = pl->character.data->unk_1A;
    }
    frame = pl->obj->frame * 200 / frames;
    frame = frame < 0 ? 0 : ( frame > 200 ? 200 : frame );
    Cam_ReadPathFrame( file, frame, camera->eye, camera->unk_18 );
    camera->at[2] = pl->obj->joints[joint].mtx[3][2];
    camera->at[0] = pl->obj->joints[joint].mtx[3][0];
    camera->at[1] = pl->obj->joints[joint].mtx[3][1];
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011C468_gacode

Camera handler: puts the eye on the ga_followCamPaths path (one of ten, by D_8012D8D8_gacode) at the frame that
follows the ball down the lane, looking at the ball.
================
*/
void func_8011C468_gacode( s32 player, gaCamera_t *camera ) {
    gaLane_t *lane = ga_players[player].lane;
    s16 frame;

    frame = ( 730.0f - lane->spots[0].pos[2] ) * 480.0f / 1780.0;
    frame = frame < 0 ? 0 : ( frame > 480 ? 480 : frame );
    Cam_ReadPathFrame( ga_followCamPaths[D_8012D8D8_gacode % 10], frame, camera->eye, camera->unk_18 );
    camera->at[2] = lane->bodies[0].pos[2];
    camera->at[0] = lane->bodies[0].pos[0];
    camera->at[1] = lane->bodies[0].pos[1];
}

/*
================
func_8011C57C_gacode

Camera handler: puts the eye on frame D_8012D8D8_gacode % 5 of the ga_laneCamPaths paths, looking down the
lane.
================
*/
void func_8011C57C_gacode( s32 player, gaCamera_t *camera ) {
    gaPlayer_t *pl = &ga_players[player];

    Cam_ReadPathFrame( ga_laneCamPaths[D_8012D8D8_gacode % 5], 0, camera->eye, camera->unk_18 );
    camera->at[2] = -1096.0f;
    camera->at[0] = pl->lane->offsetX;
    camera->at[1] = ga_stage->unk_14;
    camera->unk_18[2] = camera->eye[2];
    camera->unk_18[0] = camera->eye[0];
    camera->unk_18[1] = 512.0f;
}

/*
================
func_8011C650_gacode

A cameraFunc handler that leaves the camera as it is.
================
*/
void func_8011C650_gacode( s32 player, gaCamera_t *camera ) {
}

/*
================
GA_CpuInput

Input of a player the game plays, by the lane's inputFunc step: sets the player up first, then moves the
character's aim and stance towards cpuAim and cpuStance, and presses after speed (doubled in
func_80118014_gacode) frames or when the animation reaches the character's frame.
================
*/
s32 GA_CpuInput( gaPlayer_t *pl, s32 player ) {
    s16 wait = pl->character.speed;
    s16 longWait = wait * 2;
    s32 steps;

    steps = ( D_8005C160 & 0xF ) == 3 ? 20 : 25;
    if ( pl->lane->inputFunc == func_80117AE0_gacode ) {
        if ( ( ( D_8005C160 >> 4 ) & 3 ) == 2 && D_8005C163 == 0 && ( D_8005C0E4[player] & 0xF ) == 0 ) {
            func_8011CEE4_gacode( pl, player );
        } else {
            GA_CpuSetup( pl, player );
        }
        return 0;
    } else if ( pl->lane->inputFunc == func_80117BDC_gacode ) {
        if ( FABS( pl->character.aim[0] ) >= FABS( pl->cpuAim ) ) {
            pl->character.aim[0] = pl->cpuAim;
            if ( pl->lane->timer++ < wait ) {
                return 0;
            }
            pl->lane->timer = 0;
            return 1;
        } else {
            pl->character.aim[0] += pl->cpuAim / steps;
        }
    } else if ( pl->lane->inputFunc == func_80117D20_gacode ) {
        if ( FABS( pl->character.stance[0] ) >= FABS( pl->cpuStance ) ) {
            pl->character.stance[0] = pl->cpuStance;
            if ( pl->lane->timer++ >= wait ) {
                pl->lane->timer = 0;
                return 1;
            }
        } else {
            pl->character.stance[0] += pl->cpuStance / steps;
        }
    } else if ( pl->lane->inputFunc == func_80118014_gacode ) {
        if ( pl->lane->timer++ < longWait ) {
            return 0;
        }
        pl->lane->timer = 0;
        return 1;
    } else if ( pl->lane->inputFunc == func_8011840C_gacode ) {
        if ( pl->obj->frame == pl->character.data->unk_19 ) {
            return 1;
        }
    }
    return 0;
}

/*
================
GA_CpuSetup

Set-up of a player the game plays: picks speed at random between the character's limits, takes the
GA_PickAimPin target with a random sideways error (bounded by the mode and D_8009D098.unk_78), then
chooses cpuAim and cpuStance (where GA_CpuInput moves aim and stance) and the spin that
func_801205A0_gacode gives for them. A character whose maxSpin exceeds 12 tries a crossing set-up first; when
spin falls outside the character's range, a set-up from 30.48 to the side follows, and last one with spin 0.
================
*/
void GA_CpuSetup( gaPlayer_t *pl, s32 player ) {
    s32 side = pl->character.reverseSpin ? -1 : 1;
    s32 range = ( pl->character.maxSpeed - pl->character.minSpeed ) * 100.0f;
    s32 spread;
    s32 bias;
    f32 gutter;
    vec3_t target;
    vec3_t dir;
    vec3_t to;
    vec3_t from;

    pl->character.speed = RAND( range ) / 100.0f + pl->character.minSpeed;
    GA_PickAimPin( target, pl->lane->bodies, pl->lane->spots, pl->character.reverseSpin );
    if ( ( D_8005C160 & 0xF ) == 3 || ( ( D_8005C160 >> 4 ) & 3 ) == 1 ) {
        switch ( D_8009D098.unk_78 ) {
            case 0:
                spread = 32;
                bias = 16;
                break;
            case 1:
                spread = 6;
                bias = 3;
                break;
            case 2:
                spread = 2;
                bias = 1;
                break;
        }
    } else {
        switch ( D_8009D098.unk_78 ) {
            case 0:
                spread = 96;
                bias = 48;
                break;
            case 1:
                spread = 16;
                bias = 8;
                break;
            case 2:
                spread = 2;
                bias = 1;
                break;
        }
    }
    target[0] += ( spread != 0 ? RAND( spread ) * 100 / 100.0f : 0.0f ) - bias;

    if ( pl->character.maxSpin > 12.0 ) {
        target[0] -= side * 0.9;
        pl->cpuAim = side * 5.0;
        pl->cpuStance = side * -5.0;
        to[0] = pl->cpuAim;
        to[1] = pl->character.aim[1];
        to[2] = pl->character.aim[2];
        from[0] = pl->cpuStance;
        from[1] = pl->character.stance[1];
        from[2] = pl->character.stance[2];
        GA_VecDirection( dir, from, to );
        pl->character.spin = func_801205A0_gacode( target, dir, from[0], pl->character.speed );
        if ( ga_stage->unk_14 >= 0.0f && FABS( pl->character.spin ) <= FABS( pl->character.maxSpin ) ) {
            return;
        }
    }

    target[0] += side * 0.9;
    pl->cpuAim = side * 30.48;
    if ( ( side > 0 && target[0] >= pl->cpuAim ) || ( side < 0 && target[0] <= pl->cpuAim ) ) {
        gutter = FABS( target[2] - 730.0f );
        pl->cpuStance = target[0] - ( target[0] - pl->cpuAim ) * gutter / ( gutter - 395.0f );
        pl->character.spin = 0.0f;
    } else {
        pl->cpuStance = pl->cpuAim;
        to[0] = pl->cpuAim;
        to[1] = pl->character.aim[1];
        to[2] = pl->character.aim[2];
        from[0] = pl->cpuStance;
        from[1] = pl->character.stance[1];
        from[2] = pl->character.stance[2];
        GA_VecDirection( dir, from, to );
        pl->character.spin = func_801205A0_gacode( target, dir, from[0], pl->character.speed );
    }
    if ( ga_stage->unk_14 >= 0.0f && FABS( pl->character.spin ) <= FABS( pl->character.maxSpin ) ) {
        return;
    }

    target[0] += side * 2.0;
    pl->cpuStance = side * 30.48;
    gutter = FABS( target[2] - 730.0f );
    pl->cpuAim = target[0] + ( pl->cpuStance - target[0] ) * ( gutter - 395.0f ) / gutter;
    pl->character.spin = 0.0f;
}

/*
================
func_8011CEE4_gacode

Gives the player the four values of its character's ga_cpuThrows row.
================
*/
void func_8011CEE4_gacode( gaPlayer_t *pl, s32 player ) {
    f32 *values = ga_cpuThrows[D_800DF59C.unk_04[D_8009D19C[player]]];

    pl->cpuAim = values[0];
    pl->cpuStance = values[1];
    pl->character.speed = values[2];
    pl->character.spin = values[3];
}

/*
================
GA_DebugText

Debug text for the player: the stage's unk_10, the animation frame, speed and spin, the pin table (in the
modes that have one), and a value of the lane's current inputFunc step.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register allocation only; retail gives row s2, lane s3, player s4 and &D_800F8094 s5 (here
// player s2, &D_800F8094 s3, row s4, lane s5); instructions, constants, relocations and schedule are the same
// (the line counter stepped per line puts the TABLE row's increment after its sprintf, as retail). Tried: row
// as u8/s16, lane as s32, the two locals' declaration order
void GA_DebugText( s32 player ) {
    gaPlayer_t *pl = &ga_players[player];
    u8 lane;
    s32 row;
    u8 text[40];
    vec3_t target;

    lane = pl->laneIndex;
    row = 14;
    sprintf( text, "SL[%8.4f]", ga_stage->unk_10 );
    Txt_DrawString( &D_800F8094, lane * 144 + 16, ( row + 2 ) * 8, text );
    row++;
    sprintf( text, "FRM[%3d]", pl->obj->frame );
    Txt_DrawString( &D_800F8094, lane * 144 + 16, ( row + 2 ) * 8, text );
    row++;
    sprintf( text, "T[%6.2f]R[%6.2f]", pl->character.speed, pl->character.spin );
    Txt_DrawString( &D_800F8094, lane * 144 + 16, ( row + 2 ) * 8, text );
    row++;
    if ( ga_gameMode == func_801156D8_gacode || ga_gameMode == func_801155BC_gacode ) {
        sprintf( text, "TABLE[%d]", D_800DF59C.unk_04[D_8009D19C[player]] );
        Txt_DrawString( &D_800F8094, lane * 144 + 16, ( row + 2 ) * 8, text );
        row++;
    }
    if ( pl->lane->inputFunc == func_80117BDC_gacode ) {
        sprintf( text, "ATX[%6.2f]", pl->character.aim[0] );
    } else if ( pl->lane->inputFunc == func_80117D20_gacode ) {
        sprintf( text, "STX[%6.2f]", pl->character.stance[0] );
    } else if ( pl->lane->inputFunc == func_80118014_gacode ) {
        if ( pl->port < 0 ) {
            return;
        }
        GA_PickAimPin( target, pl->lane->bodies, pl->lane->spots, pl->character.reverseSpin );
        sprintf( text, "TECH RO[%6.2f]",
            func_801205A0_gacode( target, pl->character.direction, pl->character.stance[0], pl->character.speed ) );
    } else if ( pl->lane->inputFunc == func_80118270_gacode || pl->lane->inputFunc == func_8011840C_gacode
        || pl->lane->inputFunc == func_80118524_gacode ) {
        sprintf( text, "FR SIN[%8.4f]", sinf( pl->character.unk_34 ) );
    } else if ( pl->lane->inputFunc == func_801187D0_gacode || pl->lane->inputFunc == func_80118F68_gacode ) {
        sprintf( text, "NOW Z[%8.4f]", pl->lane->spots[0].pos[2] );
    } else if ( pl->lane->inputFunc == func_80118CCC_gacode || pl->lane->inputFunc == func_80118E04_gacode ) {
        return;
    } else if ( pl->lane->inputFunc == func_80118F08_gacode || pl->lane->inputFunc == func_80118F38_gacode ) {
        sprintf( text, " WAIT... " );
    } else {
        return;
    }
    Txt_DrawString( &D_800F8094, lane * 144 + 16, ( row + 2 ) * 8, text );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/3E350", GA_DebugText);
#endif
