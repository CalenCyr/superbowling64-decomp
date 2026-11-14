/* 59490.c -- game-select overlay: the menu of the five game modes */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
func_801128A0_gscode

Frame of the first menu, five choices laid out as two rows of two around one in the middle (choice 4):
the pad moves the cursor between neighbours and A or Z takes the highlighted choice, which sets the
game mode in the low nibble of D_8005C160. There is no backing out. Once the menu has slid in it
resets the game setup to one player with no characters. Returns 0 while the menu runs and 1 once it
has slid out after a choice.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail's frame is 0x60, this draft's 0x30 (0x30 bytes of unused stack, likely an inlined helper's locals); cases 1 to 3 of the first switch share one tail in retail. Banking it needs symbol_addrs entries D_8011B5B0_gscode size:0x78 and D_8011B664_gscode size:0x3C
s32 func_801128A0_gscode( void ) {
    s32 i;
    u8 choice;

    if ( D_8011B6A1_gscode == CHOICE_RUN && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
        func_80037870( SE_DECIDE, 0 );
        switch ( D_8011B6A2_gscode ) {
        case 0:
            D_8011B6A1_gscode = CHOICE_TAKEN;
            D_8005C160 = D_8005C160 & 0xF0;
            break;
        case 1:
            D_8005C160 = 1;
            D_8011B6A1_gscode = CHOICE_TAKEN;
            break;
        case 2:
            D_8005C160 = 2;
            D_8011B6A1_gscode = CHOICE_TAKEN;
            break;
        case 3:
            D_8005C160 = 3;
            D_8011B6A1_gscode = CHOICE_TAKEN;
            break;
        case 4:
            D_8011B6A1_gscode = CHOICE_TAKEN;
            D_8005C160 = ( D_8005C160 & 0xF0 ) | 4;
            break;
        }
    }

    func_8003068C( 0xE0, 0xE0, 0xE0 );
    func_8002DF3C( 0xA58, 0, 0 );
    func_800386E0( D_8011B5B0_gscode, D_8011B6A0_gscode );
    for ( i = 0; i < 5; i++ ) {
        func_8003893C( D_8011B628_gscode, i + 1, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }

    if ( D_8011B6A1_gscode == CHOICE_RUN ) {
        if ( ++D_8011B6A3_gscode == 30 ) {
            D_8011B6A3_gscode = 0;
        }
        choice = D_8011B6A2_gscode;
        switch ( choice ) {
        case 0:
            func_8003893C( D_8011B628_gscode, 1, D_8011B664_gscode[1][D_8011B6A3_gscode / 5], MENU_NONE, MENU_NONE, 0xFF );
            func_8002C950( 0x38, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                D_8011B6A2_gscode = 1;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B6A2_gscode = 2;
            }
            break;
        case 1:
            func_8003893C( D_8011B628_gscode, 2, D_8011B664_gscode[2][D_8011B6A3_gscode / 5], MENU_NONE, MENU_NONE, 0xFF );
            func_8002C950( 0xA8, 0x30, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B6A2_gscode = 0;
            }
            if ( D_800E6758 & D_JPAD ) {
                D_8011B6A2_gscode = 3;
            }
            break;
        case 2:
            func_8003893C( D_8011B628_gscode, 3, D_8011B664_gscode[3][D_8011B6A3_gscode / 5], MENU_NONE, MENU_NONE, 0xFF );
            func_8002C950( 0x18, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & R_JPAD ) {
                D_8011B6A2_gscode = 4;
            }
            if ( D_800E6758 & U_JPAD ) {
                D_8011B6A2_gscode = 0;
            }
            break;
        case 3:
            func_8003893C( D_8011B628_gscode, 5, D_8011B664_gscode[4][D_8011B6A3_gscode / 5], MENU_NONE, MENU_NONE, 0xFF );
            func_8002C950( 0xC8, 0x98, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B6A2_gscode = 4;
            }
            if ( D_800E6758 & U_JPAD ) {
                D_8011B6A2_gscode = 1;
            }
            break;
        case 4:
            func_8003893C( D_8011B628_gscode, 4, D_8011B664_gscode[0][D_8011B6A3_gscode / 5], MENU_NONE, MENU_NONE, 0xFF );
            func_8002C950( 0x70, 0x70, 0x60, 0x48, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_800E6758 & L_JPAD ) {
                D_8011B6A2_gscode = 2;
            } else if ( D_800E6758 & R_JPAD ) {
                D_8011B6A2_gscode = 3;
            } else if ( D_800E6758 & U_JPAD ) {
                D_8011B6A2_gscode = 0;
            } else if ( D_800E6758 & D_JPAD ) {
                D_8011B6A2_gscode = 3;
            }
            break;
        }
        if ( D_8011B6A2_gscode != choice ) {
            D_8011B6A3_gscode = 0;
            func_80026B28();
            func_80037870( SE_CURSOR, SE_NO_REUSE );
        }
    }

    func_800386E0( D_8011B628_gscode, D_8011B6A0_gscode );
    switch ( D_8011B6A1_gscode ) {
    case CHOICE_IN:
        D_8011B5B0_gscode[5].slide = SLIDE_LEFT;
        D_8011B6A0_gscode -= 2;
        if ( D_8011B6A0_gscode == 0 ) {
            D_8011B6A1_gscode = CHOICE_RUN;
            func_80026B28();
            D_8005C160 = 0;
            D_8005C161 = 1;
            for ( i = 3; i >= 0; i-- ) {
                D_8005C0E4[i] = 0;
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011B5B0_gscode[5].slide = SLIDE_RIGHT;
        D_8011B6A0_gscode += 2;
        if ( D_8011B6A0_gscode == 30 ) {
            if ( D_8011B6A1_gscode == CHOICE_TAKEN ) {
                D_8011B6A1_gscode = CHOICE_IN;
                return 1;
            }
            D_8011B6A1_gscode = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/59490", func_801128A0_gscode);
#endif
