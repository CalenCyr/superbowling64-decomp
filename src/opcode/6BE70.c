/* 6BE70.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "opcode/opcode.h"

/*
================
func_80112350_opcode

Entry of the overlay: in mode 0xA, Start held selects mode 0xFE, and no controller in port 1 mode 0xFD.
================
*/
void func_80112350_opcode( void ) {
    D_80114350_opcode = 0;
    if ( D_8005C0E8 == 10 ) {
        if ( D_800994BC & START_BUTTON ) {
            D_8005C0E8 = 0xFE;
        }
        if ( D_8005B194 == 1 ) {
            D_8005C0E8 = 0xFD;
        }
    }
}

/*
================
func_801123A8_opcode

The opening's modes: two logo screens, the title with its demo games, then func_80112958_opcode's and
func_80113168_opcode's screens. Each logo and the title count frames in D_80114350_opcode, then ask to leave.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: in mode 1's leave block the scheduler places `li a0,2` (D_8005C0BF = 2) after the D_8005C0C0 store; retail hoists it under the D_8005C0F0 load
// RULE-EXCEPTION: s32 with no return value: retail keeps v0 live at the exit
s32 func_801123A8_opcode( void ) {
    s32 i;

    switch ( D_8005C0E8 ) {
    case 10:
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0xB58, 0, 0 );
        if ( ++D_80114350_opcode >= 150 ) {
            D_8005C0BF = 1;
            if ( D_8005C0C0 == 0xFF ) {
                D_8005C0C0 = 0x107;
                D_8005C0BF = 2;
                D_8005C0E8 = 0;
                D_80114350_opcode = 0;
                func_80037870( 0x43, 0 );
            }
        }
        break;
    case 0:
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0x925, 0x3C, 0x58 );
        if ( ++D_80114350_opcode >= 70 ) {
            D_8005C0BF = 1;
            if ( D_8005C0C0 == 0xFF ) {
                D_8005C0C0 = 0x107;
                D_8005C0BF = 2;
                D_8005C0E8 = 1;
                D_80114350_opcode = 0;
            }
        }
        break;
    case 1:
        if ( D_80114350_opcode == 6 ) {
            func_800379C8( 0 );
        }
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0xADD, 0, 0 );
        func_8002E4B0( 0x890, 0x44, 0xA0, D_8009CFB0, 1.0f, 1.0f );
        if ( ++D_80114350_opcode >= 450 ) {
            D_8005C0BF = 1;
            if ( D_8005C0C0 == 0xFF ) {
                D_8005C0C0 = 0x107;
                D_800693B0 = 1200;
                D_8005C0B6 = 0;
                D_8005C0BF = 2;
                D_8005C0EC = 1;
                D_8009D098.unk_79 = 0;
                if ( ++D_8005C0F0 == 4 ) {
                    D_8005C0F0 = 0;
                }
                D_8005C160 = 0;
                switch ( D_8005C0F0 ) {
                case 0:
                    D_8005C162 = guRandom() % 2;
                    D_8005C161 = D_8005C162 + 1;
                    break;
                case 1:
                    D_8005C160 = 0x10;
                    D_8005C162 = guRandom() % 2;
                    D_8005C161 = D_8005C162 + 1;
                    break;
                case 2:
                    D_8005C160 = 0x20;
                    D_8005C162 = guRandom() % 2;
                    D_8005C161 = D_8005C162 + 1;
                    break;
                case 3:
                    D_8005C160 = D_8005C0F0;   /* demo 3 plays game mode 3 */
                    D_8005C162 = 1;
                    D_8005C161 = 2;
                    break;
                }
                D_8005C163 = guRandom() % 80 / 10;
                for ( i = 0; i < D_8005C161; i++ ) {
                    D_8005C0E4[i] = ( D_8005C0F4++ & 7 ) | 0x80;
                }
                for ( i = 0; i < D_8005C161; i++ ) {
                    if ( ( D_8005C0E4[i] >> 7 ) || !( ( D_8005C0E4[i] >> 4 ) & 7 ) ) {
                        D_80068218[i] = D_8005C0F8[D_8005C0E4[i] & 0xF];
                    }
                }
            }
        }
        if ( ( D_800E44E0[0] & START_BUTTON ) && !D_8005C0BF ) {
            func_80037870( 1, 0 );
            D_8005C0E8 = 2;
            D_80114350_opcode = 0;
        }
        break;
    case 2:
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0xADD, 0, 0 );
        if ( !func_80112958_opcode() ) {
            D_8005C0BF = 1;
            D_8005C0E8 = 3;
        }
        break;
    case 3:
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0xADD, 0, 0 );
        if ( D_8005C0C0 == 0xFF ) {
            D_8005C0C0 = 0x107;
            D_8005C0BF = 2;
            D_8005C0B6 = 4;
        }
        break;
    case 0xFD:
        func_8003068C( 0xE0, 0xE0, 0xE0 );
        func_8002DF3C( 0xB5A, 0x48, 0x60 );
        break;
    case 0xFE:
        if ( !func_80113168_opcode() ) {
            D_8005C0E8 = 10;
        }
        break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/opcode/6BE70", func_801123A8_opcode);
#endif

