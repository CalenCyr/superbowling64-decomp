/* 474E0.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "gacode/gacode.h"

/* ===== Data ===== */

/* per side: the pins (1 to 10) from the back row forward (GA_PickAimPin) */
static u8 ga_pinRows[2][NUM_PINS] = {
    { 10, 9, 7, 8, 6, 4, 3, 2, 5, 1 },
    { 7, 8, 10, 9, 4, 6, 2, 3, 5, 1 },
};

/*
================
GA_BodyMatrix

Builds the placement matrix of a body: rotation rot (degrees), then scale, then translation pos.
================
*/
// Declared inline: GA_TransformPoint expands it.
inline void GA_BodyMatrix( f32 mf[4][4], vec3_t pos, vec3_t rot, f32 scale ) {
    // R36: unused in the original: retail's frame (and GA_TransformPoint's) reserves 0x80 untouched bytes
    f32 rotMf[4][4], scaleMf[4][4];

    guPositionF( mf, rot[0], rot[1], rot[2], scale, pos[0], pos[1], pos[2] );
}

/*
================
GA_TransformPoint

out = in placed by rotation rot (degrees) and translation pos.
================
*/
void GA_TransformPoint( vec3_t out, vec3_t in, vec3_t rot, vec3_t pos ) {
    f32 mf[4][4];

    GA_BodyMatrix( mf, pos, rot, 1.0f );
    guMtxXFMF( mf, in[0], in[1], in[2], &out[0], &out[1], &out[2] );
}

/*
================
GA_VecSub

out = to - from
Declared inline: GA_VecDirection expands it.
================
*/
inline void GA_VecSub( vec3_t out, vec3_t from, vec3_t to ) {
    out[0] = to[0] - from[0];
    out[1] = to[1] - from[1];
    out[2] = to[2] - from[2];
}

/*
================
GA_VecLength

Length of v.
Declared inline: GA_VecCosAngle expands it.
================
*/
inline f32 GA_VecLength( vec3_t v ) {
    return sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

/*
================
GA_VecNormalize

out = in scaled to length 1, or (0, -1, 0) when in is zero; returns the length of in.
Declared inline: GA_VecDirection and GA_VecCrossNormal expand it.
================
*/
inline f32 GA_VecNormalize( vec3_t out, vec3_t in ) {
    f32 length = sqrtf( in[0] * in[0] + in[1] * in[1] + in[2] * in[2] );

    if ( length == 0.0f ) {
        out[0] = 0.0f;
        out[1] = -1.0f;
        out[2] = 0.0f;
    } else {
        out[0] = in[0] / length;
        out[1] = in[1] / length;
        out[2] = in[2] / length;
    }
    return length;
}

/*
================
GA_VecDirection

out = the unit vector from from to to; returns their distance.
================
*/
f32 GA_VecDirection( vec3_t out, vec3_t from, vec3_t to ) {
    vec3_t delta;

    GA_VecSub( delta, from, to );
    return GA_VecNormalize( out, delta );
}

/*
================
GA_VecCosAngle

Cosine of the angle between a and b; 1 when either is zero.
Declared inline: GA_SlopeContact expands it.
================
*/
inline f32 GA_VecCosAngle( vec3_t a, vec3_t b ) {
    f32 lengths = GA_VecLength( a ) * GA_VecLength( b );

    if ( lengths == 0.0f ) {
        return 1.0f;
    }
    return ( a[0] * b[0] + a[1] * b[1] + a[2] * b[2] ) / lengths;
}

/*
================
GA_VecCrossNormal

out = the unit vector of a x b, or (0, -1, 0) when a and b are parallel.
Declared inline: GA_FloorHeight expands it.
================
*/
inline void GA_VecCrossNormal( vec3_t out, vec3_t a, vec3_t b ) {
    vec3_t cross;

    cross[0] = a[1] * b[2] - a[2] * b[1];
    cross[1] = a[2] * b[0] - a[0] * b[2];
    cross[2] = a[0] * b[1] - a[1] * b[0];
    GA_VecNormalize( out, cross );
}

/*
================
GA_SetBallHeading

Sets the ball's rot[1] (degrees) from the heading of v in the x-z plane, approximated from |v[0] / v[2]|
(45 * its square root below 1) rather than an arctangent; with v[2] zero it turns rot[1] by 90 instead.
Clamped to -180..179.9999.
================
*/
void GA_SetBallHeading( vec3_t v, gaBody_t *ball ) {
    f32 ratio;
    f32 angle;

    if ( v[2] == 0.0f ) {
        ball->rot[1] += v[0] == 0.0f ? 0.0f : v[0] > 0.0f ? -90.0f : 90.0f;
    } else {
        ratio = FABS( v[0] / v[2] );
        if ( ratio < 1.0 ) {
            angle = sqrtf( ratio ) * 45.0f;
        } else {
            angle = ( ratio - 1.0 ) * 0.02 + 45.0;
        }
        if ( v[0] <= 0.0f ) {
            if ( v[2] < 0.0f ) {
                ball->rot[1] = angle;
            } else {
                ball->rot[1] = -angle + 180.0f;
            }
        } else {
            if ( v[2] < 0.0f ) {
                ball->rot[1] = -angle;
            } else {
                ball->rot[1] = angle - 180.0f;
            }
        }
    }
    ball->rot[1] = ball->rot[1] < -180.0f ? -180.0f : ball->rot[1] > 179.9999 ? 179.9999f : ball->rot[1];
}

/*
================
GA_SpotsOverlap

Nonzero when the spheres of spots a and b (centre pos, radius) overlap: their boxes first, axis by
axis, then the distance.
================
*/
s32 GA_SpotsOverlap( gaSpot_t *a, gaSpot_t *b ) {
    f32 dx;
    f32 dy;
    f32 dz;

    if ( FABS( a->pos[0] - b->pos[0] ) < a->radius + b->radius &&
         FABS( a->pos[1] - b->pos[1] ) < a->radius + b->radius &&
         FABS( a->pos[2] - b->pos[2] ) < a->radius + b->radius ) {
        dx = a->pos[0] - b->pos[0];
        dy = a->pos[1] - b->pos[1];
        dz = a->pos[2] - b->pos[2];
        if ( dx * dx + dy * dy + dz * dz < ( a->radius + b->radius ) * ( a->radius + b->radius ) ) {
            return 1;
        }
    }
    return 0;
}

/*
================
GA_SegmentHitsSpot

Nonzero when the segment from from to to passes within radius plus the spot's radius of the spot's centre,
at a point between its ends.
Declared inline: GA_CollideLane expands it.
================
*/
inline s32 GA_SegmentHitsSpot( gaSpot_t *spot, vec3_t from, vec3_t to, f32 radius ) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 lengthSq;
    f32 t;
    f32 ox;
    f32 oy;
    f32 oz;

    dx = to[0] - from[0];
    dy = to[1] - from[1];
    dz = to[2] - from[2];
    lengthSq = dx * dx + dy * dy + dz * dz;
    if ( lengthSq == 0.0f ) {
        return 0;
    }
    t = ( ( spot->pos[0] - from[0] ) * dx + ( spot->pos[1] - from[1] ) * dy + ( spot->pos[2] - from[2] ) * dz ) /
        lengthSq;
    if ( t < 0.0f || 1.0f < t ) {
        return 0;
    }
    ox = dx * t + from[0] - spot->pos[0];
    oy = dy * t + from[1] - spot->pos[1];
    oz = dz * t + from[2] - spot->pos[2];
    if ( ( spot->radius + radius ) * ( spot->radius + radius ) <= ox * ox + oy * oy + oz * oz ) {
        return 0;
    }
    return 1;
}

