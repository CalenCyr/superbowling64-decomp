/* 605D0.c -- game-select overlay: the options screen, its records page and the credits */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"
#include "dbgtext.h"

/* ===== Data ===== */

/* the options screen; [5] its title, id 0 the cursor */
static menuItem_t gs_optionsMenu[20] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x8DB, 100, 12, 0xFF, SLIDE_LEFT },
    { -1, 0x8DC, 56, 64, 0xFF, SLIDE_BOTTOM },
    { 0, 0x8F0, 73, 70, 0xFF, SLIDE_NONE },
    { 10, 0x8DD, 184, 114, 0xFF, SLIDE_NONE },
    { 11, 0x8DE, 175, 130, 0xFF, SLIDE_NONE },
    { 1, 0x8E4, 176, 74, 0xFF, SLIDE_NONE },
    { 2, 0x8E1, 180, 88, 0xFF, SLIDE_NONE },
    { 3, 0x8EE, 216, 115, 0xFF, SLIDE_NONE },
    { 4, 0x8EE, 228, 115, 0xFF, SLIDE_NONE },
    { 8, 0x8DF, 189, 130, 0xFF, SLIDE_NONE },
    { 7, 0x8DF, 202, 130, 0xFF, SLIDE_NONE },
    { 6, 0x8DF, 215, 130, 0xFF, SLIDE_NONE },
    { 5, 0x8DF, 228, 130, 0xFF, SLIDE_NONE },
    { 12, 0x8E1, 180, 148, 0xFF, SLIDE_NONE },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* the credits script */
