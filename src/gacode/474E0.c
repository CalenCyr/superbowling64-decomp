/* 474E0.c -- game overlay */

#include "common.h"
#include "libultra.h"
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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D304_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_8011D424_gacode);

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

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120A7C_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/474E0", func_80120B10_gacode);

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
