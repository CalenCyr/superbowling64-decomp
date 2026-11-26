/* 61640.c -- game-select overlay: the character menu */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the character menu: [0-7] the characters, [9] the cursor, [10] the name */
static menuItem_t gs_menuItems[17] = {
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
static s16 gs_menuPortraits[8] = {
    0x8C2, 0x8C5, 0x8BE, 0x8C3, 0x8BF, 0x8C4, 0x8C1, 0x8C0,
};

/* the scene of the character menu */
static objParams_t gs_menuScene[] = {
    { 1.0f, 0.0f, { 2056.0f, 320.0f, 240.0f } },
    { 0.0f, 10.0f, { 568.0f, 2032.0f, -100.0f, -100.0f, 300.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f } },
    { -2.0f, 0.0f },
};

/* per character: scale of its model */
static f32 gs_modelScales[8] = {
    1.0f, 1.0f, 1.0f, 1.2f, 1.2f, 0.7f, 1.1f, 1.0f,
};

/* per character: file of its model */
static s16 gs_modelFiles[8] = {
    0x462, 0x4E0, 0x6DC, 0x5BE, 0x13A, 0x2AA, 0x590, 0x238,
};

/* per character: animations in gs_animFiles */
static u8 gs_animCounts[8] = {
    4, 5, 5, 4, 5, 3, 5, 4,
};

/* per character: files of its animations; [0] the idle one */
static s16 gs_animFiles[8][6] = {
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
static s16 gs_menuNames[8] = {
    0xA95, 0xA94, 0xA93, 0xA92, 0xA91, 0xA90, 0xA8F, 0xA8E,
};

static u8 gs_menuSlideStep = 30; /* slide step of the character menu */
static u8 gs_menuChoice = 0; /* CHOICE_IN etc. */
static u8 gs_menuCharacter = 0; /* highlighted character, 0 to 7 */
static u8 gs_menuFlags = 1; /* bit 0: the character menu takes input */
static u8 gs_menuAnim = 0; /* animation of the highlighted character */

/* ===== State ===== */

static u16 gs_shownModel;               /* character whose model is shown; 0xFFFF: none */
static fileType1_t *gs_models[8];   /* per character: its loaded model */
static u16 gs_loadedAnims[8];            /* per character: file of its loaded animation */

/*
================
GS_CharacterMenu

Frame of the character menu: the eight characters' images (the locked ones as a blank) slide in, the
highlighted character's model turns in the scene behind them, left and right move the cursor between the
unlocked characters, A or Z plays its next animation and B backs out. Returns 0 while the menu runs; once
it has slid out, 1 when a character was taken and 2 for backing out.
================
*/
s32 GS_CharacterMenu( void ) {
    s32 i;

    Menu_SetItem( gs_menuItems, 10, gs_menuNames[gs_menuCharacter], MENU_NONE, MENU_NONE, MENU_NONE );
    for ( i = 0; i < 8; i++ ) {
        if ( save_data.unlocks & ( 1 << i ) ) {
            Menu_SetItem( gs_menuItems, i, gs_menuPortraits[i], MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            Menu_SetItem( gs_menuItems, i, 0x8BC, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }

    if ( ( gs_menuFlags & 1 ) && gs_menuChoice == CHOICE_RUN ) {
        if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
            Sound_PlaySe( SE_DECIDE, 0 );
            if ( ++gs_menuAnim == gs_animCounts[gs_menuCharacter] ) {
                gs_menuAnim = 0;
            }
            ( (sceneObject_t *)obj_scene[10] )->frame = 1;
            Cache_Release( gs_loadedAnims[gs_menuCharacter] );
            gs_loadedAnims[gs_menuCharacter] = gs_animFiles[gs_menuCharacter][gs_menuAnim + 1];
            ( (sceneObject_t *)obj_scene[10] )->anim =
                Cache_GetFile( gs_animFiles[gs_menuCharacter][gs_menuAnim + 1], FILE_TYPE2 );
        }
        if ( cont_allPressed & B_BUTTON ) {
            Sound_PlaySe( 2, 0 );
            gs_menuChoice = CHOICE_BACK;
        }
        if ( cont_allDpadRepeat & L_JPAD ) {
            do {
                if ( --gs_menuCharacter == 0xFF ) {
                    gs_menuCharacter = 7;
                }
            } while ( !( save_data.unlocks & ( 1 << gs_menuCharacter ) ) );
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
        if ( cont_allDpadRepeat & R_JPAD ) {
            do {
                if ( ++gs_menuCharacter == 8 ) {
                    gs_menuCharacter = 0;
                }
            } while ( !( save_data.unlocks & ( 1 << gs_menuCharacter ) ) );
            Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
        }
    }

    Menu_SetItem( gs_menuItems, 9, MENU_NONE, ( gs_menuCharacter + 1 ) * 32, MENU_NONE, MENU_NONE );
    Draw_ClearScreen( 0, 0, 0 );
    if ( gs_menuFlags & 1 ) {
        Draw_Image( 0xA58, 0, 0 );
    }
    Menu_DrawItems( gs_menuItems, gs_menuSlideStep );

    if ( gs_menuChoice == CHOICE_RUN ) {
        if ( gs_shownModel == gs_menuCharacter ) {
            if ( ( (sceneObject_t *)obj_scene[10] )->frame ) {
                goto draw;
            }
        } else {
            gs_menuAnim = 0;
        }
        ( (sceneObject_t *)obj_scene[10] )->frame = 0;
        ( (sceneObject_t *)obj_scene[10] )->model = gs_models[gs_menuCharacter];
        gs_shownModel = gs_menuCharacter;
        Cache_Release( gs_loadedAnims[gs_menuCharacter] );
        gs_loadedAnims[gs_menuCharacter] = gs_animFiles[gs_menuCharacter][0];
        ( (sceneObject_t *)obj_scene[10] )->anim = Cache_GetFile( gs_animFiles[gs_menuCharacter][0], FILE_TYPE2 );
        Mtx_SetRotScaleTrans( ( (sceneObject_t *)obj_scene[10] )->mtx, 0.0f, 0.0f, 0.0f,
                       gs_modelScales[gs_menuCharacter] * 1.2, -100.0f, -100.0f, 300.0f );
    draw:
        Obj_Frame();
    }

    switch ( gs_menuChoice ) {
    case CHOICE_IN:
        gs_menuSlideStep -= 2;
        if ( gs_menuSlideStep == 0 ) {
            gs_menuChoice = CHOICE_RUN;
            Game_StartPulse();
            Obj_CreateAll( gs_classes, gs_menuScene );
            gs_shownModel = 0xFFFF;
            cam_cameras[0]->path->flags |= 4;
        } else if ( gs_menuSlideStep == 28 ) {
            for ( i = 0; i < 8; i++ ) {
                gs_models[i] = Cache_GetFile( gs_modelFiles[i], FILE_TYPE1 );
                Cache_GetFile( gs_animFiles[i][0], FILE_TYPE2 );
                gs_loadedAnims[i] = gs_animFiles[i][0];
            }
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_menuItems[5].slide = SLIDE_RIGHT;
        gs_menuSlideStep += 2;
        if ( gs_menuSlideStep == 30 ) {
            Sound_PlayBgm( 1 );
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            gs_menuItems[5].slide = SLIDE_LEFT;
            if ( gs_menuChoice == CHOICE_TAKEN ) {
                gs_menuChoice = CHOICE_IN;
                return 1;
            }
            gs_menuChoice = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
