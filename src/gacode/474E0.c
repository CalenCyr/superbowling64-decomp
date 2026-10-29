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
================
*/
f32 func_8011D5B8_gacode( vec3_t a, vec3_t b ) {
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
================
*/
void func_8011D6C4_gacode( vec3_t out, vec3_t a, vec3_t b ) {
    vec3_t cross;

    cross[0] = a[1] * b[2] - a[2] * b[1];
    cross[1] = a[2] * b[0] - a[0] * b[2];
    cross[2] = a[0] * b[1] - a[1] * b[0];
    func_8011D424_gacode( out, cross );
}

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D7CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D9CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DB54_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DCB8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E014_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E12C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E504_gacode);

/*
================
func_8011E63C_gacode

Moves *speed one step toward limit's side; once *value reaches limit (from below for a positive limit,
from above for a negative one) clamps it there, stops *speed and returns 1.
================
*/
s32 func_8011E63C_gacode( f32 *value, f32 *speed, f32 limit ) {
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E6F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F334_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F4CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F87C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011FD08_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8012016C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801203AC_gacode);

/*
================
func_801205A0_gacode
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: same operations, but retail evaluates dx before dz and the products of the quotient in another
// order (float register allocation and schedule differ throughout)
f32 func_801205A0_gacode( vec3_t pos, vec3_t vel, f32 x, f32 scale ) {
    f32 dx = x - pos[0];
    f32 dz = pos[2] - 730.0f;
    f32 width = 730.0f - D_8012D220_gacode->unk_08;
    f32 vx;
    f32 vz;

    if ( dz < 0.0f ) {
        dz = 0.0f - dz - width;
    } else {
        dz = dz - width;
    }
    vz = scale * vel[2];
    vx = scale * vel[0];
    return ( vz * vz * 2 * dx - vx * 2 * vz * ( width + dz ) )
         / ( width * width + width * 2 * dz + dz * dz * D_8012D220_gacode->unk_10 ) * 15.0 / D_8012D220_gacode->unk_0C;
}
#else
INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801205A0_gacode);
#endif

/*
================
func_80120680_gacode

Binary search of the count sorted entries of table for key; its index, or -1.
================
*/
s32 func_80120680_gacode( u16 key, s32 count, u16 *table ) {
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801206E0_gacode);

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
================
*/
void func_80120B50_gacode( gaSprite_t *sprite, u16 color, u16 index ) {
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120C0C_gacode);

/*
================
func_80120D30_gacode

Starts sound id for a lane; returns its handle.
================
*/
u64 func_80120D30_gacode( s32 lane, s32 id ) {
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120DD8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801213B4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801219CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80121E34_gacode);

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

    if ( force || throws >= D_80099458[frame] + 5 ) {
        D_800EA090[player] += D_800DF630[player][frame] = throws - D_80099458[frame];
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
