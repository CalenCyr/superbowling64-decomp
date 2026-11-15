/* 605D0.c -- game-select overlay: the options screen, its records page and the credits */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"
#include "dbgtext.h"

/* ===== func_801199E0_gscode ===== */

#ifdef NON_MATCHING
// NON_MATCHING: i gets s0 where retail has s1 (and s0/s1 swap in every loop); the sound test loads
// D_80120427_gscode before saving D_8009D098.unk_79 (retail after); the credits' clear loop keeps its counter
// where retail counts down the byte offset alone
/*
 * One frame of the options screen: its settings, the sound test, the records page and the credits, which the
 * script D_8011FD58_gscode drives. Returns 1 when the screen slid out, 2 when it was backed out of, else 0.
 */
s32 func_801199E0_gscode( void ) {
    s32 i;
    s16 x;
    s16 y;
    u8 alpha;
    u8 saved;

    if ( D_80120425_gscode == 1 && ( D_800E4D62 & B_BUTTON ) && D_80120428_gscode == 0 && D_8005C0BF == 0 ) {
        func_80037870( 2, 0 );
        D_80120425_gscode = 3;
    }

    func_8003068C( 0, 0, 0 );
    func_8002DF3C( 0xA58, 0, 0 );

    for ( i = 0; i < 13; i++ ) {
        func_8003893C( D_8011FC90_gscode, i, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }

    if ( D_80120425_gscode == 1 && D_8005C0BF == 0 ) {
        func_8003893C( D_8011FC90_gscode, 10, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( D_8011FC90_gscode, 11, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( D_8011FC90_gscode, 1, D_80120404_gscode[D_8009D098.unk_78], MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( D_8011FC90_gscode, 2, D_8012040C_gscode[D_8009D098.unk_79], MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( D_8011FC90_gscode, 12, D_8012040C_gscode[D_8009D098.unk_7B], MENU_NONE, MENU_NONE, 0xFF );
        if ( (u8)( D_80120427_gscode / 10 ) ) {
            func_8003893C( D_8011FC90_gscode, 3, D_80120410_gscode[(u8)( D_80120427_gscode / 10 )], MENU_NONE,
                           MENU_NONE, 0xFF );
        }
        func_8003893C( D_8011FC90_gscode, 4, D_80120410_gscode[(u8)( D_80120427_gscode % 10 )], MENU_NONE,
                       MENU_NONE, 0xFF );
        for ( i = 0; i < 4 - D_8009D098.unk_7A; i++ ) {
            func_8003893C( D_8011FC90_gscode, i + 5, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        }

        if ( D_80120428_gscode == 0
             && ( ( D_800E6758 & U_JPAD ) || ( D_80120426_gscode == OPTION_EXIT && ( D_800E6758 & L_JPAD ) ) ) ) {
            if ( --D_80120426_gscode == 0xFF ) {
                D_80120426_gscode = OPTION_EXIT;
            }
            func_80026B28();
            func_80037870( 0, 0x80 );
        } else if ( D_80120428_gscode == 0
                    && ( ( D_800E6758 & D_JPAD )
                         || ( D_80120426_gscode == OPTION_CREDITS && ( D_800E6758 & R_JPAD ) ) ) ) {
            if ( ++D_80120426_gscode == 8 ) {
                D_80120426_gscode = 0;
            }
            func_80026B28();
            func_80037870( 0, 0x80 );
        }

        switch ( D_80120426_gscode ) {
        case OPTION_SOUND:
            if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || ( D_800E6758 & R_JPAD ) ) {
                if ( ++D_8009D098.unk_78 == 3 ) {
                    D_8009D098.unk_78 = 0;
                }
                func_80037870( 0, 0x80 );
            }
            if ( D_800E6758 & L_JPAD ) {
                if ( --D_8009D098.unk_78 == 0xFF ) {
                    D_8009D098.unk_78 = 2;
                }
                func_80037870( 0, 0x80 );
            }
            break;
        case OPTION_79:
            if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || ( D_800E6758 & ( L_JPAD | R_JPAD ) ) ) {
                if ( ++D_8009D098.unk_79 == 2 ) {
                    D_8009D098.unk_79 = 0;
                }
                func_80037870( 0, 0x80 );
            }
            break;
        case OPTION_SOUND_TEST:
            D_80120400_gscode++;
            if ( D_800E6758 & R_JPAD ) {
                if ( ++D_80120427_gscode == NUM_SOUND_TESTS ) {
                    D_80120427_gscode = 0;
                }
                func_80037870( 0, 0x80 );
            }
            if ( D_800E6758 & L_JPAD ) {
                if ( --D_80120427_gscode == 0xFF ) {
                    D_80120427_gscode = NUM_SOUND_TESTS - 1;
                }
                func_80037870( 0, 0x80 );
            }
            if ( D_80120400_gscode >= 16 && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
                D_80120400_gscode = 0;
                func_80037870( 1, 0x80 );
                saved = D_8009D098.unk_79;
                D_8009D098.unk_79 = 1;
                func_800379C8( D_80120427_gscode );
                D_8009D098.unk_79 = saved;
            }
            break;
        case OPTION_BGM_VOLUME:
            if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || ( D_800E6758 & R_JPAD ) ) {
                if ( ++D_8009D098.unk_7A == 5 ) {
                    D_8009D098.unk_7A = 0;
                }
                func_80037B84();
                func_80037870( 0, 0x80 );
            }
            if ( D_800E6758 & L_JPAD ) {
                if ( --D_8009D098.unk_7A == 0xFF ) {
                    D_8009D098.unk_7A = 4;
                }
                func_80037B84();
                func_80037870( 0, 0x80 );
            }
            break;
        case OPTION_7B:
            if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || ( D_800E6758 & ( L_JPAD | R_JPAD ) ) ) {
                if ( ++D_8009D098.unk_7B == 2 ) {
                    D_8009D098.unk_7B = 0;
                }
                func_80037870( 0, 0x80 );
            }
            break;
        case OPTION_RECORDS:
            if ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
                func_80037870( 1, 0 );
                D_80120428_gscode ^= 1;
            }
            break;
        case OPTION_CREDITS:
            if ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
                func_80037870( 1, 0 );
                D_8005C0BF = 1;
            }
            break;
        case OPTION_EXIT:
            break;
        }

        if ( D_80120426_gscode < OPTION_EXIT ) {
            func_8003893C( D_8011FC90_gscode, 0, 0x8F0, 0x49, D_80120426_gscode * 20 + 0x46, D_8009CFB0 );
        } else {
            func_8003893C( D_8011FC90_gscode, 0, 0x8EF, 0xD0, 0xB0, D_8009CFB0 );
        }
    }

    if ( D_8005C0BF == 1 && D_8005C0C0 == 0xFF ) {
        D_8005C0BF = 2;
        D_8005C0C0 = 0x107;
        if ( D_80120428_gscode == 0 ) {
            func_800379C8( 0x13 );
            D_80120428_gscode = 2;
            D_80120A18_gscode = 1;
            D_80120A1A_gscode = 0;
            for ( i = NUM_CREDIT_SPRITES - 1; i >= 0; i-- ) {
                D_80120860_gscode[i].image = -1;
            }
        } else {
            func_800379C8( 1 );
            D_80120428_gscode = 0;
        }
    }

    if ( D_80120428_gscode != 2 ) {
        func_800386E0( D_8011FC90_gscode, D_80120424_gscode );
        if ( D_80120425_gscode == 1 ) {
            for ( i = 0; i < 5; i++ ) {
                if ( D_80120426_gscode != i ) {
                    func_8002FDE4( 0xB0, i * 20 + 0x46, 0x46, 0x12, 0, 0, 0, 0x80 );
                }
            }
            if ( D_80120426_gscode == OPTION_EXIT && ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) ) {
                func_80037870( 1, 0 );
                D_80120425_gscode = 3;
            }
            if ( D_80120428_gscode == 1 ) {
                for ( i = 0; i < 4; i++ ) {
                    func_8003B034( &D_8009D098.unk_08[i], i, 0 );
                }
                func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                func_80028EA8( &D_800F8094 );
                func_8003A970( 4, D_8009D098.unk_08, 0, 0x18, 0x38 );
            }
        }
    } else {
        D_80120426_gscode = OPTION_CREDITS;
        if ( --D_80120A18_gscode == 0 ) {
            do {
                switch ( D_8011FD58_gscode[D_80120A1A_gscode].op ) {
                case CREDIT_SHOW:
                    i = D_8011FD58_gscode[D_80120A1A_gscode].sprite;
                    D_80120860_gscode[i].image = D_8011FD58_gscode[D_80120A1A_gscode].a;
                    D_80120860_gscode[i].x = D_80120860_gscode[i].targetX = D_8011FD58_gscode[D_80120A1A_gscode].b;
                    D_80120860_gscode[i].y = D_80120860_gscode[i].targetY = D_8011FD58_gscode[D_80120A1A_gscode].c;
                    D_80120860_gscode[i].alpha = D_80120860_gscode[i].targetAlpha
                        = D_8011FD58_gscode[D_80120A1A_gscode].alpha;
                    D_80120860_gscode[i].moveFrames = D_80120860_gscode[i].moveFrame = 0;
                    D_80120860_gscode[i].fadeFrames = D_80120860_gscode[i].fadeFrame = 0;
                    break;
                case CREDIT_MOVE:
                    i = D_8011FD58_gscode[D_80120A1A_gscode].sprite;
                    D_80120860_gscode[i].targetX = D_8011FD58_gscode[D_80120A1A_gscode].a;
                    D_80120860_gscode[i].targetY = D_8011FD58_gscode[D_80120A1A_gscode].b;
                    D_80120860_gscode[i].moveFrames = D_8011FD58_gscode[D_80120A1A_gscode].c;
                    D_80120860_gscode[i].moveFrame = 0;
                    break;
                case CREDIT_FADE:
                    i = D_8011FD58_gscode[D_80120A1A_gscode].sprite;
                    D_80120860_gscode[i].targetAlpha = D_8011FD58_gscode[D_80120A1A_gscode].a;
                    D_80120860_gscode[i].fadeFrames = D_8011FD58_gscode[D_80120A1A_gscode].b;
                    D_80120860_gscode[i].fadeFrame = 0;
                    break;
                case CREDIT_WAIT:
                    D_80120A18_gscode = D_8011FD58_gscode[D_80120A1A_gscode].sprite;
                    break;
                case CREDIT_END:
                    D_80120A18_gscode = 0xFFFF;
                    D_8005C0BF = 1;
                    break;
                }
                D_80120A1A_gscode++;
            } while ( D_80120A18_gscode == 0 );
        }

        for ( i = 0; i < NUM_CREDIT_SPRITES; i++ ) {
            if ( D_80120860_gscode[i].image == -1 ) {
                continue;
            }
            if ( D_80120860_gscode[i].moveFrame < D_80120860_gscode[i].moveFrames ) {
                D_80120860_gscode[i].moveFrame++;
                if ( D_80120860_gscode[i].moveFrames == D_80120860_gscode[i].moveFrame ) {
                    D_80120860_gscode[i].moveFrames = 0;
                    D_80120860_gscode[i].x = D_80120860_gscode[i].targetX;
                    D_80120860_gscode[i].y = D_80120860_gscode[i].targetY;
                }
            }
            if ( D_80120860_gscode[i].fadeFrame < D_80120860_gscode[i].fadeFrames ) {
                D_80120860_gscode[i].fadeFrame++;
                if ( D_80120860_gscode[i].fadeFrames == D_80120860_gscode[i].fadeFrame ) {
                    D_80120860_gscode[i].fadeFrames = 0;
                    D_80120860_gscode[i].alpha = D_80120860_gscode[i].targetAlpha;
                }
            }
            if ( D_80120860_gscode[i].moveFrames != 0 ) {
                x = D_80120860_gscode[i].targetX
                    - ( D_80120860_gscode[i].targetX - D_80120860_gscode[i].x )
                          * (f32)( D_80120860_gscode[i].moveFrames - D_80120860_gscode[i].moveFrame )
                          / D_80120860_gscode[i].moveFrames;
                y = D_80120860_gscode[i].targetY
                    - ( D_80120860_gscode[i].targetY - D_80120860_gscode[i].y )
                          * (f32)( D_80120860_gscode[i].moveFrames - D_80120860_gscode[i].moveFrame )
                          / D_80120860_gscode[i].moveFrames;
            } else {
                x = D_80120860_gscode[i].x;
                y = D_80120860_gscode[i].y;
            }
            if ( D_80120860_gscode[i].fadeFrames != 0 ) {
                alpha = D_80120860_gscode[i].targetAlpha
                        - ( D_80120860_gscode[i].targetAlpha - D_80120860_gscode[i].alpha )
                              * (f32)( D_80120860_gscode[i].fadeFrames - D_80120860_gscode[i].fadeFrame )
                              / D_80120860_gscode[i].fadeFrames;
            } else {
                alpha = D_80120860_gscode[i].alpha;
            }
            func_8002E4B0( D_80120860_gscode[i].image, x, y, alpha, 1.0f, 1.0f );
        }
    }

    switch ( D_80120425_gscode ) {
    case 0:
        D_8011FC90_gscode[5].slide = SLIDE_LEFT;
        D_80120424_gscode -= 2;
        if ( D_80120424_gscode == 0 ) {
            D_80120425_gscode = 1;
            func_80026B28();
        }
        break;
    case 2:
    case 3:
        D_8011FC90_gscode[5].slide = SLIDE_RIGHT;
        D_80120424_gscode += 2;
        if ( D_80120424_gscode == 30 ) {
            if ( D_8005C9E8 != 1 ) {
                func_800379C8( 1 );
            }
            if ( D_80120425_gscode == 2 ) {
                D_80120425_gscode = 0;
                return 1;
            }
            D_80120425_gscode = 0;
            return 2;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/605D0", func_801199E0_gscode);
#endif