static creditCmd_t gs_creditsScript[142] = {
    { CREDIT_SHOW, 0, 0x971, 168, 18, 0, 255 },
    { CREDIT_WAIT, 30, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 3, 0xAB4, 88, 64, 0, 0 },
    { CREDIT_FADE, 3, 255, 20, 0, 0, 0 },
    { CREDIT_SHOW, 1, 0xA9A, 56, 48, 0, 0 },
    { CREDIT_FADE, 1, 255, 20, 0, 0, 0 },
    { CREDIT_SHOW, 5, 0xAB1, 320, 88, 0, 0 },
    { CREDIT_MOVE, 5, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 5, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 6, 0xAB0, 320, 112, 0, 0 },
    { CREDIT_MOVE, 6, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 6, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 5, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 6, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 1, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 2, 0xAB2, 56, 48, 0, 0 },
    { CREDIT_FADE, 2, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 9, 0xAAF, 320, 88, 0, 0 },
    { CREDIT_MOVE, 9, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 9, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 10, 0xAAE, 320, 112, 0, 0 },
    { CREDIT_MOVE, 10, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 10, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 11, 0xAAD, 320, 136, 0, 0 },
    { CREDIT_MOVE, 11, 96, 136, 20, 0, 0 },
    { CREDIT_FADE, 11, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 9, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 10, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 11, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 2, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 1, 0xA97, 56, 48, 0, 0 },
    { CREDIT_FADE, 1, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 5, 0xAAC, 320, 88, 0, 0 },
    { CREDIT_MOVE, 5, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 5, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 6, 0xAAB, 320, 112, 0, 0 },
    { CREDIT_MOVE, 6, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 6, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 5, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 6, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 1, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 2, 0xA9C, 56, 48, 0, 0 },
    { CREDIT_FADE, 2, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 9, 0xAAA, 320, 88, 0, 0 },
    { CREDIT_MOVE, 9, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 9, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 9, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 2, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 1, 0xA99, 56, 48, 0, 0 },
    { CREDIT_FADE, 1, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 5, 0xAA9, 320, 88, 0, 0 },
    { CREDIT_MOVE, 5, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 5, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 6, 0xAA8, 320, 112, 0, 0 },
    { CREDIT_MOVE, 6, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 6, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 5, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 6, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 1, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 2, 0xA98, 56, 48, 0, 0 },
    { CREDIT_FADE, 2, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 9, 0xAA7, 320, 88, 0, 0 },
    { CREDIT_MOVE, 9, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 9, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 10, 0xAA6, 320, 112, 0, 0 },
    { CREDIT_MOVE, 10, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 10, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 9, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 10, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 2, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 1, 0xA96, 56, 48, 0, 0 },
    { CREDIT_FADE, 1, 255, 60, 0, 0, 0 },
    { CREDIT_FADE, 3, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 4, 0xAB3, 88, 64, 0, 0 },
    { CREDIT_FADE, 4, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 5, 0xAA5, 320, 88, 0, 0 },
    { CREDIT_MOVE, 5, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 5, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 6, 0xAA4, 320, 112, 0, 0 },
    { CREDIT_MOVE, 6, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 6, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 7, 0xAA3, 320, 136, 0, 0 },
    { CREDIT_MOVE, 7, 96, 136, 20, 0, 0 },
    { CREDIT_FADE, 7, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 8, 0xAA2, 320, 160, 0, 0 },
    { CREDIT_MOVE, 8, 96, 160, 20, 0, 0 },
    { CREDIT_FADE, 8, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 5, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 6, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 7, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 8, 0, 40, 0, 0, 0 },
    { CREDIT_SHOW, 9, 0xAA1, 320, 88, 0, 0 },
    { CREDIT_MOVE, 9, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 9, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 10, 0xAA0, 320, 112, 0, 0 },
    { CREDIT_MOVE, 10, 96, 112, 20, 0, 0 },
    { CREDIT_FADE, 10, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 11, 0xA9F, 320, 136, 0, 0 },
    { CREDIT_MOVE, 11, 96, 136, 20, 0, 0 },
    { CREDIT_FADE, 11, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 20, 0, 0, 0, 0, 0 },
    { CREDIT_SHOW, 12, 0xA9E, 320, 160, 0, 0 },
    { CREDIT_MOVE, 12, 96, 160, 20, 0, 0 },
    { CREDIT_FADE, 12, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 9, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 10, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 11, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 12, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 1, 0, 60, 0, 0, 0 },
    { CREDIT_SHOW, 2, 0xA9B, 56, 48, 0, 0 },
    { CREDIT_FADE, 2, 255, 60, 0, 0, 0 },
    { CREDIT_SHOW, 5, 0xA9D, 320, 88, 0, 0 },
    { CREDIT_MOVE, 5, 96, 88, 20, 0, 0 },
    { CREDIT_FADE, 5, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 120, 0, 0, 0, 0, 0 },
    { CREDIT_FADE, 2, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 4, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 5, 0, 40, 0, 0, 0 },
    { CREDIT_FADE, 0, 0, 40, 0, 0, 0 },
    { CREDIT_SHOW, 9, 0x88A, 104, 88, 0, 0 },
    { CREDIT_FADE, 9, 255, 60, 0, 0, 0 },
    { CREDIT_WAIT, 150, 0, 0, 0, 0, 0 },
    { CREDIT_END, 0, 0, 0, 0, 0, 0 },
};

static u32 gs_soundTestFrames = 0; /* frames since the last sound test */

/* per save_data.difficulty: its image */
static s16 gs_difficultyImages[4] = {
    0x8E4, 0x8E3, 0x8E2, 0x0,
};

/* per save_data.music and unk_7B: their image */
static s16 gs_onOffImages[2] = {
    0x8E0, 0x8E1,
};

/* image of each digit */
static s16 gs_digitImages[10] = {
    0x8EE, 0x8ED, 0x8EC, 0x8EB, 0x8EA, 0x8E9, 0x8E8, 0x8E7,
    0x8E6, 0x8E5,
};

static u8 gs_optionsSlide = 30; /* slide step of the options screen */
static u8 gs_optionsState = 0; /* 0 sliding in, 1 in place, 2 and 3 sliding out (3: backed out) */
static u8 gs_optionsCursor = 0; /* OPTION_DIFFICULTY etc. */
static u8 gs_soundTestSound = 0; /* sound of the sound test, 0 to NUM_SOUND_TESTS - 1 */
static u8 gs_optionsPage = 0; /* 0 the options, 1 the records page, 2 the credits */

