/* 5E2B0.c -- game-select overlay: the character select */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/*
================
GS_CharacterSelect

One frame of the character select: the portraits of the unlocked characters, input, the viewport zooming in on the
highlighted character's model as the camera circles it, and the menu sliding in and out. Returns 1 once a character
is taken, 2 when backed out, else 0.
================
*/
s32 GS_CharacterSelect( void ) {
    s32 i;

    for ( i = 0; i < 8; i++ ) {
        if ( D_8009D098.unlocks & ( 1 << i ) ) {
            Menu_SetItem( D_8011BD50_gscode, i, D_8011F8A8_gscode[i], MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }

    if ( D_8011F8B9_gscode == CHOICE_RUN ) {
        if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
            Sound_PlaySe( SE_DECIDE, 0 );
            if ( D_8009D098.rumble ) {
                GS_StartPakCheck();
                D_8011F8B9_gscode = 4;
            } else {
                game_leaveState = 1;
                D_8011F8B9_gscode = CHOICE_TAKEN;
            }
        }
        if ( D_800E4D62 & B_BUTTON ) {
            Sound_PlaySe( 2, 0 );
            D_8011F8B9_gscode = CHOICE_BACK;
        }
    }

    if ( D_8011F8BA_gscode < 15 && D_8011F8B9_gscode == CHOICE_RUN ) {
        D_8011F8BA_gscode++;
        if ( D_8011F8BA_gscode == 10 ) {
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            Obj_CreateAll( gs_classes, D_8011F880_gscode[game_stage] );
            D_8011F8A0_gscode = 30;
            D_8011F8A4_gscode = 3.1415927f;
            D_8009CFA0[0]->path->flags |= 4;
        }
        if ( D_8011F8BA_gscode < 7 ) {
            D_8009CFA0[0]->width = 224.0f - D_8011F8BA_gscode * 32;
            D_8009CFA0[0]->height = 120.0f - D_8011F8BA_gscode * 17;
        } else if ( D_8011F8BA_gscode >= 9 ) {
            D_8009CFA0[0]->width = ( D_8011F8BA_gscode - 8 ) * 32.0f;
            D_8009CFA0[0]->height = ( D_8011F8BA_gscode - 8 ) * 17.0f;
        } else {
            D_8009CFA0[0]->width = 32;
            D_8009CFA0[0]->height = 17;
        }
        D_8009CFA0[0]->left = 160 - D_8009CFA0[0]->width / 2;
        D_8009CFA0[0]->top = 114 - D_8009CFA0[0]->height / 2;
    }

    if ( D_8011F8B9_gscode == CHOICE_RUN ) {
        if ( D_800E6758 & L_JPAD ) {
            if ( D_8011F8BA_gscode == 15 ) {
                D_8011F8BA_gscode = 0;
            } else if ( D_8011F8BA_gscode >= 9 ) {
                D_8011F8BA_gscode = 6;
            }
            do {
                game_stage--;
                if ( game_stage == 0xFF ) {
                    game_stage = 7;
                }
            } while ( !( D_8009D098.unlocks & ( 1 << game_stage ) ) );
            Sound_PlaySe( SE_CURSOR, 0 );
        }
        if ( D_800E6758 & R_JPAD ) {
            if ( D_8011F8BA_gscode == 15 ) {
                D_8011F8BA_gscode = 0;
            } else if ( D_8011F8BA_gscode >= 9 ) {
                D_8011F8BA_gscode = 6;
            }
            do {
                game_stage++;
                if ( game_stage == 8 ) {
                    game_stage = 0;
                }
            } while ( !( D_8009D098.unlocks & ( 1 << game_stage ) ) );
            Sound_PlaySe( SE_CURSOR, 0 );
        }
    }

    Menu_SetItem( D_8011BDF0_gscode, 8, D_8011BE10_gscode[game_stage], D_8011BE20_gscode[game_stage], MENU_NONE,
                   MENU_NONE );
    Menu_SetItem( D_8011BD50_gscode, 9, MENU_NONE, ( game_stage + 1 ) * 32, MENU_NONE, MENU_NONE );
    Menu_SetItem( D_8011BDF0_gscode, 10, D_8011BE30_gscode[D_8011BE38_gscode[game_stage]], MENU_NONE, MENU_NONE,
                   D_8009CFB0 );
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_DrawItems( D_8011BD50_gscode, D_8011F8B8_gscode );

    if ( D_8011F8B9_gscode == CHOICE_RUN && D_8011F8BA_gscode != 7 && D_8011F8BA_gscode != 8 &&
         D_8011F8BA_gscode != 9 ) {
        Draw_FillRect( D_8009CFA0[0]->left, D_8009CFA0[0]->top, D_8009CFA0[0]->width - 1,
                       D_8009CFA0[0]->height - 1, D_8009CFA0[0]->path->clearColor[0],
                       D_8009CFA0[0]->path->clearColor[1], D_8009CFA0[0]->path->clearColor[2], 0xFF );
        D_8009CFA0[0]->frame = -1;
        D_8009CFA0[0]->path->fovy = 30.0f;
        D_8009CFA0[0]->eye[0] = 0.0f;
        D_8009CFA0[0]->eye[1] = 100.0f;
        D_8009CFA0[0]->eye[2] = 0.0f;
        D_8009CFA0[0]->at[2] = cosf( D_8011F8A4_gscode ) * 100.0f;
        D_8009CFA0[0]->at[0] = sinf( D_8011F8A4_gscode ) * 100.0f;
        D_8009CFA0[0]->at[1] = D_8009CFA0[0]->eye[1];
        if ( D_8011F8A0_gscode ) {
            D_8011F8A0_gscode--;
        } else {
            D_8011F8A4_gscode += 0.010471975511965976;
        }
        D_8009CFA0[0]->upPoint[2] = D_8009CFA0[0]->eye[2];
        D_8009CFA0[0]->upPoint[0] = D_8009CFA0[0]->eye[0];
        D_8009CFA0[0]->upPoint[1] = -100.0f;
        func_8002A2F0();
    }

    Menu_DrawItems( D_8011BDF0_gscode, D_8011F8B8_gscode );

    switch ( D_8011F8B9_gscode ) {
    case CHOICE_IN:
        D_8011BD50_gscode[5].slide = SLIDE_LEFT;
        D_8011F8B8_gscode -= 2;
        if ( D_8011F8B8_gscode == 0 ) {
            D_8011F8B9_gscode = CHOICE_RUN;
            func_80026B28();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        D_8011BD50_gscode[5].slide = SLIDE_RIGHT;
        D_8011F8B8_gscode += 2;
        if ( D_8011F8B8_gscode == 30 ) {
            if ( game_leaveState == 1 ) {
                if ( game_fade != 0xFF ) {
                    D_8011F8B8_gscode = 28;
                    return 0;
                }
                game_leaveState = 2;
                game_fade = 0x107;
            }
            D_8011F8BA_gscode = 8;
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            if ( D_8011F8B9_gscode == CHOICE_TAKEN ) {
                D_8011F8B9_gscode = CHOICE_IN;
                return 1;
            }
            D_8011F8B9_gscode = CHOICE_IN;
            return 2;
        }
        break;
    case 4:
        if ( GS_PakCheck() ) {
            game_leaveState = 1;
            D_8011F8B9_gscode = CHOICE_TAKEN;
        }
        break;
    }
    return 0;
}
