/* 4C560.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "gacode/gacode.h"

/*
================
func_80122340_gacode

Counts a frame; in mode 3, gives resources 1673 and 1674 the D_8009CFB0 value in their byte 7.
Declared inline: retail repeats its body in each caller below, and still has this out-of-line copy, which
nothing calls (gcc 2.7 emits an extern inline function).
================
*/
inline void func_80122340_gacode( void ) {
    D_8012C924_gacode++;
    if ( ( D_8005C160 & 0xF ) == 3 ) {
        ( (unkImage_t *)D_800F80B0[1673].data )->unk_07 = D_8009CFB0;
        ( (unkImage_t *)D_800F80B0[1674].data )->unk_07 = D_8009CFB0;
    }
}

/*
================
func_8012239C_gacode

With set nonzero, gives each player's scene object (140 on) the flag bits of the low nibble of its
D_8005C0E4 entry.
================
*/
void func_8012239C_gacode( s32 set ) {
    s32 i;

    if ( !set ) {
        return;
    }
    for ( i = 0; i < D_8005C161; i++ ) {
        switch ( D_8005C0E4[i] & 0xF ) {
        case 1:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1400;
            break;
        case 7:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1C00;
            break;
        case 0:
        case 2:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1000;
            break;
        case 4:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1E00;
            break;
        case 3:
        case 6:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x3000;
            break;
        case 5:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x2800;
            break;
        }
    }
}

/*
================
func_80122470_gacode
================
*/
void func_80122470_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 0 );
    }
    func_80122340_gacode();
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_801224F8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80122788_gacode);

/*
================
func_80122BD8_gacode
================
*/
void func_80122BD8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 1 );
    }
    func_80122340_gacode();
}

/*
================
func_80122C60_gacode
================
*/
void func_80122C60_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 0 );
    }
    func_80122340_gacode();
}

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80122CE8_gacode);

INCLUDE_ASM("asm/nonmatchings/gacode/4C560", func_80123004_gacode);

/*
================
func_801233A8_gacode
================
*/
void func_801233A8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        func_8012239C_gacode( 1 );
    }
    func_80122340_gacode();
}

/*
================
func_80123430_gacode

Loads the image files of D_8012C9A0_gacode[index] and points the palettes of the first four parts' second
files of the lane's scene object (100 or 120) at them; the first part takes file 1, or file 0 when alt is set.
================
*/
void func_80123430_gacode( s32 lane, s32 alt, s32 index ) {
    sceneObject_t *obj;

    if ( lane == 0 ) {
        obj = D_800EA120[100];
    } else {
        obj = D_800EA120[120];
    }
    func_80030C04( D_8012C9A0_gacode[index][2], FILE_RAW );
    func_80030C04( D_8012C9A0_gacode[index][3], FILE_RAW );
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[1].unk_02].data )->unk_30 =
        ( (unkImage_t *)D_800F80B0[D_8012C9A0_gacode[index][3]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[2].unk_02].data )->unk_30 =
        ( (unkImage_t *)D_800F80B0[D_8012C9A0_gacode[index][2]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[3].unk_02].data )->unk_30 =
        ( (unkImage_t *)D_800F80B0[D_8012C9A0_gacode[index][2]].data )->palette;
    if ( alt == 0 ) {
        func_80030C04( D_8012C9A0_gacode[index][1], FILE_RAW );
        ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[0].unk_02].data )->unk_30 =
            ( (unkImage_t *)D_800F80B0[D_8012C9A0_gacode[index][1]].data )->palette;
    } else {
        func_80030C04( D_8012C9A0_gacode[index][0], FILE_RAW );
        ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[0].unk_02].data )->unk_30 =
            ( (unkImage_t *)D_800F80B0[D_8012C9A0_gacode[index][0]].data )->palette;
    }
}
