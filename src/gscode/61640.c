/* 61640.c -- game-select overlay: the character menu */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

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
        func_8002A2F0();
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
