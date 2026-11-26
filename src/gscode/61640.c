/* 61640.c -- game-select overlay: the character menu */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the character menu: [0-7] the characters, [9] the cursor, [10] the name */
static menuItem_t D_80120430_gscode[17] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0xA8D, 40, 12, 0xFF, SLIDE_LEFT },
    { 10, 0xA95, 216, 48, 0xFF, SLIDE_RIGHT },
    { 0, 0x8C2, 32, 188, 0xFF, SLIDE_BOTTOM },
    { 1, 0x8C5, 64, 188, 0xFF, SLIDE_BOTTOM },
    { 2, 0x8BE, 96, 188, 0xFF, SLIDE_BOTTOM },
    { 3, 0x8C3, 128, 188, 0xFF, SLIDE_BOTTOM },
    { 4, 0x8BF, 160, 188, 0xFF, SLIDE_BOTTOM },
    { 5, 0x8C4, 192, 188, 0xFF, SLIDE_BOTTOM },
    { 6, 0x8C1, 224, 188, 0xFF, SLIDE_BOTTOM },
    { 7, 0x8C0, 256, 188, 0xFF, SLIDE_BOTTOM },
    { 9, 0x92D, 32, 180, 0xFF, SLIDE_BOTTOM },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per character: its image once unlocked */
static s16 D_801204DC_gscode[8] = {
    0x8C2, 0x8C5, 0x8BE, 0x8C3, 0x8BF, 0x8C4, 0x8C1, 0x8C0,
};

/* the scene of the character menu */
static objParams_t D_801204EC_gscode[] = {
    { 1.0f, 0.0f, { 2056.0f, 320.0f, 240.0f } },
    { 0.0f, 10.0f, { 568.0f, 2032.0f, -100.0f, -100.0f, 300.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f } },
    { -2.0f, 0.0f },
};

/* per character: scale of its model */
static f32 D_801205AC_gscode[8] = {
    1.0f, 1.0f, 1.0f, 1.2f, 1.2f, 0.7f, 1.1f, 1.0f,
};

/* per character: file of its model */
static s16 D_801205CC_gscode[8] = {
    0x462, 0x4E0, 0x6DC, 0x5BE, 0x13A, 0x2AA, 0x590, 0x238,
};

/* per character: animations in D_801205E4_gscode */
static u8 D_801205DC_gscode[8] = {
    4, 5, 5, 4, 5, 3, 5, 4,
};

/* per character: files of its animations; [0] the idle one */
static s16 D_801205E4_gscode[8][6] = {
    { 0x7D8, 0x7FE, 0x7D5, 0x7D7, 0x7D9, 0x7FE },
    { 0x7DB, 0x7DE, 0x7DD, 0x7E0, 0x7DA, 0x7E1 },
    { 0x7F9, 0x7F7, 0x7F6, 0x7F8, 0x7F5, 0x7FA },
    { 0x7F0, 0x7ED, 0x7EC, 0x7EE, 0x7F1, 0x7ED },
    { 0x7B3, 0x7B5, 0x7B4, 0x7B7, 0x7B2, 0x7B8 },
    { 0x7C8, 0x7CA, 0x7C9, 0x7CC, 0x7CC, 0x7CA },
    { 0x7EA, 0x7E7, 0x7E6, 0x7E8, 0x7E5, 0x7EB },
    { 0x7C2, 0x7C4, 0x7C3, 0x7C6, 0x7C1, 0x7C3 },
};

/* per character: image of its name */
static s16 D_80120644_gscode[8] = {
    0xA95, 0xA94, 0xA93, 0xA92, 0xA91, 0xA90, 0xA8F, 0xA8E,
};

static u8 D_80120654_gscode = 30; /* slide step of the character menu */
static u8 D_80120655_gscode = 0; /* CHOICE_IN etc. */
static u8 D_80120656_gscode = 0; /* highlighted character, 0 to 7 */
static u8 D_80120657_gscode = 1; /* bit 0: the character menu takes input */
static u8 D_80120658_gscode = 0; /* animation of the highlighted character */

/* ===== State ===== */

static u16 D_80120A20_gscode;               /* character whose model is shown; 0xFFFF: none */
static fileType1_t *D_80120A28_gscode[8];   /* per character: its loaded model */
static u16 D_80120A48_gscode[8];            /* per character: file of its loaded animation */