/*
================
GA_SpotContact

Contact of spot a, moving along vel, with spot b: normal = the unit vector from a's centre, pushed back
along vel by what the spheres overlap, to b's centre; returns 1 minus the closest distance of b's centre
from a's path over the sum of the radii.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register allocation in the two expansions of GA_VecNormalize only: retail copies the
// first one's length into $f4 before the zero test and divides by $f4, and in the second takes sqrt.s into $f0
// and copies it to $f2 after the sqrtf fallback joins; this draft uses the sqrt register directly. Everything
// else matches, including $f20/$f22, which need along reused as the push and t as the clamped distance (with
// separate push and dist locals they land in caller-saved registers instead). Also tried: the inline's length
// through GA_VecLength (no change).
// Permuter clue: it reaches exact only by giving GA_VecNormalize an `unsigned int` return (the length converted
// and discarded), which breaks GA_VecDirection's f32 result; not a source change.
f32 GA_SpotContact( vec3_t vel, gaSpot_t *a, gaSpot_t *b, vec3_t normal ) {
    f32 along;
    f32 speedSq;
    f32 t;
    vec3_t dir;
    vec3_t gap;

    along = ( b->pos[0] - a->pos[0] ) * vel[0] + ( b->pos[1] - a->pos[1] ) * vel[1] + ( b->pos[2] - a->pos[2] ) * vel[2];
    speedSq = vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2];
    if ( speedSq == 0.0f ) {
        t = 0.0f;
    } else {
        t = along / speedSq;
    }
    dir[0] = vel[0] * t + a->pos[0];
    dir[1] = vel[1] * t + a->pos[1];
    dir[2] = vel[2] * t + a->pos[2];
    GA_VecSub( gap, dir, b->pos );
    t = GA_VecLength( gap );
    t = t < 0.0 ? 0.0f : a->radius + b->radius < t ? a->radius + b->radius : t;
    along = a->radius + b->radius - t * 0.85;
    GA_VecNormalize( dir, vel );
    dir[0] *= along;
    dir[1] *= along;
    dir[2] *= along;
    gap[0] += dir[0];
    gap[1] += dir[1];
    gap[2] += dir[2];
    GA_VecNormalize( normal, gap );
    return 1.0 - t / ( a->radius + b->radius );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", GA_SpotContact);
#endif

/*
================
GA_CollideBodies

Collision of bodies a and b (spots sa and sb): the one with more momentum is the mover of
GA_CollideMomentum, and when it had strictly more, the other takes its spin about y, reversed and cut by
the restitution.
Declared inline: GA_CollideLane expands it.
================
*/
inline void GA_CollideBodies( gaMotion_t *a, gaMotion_t *b, gaSpot_t *sa, gaSpot_t *sb ) {
    gaMotion_t *in[2];
    gaMotion_t *out[2];
    gaSpot_t *spots[2];
    f32 momentum[2];
    f32 swap;
    f32 restitution;
    f32 keep;

    momentum[0] = ( a->vel[0] * a->vel[0] + a->vel[1] * a->vel[1] + a->vel[2] * a->vel[2] ) * ( a->mass * a->mass );
    momentum[1] = ( b->vel[0] * b->vel[0] + b->vel[1] * b->vel[1] + b->vel[2] * b->vel[2] ) * ( b->mass * b->mass );
    if ( momentum[1] <= momentum[0] ) {
        in[0] = out[0] = a;
        in[1] = out[1] = b;
        spots[0] = sa;
        spots[1] = sb;
    } else {
        in[0] = out[0] = b;
        in[1] = out[1] = a;
        spots[0] = sb;
        spots[1] = sa;
        swap = momentum[0];
        momentum[0] = momentum[1];
        momentum[1] = swap;
    }
    restitution = GA_CollideMomentum( out, in, spots );
    if ( momentum[0] > momentum[1] ) {
        keep = 1.0 - restitution;
        out[1]->spin[1] = -( in[0]->spin[1] * keep );
    }
}

/*
================
GA_CollideMomentum

Collision of in[0] (moving) with in[1], weighted by their mass, with the restitution of
GA_SpotContact: out[1] leaves along the contact normal, out[0] keeps the rest of the momentum at its own
speed. Returns the restitution.
================
*/
f32 GA_CollideMomentum( gaMotion_t **out, gaMotion_t **in, gaSpot_t **spots ) {
    gaMotion_t after[2];
    vec3_t normal;
    vec3_t vel0;
    vec3_t vel1;
    f32 restitution;
    f32 speed;
    f32 speed0;

    restitution = GA_SpotContact( in[0]->vel, spots[0], spots[1], normal );
    after[0].vel[0] = ( ( in[0]->mass - restitution * in[1]->mass ) * in[0]->vel[0] +
                        ( in[1]->mass + restitution * in[1]->mass ) * in[1]->vel[0] ) /
                      ( in[0]->mass + in[1]->mass );
    after[0].vel[1] = ( ( in[0]->mass - restitution * in[1]->mass ) * in[0]->vel[1] +
                        ( in[1]->mass + restitution * in[1]->mass ) * in[1]->vel[1] ) /
                      ( in[0]->mass + in[1]->mass );
    after[0].vel[2] = ( ( in[0]->mass - restitution * in[1]->mass ) * in[0]->vel[2] +
                        ( in[1]->mass + restitution * in[1]->mass ) * in[1]->vel[2] ) /
                      ( in[0]->mass + in[1]->mass );
    after[1].vel[0] = ( ( in[0]->mass + restitution * in[0]->mass ) * in[0]->vel[0] +
                        ( in[1]->mass - restitution * in[0]->mass ) * in[1]->vel[0] ) /
                      ( in[0]->mass + in[1]->mass );
    after[1].vel[1] = ( ( in[0]->mass + restitution * in[0]->mass ) * in[0]->vel[1] +
                        ( in[1]->mass - restitution * in[0]->mass ) * in[1]->vel[1] ) /
                      ( in[0]->mass + in[1]->mass );
    after[1].vel[2] = ( ( in[0]->mass + restitution * in[0]->mass ) * in[0]->vel[2] +
                        ( in[1]->mass - restitution * in[0]->mass ) * in[1]->vel[2] ) /
                      ( in[0]->mass + in[1]->mass );
    speed = GA_VecLength( after[1].vel );
    vel1[0] = normal[0] * speed;
    vel1[1] = normal[1] * speed;
    vel1[2] = normal[2] * speed;
    vel0[0] = ( after[0].vel[0] * in[0]->mass + after[1].vel[0] * in[1]->mass - vel1[0] * in[1]->mass ) /
              in[0]->mass;
    vel0[1] = ( after[0].vel[1] * in[0]->mass + after[1].vel[1] * in[1]->mass - vel1[1] * in[1]->mass ) /
              in[0]->mass;
    vel0[2] = ( after[0].vel[2] * in[0]->mass + after[1].vel[2] * in[1]->mass - vel1[2] * in[1]->mass ) /
              in[0]->mass;
    speed = GA_VecLength( after[0].vel );
    speed0 = GA_VecLength( vel0 );
    if ( speed0 == 0.0f ) {
        speed = speed0 = 1.0f;
    }
    vel0[0] *= speed / speed0;
    vel0[1] *= speed / speed0;
    vel0[2] *= speed / speed0;
    out[0]->vel[0] = vel0[0];
    out[0]->vel[1] = vel0[1];
    out[0]->vel[2] = vel0[2];
    out[1]->vel[0] = vel1[0];
    out[1]->vel[1] = vel1[1];
    out[1]->vel[2] = vel1[2];
    return restitution;
}

