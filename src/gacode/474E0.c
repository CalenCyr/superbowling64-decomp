/* 474E0.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "gacode/gacode.h"

/*
================
func_8011D2C0_gacode

Builds the placement matrix of a body: rotation rot (degrees), then scale, then translation pos.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: stack frame; retail's is 0xA8 bytes, 0x80 more than this body needs, as if two 4x4 float
// matrices were declared and never used; every instruction else is the same
void func_8011D2C0_gacode( f32 mf[4][4], vec3_t pos, vec3_t rot, f32 scale ) {
    guPositionF( mf, rot[0], rot[1], rot[2], scale, pos[0], pos[1], pos[2] );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D2C0_gacode);
#endif

#ifdef NON_MATCHING
// NON_MATCHING: stack frame; retail's is 0xF0 bytes, 0x80 more than this body needs (as func_8011D2C0_gacode)
/*
================
func_8011D304_gacode

out = in placed by rotation rot (degrees) and translation pos.
================
*/
void func_8011D304_gacode( vec3_t out, vec3_t in, vec3_t rot, vec3_t pos ) {
    f32 mf[4][4];

    guPositionF( mf, rot[0], rot[1], rot[2], 1.0f, pos[0], pos[1], pos[2] );
    guMtxXFMF( mf, in[0], in[1], in[2], &out[0], &out[1], &out[2] );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D304_gacode);
#endif

/*
================
func_8011D3A0_gacode

out = to - from
Declared inline: func_8011D4E4_gacode expands it.
================
*/
inline void func_8011D3A0_gacode( vec3_t out, vec3_t from, vec3_t to ) {
    out[0] = to[0] - from[0];
    out[1] = to[1] - from[1];
    out[2] = to[2] - from[2];
}

