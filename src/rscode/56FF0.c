/* 56FF0.c -- results overlay */

#include "common.h"
#include "libultra.h"
#include "rscode/rscode.h"

/*
================
func_80112350_rscode

Entry of the overlay: starts the first state and the mode's results.
================
*/
void func_80112350_rscode( void ) {
    func_80037B18();
    D_80114250_rscode = 0;
    D_80114252_rscode = 0;
    D_80114253_rscode = 0;
    D_801142AC_rscode = func_80112B60_rscode;
    D_801142A8_rscode = D_8005C0B6;
    func_80112574_rscode();
}

/*
================
func_801123A8_rscode

Frame of the overlay: reads the input, draws the background, runs the current state and pulses the boxes'
alpha.
================
*/
void func_801123A8_rscode( void ) {
    if ( !D_8005C0BF && D_80114250_rscode ) {
        D_8005C0BF = 1;
        func_80037870( 50, 0 );
    }
    if ( D_8005C0BF == 1 && D_8005C0C0 == 255 ) {
        D_8005C0BF = 2;
        D_8005C0C0 = 263;
        D_8005C0B6 = D_801142A8_rscode;
    }
    D_801142A6_rscode = D_8005C0BF ? 0 : D_800E6758;
    D_801142A0_rscode = D_8005C0BF ? 0 : D_800E4D62;
    func_80028EA8( &D_800F8094 );
    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    D_801142AC_rscode();
    if ( D_80114253_rscode == 1 ) {
        D_80114252_rscode -= 8;
        if ( D_80114252_rscode <= 0 ) {
            D_80114252_rscode = 0;
            D_80114253_rscode = 0;
        }
    } else {
        D_80114252_rscode += 8;
        if ( D_80114252_rscode >= 0x80 ) {
            D_80114252_rscode = 0x80;
            D_80114253_rscode = 1;
        }
    }
    func_80028A0C( &D_800F8094 );
    func_80028950( &D_800F8094, 0 );
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
}

/*
================
func_80112574_rscode

Picks the first state and the BGM for the game mode.
================
*/
void func_80112574_rscode( void ) {
    switch ( D_8005C160 & 0xF ) {
    case 0:
        func_80112674_rscode();
        func_800379C8( 10 );
        break;
    case 1:
        D_801142AC_rscode = func_8011311C_rscode;
        func_800379C8( D_800EA090[0] <= D_800EA090[1] ? 15 : 14 );
        func_80037870( D_800EA090[0] <= D_800EA090[1] ? 0x6D : 0x6C, 0 );
        break;
    case 2:
        break;
    case 3:
        D_801142AC_rscode = D_801142B0_rscode = func_80113564_rscode;
        func_800379C8( 18 );
        break;
    }
}

/*
================
func_80112674_rscode

Picks the state of mode 0's results by its sub-mode; a perfect game (bit 0x200 of D_8009D098.unk_04) or a
finished game (bit 0x100) first shows its message, once.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: control flow; retail's switch on D_8005C160 >> 6 keeps a case 1 that goes to the end (its
// decision tree tests 1, then < 2, then 0), which an empty case here does not keep
void func_80112674_rscode( void ) {
    s32 found = 0;
    s32 i;

    switch ( ( D_8005C160 >> 4 ) & 3 ) {
    case 0:
        func_801129CC_rscode();
        switch ( D_8005C160 >> 6 ) {
        case 0:
            if ( D_8005C163 ) {
                break;
            }
            for ( i = 0; i < D_8005C161; i++ ) {
                if ( !( D_8005C0E4[i] >> 7 ) && D_800DF630[i][9] == 300 ) {
                    found = 1;
                    break;
                }
            }
            if ( found && !( D_8009D098.unk_04 & 0x200 ) ) {
                func_80037870( 0x35, 0 );
                D_801142AC_rscode = func_8011291C_rscode;
                D_8009D098.unk_04 |= 0x200;
            }
            break;
        case 1:
            break;
        }
        break;
    case 1:
        D_801142AC_rscode = func_80113030_rscode;
        break;
    case 2:
        D_801142AC_rscode = func_801130AC_rscode;
        if ( D_8005C163 ) {
            break;
        }
        for ( i = 0; i < D_8005C161; i++ ) {
            if ( D_8009D19C[i] == 10 ) {
                found = 1;
                break;
            }
        }
        if ( found && !( D_8009D098.unk_04 & 0x100 ) ) {
            func_80037870( 0x35, 0 );
            D_801142AC_rscode = func_8011286C_rscode;
            D_8009D098.unk_04 |= 0x100;
        }
        break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80112674_rscode);
#endif

/*
================
func_8011286C_rscode

Shows message 0x88C in a box for 300 frames or until A or Z, then goes to func_801130AC_rscode.
================
*/
void func_8011286C_rscode( void ) {
    func_8002C950( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, D_80114252_rscode + 0x40 );
    func_8002DF3C( 0x88C, 0x26, 0x50 );
    if ( ++D_80114254_rscode >= 300 || ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_801130AC_rscode;
    }
}

/*
================
func_8011291C_rscode

Shows message 0xAE3 in a box for 300 frames or until A or Z, then goes to func_80112B60_rscode.
================
*/
void func_8011291C_rscode( void ) {
    func_8002C950( 0x26, 0x50, 0xF0, 0x40, 0xE0, 0xE0, 0xE0, D_80114252_rscode + 0x40 );
    func_8002DF3C( 0xAE3, 0x26, 0x50 );
    if ( ++D_80114254_rscode >= 300 || ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80112B60_rscode;
    }
}

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_801129CC_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80112B60_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80112E04_rscode);

/*
================
func_80113030_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_80113030_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_80039CBC( D_8005C160 >> 6, 1, 0x20, 0x28 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80113030_rscode;
    }
}

/*
================
func_801130AC_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_801130AC_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    func_8003A3F0( 1, 0x18, 0x28 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_801130AC_rscode;
    }
}

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_8011311C_rscode);

/*
================
func_80113504_rscode

Results page; A or Z opens the menu of func_80113564_rscode, which comes back here.
================
*/
void func_80113504_rscode( void ) {
    func_8002DF3C( 0x88D, 0x64, 0x10 );
    if ( D_801142A0_rscode & ( A_BUTTON | Z_TRIG ) ) {
        func_80037870( SE_DECIDE, 0 );
        D_801142AC_rscode = func_80113564_rscode;
        D_801142B0_rscode = func_80113504_rscode;
    }
}

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113564_rscode);

/*
================
func_8011372C_rscode
================
*/
void func_8011372C_rscode( void ) {
    s32 choice;

    choice = func_8003B204();
    if ( choice == 2 ) {
        D_801142AC_rscode = func_80113564_rscode;
    } else if ( choice == 1 ) {
        D_801142AC_rscode = func_8011377C_rscode;
    }
}

/*
================
func_8011377C_rscode

Waits for func_80037410, then leaves the overlay.
================
*/
void func_8011377C_rscode( void ) {
    if ( func_80037410() ) {
        return;
    }
    D_80114250_rscode = 1;
    D_801142A8_rscode = 4;
    D_8005CB60 = 0;
    D_801142AC_rscode = func_801137CC_rscode;
}

/*
================
func_801137CC_rscode

Last state: does nothing while the overlay is left.
================
*/
void func_801137CC_rscode( void ) {
}

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_801137D4_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113904_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113DC8_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113EC0_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_80113FCC_rscode);

INCLUDE_ASM("asm/nonmatchings/rscode/56FF0", func_801140EC_rscode);