/*
================
GA_TipBody

For a body with rot[0] 0 that moves at speed 1 or more: by kind 0 or 1 sets its spin about x, heads it along
its velocity (GA_SetBallHeading), moves standState from 1 (still standing) to 2, and plays sound 0x32 (0x4B with
flags bit 1) on the lane of flags bit 0; returns 1. Otherwise returns 0.
Declared inline: GA_CollideLane expands it.
================
*/
inline s32 GA_TipBody( u8 kind, gaMotion_t *motion, gaBody_t *body, u8 flags ) {
    if ( body->rot[0] == 0.0f && GA_VecLength( motion->vel ) >= 1.0 ) {
        switch ( kind ) {
        case 0:
            motion->spin[0] = 1.5f;
            break;
        case 1:
            motion->spin[0] = -2.0f;
            break;
        }
        GA_SetBallHeading( motion->vel, body );
        if ( body->standState == 1 ) {
            body->standState = 2;
        }
        GA_PlayLaneSound( flags & 1, flags & 2 ? 0x4B : 0x32 );
        return 1;
    }
    return 0;
}

/*
================
GA_ApproachLimit

Moves *speed one step toward limit's side; once *value reaches limit (from below for a positive limit,
from above for a negative one) clamps it there, stops *speed and returns 1.
Declared inline: GA_SettleBodies expands it.
================
*/
inline s32 GA_ApproachLimit( f32 *value, f32 *speed, f32 limit ) {
    if ( ( limit > 0.0f && *value >= limit ) || ( limit < 0.0f && *value <= limit ) ) {
        *value = limit;
        *speed = 0.0f;
        return 1;
    }
    if ( *value > limit ) {
        *speed -= 1.0;
    } else {
        *speed += 1.0;
    }
    return 0;
}

/*
================
GA_CollideLane

Collisions between the bodies of a lane. Each spot in use (spots[0..10], then spot2 of each body in
spots[11..21]) of a body still in play is tested against each later body in play, first against its
spot (or the segment from that spot to the midpoint with its spot2), then against its spot2 (or the
segment from that midpoint to spot2). A new contact runs GA_CollideBodies on the two bodies, and each
of them of kind 1 goes through GA_TipBody: when it stands flat (rot[0] 0) and was hit at 1 or more it tips toward its new
heading, falls (standState 2) and plays its sound (0x4B under the lane, else 0x32). contacts of a spot holds
bit 0 (it was hit this frame) and one bit per spot it touches. Returns 1 when the last contact with
the first spots knocked a body over.
================
*/
s32 GA_CollideLane( s32 player, gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots, u8 lane ) {
    gaSpot_t mid;
    s32 i;
    s32 a;
    s32 b;
    s32 b2;
    s32 which;
    s32 hit = 0;

    for ( b = 0; b < NUM_BODIES; b++ ) {
        spots[b].contacts &= ~1;
    }
    for ( i = 0; i < NUM_BODIES * 2; i++ ) {
        if ( spots[i].radius == 0.0f ) {
            continue;
        }
        a = i % NUM_BODIES;
        which = i / NUM_BODIES;
        if ( bodies[a].standState == 0 ) {
            continue;
        }
        for ( b = a + 1; b < NUM_BODIES; b++ ) {
            b2 = b + NUM_BODIES;
            if ( bodies[b].standState == 0 ) {
                continue;
            }
            if ( spots[b2].radius != 0.0f ) {
                mid.pos[0] = ( spots[b].pos[0] + spots[b2].pos[0] ) / 2.0f;
                mid.pos[1] = ( spots[b].pos[1] + spots[b2].pos[1] ) / 2.0f;
                mid.pos[2] = ( spots[b].pos[2] + spots[b2].pos[2] ) / 2.0f;
                mid.radius = spots[b2].radius;
            }
            if ( !( ( spots[i].contacts >> b2 ) & 1 ) &&
                 ( GA_SpotsOverlap( &spots[i], &spots[b] ) ||
                   ( spots[b2].radius != 0.0f &&
                     GA_SegmentHitsSpot( &spots[i], spots[b].pos, mid.pos, spots[b].radius ) ) ) ) {
                if ( !( ( spots[i].contacts >> b ) & 1 ) ) {
                    GA_CollideBodies( &motion[a], &motion[b], &spots[i], &spots[b] );
                    lane |= spots[i].pos[1] < -190.0f ? 2 : 0;
                    if ( bodies[b].kind == 1 ) {
                        hit = GA_TipBody( 0, &motion[b], &bodies[b], lane );
                    }
                    if ( bodies[a].kind == 1 ) {
                        hit = GA_TipBody( which, &motion[a], &bodies[a], lane );
                    }
                    spots[i].contacts |= 1 << b;
                    spots[b].contacts |= 1;
                }
            } else if ( spots[b2].radius != 0.0f && !( spots[b].contacts & 1 ) &&
                        !( ( spots[i].contacts >> b ) & 1 ) &&
                        ( GA_SpotsOverlap( &spots[i], &spots[b2] ) ||
                          GA_SegmentHitsSpot( &spots[i], mid.pos, spots[b2].pos,
                                                spots[b2].radius ) ) ) {
                if ( !( ( spots[i].contacts >> b2 ) & 1 ) ) {
                    GA_CollideBodies( &motion[a], &motion[b], &spots[i], &spots[b2] );
                    lane |= spots[i].pos[1] < -190.0f ? 2 : 0;
                    if ( bodies[b].kind == 1 ) {
                        GA_TipBody( 1, &motion[b], &bodies[b], lane );
                    }
                    if ( bodies[a].kind == 1 ) {
                        GA_TipBody( which, &motion[a], &bodies[a], lane );
                    }
                    spots[i].contacts |= 1 << b2;
                    spots[b].contacts |= 1;
                }
            } else {
                spots[i].contacts = ~( 1 << b ) & spots[i].contacts;
                spots[i].contacts = ~( 1 << b2 ) & spots[i].contacts;
            }
        }
    }
    return hit;
}