/*
================
func_8011D3D4_gacode

Length of v.
Declared inline: func_8011D5B8_gacode expands it.
================
*/
inline f32 func_8011D3D4_gacode( vec3_t v ) {
    return sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

/*
================
func_8011D424_gacode

out = in scaled to length 1, or (0, -1, 0) when in is zero; returns the length of in.
Declared inline: func_8011D4E4_gacode and func_8011D6C4_gacode expand it.
================
*/
inline f32 func_8011D424_gacode( vec3_t out, vec3_t in ) {
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
func_8011D4E4_gacode

out = the unit vector from from to to; returns their distance.
================
*/
f32 func_8011D4E4_gacode( vec3_t out, vec3_t from, vec3_t to ) {
    vec3_t delta;

    func_8011D3A0_gacode( delta, from, to );
    return func_8011D424_gacode( out, delta );
}

/*
================
func_8011D5B8_gacode

Cosine of the angle between a and b; 1 when either is zero.
Declared inline: func_8011FD08_gacode expands it.
================
*/
inline f32 func_8011D5B8_gacode( vec3_t a, vec3_t b ) {
    f32 lengths = func_8011D3D4_gacode( a ) * func_8011D3D4_gacode( b );

    if ( lengths == 0.0f ) {
        return 1.0f;
    }
    return ( a[0] * b[0] + a[1] * b[1] + a[2] * b[2] ) / lengths;
}

/*
================
func_8011D6C4_gacode

out = the unit vector of a x b, or (0, -1, 0) when a and b are parallel.
Declared inline: func_8011F87C_gacode expands it.
================
*/
inline void func_8011D6C4_gacode( vec3_t out, vec3_t a, vec3_t b ) {
    vec3_t cross;

    cross[0] = a[1] * b[2] - a[2] * b[1];
    cross[1] = a[2] * b[0] - a[0] * b[2];
    cross[2] = a[0] * b[1] - a[1] * b[0];
    func_8011D424_gacode( out, cross );
}

/*
================
func_8011D7CC_gacode

Sets the ball's rot[1] (degrees) from the heading of v in the x-z plane, approximated from |v[0] / v[2]|
(45 * its square root below 1) rather than an arctangent; with v[2] zero it turns rot[1] by 90 instead.
Clamped to -180..179.9999.
================
*/
void func_8011D7CC_gacode( vec3_t v, gaBody_t *ball ) {
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
func_8011D9CC_gacode

Nonzero when the spheres of spots a and b (centre pos, radius unk_18) overlap: their boxes first, axis by
axis, then the distance.
================
*/
s32 func_8011D9CC_gacode( gaSpot_t *a, gaSpot_t *b ) {
    f32 dx;
    f32 dy;
    f32 dz;

    if ( FABS( a->pos[0] - b->pos[0] ) < a->unk_18 + b->unk_18 &&
         FABS( a->pos[1] - b->pos[1] ) < a->unk_18 + b->unk_18 &&
         FABS( a->pos[2] - b->pos[2] ) < a->unk_18 + b->unk_18 ) {
        dx = a->pos[0] - b->pos[0];
        dy = a->pos[1] - b->pos[1];
        dz = a->pos[2] - b->pos[2];
        if ( dx * dx + dy * dy + dz * dz < ( a->unk_18 + b->unk_18 ) * ( a->unk_18 + b->unk_18 ) ) {
            return 1;
        }
    }
    return 0;
}

/*
================
func_8011DB54_gacode

Nonzero when the segment from from to to passes within radius plus the spot's radius of the spot's centre,
at a point between its ends.
Declared inline: func_8011E6F8_gacode (parked draft) expands it.
================
*/
inline s32 func_8011DB54_gacode( gaSpot_t *spot, vec3_t from, vec3_t to, f32 radius ) {
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
    if ( ( spot->unk_18 + radius ) * ( spot->unk_18 + radius ) <= ox * ox + oy * oy + oz * oz ) {
        return 0;
    }
    return 1;
}

/*
================
func_8011DCB8_gacode

Contact of spot a, moving along vel, with spot b: normal = the unit vector from a's centre, pushed back
along vel by what the spheres overlap, to b's centre; returns 1 minus the closest distance of b's centre
from a's path over the sum of the radii.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register allocation in the two expansions of func_8011D424_gacode only: retail copies the
// first one's length into $f4 before the zero test and divides by $f4, and in the second takes sqrt.s into $f0
// and copies it to $f2 after the sqrtf fallback joins; this draft uses the sqrt register directly. Everything
// else matches, including $f20/$f22, which need along reused as the push and t as the clamped distance (with
// separate push and dist locals they land in caller-saved registers instead).
f32 func_8011DCB8_gacode( vec3_t vel, gaSpot_t *a, gaSpot_t *b, vec3_t normal ) {
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
    func_8011D3A0_gacode( gap, dir, b->pos );
    t = func_8011D3D4_gacode( gap );
    t = t < 0.0 ? 0.0f : a->unk_18 + b->unk_18 < t ? a->unk_18 + b->unk_18 : t;
    along = a->unk_18 + b->unk_18 - t * 0.85;
    func_8011D424_gacode( dir, vel );
    dir[0] *= along;
    dir[1] *= along;
    dir[2] *= along;
    gap[0] += dir[0];
    gap[1] += dir[1];
    gap[2] += dir[2];
    func_8011D424_gacode( normal, gap );
    return 1.0 - t / ( a->unk_18 + b->unk_18 );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DCB8_gacode);
#endif

/*
================
func_8011E014_gacode

Collision of bodies a and b (spots sa and sb): the one with more momentum is the mover of
func_8011E12C_gacode, and when it had strictly more, the other takes its spin about y, reversed and cut by
the restitution.
Declared inline: func_8011E6F8_gacode (parked draft) expands it.
================
*/
inline void func_8011E014_gacode( gaMotion_t *a, gaMotion_t *b, gaSpot_t *sa, gaSpot_t *sb ) {
    gaMotion_t *in[2];
    gaMotion_t *out[2];
    gaSpot_t *spots[2];
    f32 momentum[2];
    f32 swap;
    f32 restitution;
    f32 keep;

    momentum[0] = ( a->vel[0] * a->vel[0] + a->vel[1] * a->vel[1] + a->vel[2] * a->vel[2] ) * ( a->unk_30 * a->unk_30 );
    momentum[1] = ( b->vel[0] * b->vel[0] + b->vel[1] * b->vel[1] + b->vel[2] * b->vel[2] ) * ( b->unk_30 * b->unk_30 );
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
    restitution = func_8011E12C_gacode( out, in, spots );
    if ( momentum[0] > momentum[1] ) {
        keep = 1.0 - restitution;
        out[1]->spin[1] = -( in[0]->spin[1] * keep );
    }
}

/*
================
func_8011E12C_gacode

Collision of in[0] (moving) with in[1], weighted by their unk_30, with the restitution of
func_8011DCB8_gacode: out[1] leaves along the contact normal, out[0] keeps the rest of the momentum at its own
speed. Returns the restitution.
================
*/
f32 func_8011E12C_gacode( gaMotion_t **out, gaMotion_t **in, gaSpot_t **spots ) {
    gaMotion_t after[2];
    vec3_t normal;
    vec3_t vel0;
    vec3_t vel1;
    f32 restitution;
    f32 speed;
    f32 speed0;

    restitution = func_8011DCB8_gacode( in[0]->vel, spots[0], spots[1], normal );
    after[0].vel[0] = ( ( in[0]->unk_30 - restitution * in[1]->unk_30 ) * in[0]->vel[0] +
                        ( in[1]->unk_30 + restitution * in[1]->unk_30 ) * in[1]->vel[0] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    after[0].vel[1] = ( ( in[0]->unk_30 - restitution * in[1]->unk_30 ) * in[0]->vel[1] +
                        ( in[1]->unk_30 + restitution * in[1]->unk_30 ) * in[1]->vel[1] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    after[0].vel[2] = ( ( in[0]->unk_30 - restitution * in[1]->unk_30 ) * in[0]->vel[2] +
                        ( in[1]->unk_30 + restitution * in[1]->unk_30 ) * in[1]->vel[2] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    after[1].vel[0] = ( ( in[0]->unk_30 + restitution * in[0]->unk_30 ) * in[0]->vel[0] +
                        ( in[1]->unk_30 - restitution * in[0]->unk_30 ) * in[1]->vel[0] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    after[1].vel[1] = ( ( in[0]->unk_30 + restitution * in[0]->unk_30 ) * in[0]->vel[1] +
                        ( in[1]->unk_30 - restitution * in[0]->unk_30 ) * in[1]->vel[1] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    after[1].vel[2] = ( ( in[0]->unk_30 + restitution * in[0]->unk_30 ) * in[0]->vel[2] +
                        ( in[1]->unk_30 - restitution * in[0]->unk_30 ) * in[1]->vel[2] ) /
                      ( in[0]->unk_30 + in[1]->unk_30 );
    speed = func_8011D3D4_gacode( after[1].vel );
    vel1[0] = normal[0] * speed;
    vel1[1] = normal[1] * speed;
    vel1[2] = normal[2] * speed;
    vel0[0] = ( after[0].vel[0] * in[0]->unk_30 + after[1].vel[0] * in[1]->unk_30 - vel1[0] * in[1]->unk_30 ) /
              in[0]->unk_30;
    vel0[1] = ( after[0].vel[1] * in[0]->unk_30 + after[1].vel[1] * in[1]->unk_30 - vel1[1] * in[1]->unk_30 ) /
              in[0]->unk_30;
    vel0[2] = ( after[0].vel[2] * in[0]->unk_30 + after[1].vel[2] * in[1]->unk_30 - vel1[2] * in[1]->unk_30 ) /
              in[0]->unk_30;
    speed = func_8011D3D4_gacode( after[0].vel );
    speed0 = func_8011D3D4_gacode( vel0 );
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
func_8011E504_gacode

For a body with rot[0] 0 that moves at speed 1 or more: by kind 0 or 1 sets its spin about x, heads it along
its velocity (func_8011D7CC_gacode), moves unk_5D from 1 (still standing) to 2, and plays sound 0x32 (0x4B with
flags bit 1) on the lane of flags bit 0; returns 1. Otherwise returns 0.
================
*/
s32 func_8011E504_gacode( u8 kind, gaMotion_t *motion, gaBody_t *body, u8 flags ) {
    if ( body->rot[0] == 0.0f && func_8011D3D4_gacode( motion->vel ) >= 1.0 ) {
        switch ( kind ) {
        case 0:
            motion->spin[0] = 1.5f;
            break;
        case 1:
            motion->spin[0] = -2.0f;
            break;
        }
        func_8011D7CC_gacode( motion->vel, body );
        if ( body->unk_5D == 1 ) {
            body->unk_5D = 2;
        }
        func_80120D30_gacode( flags & 1, flags & 2 ? 0x4B : 0x32 );
        return 1;
    }
    return 0;
}

/*
================
func_8011E63C_gacode

Moves *speed one step toward limit's side; once *value reaches limit (from below for a positive limit,
from above for a negative one) clamps it there, stops *speed and returns 1.
Declared inline: func_8011F334_gacode expands it.
================
*/
inline s32 func_8011E63C_gacode( f32 *value, f32 *speed, f32 limit ) {
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
func_8011E6F8_gacode

Collisions between the bodies of a lane. Each spot in use (spots[0..10], then spot2 of each body in
spots[11..21]) of a body still in play is tested against each later body in play, first against its
spot (or the segment from that spot to the midpoint with its spot2), then against its spot2 (or the
segment from that midpoint to spot2). A new contact runs func_8011E014_gacode on the two bodies, and each
of them that stands flat (unk_5C 1, rot[0] 0) and was hit at 1 or more starts to tip toward its new
heading, falls (unk_5D 2) and plays its sound (0x4B under the lane, else 0x32). unk_1C of a spot holds
bit 0 (it was hit this frame) and one bit per spot it touches. Returns 1 when the last contact with
the first spots knocked a body over.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: same calls, CFG and inline expansions (func_8011DB54_gacode, func_8011E014_gacode,
// func_8011D3D4_gacode); register allocation differs: retail keeps lane in s7, a in s8 and hit on the stack
// (0x7C), recomputes &bodies[a] at each use, and spills which as a word (andi 0xFF in the inner loop's
// preheader, so it may be (u8)( i / NUM_BODIES ) inside the inner loop); this draft hoists the constant 1 into
// s7 and &bodies[a] into s8, and its frame is 8 bytes larger
s32 func_8011E6F8_gacode( s32 player, gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots, u8 lane ) {
    gaSpot_t mid;
    s32 hit = 0;
    s32 i;
    s32 a;
    s32 b;
    s32 b2;
    u8 which;

    for ( b = 0; b < NUM_BODIES; b++ ) {
        spots[b].unk_1C &= ~1;
    }
    for ( i = 0; i < NUM_BODIES * 2; i++ ) {
        if ( spots[i].unk_18 == 0.0f ) {
            continue;
        }
        a = i % NUM_BODIES;
        which = i / NUM_BODIES;
        if ( bodies[a].unk_5D == 0 ) {
            continue;
        }
        for ( b = a + 1; b < NUM_BODIES; b++ ) {
            b2 = b + NUM_BODIES;
            if ( bodies[b].unk_5D == 0 ) {
                continue;
            }
            if ( spots[b2].unk_18 != 0.0f ) {
                mid.pos[0] = ( spots[b].pos[0] + spots[b2].pos[0] ) / 2.0f;
                mid.pos[1] = ( spots[b].pos[1] + spots[b2].pos[1] ) / 2.0f;
                mid.pos[2] = ( spots[b].pos[2] + spots[b2].pos[2] ) / 2.0f;
                mid.unk_18 = spots[b2].unk_18;
            }
            if ( !( ( spots[i].unk_1C >> ( b2 ) ) & 1 ) &&
                 ( func_8011D9CC_gacode( &spots[i], &spots[b] ) ||
                   ( spots[b2].unk_18 != 0.0f &&
                     func_8011DB54_gacode( &spots[i], spots[b].pos, mid.pos, spots[b].unk_18 ) ) ) ) {
                if ( !( ( spots[i].unk_1C >> b ) & 1 ) ) {
                    func_8011E014_gacode( &motion[a], &motion[b], &spots[i], &spots[b] );
                    lane = spots[i].pos[1] < -190.0f ? lane | 2 : lane;
                    if ( bodies[b].unk_5C == 1 ) {
                        if ( bodies[b].rot[0] == 0.0f && func_8011D3D4_gacode( motion[b].vel ) >= 1.0 ) {
                            motion[b].spin[0] = 1.5f;
                            func_8011D7CC_gacode( motion[b].vel, &bodies[b] );
                            if ( bodies[b].unk_5D == 1 ) {
                                bodies[b].unk_5D = 2;
                            }
                            func_80120D30_gacode( lane & 1, ( lane & 2 ) ? 0x4B : 0x32 );
                            hit = 1;
                        } else {
                            hit = 0;
                        }
                    }
                    if ( bodies[a].unk_5C == 1 ) {
                        if ( bodies[a].rot[0] == 0.0f && func_8011D3D4_gacode( motion[a].vel ) >= 1.0 ) {
                            switch ( which ) {
                            case 0:
                                motion[a].spin[0] = 1.5f;
                                break;
                            case 1:
                                motion[a].spin[0] = -2.0f;
                                break;
                            }
                            func_8011D7CC_gacode( motion[a].vel, &bodies[a] );
                            if ( bodies[a].unk_5D == 1 ) {
                                bodies[a].unk_5D = 2;
                            }
                            func_80120D30_gacode( lane & 1, ( lane & 2 ) ? 0x4B : 0x32 );
                            hit = 1;
                        } else {
                            hit = 0;
                        }
                    }
                    spots[i].unk_1C |= 1 << b;
                    spots[b].unk_1C |= 1;
                }
            } else if ( spots[b2].unk_18 != 0.0f && !( spots[b].unk_1C & 1 ) &&
                        !( ( spots[i].unk_1C >> b ) & 1 ) &&
                        ( func_8011D9CC_gacode( &spots[i], &spots[b2] ) ||
                          func_8011DB54_gacode( &spots[i], mid.pos, spots[b2].pos,
                                                spots[b2].unk_18 ) ) ) {
                if ( !( ( spots[i].unk_1C >> ( b2 ) ) & 1 ) ) {
                    func_8011E014_gacode( &motion[a], &motion[b], &spots[i], &spots[b2] );
                    lane = spots[i].pos[1] < -190.0f ? lane | 2 : lane;
                    if ( bodies[b].unk_5C == 1 && bodies[b].rot[0] == 0.0f &&
                         func_8011D3D4_gacode( motion[b].vel ) >= 1.0 ) {
                        motion[b].spin[0] = -2.0f;
                        func_8011D7CC_gacode( motion[b].vel, &bodies[b] );
                        if ( bodies[b].unk_5D == 1 ) {
                            bodies[b].unk_5D = 2;
                        }
                        func_80120D30_gacode( lane & 1, ( lane & 2 ) ? 0x4B : 0x32 );
                    }
                    if ( bodies[a].unk_5C == 1 && bodies[a].rot[0] == 0.0f &&
                         func_8011D3D4_gacode( motion[a].vel ) >= 1.0 ) {
                        switch ( which ) {
                        case 0:
                            motion[a].spin[0] = 1.5f;
                            break;
                        case 1:
                            motion[a].spin[0] = -2.0f;
                            break;
                        }
                        func_8011D7CC_gacode( motion[a].vel, &bodies[a] );
                        if ( bodies[a].unk_5D == 1 ) {
                            bodies[a].unk_5D = 2;
                        }
                        func_80120D30_gacode( lane & 1, ( lane & 2 ) ? 0x4B : 0x32 );
                    }
                    spots[i].unk_1C |= 1 << ( b2 );
                    spots[b].unk_1C |= 1;
                }
            } else {
                spots[i].unk_1C &= ~( 1 << b );
                spots[i].unk_1C &= ~( 1 << ( b2 ) );
            }
        }
    }
    return hit;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E6F8_gacode);
#endif

/*
================
func_8011F334_gacode

Each frame, for every standing body that is down (unk_5C 1) and has motion flag 2: one lying flat slows to
0.75 of its speed along the lane; one still tipping turns toward 90 degrees on its side and, once there,
slows to 0.85. The spin about y decays to 0.925 either way.
================
*/
void func_8011F334_gacode( gaBody_t *bodies, gaMotion_t *motion, gaSpot_t *spots ) {
    s32 i;
    s32 down;

    for ( i = 0; i < NUM_BODIES; i++ ) {
        if ( bodies[i].unk_5D != 0 && bodies[i].unk_5C == 1 && ( ( motion[i].flags >> 1 ) & 1 ) ) {
            if ( bodies[i].rot[0] == 0.0f ) {
                motion[i].vel[0] *= 0.75;
                motion[i].vel[2] *= 0.75;
            } else {
                if ( bodies[i].rot[0] >= 0.0f ) {
                    down = func_8011E63C_gacode( &bodies[i].rot[0], &motion[i].spin[0], 90.0f );
                } else {
                    down = func_8011E63C_gacode( &bodies[i].rot[0], &motion[i].spin[0], -90.0f );
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
func_8011F4CC_gacode

Keeps a body's spot (and spot2, unless its unk_18 is 0) inside the floor grid and on the floor: a spot
that sank under the floor is lifted back onto it (unless func_8011FD08_gacode handles the contact) and its
fall turned into a damped bounce (motion flag 2). Returns 1 when the body lands with spot, 2 when it
bounces with spot, else 0. Gravity (1.0889) is then applied.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact, but only with the empty `else if` branch (R17): retail skips the in-air case with
// `and` of the two booleans (spot2 in use, spot2 on the floor); `!( a && b ) && c`, a nested if and a named
// flag all give `or` or a different body. The empty branch may stand for a compiled-out macro (debug print)
s32 func_8011F4CC_gacode( gaMotion_t *motion, gaSpot_t *spot, gaSpot_t *spot2, f32 ceiling, f32 arg4 ) {
    vec3_t normal;
    f32 floor;
    f32 bottom2;
    f32 bottom;
    s32 handled;
    s32 landed;

    func_8012016C_gacode( spot, spot2, motion, ceiling );
    floor = func_8011F87C_gacode( spot->pos[0], spot->pos[2], normal );
    motion->flags &= ~0x40;
    landed = 0;
    if ( spot2->unk_18 != 0.0f && ( bottom2 = spot2->pos[1] - spot2->unk_18 ) < floor ) {
        if ( !func_8011FD08_gacode( motion, spot2, normal ) ) {
            spot->pos[1] += floor - bottom2;
            spot2->pos[1] += floor - bottom2;
        }
        motion->vel[1] = -motion->vel[1] - motion->unk_30 * 0.7;
        if ( motion->vel[1] <= 0.0f ) {
            motion->vel[1] = 0.0f;
        }
        if ( spot2->pos[2] <= -1186.0f ) {
            motion->vel[1] = 0.0f;
        }
        motion->flags |= 2;
    } else if ( ( bottom = spot->pos[1] - spot->unk_18 ) < floor ) {
        handled = func_8011FD08_gacode( motion, spot, normal );
        if ( !handled ) {
            spot->pos[1] += floor - bottom;
            if ( spot2->unk_18 != 0.0f ) {
                spot2->pos[1] += floor - bottom;
            }
        }
        motion->vel[1] = -motion->vel[1] - motion->unk_30 * 0.7;
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
    } else if ( spot2->unk_18 != 0.0f && bottom2 == floor ) {
    } else if ( bottom != floor ) {
        if ( ( motion->flags >> 4 ) & 1 ) {
            spot->pos[1] += floor - bottom;
            if ( spot2->unk_18 != 0.0f ) {
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
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F4CC_gacode);
#endif

/*
================
func_8011F87C_gacode

Height of the floor grid at (x, z), interpolated over the triangle of its cell that holds the point; when
normal is not 0 it receives the unit normal of that triangle.
================
*/
f32 func_8011F87C_gacode( f32 x, f32 z, vec3_t normal ) {
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

    pos[0] = x + -D_80100100[0].originX;
    pos[1] = z + -D_80100100[0].originZ;
    cell[0] = (s16)pos[0] / D_80100100[0].stepX;
    cell[1] = (s16)pos[1] / D_80100100[0].stepZ;
    if ( cell[0] < 0 ) {
        cell[0] = 0;
    } else if ( cell[0] >= D_80100100[0].numX - 1 ) {
        cell[0] = D_80100100[0].numX - 2;
    }
    if ( cell[1] < 0 ) {
        cell[1] = 0;
    } else if ( cell[1] >= D_80100100[0].numZ - 1 ) {
        cell[1] = D_80100100[0].numZ - 2;
    }
    h[0] = D_80100100[0].heights[cell[1] * D_80100100[0].numX + cell[0]];
    h[1] = D_80100100[0].heights[cell[1] * D_80100100[0].numX + cell[0] + 1];
    h[2] = D_80100100[0].heights[( cell[1] + 1 ) * D_80100100[0].numX + cell[0]];
    h[3] = D_80100100[0].heights[( cell[1] + 1 ) * D_80100100[0].numX + cell[0] + 1];

    dx = pos[0] - cell[0] * D_80100100[0].stepX;
    dz = pos[1] - cell[1] * D_80100100[0].stepZ;
    edge = D_80100100[0].stepZ * dx / D_80100100[0].stepX;
    if ( dz < edge && dx != 0.0f ) {
        a = ( h[1] - h[0] ) * dx / D_80100100[0].stepX + h[0];
        b = ( h[3] - h[0] ) * dx / D_80100100[0].stepX + h[0];
        height = ( b - a ) * dz / edge + a;
        if ( normal != 0 ) {
            alongZ[2] = D_80100100[0].stepZ;
            alongZ[0] = 0.0f;
            alongX[2] = 0.0f;
            alongX[0] = D_80100100[0].stepX;
            alongX[1] = h[1] - h[0];
            alongZ[1] = h[3] - h[1];
            func_8011D6C4_gacode( normal, alongZ, alongX );
        }
    } else {
        a = ( h[3] - h[0] ) * dx / D_80100100[0].stepX + h[0];
        b = ( h[3] - h[2] ) * dx / D_80100100[0].stepX + h[2];
        height = ( b - a ) * ( dz - edge ) / ( D_80100100[0].stepZ - edge ) + a;
        if ( normal != 0 ) {
            alongZ[2] = D_80100100[0].stepZ;
            alongZ[0] = 0.0f;
            alongX[2] = 0.0f;
            alongX[0] = D_80100100[0].stepX;
            alongX[1] = h[3] - h[2];
            alongZ[1] = h[2] - h[0];
            func_8011D6C4_gacode( normal, alongZ, alongX );
        }
    }
    return height;
}

/*
================
func_8011FD08_gacode

Contact of a body's spot with a sloped floor (normal not straight up), unless motion flag 1: the slope is
taken along x or z, whichever the normal leans more; against a slope steeper than 30 degrees from the
spot's unk_0C the velocity along it bounces back (motion flag 0x40) or is pushed down the slope, else it
slides down the slope (motion flag 0x10 along x, 0x20 along z). Returns 1 on a bounce; else the spot's
unk_0C takes the normal.
================
*/
s32 func_8011FD08_gacode( gaMotion_t *motion, gaSpot_t *spot, vec3_t normal ) {
    vec3_t from;
    vec3_t slope;
    s32 hit = 0;

    if ( ( normal[0] != 0.0f || normal[2] != 0.0f ) && !( motion->flags & 1 ) ) {
        from[1] = spot->unk_0C[1];
        slope[1] = normal[1];
        if ( FABS( normal[0] ) > FABS( normal[2] ) ) {
            from[2] = 0.0f;
            from[0] = spot->unk_0C[0];
            slope[2] = 0.0f;
            slope[0] = normal[0];
            if ( func_8011D5B8_gacode( from, slope ) < 0.866 ) {
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
            from[2] = spot->unk_0C[2];
            from[0] = 0.0f;
            slope[2] = normal[2];
            slope[0] = 0.0f;
            if ( func_8011D5B8_gacode( from, slope ) < 0.866 ) {
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
        spot->unk_0C[0] = normal[0];
        spot->unk_0C[1] = normal[1];
        spot->unk_0C[2] = normal[2];
    }
    return hit;
}

/*
================
func_8012016C_gacode

Keeps spot inside the floor grid (along z and x) and below ceiling: a spot past an edge is put back on
it, spot2 moved with it unless its unk_18 is 0, and the velocity across that edge bounced (z: -0.1, motion
flag 4; x: 0.5, flag 1; y: -0.0001).
================
*/
void func_8012016C_gacode( gaSpot_t *spot, gaSpot_t *spot2, gaMotion_t *motion, f32 ceiling ) {
    f32 edge;

    motion->flags &= ~4;
    motion->flags &= ~1;

    edge = (f32)( ( D_80100100[0].numZ - 1 ) * D_80100100[0].stepZ + D_80100100[0].originZ ) - 0.0002;
    if ( spot->pos[2] > edge || spot->pos[2] < ( edge = D_80100100[0].originZ ) ) {
        spot->pos[2] -= spot->pos[2] - edge;
        if ( spot2->unk_18 != 0.0f ) {
            spot2->pos[2] -= spot->pos[2] - edge;
        }
        motion->vel[2] *= -0.1;
        motion->flags |= 4;
    }

    edge = ( D_80100100[0].numX - 1 ) * D_80100100[0].stepX + D_80100100[0].originX;
    if ( spot->pos[0] > edge || spot->pos[0] < ( edge = D_80100100[0].originX ) ) {
        spot->pos[0] -= spot->pos[0] - edge;
        if ( spot2->unk_18 != 0.0f ) {
            spot2->pos[0] -= spot2->pos[0] - edge;
        }
        motion->vel[0] *= 0.5;
        motion->flags |= 1;
    }

    edge = ceiling - 0.0002;
    if ( spot->pos[1] > edge ) {
        spot->pos[1] -= spot->pos[1] - edge;
        if ( spot2->unk_18 != 0.0f ) {
            spot2->pos[1] -= spot2->pos[1] - edge;
        }
        motion->vel[1] *= -0.0001;
    }
}

/*
================
func_801203AC_gacode

Picks the pin to aim at from side's view: with one pin standing, that pin; with the last standing pin
checked 1 or 5, that pin, offset 4.9 toward side; else the group (of two, split by D_8012C900_gacode's
order) with more standing pins, offset 15 (unless pin 7 or 10) or 5. Writes its x and z to out and
returns the pin.
================
*/
s32 func_801203AC_gacode( vec3_t out, gaBody_t *bodies, gaSpot_t *spots, s32 side ) {
    s32 right = 0;
    s32 left = 0;
    s32 numRight = 0;
    s32 numLeft = 0;
    s32 numStanding = 0;
    s32 dir = side ? -1 : 1;
    s32 last;
    s32 i;

    for ( i = 0; i < NUM_PINS; i++ ) {
        if ( bodies[D_8012C900_gacode[side][i]].unk_5D != 1 ) {
            continue;
        }
        switch ( i ) {
        case 2:
        case 3:
        case 5:
        case 8:
            right = D_8012C900_gacode[side][i];
            numRight++;
            break;
        case 0:
        case 1:
        case 4:
        case 7:
            left = D_8012C900_gacode[side][i];
            numLeft++;
            break;
        default:
            right = left = D_8012C900_gacode[side][i];
            numRight++;
            numLeft++;
            break;
        }
        last = D_8012C900_gacode[side][i];
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
func_801205A0_gacode
================
*/
f32 func_801205A0_gacode( vec3_t pos, vec3_t vel, f32 x, f32 scale ) {
    f32 dx = x - pos[0];
    f32 width = 730.0f - D_8012D220_gacode->unk_08;
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
         / ( width * width + width * 2 * dz + D_8012D220_gacode->unk_10 * ( dz * dz ) ) * 15.0 / D_8012D220_gacode->unk_0C;
}

/*
================
func_80120680_gacode

Binary search of the count sorted entries of table for key; its index, or -1.
Declared inline: func_801206E0_gacode expands it.
================
*/
inline s32 func_80120680_gacode( u16 key, s32 count, u16 *table ) {
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
func_801206E0_gacode

1 when the head pin is down and the mask of the standing pins is one of the leaves listed for their
number (2 to 6, tables D_801236C0_gacode to D_801237EC_gacode); else 0.
================
*/
u8 func_801206E0_gacode( gaLane_t *lane ) {
    u8 listed = 0;
    s32 mask = 0;
    s32 numStanding = 0;
    s32 i;

    if ( lane->bodies[1].unk_5D == 1 ) {
        return 0;
    }
    for ( i = 2; i < NUM_BODIES; i++ ) {
        if ( lane->bodies[i].unk_5D == 1 ) {
            mask |= lane->bodies[i].unk_5D << ( 10 - i );
            numStanding++;
        }
    }
    switch ( numStanding ) {
    case 2:
        if ( func_80120680_gacode( mask, D_801236C0_gacode[0], &D_801236C0_gacode[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 3:
        if ( func_80120680_gacode( mask, D_801236F0_gacode[0], &D_801236F0_gacode[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 4:
        if ( func_80120680_gacode( mask, D_80123748_gacode[0], &D_80123748_gacode[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 5:
        if ( func_80120680_gacode( mask, D_801237BC_gacode[0], &D_801237BC_gacode[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    case 6:
        if ( func_80120680_gacode( mask, D_801237EC_gacode[0], &D_801237EC_gacode[1] ) >= 0 ) {
            listed = 1;
        }
        break;
    }
    return listed;
}

/*
================
func_80120980_gacode

Unpacks a pin mask, pin 1 in bit 9, into one flag per pin.
================
*/
void func_80120980_gacode( u8 *pins, u16 mask ) {
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
func_801209C0_gacode

Fills order with first, first + 1, ... (count values) and shuffles it.
================
*/
void func_801209C0_gacode( u8 *order, u8 count, u8 first ) {
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

/*
================
func_80120A7C_gacode

Sets up a sprite of resource id with palette range first to last, keeping a copy of its palette.
================
*/
void func_80120A7C_gacode( gaSprite_t *sprite, u16 id, u16 first, u16 last ) {
    func_8002DF3C( id, 320, 240 );
    if ( D_800F80B0[id].data != NULL ) {
        memmove( sprite->palette, ( (unkImage_t *)D_800F80B0[id].data )->palette, sizeof( sprite->palette ) );
        sprite->id = id;
        sprite->first = first;
        sprite->last = last;
        sprite->unk_206 = 0;
    }
}

/*
================
func_80120B10_gacode

Puts the sprite's saved palette back into its resource.
================
*/
void func_80120B10_gacode( gaSprite_t *sprite ) {
    if ( D_800F80B0[sprite->id].data != NULL ) {
        memmove( ( (unkImage_t *)D_800F80B0[sprite->id].data )->palette, sprite->palette, sizeof( sprite->palette ) );
    }
}

/*
================
func_80120B50_gacode

Rewrites the sprite's palette range in its resource from the saved copy: with color 0, the entries before
index keep their colour and the others go black; else only entry index takes color.

Declared inline: nothing calls this copy, and func_80120C0C_gacode repeats its body.
================
*/
inline void func_80120B50_gacode( gaSprite_t *sprite, u16 color, u16 index ) {
    u16 *dst;
    s32 i;

    dst = D_800F80B0[sprite->id].data;
    if ( dst == NULL ) {
        return;
    }
    dst = ( (unkImage_t *)D_800F80B0[sprite->id].data )->palette;
    dst += sprite->first;
    if ( index < sprite->first ) {
        index = sprite->first;
    } else if ( index > sprite->last ) {
        index = sprite->last;
    }
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
func_80120C0C_gacode

func_80120B50_gacode at fraction t of the sprite's palette range.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register colouring only: retail keeps the unclamped index in a3 and its u16 copy in t1; this
// swaps them (tried: an index local, operand order, u16/u32/s32 offsets, the inline's local order and clamp
// spelling; 22k permuter iterations from base 50 found nothing lower)
void func_80120C0C_gacode( gaSprite_t *sprite, u16 color, f32 t ) {
    u16 count = sprite->last - sprite->first + 1;

    func_80120B50_gacode( sprite, color, sprite->first + (u32)( count * t ) );
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120C0C_gacode);
#endif

/*
================
func_80120D30_gacode

Starts sound id for a lane; returns its handle.
================
*/
u64 func_80120D30_gacode( u8 lane, s32 id ) {
    u8 flags;

    if ( D_8005C162 ) {
        flags = lane + 1;
    } else {
        flags = 0;
    }
    if ( id != 0x4E && id != 0x4F ) {
        flags |= 0x80;
    }
    return func_80037870( id, flags );
}

/*
================
func_80120D7C_gacode

Stops the sound of *handle, if any, and clears it.
================
*/
void func_80120D7C_gacode( s64 *handle ) {
    if ( *handle != 0 ) {
        func_800379A8( *handle );
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
func_80120DD8_gacode

Scores the first ball of player's frame from the D_8012D8DA_gacode pins it knocked down, by
D_8012CDE0_gacode (0 nothing owed, 1 a spare's bonus, 2 a strike's, 3 two strikes'): the scores owed
are settled as far as this ball allows and D_8012CDE0_gacode takes its next state (4 to 6 past this
ball, as func_801213B4_gacode reads them). A strike before the tenth frame ends the frame.
================
*/
void func_80120DD8_gacode( u8 player ) {
    if ( D_8012D8DA_gacode == 10 ) {
        if ( D_8009D19C[player] == 9 ) {
            D_80066E18[player][D_8009D19C[player]] = -1;
            D_800DF630[player][D_8009D19C[player]] = -1;
            switch ( D_8012CDE0_gacode[player] ) {
            case 0:
                D_8012CDE0_gacode[player] = 5;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 1;
                break;
            case 1:
                D_800DF630[player][D_8009D19C[player] - 1] = D_800DF630[player][D_8009D19C[player] - 2] + 20;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
                D_8012CDE0_gacode[player] = 5;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 1;
                break;
            case 2:
                D_8012CDE0_gacode[player] = 6;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 1;
                break;
            case 3:
                D_800DF630[player][D_8009D19C[player] - 2] = D_800DF630[player][D_8009D19C[player] - 3] + 30;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 2];
                D_8012CDE0_gacode[player] = 6;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 1;
                break;
            }
        } else {
            D_80066E18[player][D_8009D19C[player]] = -1;
            D_800A3020[player][D_8009D19C[player]] = -2;
            D_800DF630[player][D_8009D19C[player]] = -1;
            switch ( D_8012CDE0_gacode[player] ) {
            case 0:
                D_8012CDE0_gacode[player] = 2;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 0;
                D_8009D19C[player]++;
                break;
            case 1:
                D_800DF630[player][D_8009D19C[player] - 1] = D_800DF630[player][D_8009D19C[player] - 2] + 20;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
                D_8012CDE0_gacode[player] = 2;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 0;
                D_8009D19C[player]++;
                break;
            case 2:
                D_8012CDE0_gacode[player] = 3;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 0;
                D_8009D19C[player]++;
                break;
            case 3:
                if ( D_8009D19C[player] == 2 ) {
                    D_800DF630[player][D_8009D19C[player] - 2] = 30;
                } else {
                    D_800DF630[player][D_8009D19C[player] - 2] = D_800DF630[player][D_8009D19C[player] - 3] + 30;
                }
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 2];
                D_8012CDE0_gacode[player] = 3;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 0;
                D_8009D19C[player]++;
                break;
            }
        }
    } else {
        D_80066E18[player][D_8009D19C[player]] = D_8012D8DA_gacode;
        switch ( D_8012CDE0_gacode[player] ) {
        case 0:
            D_8012CDE0_gacode[player] = 0;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 1;
            break;
        case 1:
            D_800DF630[player][D_8009D19C[player] - 1] =
                10 + D_80066E18[player][D_8009D19C[player]] + D_800DF630[player][D_8009D19C[player] - 2];
            D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
            D_8012CDE0_gacode[player] = 0;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 1;
            break;
        case 2:
            D_8012CDE0_gacode[player] = 4;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 1;
            break;
        case 3:
            if ( D_8009D19C[player] == 2 ) {
                D_800DF630[player][D_8009D19C[player] - 2] = 20 + D_80066E18[player][D_8009D19C[player]];
            } else {
                D_800DF630[player][D_8009D19C[player] - 2] =
                    20 + D_80066E18[player][D_8009D19C[player]] + D_800DF630[player][D_8009D19C[player] - 3];
            }
            D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 2];
            D_8012CDE0_gacode[player] = 4;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 1;
            break;
        }
    }
}

/*
================
func_801213B4_gacode

Scores the second ball of player's frame from the D_8012D8DA_gacode pins it knocked down, by
D_8012CDE0_gacode: after an open first ball (0, or 4 with a strike's bonus owed) a spare leaves a
spare's bonus owed (1) and an open frame is scored; in the tenth frame after a strike (5, or 6 with a
strike's bonus owed) a strike leaves 3, anything else 2, and the third ball follows.
================
*/
void func_801213B4_gacode( u8 player ) {
    if ( D_8012CDE0_gacode[player] == 0 || D_8012CDE0_gacode[player] == 4 ) {
        if ( D_80066E18[player][D_8009D19C[player]] + D_8012D8DA_gacode == 10 ) {
            D_800A3020[player][D_8009D19C[player]] = -1;
            D_800DF630[player][D_8009D19C[player]] = -1;
            switch ( D_8012CDE0_gacode[player] ) {
            case 0:
                if ( D_8009D19C[player] == 9 ) {
                    D_8012CDE0_gacode[player] = 1;
                    D_8012D8DA_gacode = 0;
                    D_80068A50[player] = 2;
                } else {
                    D_8012CDE0_gacode[player] = 1;
                    D_8012D8DA_gacode = 0;
                    D_80068A50[player] = 0;
                    D_8009D19C[player]++;
                }
                break;
            case 4:
                D_800DF630[player][D_8009D19C[player] - 1] = D_800DF630[player][D_8009D19C[player] - 2] + 20;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
                if ( D_8009D19C[player] == 9 ) {
                    D_8012CDE0_gacode[player] = 1;
                    D_8012D8DA_gacode = 0;
                    D_80068A50[player] = 2;
                } else {
                    D_8012CDE0_gacode[player] = 1;
                    D_8012D8DA_gacode = 0;
                    D_80068A50[player] = 0;
                    D_8009D19C[player]++;
                }
                break;
            }
        } else {
            D_800A3020[player][D_8009D19C[player]] = D_8012D8DA_gacode;
            if ( D_8012CDE0_gacode[player] == 4 ) {
                if ( D_8009D19C[player] == 1 ) {
                    D_800DF630[player][D_8009D19C[player] - 1] =
                        10 + D_80066E18[player][D_8009D19C[player]] + D_800A3020[player][D_8009D19C[player]];
                } else {
                    D_800DF630[player][D_8009D19C[player] - 1] = D_800DF630[player][D_8009D19C[player] - 2] +
                        ( 10 + D_80066E18[player][D_8009D19C[player]] + D_800A3020[player][D_8009D19C[player]] );
                }
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
            }
            if ( D_8009D19C[player] == 0 ) {
                D_800DF630[player][D_8009D19C[player]] =
                    D_80066E18[player][D_8009D19C[player]] + D_800A3020[player][D_8009D19C[player]];
            } else {
                D_800DF630[player][D_8009D19C[player]] = D_800DF630[player][D_8009D19C[player] - 1] +
                    ( D_80066E18[player][D_8009D19C[player]] + D_800A3020[player][D_8009D19C[player]] );
            }
            D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
            if ( D_8009D19C[player] == 9 ) {
                D_800994BE[player] = 1;
                D_80068A50[player]++;
            } else {
                D_8012CDE0_gacode[player] = 0;
                D_8012D8DA_gacode = 0;
                D_80068A50[player] = 0;
                D_8009D19C[player]++;
            }
        }
    }
    if ( D_8012CDE0_gacode[player] == 5 || D_8012CDE0_gacode[player] == 6 ) {
        if ( D_8012D8DA_gacode == 10 ) {
            D_800A3020[player][D_8009D19C[player]] = -1;
            D_800DF630[player][D_8009D19C[player]] = -1;
            if ( D_8012CDE0_gacode[player] == 6 ) {
                D_800DF630[player][D_8009D19C[player] - 1] = D_800DF630[player][D_8009D19C[player] - 2] + 30;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
            }
            D_8012CDE0_gacode[player] = 3;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 2;
        } else {
            D_800A3020[player][D_8009D19C[player]] = D_8012D8DA_gacode;
            if ( D_8012CDE0_gacode[player] == 6 ) {
                D_800DF630[player][D_8009D19C[player] - 1] =
                    20 + D_800A3020[player][D_8009D19C[player]] + D_800DF630[player][D_8009D19C[player] - 2];
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player] - 1];
            }
            D_8012CDE0_gacode[player] = 2;
            D_8012D8DA_gacode = 0;
            D_80068A50[player] = 2;
        }
    }
}

/*
================
func_801219CC_gacode

Scores the third ball of player's frame (D_80068A50_gacode 2) from the D_8012D8DA_gacode pins it knocked
down, by D_8012CDE0_gacode: 1 and 3 add 10 or 20 to the previous score plus the third ball (a strike
counts 10 more), 2 adds 10 plus the second and third balls (20 when they make a spare). The third ball
is stored in D_800994B8 (-1 for a strike or spare) and the player's game ends (D_800994BE).
================
*/
void func_801219CC_gacode( u8 player ) {
    if ( D_8012CDE0_gacode[player] == 1 || D_8012CDE0_gacode[player] == 3 ) {
        if ( D_8012D8DA_gacode == 10 ) {
            D_800994B8[player] = -1;
            if ( D_8012CDE0_gacode[player] == 1 ) {
                D_800DF630[player][D_8009D19C[player]] = D_800DF630[player][D_8009D19C[player] - 1] + 20;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
                D_80068A50[player]++;
                D_800994BE[player] = 1;
            }
            if ( D_8012CDE0_gacode[player] == 3 ) {
                D_800DF630[player][D_8009D19C[player]] = D_800DF630[player][D_8009D19C[player] - 1] + 30;
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
                D_80068A50[player]++;
                D_800994BE[player] = 1;
            }
        } else {
            D_800994B8[player] = D_8012D8DA_gacode;
            if ( D_8012CDE0_gacode[player] == 1 ) {
                D_800DF630[player][D_8009D19C[player]] =
                    10 + D_800994B8[player] + D_800DF630[player][D_8009D19C[player] - 1];
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
                D_80068A50[player]++;
                D_800994BE[player] = 1;
            }
            if ( D_8012CDE0_gacode[player] == 3 ) {
                D_800DF630[player][D_8009D19C[player]] =
                    20 + D_800994B8[player] + D_800DF630[player][D_8009D19C[player] - 1];
                D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
                D_80068A50[player]++;
                D_800994BE[player] = 1;
            }
        }
    }
    if ( D_8012CDE0_gacode[player] == 2 ) {
        if ( D_800A3020[player][D_8009D19C[player]] + D_8012D8DA_gacode == 10 ) {
            D_800994B8[player] = -1;
            D_800DF630[player][D_8009D19C[player]] = D_800DF630[player][D_8009D19C[player] - 1] + 20;
            D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
            D_80068A50[player]++;
            D_800994BE[player] = 1;
        } else {
            D_800994B8[player] = D_8012D8DA_gacode;
            D_800DF630[player][D_8009D19C[player]] =
                10 + D_800A3020[player][D_8009D19C[player]] + D_800994B8[player] +
                D_800DF630[player][D_8009D19C[player] - 1];
            D_800EA090[player] = D_800DF630[player][D_8009D19C[player]];
            D_80068A50[player]++;
            D_800994BE[player] = 1;
        }
    }
}

/*
================
func_80121E34_gacode

Clears every player's scores, results and counters for a new game.
================
*/
void func_80121E34_gacode( void ) {
    u8 player;
    u8 frame;

    for ( player = 0; player < 4; player++ ) {
        for ( frame = 0; frame < 10; frame++ ) {
            D_80066E18[player][frame] = 0;
            D_800A3020[player][frame] = 0;
            D_80099458[player][frame] = 0;
            D_800DF630[player][frame] = 0;
        }
        for ( frame = 0; frame < 12; frame++ ) {
            D_800A3050[player][frame] = 0;
            D_800F7C18[player][frame] = 0;
        }
        D_800DF6B4[player] = 0;
        D_8009D088[player] = 0;
        D_8009CF80[player] = 0;
        D_800EA090[player] = 0;
        D_800994B8[player] = 0;
        D_8009D19C[player] = 0;
        D_80068A50[player] = 0;
        D_8012CDE0_gacode[player] = 0;
        D_800994BE[player] = 0;
        D_800A304C[player] = 0;
        D_80066C00.unk_20[player] = 0;
    }
    D_8012D8DA_gacode = 0;
    D_8012D651_gacode = 0;
    D_8005C164 = 1;
}

/*
================
func_80121FD0_gacode
================
*/
void func_80121FD0_gacode( u8 arg0, u8 arg1 ) {
    D_8012D651_gacode = arg0;
    D_8012D8DA_gacode = arg1;
    switch ( D_80068A50[arg0] ) {
    case 0:
        func_80120DD8_gacode( arg0 );
        break;
    case 1:
        func_801213B4_gacode( arg0 );
        break;
    case 2:
        func_801219CC_gacode( arg0 );
        break;
    }
}

/*
================
func_80122060_gacode

Ends the player's frame: with done set, marks the frame's two results -1 and moves to the next frame (and
returns 1); else takes one from D_80068A50. The player is finished after frame 10 or when D_80068A50
reaches 0.
================
*/
s32 func_80122060_gacode( u8 player, s32 done ) {
    u8 frame = D_8009D19C[player];
    s32 advanced = 0;

    if ( done ) {
        D_80066E18[player][frame] = D_800A3020[player][frame] = -1;
        D_8009D19C[player]++;
        advanced = 1;
    } else {
        D_80068A50[player]--;
    }
    if ( D_8009D19C[player] == 10 || D_80068A50[player] == 0 ) {
        D_800994BE[player] = 1;
    }
    return advanced;
}

/*
================
func_80122140_gacode

Counts a throw of the player's frame; once forced, or at D_80099458 + 5 throws, scores the frame against
D_80099458 and moves on (returns 1 then). The player is finished after frame 9.
================
*/
s32 func_80122140_gacode( u8 player, s32 force ) {
    u8 frame;
    s8 throws;
    s32 advanced = 0;

    frame = D_8009D19C[player];
    throws = ++D_80066E18[player][frame];

    if ( force || throws >= D_80099458[0][frame] + 5 ) {
        D_800EA090[player] += D_800DF630[player][frame] = throws - D_80099458[0][frame];
        D_8009D19C[player]++;
        advanced = 1;
    }
    if ( D_8009D19C[player] == 9 ) {
        D_800994BE[player] = 1;
    }
    return advanced;
}

/*
================
func_8012224C_gacode

Decides a frame between players 0 and 1: the lower D_80066E18 result takes 2 and the other 0, a tie gives
both 1; a 2 counts in the player's D_800994B8.
================
*/
void func_8012224C_gacode( u8 frame ) {
    if ( D_80066E18[0][frame] < D_80066E18[1][frame] ) {
        D_800A3020[0][frame] = 2;
        D_800A3020[1][frame] = 0;
    } else if ( D_80066E18[0][frame] > D_80066E18[1][frame] ) {
        D_800A3020[1][frame] = 2;
        D_800A3020[0][frame] = 0;
    } else {
        D_800A3020[1][frame] = 1;
        D_800A3020[0][frame] = 1;
    }
    D_800994B8[0] += D_800A3020[0][frame] == 2 ? 1 : 0;
    D_800994B8[1] += D_800A3020[1][frame] == 2 ? 1 : 0;
}