/*
================
GS_CharacterMenu

Frame of the character menu: the eight characters' images (the locked ones as a blank) slide in, the
highlighted character's model turns in the scene behind them, left and right move the cursor between the
unlocked characters, A or Z plays its next animation and B backs out. Returns 0 while the menu runs; once
it has slid out, 1 when a character was taken and 2 for backing out.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail loads the argument of both Cache_Release calls with lhu (passed unconverted, as with no
// prototype in scope); the s16 prototype makes it lh. Every other instruction is the same.
s32 GS_CharacterMenu( void ) {
    s32 i;
    sceneObject_t *obj;

    Menu_SetItem( D_80120430_gscode, 10, D_80120644_gscode[D_80120656_gscode], MENU_NONE, MENU_NONE, MENU_NONE );
    for ( i = 0; i < 8; i++ ) {
        if ( save_data.unlocks & ( 1 << i ) ) {
            Menu_SetItem( D_80120430_gscode, i, D_801204DC_gscode[i], MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( D_80120430_gscode, i, 0x8BC, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }

    if ( ( D_80120657_gscode & 1 ) && D_80120655_gscode == CHOICE_RUN ) {
        if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
            Sound_PlaySe( SE_DECIDE, 0 );
            if ( ++D_80120658_gscode == D_801205DC_gscode[D_80120656_gscode] ) {
                D_80120658_gscode = 0;
            }
            ( (sceneObject_t *)obj_scene[10] )->frame = 1;
            Cache_Release( D_80120A48_gscode[D_80120656_gscode] );
            D_80120A48_gscode[D_80120656_gscode] = D_801205E4_gscode[D_80120656_gscode][D_80120658_gscode + 1];
            ( (sceneObject_t *)obj_scene[10] )->anim =
                Cache_GetFile( D_801205E4_gscode[D_80120656_gscode][D_80120658_gscode + 1], FILE_TYPE2 );
        }
        if ( cont_allPressed & B_BUTTON ) {
            Sound_PlaySe( 2, 0 );
            D_80120655_gscode = CHOICE_BACK;
        }
        if ( cont_allDpadRepeat & L_JPAD ) {
            do {
                if ( --D_80120656_gscode == 0xFF ) {
                    D_80120656_gscode = 7;
                }
            } while ( !( save_data.unlocks & ( 1 << D_80120656_gscode ) ) );
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
        if ( cont_allDpadRepeat & R_JPAD ) {
            do {
                if ( ++D_80120656_gscode == 8 ) {
                    D_80120656_gscode = 0;
                }
            } while ( !( save_data.unlocks & ( 1 << D_80120656_gscode ) ) );
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_SetItem( D_80120430_gscode, 9, MENU_NONE, ( D_80120656_gscode + 1 ) * 32, MENU_NONE, MENU_NONE );
    Draw_ClearScreen( 0, 0, 0 );
    if ( D_80120657_gscode & 1 ) {
        Draw_Image( 0xA58, 0, 0 );
    }
    Menu_DrawItems( D_80120430_gscode, D_80120654_gscode );

    if ( D_80120655_gscode == CHOICE_RUN ) {
        if ( D_80120A20_gscode == D_80120656_gscode ) {
            if ( ( (sceneObject_t *)obj_scene[10] )->frame ) {
                goto draw;
            }
        } else {
            D_80120658_gscode = 0;
        }
        ( (sceneObject_t *)obj_scene[10] )->frame = 0;
        ( (sceneObject_t *)obj_scene[10] )->model = D_80120A28_gscode[D_80120656_gscode];
        D_80120A20_gscode = D_80120656_gscode;
        Cache_Release( D_80120A48_gscode[D_80120656_gscode] );
        D_80120A48_gscode[D_80120656_gscode] = D_801205E4_gscode[D_80120656_gscode][0];
        ( (sceneObject_t *)obj_scene[10] )->anim = Cache_GetFile( D_801205E4_gscode[D_80120656_gscode][0], FILE_TYPE2 );
        Mtx_SetRotScaleTrans( ( (sceneObject_t *)obj_scene[10] )->mtx, 0.0f, 0.0f, 0.0f,
                       D_801205AC_gscode[D_80120656_gscode] * 1.2, -100.0f, -100.0f, 300.0f );
    draw:
        Obj_Frame();
    }

    switch ( D_80120655_gscode ) {
    case CHOICE_IN:
        D_80120654_gscode -= 2;
        if ( D_80120654_gscode == 0 ) {
            D_80120655_gscode = CHOICE_RUN;
            Game_StartPulse();
            Obj_CreateAll( gs_classes, D_801204EC_gscode );
            D_80120A20_gscode = 0xFFFF;
            cam_cameras[0]->path->flags |= 4;
        } else if ( D_80120654_gscode == 28 ) {
            for ( i = 0; i < 8; i++ ) {
                D_80120A28_gscode[i] = Cache_GetFile( D_801205CC_gscode[i], FILE_TYPE1 );
                Cache_GetFile( D_801205E4_gscode[i][0], FILE_TYPE2 );
                D_80120A48_gscode[i] = D_801205E4_gscode[i][0];
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_80120430_gscode[5].slide = SLIDE_RIGHT;
        D_80120654_gscode += 2;
        if ( D_80120654_gscode == 30 ) {
            Sound_PlayBgm( 1 );
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            D_80120430_gscode[5].slide = SLIDE_LEFT;
            if ( D_80120655_gscode == CHOICE_TAKEN ) {
                D_80120655_gscode = CHOICE_IN;
                return 1;
            }
            D_80120655_gscode = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/61640", GS_CharacterMenu);
#endif