/*
================
GA_SettleBodies

Each frame, for every standing body that is down (kind 1) and has motion flag 2: one lying flat slows to
0.75 of its speed along the lane; one still tipping turns toward 90 degrees on its side and, once there,
slows to 0.85. The spin about y decays to 0.925 either way.
================
*/
void GA_SettleBodies( gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots ) {
    s32 i;
    s32 down;

    for ( i = 0; i < NUM_BODIES; i++ ) {
        if ( bodies[i].standState != 0 && bodies[i].kind == 1 && ( ( motion[i].flags >> 1 ) & 1 ) ) {
            if ( bodies[i].rot[0] == 0.0f ) {
                motion[i].vel[0] *= 0.75;
                motion[i].vel[2] *= 0.75;
            } else {
                if ( bodies[i].rot[0] >= 0.0f ) {
                    down = GA_ApproachLimit( &bodies[i].rot[0], &motion[i].spin[0], 90.0f );
                } else {
                    down = GA_ApproachLimit( &bodies[i].rot[0], &motion[i].spin[0], -90.0f );
                }
                if ( down ) {
                    motion[i].vel[0] *= 0.85;
                    motion[i].vel[2] *= 0.85;
                }
            }
            motion[i].spin[1] *= 0.925;
        }
    }
}

/*
================
GA_KeepOnFloor

Keeps a body's spot (and spot2, unless its radius is 0) inside the floor grid and on the floor: a spot
that sank under the floor is lifted back onto it (unless GA_SlopeContact handles the contact) and its
fall turned into a damped bounce (motion flag 2). Returns 1 when the body lands with spot, 2 when it
bounces with spot, else 0. Gravity (1.0889) is then applied.
================
*/
s32 GA_KeepOnFloor( gaMotion_t *motion, gaSpot_t *spot, gaSpot_t *spot2, f32 ceiling, f32 arg4 ) {
    vec3_t normal;
    f32 floor;
    f32 bottom2;
    f32 bottom;
    s32 handled;
    s32 landed;

    GA_KeepInGrid( spot, spot2, motion, ceiling );
    floor = GA_FloorHeight( spot->pos[0], spot->pos[2], normal );
    motion->flags &= ~0x40;
    landed = 0;
    if ( spot2->radius != 0.0f && ( bottom2 = spot2->pos[1] - spot2->radius ) < floor ) {
        if ( !GA_SlopeContact( motion, spot2, normal ) ) {
            spot->pos[1] += floor - bottom2;
            spot2->pos[1] += floor - bottom2;
        }
        motion->vel[1] = -motion->vel[1] - motion->mass * 0.7;
        if ( motion->vel[1] <= 0.0f ) {
            motion->vel[1] = 0.0f;
        }
        if ( spot2->pos[2] <= -1186.0f ) {
            motion->vel[1] = 0.0f;
        }
        motion->flags |= 2;
    } else if ( ( bottom = spot->pos[1] - spot->radius ) < floor ) {
        handled = GA_SlopeContact( motion, spot, normal );
        if ( !handled ) {
            spot->pos[1] += floor - bottom;
            if ( spot2->radius != 0.0f ) {
                spot2->pos[1] += floor - bottom;
            }
        }
        motion->vel[1] = -motion->vel[1] - motion->mass * 0.7;
        if ( motion->vel[1] <= 0.0f ) {
            motion->vel[1] = 0.0f;
            landed = 1;
        } else {
            landed = 2;
        }
        if ( spot->pos[2] <= -1186.0f ) {
            motion->vel[1] = 0.0f;
        }
        if ( handled || ( ( motion->flags >> 4 ) & 1 ) || ( ( motion->flags >> 5 ) & 1 ) ) {
            landed = 0;
        }
        motion->flags |= 2;
    } else if ( spot2->radius != 0.0f && bottom2 == floor ) {
        // RULE-EXCEPTION: R17, nothing to do while the second spot rests on the floor; retail tests this case and skips it
    } else if ( bottom != floor ) {
        if ( ( motion->flags >> 4 ) & 1 ) {
            spot->pos[1] += floor - bottom;
            if ( spot2->radius != 0.0f ) {
                spot2->pos[1] += floor - bottom;
            }
            motion->vel[2] += normal[2] * 1.0889;
        } else {
            motion->flags &= ~0x10;
            motion->flags &= ~0x20;
            motion->flags &= ~2;
        }
    }
    if ( ( motion->flags >> 1 ) & 1 ) {
        motion->vel[1] += 1.0889;
    }
    motion->vel[1] -= 1.0889;
    return landed;
}

/*
================
GA_FloorHeight

Height of the floor grid at (x, z), interpolated over the triangle of its cell that holds the point; when
normal is not 0 it receives the unit normal of that triangle.
================
*/
f32 GA_FloorHeight( f32 x, f32 z, vec3_t normal ) {
    s32 cell[2];
    f32 pos[2];
    f32 h[4];
    vec3_t alongZ;
    vec3_t alongX;
    f32 dx;
    f32 dz;
    f32 edge;
    f32 a;
    f32 b;
    f32 height;

    pos[0] = x + -game_floorGrids[0].originX;
    pos[1] = z + -game_floorGrids[0].originZ;
    cell[0] = (s16)pos[0] / game_floorGrids[0].stepX;
    cell[1] = (s16)pos[1] / game_floorGrids[0].stepZ;
    if ( cell[0] < 0 ) {
        cell[0] = 0;
    } else if ( cell[0] >= game_floorGrids[0].numX - 1 ) {
        cell[0] = game_floorGrids[0].numX - 2;
    }
    if ( cell[1] < 0 ) {
        cell[1] = 0;
    } else if ( cell[1] >= game_floorGrids[0].numZ - 1 ) {
        cell[1] = game_floorGrids[0].numZ - 2;
    }
    h[0] = game_floorGrids[0].heights[cell[1] * game_floorGrids[0].numX + cell[0]];
    h[1] = game_floorGrids[0].heights[cell[1] * game_floorGrids[0].numX + cell[0] + 1];
    h[2] = game_floorGrids[0].heights[( cell[1] + 1 ) * game_floorGrids[0].numX + cell[0]];
    h[3] = game_floorGrids[0].heights[( cell[1] + 1 ) * game_floorGrids[0].numX + cell[0] + 1];

    dx = pos[0] - cell[0] * game_floorGrids[0].stepX;
    dz = pos[1] - cell[1] * game_floorGrids[0].stepZ;
    edge = game_floorGrids[0].stepZ * dx / game_floorGrids[0].stepX;
    if ( dz < edge && dx != 0.0f ) {
        a = ( h[1] - h[0] ) * dx / game_floorGrids[0].stepX + h[0];
        b = ( h[3] - h[0] ) * dx / game_floorGrids[0].stepX + h[0];
        height = ( b - a ) * dz / edge + a;
        if ( normal != 0 ) {
            alongZ[2] = game_floorGrids[0].stepZ;
            alongZ[0] = 0.0f;
            alongX[2] = 0.0f;
            alongX[0] = game_floorGrids[0].stepX;
            alongX[1] = h[1] - h[0];
            alongZ[1] = h[3] - h[1];
            GA_VecCrossNormal( normal, alongZ, alongX );
        }
    } else {
        a = ( h[3] - h[0] ) * dx / game_floorGrids[0].stepX + h[0];
        b = ( h[3] - h[2] ) * dx / game_floorGrids[0].stepX + h[2];
        height = ( b - a ) * ( dz - edge ) / ( game_floorGrids[0].stepZ - edge ) + a;
        if ( normal != 0 ) {
            alongZ[2] = game_floorGrids[0].stepZ;
            alongZ[0] = 0.0f;
            alongX[2] = 0.0f;
            alongX[0] = game_floorGrids[0].stepX;
            alongX[1] = h[3] - h[2];
            alongZ[1] = h[2] - h[0];
            GA_VecCrossNormal( normal, alongZ, alongX );
        }
    }
    return height;
}