/* The unit's uninitialised state: file-scope statics, which the assembler allocates in .bss in definition order. */
static creditSprite_t gs_creditSprites[NUM_CREDIT_SPRITES];
static u16 gs_creditsWait;      /* frames before the next credits command; 0xFFFF: the script ended */
static u16 gs_creditsPos;       /* next command of the credits script */

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

    if ( gs_optionsState == 1 && ( cont_allPressed & B_BUTTON ) && gs_optionsPage == 0 && game_leaveState == 0 ) {
        Sound_PlaySe( 2, 0 );
        gs_optionsState = 3;
    }

    Draw_ClearScreen( 0, 0, 0 );
    Draw_Image( 0xA58, 0, 0 );

    for ( i = 0; i < 13; i++ ) {
        Menu_SetItem( gs_optionsMenu, i, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    }

    if ( gs_optionsState == 1 && game_leaveState == 0 ) {
        Menu_SetItem( gs_optionsMenu, 10, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        Menu_SetItem( gs_optionsMenu, 11, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        Menu_SetItem( gs_optionsMenu, 1, gs_difficultyImages[save_data.difficulty], MENU_NONE, MENU_NONE, 0xFF );
        Menu_SetItem( gs_optionsMenu, 2, gs_onOffImages[save_data.music], MENU_NONE, MENU_NONE, 0xFF );
        Menu_SetItem( gs_optionsMenu, 12, gs_onOffImages[save_data.rumble], MENU_NONE, MENU_NONE, 0xFF );
        if ( (u8)( gs_soundTestSound / 10 ) ) {
            Menu_SetItem( gs_optionsMenu, 3, gs_digitImages[(u8)( gs_soundTestSound / 10 )], MENU_NONE,
                           MENU_NONE, 0xFF );
        }
        Menu_SetItem( gs_optionsMenu, 4, gs_digitImages[(u8)( gs_soundTestSound % 10 )], MENU_NONE,
                       MENU_NONE, 0xFF );
        for ( i = 0; i < 4 - save_data.musicVolume; i++ ) {
            Menu_SetItem( gs_optionsMenu, i + 5, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        }

        if ( gs_optionsPage == 0
             && ( ( cont_allDpadRepeat & U_JPAD )
                  || ( gs_optionsCursor == OPTION_EXIT && ( cont_allDpadRepeat & L_JPAD ) ) ) ) {
            if ( --gs_optionsCursor == 0xFF ) {
                gs_optionsCursor = OPTION_EXIT;
            }
            Game_StartPulse();
            Sound_PlaySe( 0, 0x80 );
        } else if ( gs_optionsPage == 0
                    && ( ( cont_allDpadRepeat & D_JPAD )
                         || ( gs_optionsCursor == OPTION_CREDITS && ( cont_allDpadRepeat & R_JPAD ) ) ) ) {
            if ( ++gs_optionsCursor == 8 ) {
                gs_optionsCursor = 0;
            }
            Game_StartPulse();
            Sound_PlaySe( 0, 0x80 );
        }

        switch ( gs_optionsCursor ) {
        case OPTION_DIFFICULTY:
            if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) || ( cont_allDpadRepeat & R_JPAD ) ) {
                if ( ++save_data.difficulty == 3 ) {
                    save_data.difficulty = 0;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            if ( cont_allDpadRepeat & L_JPAD ) {
                if ( --save_data.difficulty == 0xFF ) {
                    save_data.difficulty = 2;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            break;
        case OPTION_MUSIC:
            if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) || ( cont_allDpadRepeat & ( L_JPAD | R_JPAD ) ) ) {
                if ( ++save_data.music == 2 ) {
                    save_data.music = 0;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            break;
        case OPTION_SOUND_TEST:
            gs_soundTestFrames++;
            if ( cont_allDpadRepeat & R_JPAD ) {
                if ( ++gs_soundTestSound == NUM_SOUND_TESTS ) {
                    gs_soundTestSound = 0;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            if ( cont_allDpadRepeat & L_JPAD ) {
                if ( --gs_soundTestSound == 0xFF ) {
                    gs_soundTestSound = NUM_SOUND_TESTS - 1;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            if ( gs_soundTestFrames >= 16 && ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) ) {
                gs_soundTestFrames = 0;
                Sound_PlaySe( 1, 0x80 );
                // RULE-EXCEPTION: R30, i holds the saved setting: retail gives the counter this longer life, which
                // ranks it below each loop's hoisted values and orders this block's loads
                i = save_data.music;
                save_data.music = 1;
                Sound_PlayBgm( gs_soundTestSound );
                save_data.music = i;
            }
            break;
        case OPTION_BGM_VOLUME:
            if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) || ( cont_allDpadRepeat & R_JPAD ) ) {
                if ( ++save_data.musicVolume == 5 ) {
                    save_data.musicVolume = 0;
                }
                Sound_UpdateBgmVolume();
                Sound_PlaySe( 0, 0x80 );
            }
            if ( cont_allDpadRepeat & L_JPAD ) {
                if ( --save_data.musicVolume == 0xFF ) {
                    save_data.musicVolume = 4;
                }
                Sound_UpdateBgmVolume();
                Sound_PlaySe( 0, 0x80 );
            }
            break;
        case OPTION_RUMBLE:
            if ( ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) || ( cont_allDpadRepeat & ( L_JPAD | R_JPAD ) ) ) {
                if ( ++save_data.rumble == 2 ) {
                    save_data.rumble = 0;
                }
                Sound_PlaySe( 0, 0x80 );
            }
            break;
        case OPTION_RECORDS:
            if ( cont_allPressed & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( 1, 0 );
                gs_optionsPage ^= 1;
            }
            break;
        case OPTION_CREDITS:
            if ( cont_allPressed & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( 1, 0 );
                game_leaveState = 1;
            }
            break;
        case OPTION_EXIT:
            break;
        }

        if ( gs_optionsCursor < OPTION_EXIT ) {
            Menu_SetItem( gs_optionsMenu, 0, 0x8F0, 0x49, gs_optionsCursor * 20 + 0x46, game_pulseAlpha );
        } else {
            Menu_SetItem( gs_optionsMenu, 0, 0x8EF, 0xD0, 0xB0, game_pulseAlpha );
        }
    }

    if ( game_leaveState == 1 && game_fade == 0xFF ) {
        game_leaveState = 2;
        game_fade = 0x107;
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
        Menu_DrawItems( gs_optionsMenu, gs_optionsSlide );
        if ( gs_optionsState == 1 ) {
            for ( i = 0; i < 5; i++ ) {
                if ( gs_optionsCursor != i ) {
                    Draw_FillRect( 0xB0, i * 20 + 0x46, 0x46, 0x12, 0, 0, 0, 0x80 );
                }
            }
            if ( gs_optionsCursor == OPTION_EXIT && ( cont_allPressed & ( A_BUTTON | B_BUTTON | Z_TRIG ) ) ) {
                Sound_PlaySe( 1, 0 );
                gs_optionsState = 3;
            }
            if ( gs_optionsPage == 1 ) {
                for ( i = 0; i < 4; i++ ) {
                    Score_SetRecordFigures( &save_data.records[i], i, 0 );
                }
                Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                Txt_LoadGlyphSheet( &game_gfx );
                Score_DrawRecordTable( 4, save_data.records, 0, 0x18, 0x38 );
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
                    game_leaveState = 1;
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
            Draw_ImageScaled( gs_creditSprites[i].image, x, y, alpha, 1.0f, 1.0f );
        }
    }

    switch ( gs_optionsState ) {
    case 0:
        gs_optionsMenu[5].slide = SLIDE_LEFT;
        gs_optionsSlide -= 2;
        if ( gs_optionsSlide == 0 ) {
            gs_optionsState = 1;
            Game_StartPulse();
        }
        break;
    case 2:
    case 3:
        gs_optionsMenu[5].slide = SLIDE_RIGHT;
        gs_optionsSlide += 2;
        if ( gs_optionsSlide == 30 ) {
            if ( sound_bgm != 1 ) {
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
