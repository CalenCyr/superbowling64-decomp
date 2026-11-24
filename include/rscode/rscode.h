#ifndef RSCODE_RSCODE_H
#define RSCODE_RSCODE_H

/* rscode.h -- the results overlay (file RSCODE  SYS, ROM 0x56FF0, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
#include "game.h"
#include "menu.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "sprite.h"
#include "gamesnd.h"
#include "menu.h"
#include "save.h"

/*
==============================================================================

OVERLAY

==============================================================================
*/

/* A state of the overlay's main loop (rs_state). Name invented. */
typedef void (*rsState_t)( void );

extern u8 rs_leaving;            /* set: leave the overlay */
extern u8 rs_sheetPage;
extern u8 rs_boxAlpha;           /* alpha of the boxes, 0 to 0x80 and back */
extern u8 rs_boxAlphaFalling;    /* 1: rs_boxAlpha falls */
extern s16 rs_messageFrames;     /* frames the current message has been shown */
extern u8 rs_menuChoice;         /* choice of RS_Menu's menu */
extern s16 rs_pinPos[10][2];     /* x and y of each pin in a frame's pin diagram */
extern u8 rs_pinPage;            /* page of the pin diagrams: 0 frames 1-9, 1 the tenth on */
extern u8 rs_ranks[4];           /* per player: rank by score, 1 the best */
extern u16 rs_buttonsPressed;    /* buttons pressed this frame (0 once leaving) */
extern u16 rs_dpadPressed;       /* D-pad presses this frame (0 once leaving) */
extern u8 D_801142B4_rscode;     /* D_8005C160 >> 6 */
extern u8 rs_nextScene;          /* copied back to D_8005C0B6 on leaving */
extern rsState_t rs_state;       /* current state */
extern rsState_t rs_returnState; /* state to return to */

void RS_StartMode( void );
void RS_StartMode0( void );
void RS_FinishedMessage( void );
void RS_PerfectMessage( void );
void RS_RankAndRecord( void );
void RS_ScoreSheet( void );
void RS_Records( void );
void RS_ResultsPage1( void );
void RS_ResultsPage2( void );
void RS_DuelResults( void );
void RS_ResultsPage( void );
void RS_Menu( void );
void RS_MenuConfirm( void );
void RS_Leave( void );
void RS_Idle( void );
void RS_DrawPinDiagram( s32 player, u8 frame, s16 x, s16 y );
void RS_DrawPinDiagrams( s16 x, s16 y );
void RS_DrawChoices( u8 choice, u16 *images, s16 y, u16 w, u16 h );
void RS_AddToRecord( UnkRecord *record, s32 player );
void RS_RankHighest( u8 *ranks, u8 count, s16 *scores, s16 low, s16 high );
void RS_RankLowest( u8 *ranks, u8 count, s16 *values, s16 low, s16 high );

#endif