/*
================
GA_SlopeContact

Contact of a body's spot with a sloped floor (normal not straight up), unless motion flag 1: the slope is
taken along x or z, whichever the normal leans more; against a slope steeper than 30 degrees from the
spot's floorNormal the velocity along it bounces back (motion flag 0x40) or is pushed down the slope, else it
slides down the slope (motion flag 0x10 along x, 0x20 along z). Returns 1 on a bounce; else the spot's
floorNormal takes the normal.
================
*/
s32 GA_SlopeContact( gaMotion_t *motion, gaSpot_t *spot, vec3_t normal ) {
    vec3_t from;
    vec3_t slope;
    s32 hit = 0;

    if ( ( normal[0] != 0.0f || normal[2] != 0.0f ) && !( motion->flags & 1 ) ) {
        from[1] = spot->floorNormal[1];
        slope[1] = normal[1];
        if ( FABS( normal[0] ) > FABS( normal[2] ) ) {
            from[2] = 0.0f;
            from[0] = spot->floorNormal[0];
            slope[2] = 0.0f;
            slope[0] = normal[0];
            if ( GA_VecCosAngle( from, slope ) < 0.866 ) {
                hit = 1;
                if ( motion->vel[0] * normal[0] <= 0.0f ) {
                    motion->vel[0] = -motion->vel[0];
                    motion->flags |= 0x40;
                } else {
                    motion->vel[0] += normal[0] * 1.0889;
                }
                motion->flags &= ~0x10;
            } else {
                motion->vel[0] += normal[0] * 1.0889;
                motion->flags |= 0x10;
            }
        } else {
            from[2] = spot->floorNormal[2];
            from[0] = 0.0f;
            slope[2] = normal[2];
            slope[0] = 0.0f;
            if ( GA_VecCosAngle( from, slope ) < 0.866 ) {
                hit = 1;
                if ( motion->vel[2] * normal[2] <= 0.0f ) {
                    motion->vel[2] = -motion->vel[2];
                    motion->flags |= 0x40;
                } else {
                    motion->vel[2] += normal[2] * 1.0889;
                }
                motion->flags &= ~0x20;
            } else {
                motion->vel[2] += normal[2] * 1.0889;
                motion->flags |= 0x20;
            }
        }
    }
    if ( !hit ) {
        spot->floorNormal[0] = normal[0];
        spot->floorNormal[1] = normal[1];
        spot->floorNormal[2] = normal[2];
    }
    return hit;
}

/*
================
GA_KeepInGrid

Keeps spot inside the floor grid (along z and x) and below ceiling: a spot past an edge is put back on
it, spot2 moved with it unless its radius is 0, and the velocity across that edge bounced (z: -0.1, motion
flag 4; x: 0.5, flag 1; y: -0.0001).
================
*/
void GA_KeepInGrid( gaSpot_t *spot, gaSpot_t *spot2, gaMotion_t *motion, f32 ceiling ) {
    f32 edge;

    motion->flags &= ~4;
    motion->flags &= ~1;

    edge = (f32)( ( game_floorGrids[0].numZ - 1 ) * game_floorGrids[0].stepZ + game_floorGrids[0].originZ ) - 0.0002;
    if ( spot->pos[2] > edge || spot->pos[2] < ( edge = game_floorGrids[0].originZ ) ) {
        spot->pos[2] -= spot->pos[2] - edge;
        if ( spot2->radius != 0.0f ) {
            spot2->pos[2] -= spot->pos[2] - edge;
        }
        motion->vel[2] *= -0.1;
        motion->flags |= 4;
    }

    edge = ( game_floorGrids[0].numX - 1 ) * game_floorGrids[0].stepX + game_floorGrids[0].originX;
    if ( spot->pos[0] > edge || spot->pos[0] < ( edge = game_floorGrids[0].originX ) ) {
        spot->pos[0] -= spot->pos[0] - edge;
        if ( spot2->radius != 0.0f ) {
            spot2->pos[0] -= spot2->pos[0] - edge;
        }
        motion->vel[0] *= 0.5;
        motion->flags |= 1;
    }

    edge = ceiling - 0.0002;
    if ( spot->pos[1] > edge ) {
        spot->pos[1] -= spot->pos[1] - edge;
        if ( spot2->radius != 0.0f ) {
            spot2->pos[1] -= spot2->pos[1] - edge;
        }
        motion->vel[1] *= -0.0001;
    }
}

/*
================
GA_PickAimPin

Picks the pin to aim at from side's view: with one pin standing, that pin; with the last standing pin
checked 1 or 5, that pin, offset 4.9 toward side; else the group (of two, split by ga_pinRows's
order) with more standing pins, offset 15 (unless pin 7 or 10) or 5. Writes its x and z to out and
returns the pin.
================
*/
s32 GA_PickAimPin( vec3_t out, gaBody_t *bodies, gaSpot_t *spots, s32 side ) {
    s32 right = 0;
    s32 left = 0;
    s32 numRight = 0;
    s32 numLeft = 0;
    s32 numStanding = 0;
    s32 dir = side ? -1 : 1;
    s32 last;
    s32 i;

    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( bodies[ga_pinRows[side][i]].standState != 1 ) {
            continue;
        }
        switch ( i ) {
        case 2:
        case 3:
        case 5:
        case 8:
            right = ga_pinRows[side][i];
            numRight++;
            break;
        case 0:
        case 1:
        case 4:
        case 7:
            left = ga_pinRows[side][i];
            numLeft++;
            break;
        default:
            right = left = ga_pinRows[side][i];
            numRight++;
            numLeft++;
            break;
        }
        last = ga_pinRows[side][i];
        numStanding++;
    }
    if ( numStanding == 1 ) {
        out[0] = spots[last].pos[0];
        out[2] = spots[last].pos[2];
        return last;
    }
    if ( last == 1 || last == 5 ) {
        out[0] = spots[last].pos[0];
        out[2] = spots[last].pos[2];
        out[0] += dir * 4.9;
        return last;
    }
    if ( numLeft >= numRight ) {
        out[0] = spots[left].pos[0];
        out[2] = spots[left].pos[2];
        if ( left != 7 && left != 10 ) {
            out[0] += dir * 15;
        }
        return left;
    }
    out[0] = spots[right].pos[0];
    out[2] = spots[right].pos[2];
    out[0] += dir * 5;
    return right;
}

/*
================
GA_AimSpin
================
*/
f32 GA_AimSpin( vec3_t pos, vec3_t vel, f32 x, f32 scale ) {
    f32 dx = x - pos[0];
    f32 width = 730.0f - ga_stage->hookStartZ;
    f32 dz;
    f32 vx;
    f32 vz;

    if ( pos[2] - 730.0f < 0.0f ) {
        dz = 0.0f - ( pos[2] - 730.0f ) - width;
    } else {
        dz = pos[2] - 730.0f - width;
    }
    vz = scale * vel[2];
    vx = scale * vel[0];
    return ( vz * vz * 2 * dx - vx * 2 * vz * ( width + dz ) )
         / ( width * width + width * 2 * dz + ga_stage->slide * ( dz * dz ) ) * 15.0 / ga_stage->hook;
}

