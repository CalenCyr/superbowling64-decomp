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
================
*/
void func_8011D3A0_gacode( vec3_t out, vec3_t from, vec3_t to ) {
    out[0] = to[0] - from[0];
    out[1] = to[1] - from[1];
    out[2] = to[2] - from[2];
}

/*
================
func_8011D3D4_gacode

Length of v.
================
*/
f32 func_8011D3D4_gacode( vec3_t v ) {
    return sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

/*
================
func_8011D424_gacode

out = in scaled to length 1, or (0, -1, 0) when in is zero; returns the length of in.
================
*/
f32 func_8011D424_gacode( vec3_t out, vec3_t in ) {
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D4E4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D5B8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D6C4_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D7CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D9CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DB54_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011DCB8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E014_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E12C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E504_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E63C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011E6F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F334_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F4CC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011F87C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011FD08_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8012016C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801203AC_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801205A0_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_801209C0_gacode);

/*
================
func_80120A7C_gacode

Sets up a sprite of resource id at (x, y), keeping a copy of its palette.
================
*/
void func_80120A7C_gacode( gaSprite_t *sprite, u16 id, s16 x, s16 y ) {
    func_8002DF3C( id, 320, 240 );
    if ( D_800F80B0[id].image != NULL ) {
        memmove( sprite->palette, D_800F80B0[id].image->palette, sizeof( sprite->palette ) );
        sprite->id = id;
        sprite->x = x;
        sprite->y = y;
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
    if ( D_800F80B0[sprite->id].image != NULL ) {
        memmove( D_800F80B0[sprite->id].image->palette, sprite->palette, sizeof( sprite->palette ) );
    }
}

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120B50_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120C0C_gacode);

/*
================
func_80120D30_gacode

Starts sound id for a lane.
================
*/
void func_80120D30_gacode( s32 lane, s32 id ) {
    u8 flags;

    if ( D_8005C162 ) {
        flags = lane + 1;
    } else {
        flags = 0;
    }
    if ( id != 0x4E && id != 0x4F ) {
        flags |= 0x80;
    }
    func_80037870( id, flags );
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80122060_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80122140_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8012224C_gacode);
