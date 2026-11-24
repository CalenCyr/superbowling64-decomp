/* 605D0.c -- game-select overlay: the options screen, its records page and the credits */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"
#include "dbgtext.h"

/* ===== GS_Options ===== */

/*
 * One frame of the options screen: its settings, the sound test, the records page and the credits, which the
 * script gs_creditsScript drives. Returns 1 when the screen slid out, 2 when it was backed out of, else 0.
 */
#ifdef NON_MATCHING
// NON_MATCHING: byte-exact (gate green) only with the sound test's saved setting held in the loop counter i
// (marked R30 below); with a separate local the s0/s1 order flips, and a reused counter is not counted as a match
s32 GS_Options( void ) {
    s32 i;
    s16 x;
    s16 y;
    u8 alpha;

    if ( gs_optionsState == 1 && ( D_800E4D62 & B_BUTTON ) && gs_optionsPage == 0 && D_8005C0BF == 0 ) {
        func_80037870( 2, 0 );
        gs_optionsState = 3;
    }

    Draw_ClearScreen( 0, 0, 0 );
    func_8002DF3C( 0xA58, 0, 0 );

    for ( i = 0; i < 13; i++ ) {
        func_8003893C( gs_optionsMenu, i, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }

    if ( gs_optionsState == 1 && D_8005C0BF == 0 ) {
        func_8003893C( gs_optionsMenu, 10, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_optionsMenu, 11, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_optionsMenu, 1, D_80120404_gscode[D_8009D098.unk_78], MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_optionsMenu, 2, D_8012040C_gscode[D_8009D098.unk_79], MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_optionsMenu, 12, D_8012040C_gscode[D_8009D098.unk_7B], MENU_NONE, MENU_NONE, 0xFF );
        if ( (u8)( gs_soundTestSound / 10 ) ) {
            func_8003893C( gs_optionsMenu, 3, D_80120410_gscode[(u8)( gs_soundTestSound / 10 )], MENU_NONE,
                           MENU_NONE, 0xFF );
        }
        func_8003893C( gs_optionsMenu, 4, D_80120410_gscode[(u8)( gs_soundTestSound % 10 )], MENU_NONE,
                       MENU_NONE, 0xFF );
        for ( i = 0; i < 4 - D_8009D098.unk_7A; i++ ) {
            func_8003893C( gs_optionsMenu, i + 5, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        }

        if ( gs_optionsPage == 0
             && ( ( D_800E6758 & U_JPAD ) || ( gs_optionsCursor == OPTION_EXIT && ( D_800E6758 & L_JPAD ) ) ) ) {
            if ( --gs_optionsCursor == 0xFF ) {
                gs_optionsCursor = OPTION_EXIT;
            }
            func_80026B28();
            func_80037870( 0, 0x80 );
        } else if ( gs_optionsPage == 0
                    && ( ( D_800E6758 & D_JPAD )
                         || ( gs_optionsCursor == OPTION_CREDITS && ( D_800E6758 & R_JPAD ) ) ) ) {
            if ( ++gs_optionsCursor == 8 ) {
                gs_optionsCursor = 0;
            }
            func_80026B28();
            func_80037870( 0, 0x80 );
        }

        switch ( gs_optionsCursor ) {
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
            gs_soundTestFrames++;
            if ( D_800E6758 & R_JPAD ) {
                if ( ++gs_soundTestSound == NUM_SOUND_TESTS ) {
                    gs_soundTestSound = 0;
                }
                func_80037870( 0, 0x80 );
            }
            if ( D_800E6758 & L_JPAD ) {
                if ( --gs_soundTestSound == 0xFF ) {
                    gs_soundTestSound = NUM_SOUND_TESTS - 1;
                }
                func_80037870( 0, 0x80 );
            }
            if ( gs_soundTestFrames >= 16 && ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) ) {
                gs_soundTestFrames = 0;
                func_80037870( 1, 0x80 );
                // RULE-EXCEPTION: R30, i holds the saved setting: retail gives the counter this longer life, which
                // ranks it below each loop's hoisted values and orders this block's loads
                i = D_8009D098.unk_79;
                D_8009D098.unk_79 = 1;
                Sound_PlayBgm( gs_soundTestSound );
                D_8009D098.unk_79 = i;
            }
            break;
        case OPTION_BGM_VOLUME:
            if ( ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) || ( D_800E6758 & R_JPAD ) ) {
                if ( ++D_8009D098.unk_7A == 5 ) {
                    D_8009D098.unk_7A = 0;
                }
                Sound_UpdateBgmVolume();
                func_80037870( 0, 0x80 );
            }
            if ( D_800E6758 & L_JPAD ) {
                if ( --D_8009D098.unk_7A == 0xFF ) {
                    D_8009D098.unk_7A = 4;
                }
                Sound_UpdateBgmVolume();
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
                gs_optionsPage ^= 1;
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

        if ( gs_optionsCursor < OPTION_EXIT ) {
            func_8003893C( gs_optionsMenu, 0, 0x8F0, 0x49, gs_optionsCursor * 20 + 0x46, D_8009CFB0 );
        } else {
            func_8003893C( gs_optionsMenu, 0, 0x8EF, 0xD0, 0xB0, D_8009CFB0 );
        }
    }

    if ( D_8005C0BF == 1 && D_8005C0C0 == 0xFF ) {
        D_8005C0BF = 2;
        D_8005C0C0 = 0x107;
        if ( gs_optionsPage == 0 ) {
            Sound_PlayBgm( 0x13 );
            gs_optionsPage = 2;
            gs_creditsWait = 1;
            gs_creditsPos = 0;
            for ( i = 0; i < NUM_CREDIT_SPRITES; i++ ) {
                gs_creditSprites[i].image = -1;
            }
        } else {
            Sound_PlayBgm( 1 );
            gs_optionsPage = 0;
        }
    }

    if ( gs_optionsPage != 2 ) {
        func_800386E0( gs_optionsMenu, gs_optionsSlide );
        if ( gs_optionsState == 1 ) {
            for ( i = 0; i < 5; i++ ) {
                if ( gs_optionsCursor != i ) {
                    func_8002FDE4( 0xB0, i * 20 + 0x46, 0x46, 0x12, 0, 0, 0, 0x80 );
                }
            }
            if ( gs_optionsCursor == OPTION_EXIT && ( D_800E4D62 & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) ) {
                func_80037870( 1, 0 );
                gs_optionsState = 3;
            }
            if ( gs_optionsPage == 1 ) {
                for ( i = 0; i < 4; i++ ) {
                    func_8003B034( &D_8009D098.unk_08[i], i, 0 );
                }
                func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                Txt_LoadGlyphSheet( &D_800F8094 );
                func_8003A970( 4, D_8009D098.unk_08, 0, 0x18, 0x38 );
            }
        }
    } else {
        gs_optionsCursor = OPTION_CREDITS;
        if ( --gs_creditsWait == 0 ) {
            do {
                switch ( gs_creditsScript[gs_creditsPos].op ) {
                case CREDIT_SHOW:
                    i = gs_creditsScript[gs_creditsPos].sprite;
                    gs_creditSprites[i].image = gs_creditsScript[gs_creditsPos].a;
                    gs_creditSprites[i].x = gs_creditSprites[i].targetX = gs_creditsScript[gs_creditsPos].b;
                    gs_creditSprites[i].y = gs_creditSprites[i].targetY = gs_creditsScript[gs_creditsPos].c;
                    gs_creditSprites[i].alpha = gs_creditSprites[i].targetAlpha
                        = gs_creditsScript[gs_creditsPos].alpha;
                    gs_creditSprites[i].moveFrames = gs_creditSprites[i].moveFrame = 0;
                    gs_creditSprites[i].fadeFrames = gs_creditSprites[i].fadeFrame = 0;
                    break;
                case CREDIT_MOVE:
                    i = gs_creditsScript[gs_creditsPos].sprite;
                    gs_creditSprites[i].targetX = gs_creditsScript[gs_creditsPos].a;
                    gs_creditSprites[i].targetY = gs_creditsScript[gs_creditsPos].b;
                    gs_creditSprites[i].moveFrames = gs_creditsScript[gs_creditsPos].c;
                    gs_creditSprites[i].moveFrame = 0;
                    break;
                case CREDIT_FADE:
                    i = gs_creditsScript[gs_creditsPos].sprite;
                    gs_creditSprites[i].targetAlpha = gs_creditsScript[gs_creditsPos].a;
                    gs_creditSprites[i].fadeFrames = gs_creditsScript[gs_creditsPos].b;
                    gs_creditSprites[i].fadeFrame = 0;
                    break;
                case CREDIT_WAIT:
                    gs_creditsWait = gs_creditsScript[gs_creditsPos].sprite;
                    break;
                case CREDIT_END:
                    gs_creditsWait = 0xFFFF;
                    D_8005C0BF = 1;
                    break;
                }
                gs_creditsPos++;
            } while ( gs_creditsWait == 0 );
        }

        for ( i = 0; i < NUM_CREDIT_SPRITES; i++ ) {
            if ( gs_creditSprites[i].image == -1 ) {
                continue;
            }
            if ( gs_creditSprites[i].moveFrame < gs_creditSprites[i].moveFrames ) {
                gs_creditSprites[i].moveFrame++;
                if ( gs_creditSprites[i].moveFrames == gs_creditSprites[i].moveFrame ) {
                    gs_creditSprites[i].moveFrames = 0;
                    gs_creditSprites[i].x = gs_creditSprites[i].targetX;
                    gs_creditSprites[i].y = gs_creditSprites[i].targetY;
                }
            }
            if ( gs_creditSprites[i].fadeFrame < gs_creditSprites[i].fadeFrames ) {
                gs_creditSprites[i].fadeFrame++;
                if ( gs_creditSprites[i].fadeFrames == gs_creditSprites[i].fadeFrame ) {
                    gs_creditSprites[i].fadeFrames = 0;
                    gs_creditSprites[i].alpha = gs_creditSprites[i].targetAlpha;
                }
            }
            if ( gs_creditSprites[i].moveFrames != 0 ) {
                x = gs_creditSprites[i].targetX
                    - ( gs_creditSprites[i].targetX - gs_creditSprites[i].x )
                          * (f32)( gs_creditSprites[i].moveFrames - gs_creditSprites[i].moveFrame )
                          / gs_creditSprites[i].moveFrames;
                y = gs_creditSprites[i].targetY
                    - ( gs_creditSprites[i].targetY - gs_creditSprites[i].y )
                          * (f32)( gs_creditSprites[i].moveFrames - gs_creditSprites[i].moveFrame )
                          / gs_creditSprites[i].moveFrames;
            } else {
                x = gs_creditSprites[i].x;
                y = gs_creditSprites[i].y;
            }
            if ( gs_creditSprites[i].fadeFrames != 0 ) {
                alpha = gs_creditSprites[i].targetAlpha
                        - ( gs_creditSprites[i].targetAlpha - gs_creditSprites[i].alpha )
                              * (f32)( gs_creditSprites[i].fadeFrames - gs_creditSprites[i].fadeFrame )
                              / gs_creditSprites[i].fadeFrames;
            } else {
                alpha = gs_creditSprites[i].alpha;
            }
            func_8002E4B0( gs_creditSprites[i].image, x, y, alpha, 1.0f, 1.0f );
        }
    }

    switch ( gs_optionsState ) {
    case 0:
        gs_optionsMenu[5].slide = SLIDE_LEFT;
        gs_optionsSlide -= 2;
        if ( gs_optionsSlide == 0 ) {
            gs_optionsState = 1;
            func_80026B28();
        }
        break;
    case 2:
    case 3:
        gs_optionsMenu[5].slide = SLIDE_RIGHT;
        gs_optionsSlide += 2;
        if ( gs_optionsSlide == 30 ) {
            if ( D_8005C9E8 != 1 ) {
                Sound_PlayBgm( 1 );
            }
            if ( gs_optionsState == 2 ) {
                gs_optionsState = 0;
                return 1;
            }
            gs_optionsState = 0;
            return 2;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/605D0", GS_Options);
#endif