/*
================
GA_BinarySearch

Binary search of the count sorted entries of table for key; its index, or -1.
Declared inline: GA_IsSplit expands it.
================
*/
inline s32 GA_BinarySearch( u16 key, s32 count, u16 *table ) {
    s32 lo = 0;
    s32 hi = count - 1;
    s32 mid;

    while ( lo <= hi ) {
        mid = ( lo + hi ) / 2;
        if ( table[mid] == key ) {
            return mid;
        }
        if ( table[mid] < key ) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return -1;
}

/*
================
GA_IsSplit

1 when the head pin is down and the mask of the standing pins is one of the leaves listed for their
number (2 to 6, tables ga_splits2 to ga_splits6); else 0.
================
*/
u8 GA_IsSplit( gaLane_t *lane ) {
    u8 listed = 0;
    s32 mask = 0;
    s32 numStanding = 0;
    s32 i;

    if ( lane->bodies[1].standState == 1 ) {
        return 0;
    }
    for ( i = 2; i < NUM_BODIES; i++ ) {
        if ( lane->bodies[i].standState == 1 ) {
            mask |= lane->bodies[i].standState << ( 10 - i );
            numStanding++;
        }
    }
    switch ( numStanding ) {
    case 2:
        if ( GA_BinarySearch( mask, ga_splits2[0], &ga_splits2[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 3:
        if ( GA_BinarySearch( mask, ga_splits3[0], &ga_splits3[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 4:
        if ( GA_BinarySearch( mask, ga_splits4[0], &ga_splits4[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 5:
        if ( GA_BinarySearch( mask, ga_splits5[0], &ga_splits5[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 6:
        if ( GA_BinarySearch( mask, ga_splits6[0], &ga_splits6[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    }
    return listed;
}

/*
================
GA_UnpackPinMask

Unpacks a pin mask, pin 1 in bit 9, into one flag per pin.
================
*/
void GA_UnpackPinMask( u8 *pins, u16 mask ) {
    s32 i;

    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( ( mask >> ( 9 - i ) ) & 1 ) {
            pins[i] = 1;
        } else {
            pins[i] = 0;
        }
    }
}

/*
================
GA_ShuffleOrder

Fills order with first, first + 1, ... (count values) and shuffles it.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with one local doing two jobs (marked R30 below); separate locals
// change the register allocation, and a reused local is not counted as a match
void GA_ShuffleOrder( u8 *order, u8 count, u8 first ) {
    s32 i;
    s32 j;          // RULE-EXCEPTION: R30, one local serves as the next value, then as the slot to swap with
    u8 swap;

    j = first;
    for ( i = 0; i < count; i++ ) {
        order[i] = j++;
    }
    for ( i = count - 1; i > 0; i-- ) {
        j = guRandom() % i;
        swap = order[i];
        order[i] = order[j];
        order[j] = swap;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", GA_ShuffleOrder);
#endif

/*
================
GA_SetupSprite

Sets up a sprite of resource id with palette range first to last, keeping a copy of its palette.
================
*/
void GA_SetupSprite( gaSprite_t *sprite, u16 id, u16 first, u16 last ) {
    Draw_Image( id, 320, 240 );
    if ( cache_files[id].data != NULL ) {
        memmove( sprite->palette, ( (unkImage_t *)cache_files[id].data )->palette, sizeof( sprite->palette ) );
        sprite->id = id;
        sprite->first = first;
        sprite->last = last;
        sprite->unk_206 = 0;
    }
}

/*
================
GA_RestorePalette

Puts the sprite's saved palette back into its resource.
================
*/
void GA_RestorePalette( gaSprite_t *sprite ) {
    if ( cache_files[sprite->id].data != NULL ) {
        memmove( ( (unkImage_t *)cache_files[sprite->id].data )->palette, sprite->palette, sizeof( sprite->palette ) );
    }
}

/*
================
GA_FillPalette

Rewrites the sprite's palette range in its resource from the saved copy: with color 0, the entries before
index keep their colour and the others go black; else only entry index takes color.

Declared inline: nothing calls this copy, and GA_FillPaletteFraction repeats its body.
================
*/
inline void GA_FillPalette( gaSprite_t *sprite, u16 color, u16 index ) {
    u16 *dst;
    s32 i;

    dst = cache_files[sprite->id].data;
    if ( dst == NULL ) {
        return;
    }
    dst = ( (unkImage_t *)cache_files[sprite->id].data )->palette;
    dst += sprite->first;
    index = index < sprite->first ? sprite->first : ( index > sprite->last ? sprite->last : index );
    for ( i = sprite->first; i <= sprite->last; i++ ) {
        if ( color != 0 ) {
            if ( i == index ) {
                *dst = color;
            } else {
                *dst = sprite->palette[i];
            }
        } else if ( i < index ) {
            *dst = sprite->palette[i];
        } else {
            *dst = 0;
        }
        dst++;
    }
}

/*
================
GA_FillPaletteFraction

GA_FillPalette at fraction t of the sprite's palette range.
================
*/
void GA_FillPaletteFraction( gaSprite_t *sprite, u16 color, f32 t ) {
    u16 count = sprite->last - sprite->first + 1;

    GA_FillPalette( sprite, color, sprite->first + (u32)( count * t ) );
}

/*
================
GA_PlayLaneSound

Starts sound id for a lane; returns its handle.
================
*/
u64 GA_PlayLaneSound( u8 lane, s32 id ) {
    u8 flags;

    if ( game_lastTeam ) {
        flags = lane + 1;
    } else {
        flags = 0;
    }
    if ( id != 0x4E && id != 0x4F ) {
        flags |= 0x80;
    }
    return Sound_PlaySe( id, flags );
}

/*
================
GA_StopLaneSound

Stops the sound of *handle, if any, and clears it.
================
*/
void GA_StopLaneSound( s64 *handle ) {
    if ( *handle != 0 ) {
        Sound_Stop( *handle );
        *handle = 0;
    }
}

/*
================
func_80120DD0_gacode
================
*/
void func_80120DD0_gacode( void ) {
}

/*
================
GA_ScoreFirstBall

Scores the first ball of player's frame from the ga_pinsDown pins it knocked down, by
ga_bonusOwed (0 nothing owed, 1 a spare's bonus, 2 a strike's, 3 two strikes'): the scores owed
are settled as far as this ball allows and ga_bonusOwed takes its next state (4 to 6 past this
ball, as GA_ScoreSecondBall reads them). A strike before the tenth frame ends the frame.
================
*/
void GA_ScoreFirstBall( u8 player ) {
    if ( ga_pinsDown == 10 ) {
        if ( game_frame[player] == 9 ) {
            game_firstRolls[player][game_frame[player]] = -1;
            game_frameScores[player][game_frame[player]] = -1;
            switch ( ga_bonusOwed[player] ) {
            case 0:
                ga_bonusOwed[player] = 5;
                ga_pinsDown = 0;
                game_throw[player] = 1;
                break;
            case 1:
                game_frameScores[player][game_frame[player] - 1] = game_frameScores[player][game_frame[player] - 2] + 20;
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
                ga_bonusOwed[player] = 5;
                ga_pinsDown = 0;
                game_throw[player] = 1;
                break;
            case 2:
                ga_bonusOwed[player] = 6;
                ga_pinsDown = 0;
                game_throw[player] = 1;
                break;
            case 3:
                game_frameScores[player][game_frame[player] - 2] = game_frameScores[player][game_frame[player] - 3] + 30;
                game_totals[player] = game_frameScores[player][game_frame[player] - 2];
                ga_bonusOwed[player] = 6;
                ga_pinsDown = 0;
                game_throw[player] = 1;
                break;
            }
        } else {
            game_firstRolls[player][game_frame[player]] = -1;
            game_secondRolls[player][game_frame[player]] = -2;
            game_frameScores[player][game_frame[player]] = -1;
            switch ( ga_bonusOwed[player] ) {
            case 0:
                ga_bonusOwed[player] = 2;
                ga_pinsDown = 0;
                game_throw[player] = 0;
                game_frame[player]++;
                break;
            case 1:
                game_frameScores[player][game_frame[player] - 1] = game_frameScores[player][game_frame[player] - 2] + 20;
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
                ga_bonusOwed[player] = 2;
                ga_pinsDown = 0;
                game_throw[player] = 0;
                game_frame[player]++;
                break;
            case 2:
                ga_bonusOwed[player] = 3;
                ga_pinsDown = 0;
                game_throw[player] = 0;
                game_frame[player]++;
                break;
            case 3:
                if ( game_frame[player] == 2 ) {
                    game_frameScores[player][game_frame[player] - 2] = 30;
                } else {
                    game_frameScores[player][game_frame[player] - 2] = game_frameScores[player][game_frame[player] - 3] + 30;
                }
                game_totals[player] = game_frameScores[player][game_frame[player] - 2];
                ga_bonusOwed[player] = 3;
                ga_pinsDown = 0;
                game_throw[player] = 0;
                game_frame[player]++;
                break;
            }
        }
    } else {
        game_firstRolls[player][game_frame[player]] = ga_pinsDown;
        switch ( ga_bonusOwed[player] ) {
        case 0:
            ga_bonusOwed[player] = 0;
            ga_pinsDown = 0;
            game_throw[player] = 1;
            break;
        case 1:
            game_frameScores[player][game_frame[player] - 1] =
                10 + game_firstRolls[player][game_frame[player]] + game_frameScores[player][game_frame[player] - 2];
            game_totals[player] = game_frameScores[player][game_frame[player] - 1];
            ga_bonusOwed[player] = 0;
            ga_pinsDown = 0;
            game_throw[player] = 1;
            break;
        case 2:
            ga_bonusOwed[player] = 4;
            ga_pinsDown = 0;
            game_throw[player] = 1;
            break;
        case 3:
            if ( game_frame[player] == 2 ) {
                game_frameScores[player][game_frame[player] - 2] = 20 + game_firstRolls[player][game_frame[player]];
            } else {
                game_frameScores[player][game_frame[player] - 2] =
                    20 + game_firstRolls[player][game_frame[player]] + game_frameScores[player][game_frame[player] - 3];
            }
            game_totals[player] = game_frameScores[player][game_frame[player] - 2];
            ga_bonusOwed[player] = 4;
            ga_pinsDown = 0;
            game_throw[player] = 1;
            break;
        }
    }
}

/*
================
GA_ScoreSecondBall

Scores the second ball of player's frame from the ga_pinsDown pins it knocked down, by
ga_bonusOwed: after an open first ball (0, or 4 with a strike's bonus owed) a spare leaves a
spare's bonus owed (1) and an open frame is scored; in the tenth frame after a strike (5, or 6 with a
strike's bonus owed) a strike leaves 3, anything else 2, and the third ball follows.
================
*/
void GA_ScoreSecondBall( u8 player ) {
    if ( ga_bonusOwed[player] == 0 || ga_bonusOwed[player] == 4 ) {
        if ( game_firstRolls[player][game_frame[player]] + ga_pinsDown == 10 ) {
            game_secondRolls[player][game_frame[player]] = -1;
            game_frameScores[player][game_frame[player]] = -1;
            switch ( ga_bonusOwed[player] ) {
            case 0:
                if ( game_frame[player] == 9 ) {
                    ga_bonusOwed[player] = 1;
                    ga_pinsDown = 0;
                    game_throw[player] = 2;
                } else {
                    ga_bonusOwed[player] = 1;
                    ga_pinsDown = 0;
                    game_throw[player] = 0;
                    game_frame[player]++;
                }
                break;
            case 4:
                game_frameScores[player][game_frame[player] - 1] = game_frameScores[player][game_frame[player] - 2] + 20;
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
                if ( game_frame[player] == 9 ) {
                    ga_bonusOwed[player] = 1;
                    ga_pinsDown = 0;
                    game_throw[player] = 2;
                } else {
                    ga_bonusOwed[player] = 1;
                    ga_pinsDown = 0;
                    game_throw[player] = 0;
                    game_frame[player]++;
                }
                break;
            }
        } else {
            game_secondRolls[player][game_frame[player]] = ga_pinsDown;
            if ( ga_bonusOwed[player] == 4 ) {
                if ( game_frame[player] == 1 ) {
                    game_frameScores[player][game_frame[player] - 1] =
                        10 + game_firstRolls[player][game_frame[player]] + game_secondRolls[player][game_frame[player]];
                } else {
                    game_frameScores[player][game_frame[player] - 1] = game_frameScores[player][game_frame[player] - 2] +
                        ( 10 + game_firstRolls[player][game_frame[player]] + game_secondRolls[player][game_frame[player]] );
                }
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
            }
            if ( game_frame[player] == 0 ) {
                game_frameScores[player][game_frame[player]] =
                    game_firstRolls[player][game_frame[player]] + game_secondRolls[player][game_frame[player]];
            } else {
                game_frameScores[player][game_frame[player]] = game_frameScores[player][game_frame[player] - 1] +
                    ( game_firstRolls[player][game_frame[player]] + game_secondRolls[player][game_frame[player]] );
            }
            game_totals[player] = game_frameScores[player][game_frame[player]];
            if ( game_frame[player] == 9 ) {
                game_playerDone[player] = 1;
                game_throw[player]++;
            } else {
                ga_bonusOwed[player] = 0;
                ga_pinsDown = 0;
                game_throw[player] = 0;
                game_frame[player]++;
            }
        }
    }
    if ( ga_bonusOwed[player] == 5 || ga_bonusOwed[player] == 6 ) {
        if ( ga_pinsDown == 10 ) {
            game_secondRolls[player][game_frame[player]] = -1;
            game_frameScores[player][game_frame[player]] = -1;
            if ( ga_bonusOwed[player] == 6 ) {
                game_frameScores[player][game_frame[player] - 1] = game_frameScores[player][game_frame[player] - 2] + 30;
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
            }
            ga_bonusOwed[player] = 3;
            ga_pinsDown = 0;
            game_throw[player] = 2;
        } else {
            game_secondRolls[player][game_frame[player]] = ga_pinsDown;
            if ( ga_bonusOwed[player] == 6 ) {
                game_frameScores[player][game_frame[player] - 1] =
                    20 + game_secondRolls[player][game_frame[player]] + game_frameScores[player][game_frame[player] - 2];
                game_totals[player] = game_frameScores[player][game_frame[player] - 1];
            }
            ga_bonusOwed[player] = 2;
            ga_pinsDown = 0;
            game_throw[player] = 2;
        }
    }
}

/*
================
GA_ScoreThirdBall

Scores the third ball of player's frame (D_80068A50_gacode 2) from the ga_pinsDown pins it knocked
down, by ga_bonusOwed: 1 and 3 add 10 or 20 to the previous score plus the third ball (a strike
counts 10 more), 2 adds 10 plus the second and third balls (20 when they make a spare). The third ball
is stored in D_800994B8 (-1 for a strike or spare) and the player's game ends (game_playerDone).
================
*/
void GA_ScoreThirdBall( u8 player ) {
    if ( ga_bonusOwed[player] == 1 || ga_bonusOwed[player] == 3 ) {
        if ( ga_pinsDown == 10 ) {
            D_800994B8[player] = -1;
            if ( ga_bonusOwed[player] == 1 ) {
                game_frameScores[player][game_frame[player]] = game_frameScores[player][game_frame[player] - 1] + 20;
                game_totals[player] = game_frameScores[player][game_frame[player]];
                game_throw[player]++;
                game_playerDone[player] = 1;
            }
            if ( ga_bonusOwed[player] == 3 ) {
                game_frameScores[player][game_frame[player]] = game_frameScores[player][game_frame[player] - 1] + 30;
                game_totals[player] = game_frameScores[player][game_frame[player]];
                game_throw[player]++;
                game_playerDone[player] = 1;
            }
        } else {
            D_800994B8[player] = ga_pinsDown;
            if ( ga_bonusOwed[player] == 1 ) {
                game_frameScores[player][game_frame[player]] =
                    10 + D_800994B8[player] + game_frameScores[player][game_frame[player] - 1];
                game_totals[player] = game_frameScores[player][game_frame[player]];
                game_throw[player]++;
                game_playerDone[player] = 1;
            }
            if ( ga_bonusOwed[player] == 3 ) {
                game_frameScores[player][game_frame[player]] =
                    20 + D_800994B8[player] + game_frameScores[player][game_frame[player] - 1];
                game_totals[player] = game_frameScores[player][game_frame[player]];
                game_throw[player]++;
                game_playerDone[player] = 1;
            }
        }
    }
    if ( ga_bonusOwed[player] == 2 ) {
        if ( game_secondRolls[player][game_frame[player]] + ga_pinsDown == 10 ) {
            D_800994B8[player] = -1;
            game_frameScores[player][game_frame[player]] = game_frameScores[player][game_frame[player] - 1] + 20;
            game_totals[player] = game_frameScores[player][game_frame[player]];
            game_throw[player]++;
            game_playerDone[player] = 1;
        } else {
            D_800994B8[player] = ga_pinsDown;
            game_frameScores[player][game_frame[player]] =
                10 + game_secondRolls[player][game_frame[player]] + D_800994B8[player] +
                game_frameScores[player][game_frame[player] - 1];
            game_totals[player] = game_frameScores[player][game_frame[player]];
            game_throw[player]++;
            game_playerDone[player] = 1;
        }
    }
}

/*
================
GA_ClearScores

Clears every player's scores, results and counters for a new game.
================
*/
void GA_ClearScores( void ) {
    u8 player;
    u8 frame;

    for ( player = 0; player < 4; player++ ) {
        for ( frame = 0; frame < 10; frame++ ) {
            game_firstRolls[player][frame] = 0;
            game_secondRolls[player][frame] = 0;
            game_frameFlags[player][frame] = 0;
            game_frameScores[player][frame] = 0;
        }
        for ( frame = 0; frame < 12; frame++ ) {
            game_firstBallPins[player][frame] = 0;
            game_secondBallPins[player][frame] = 0;
        }
        game_strikes[player] = 0;
        game_spares[player] = 0;
        game_splits[player] = 0;
        game_totals[player] = 0;
        D_800994B8[player] = 0;
        game_frame[player] = 0;
        game_throw[player] = 0;
        ga_bonusOwed[player] = 0;
        game_playerDone[player] = 0;
        game_strikeRun[player] = 0;
        game_pinFrame[player] = 0;
    }
    ga_pinsDown = 0;
    ga_lastScorer = 0;
    game_scoresCleared = 1;
}

/*
================
GA_ScoreThrow
================
*/
void GA_ScoreThrow( u8 player, u8 pinsHit ) {
    ga_lastScorer = player;
    ga_pinsDown = pinsHit;
    switch ( game_throw[player] ) {
    case 0:
        GA_ScoreFirstBall( player );
        break;
    case 1:
        GA_ScoreSecondBall( player );
        break;
    case 2:
        GA_ScoreThirdBall( player );
        break;
    }
}

/*
================
GA_EndFrame

Ends the player's frame: with done set, marks the frame's two results -1 and moves to the next frame (and
returns 1); else takes one from game_throw. The player is finished after frame 10 or when game_throw
reaches 0.
================
*/
s32 GA_EndFrame( u8 player, s32 done ) {
    u8 frame = game_frame[player];
    s32 advanced = 0;

    if ( done ) {
        game_firstRolls[player][frame] = game_secondRolls[player][frame] = -1;
        game_frame[player]++;
        advanced = 1;
    } else {
        game_throw[player]--;
    }
    if ( game_frame[player] == 10 || game_throw[player] == 0 ) {
        game_playerDone[player] = 1;
    }
    return advanced;
}

/*
================
GA_CountParThrow

Counts a throw of the player's frame; once forced, or at game_frameFlags + 5 throws, scores the frame against
game_frameFlags and moves on (returns 1 then). The player is finished after frame 9.
================
*/
s32 GA_CountParThrow( u8 player, s32 force ) {
    u8 frame;
    s8 throws;
    s32 advanced = 0;

    frame = game_frame[player];
    throws = ++game_firstRolls[player][frame];

    if ( force || throws >= game_frameFlags[0][frame] + 5 ) {
        game_totals[player] += game_frameScores[player][frame] = throws - game_frameFlags[0][frame];
        game_frame[player]++;
        advanced = 1;
    }
    if ( game_frame[player] == 9 ) {
        game_playerDone[player] = 1;
    }
    return advanced;
}

/*
================
GA_DecideFrame

Decides a frame between players 0 and 1: the lower game_firstRolls result takes 2 and the other 0, a tie gives
both 1; a 2 counts in the player's D_800994B8.
================
*/
void GA_DecideFrame( u8 frame ) {
    if ( game_firstRolls[0][frame] < game_firstRolls[1][frame] ) {
        game_secondRolls[0][frame] = 2;
        game_secondRolls[1][frame] = 0;
    } else if ( game_firstRolls[0][frame] > game_firstRolls[1][frame] ) {
        game_secondRolls[1][frame] = 2;
        game_secondRolls[0][frame] = 0;
    } else {
        game_secondRolls[1][frame] = 1;
        game_secondRolls[0][frame] = 1;
    }
    D_800994B8[0] += game_secondRolls[0][frame] == 2 ? 1 : 0;
    D_800994B8[1] += game_secondRolls[1][frame] == 2 ? 1 : 0;
}
