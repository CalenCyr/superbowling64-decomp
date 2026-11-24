#ifndef GAME_H
#define GAME_H

/*
game.h -- main's game-state globals that the code overlays (gacode, rscode, gscode) read. Their owning main units
are not identified yet; each is declared once here until they are.
*/

#include "common.h"

/* A player's name: characters ended by 0xFFFF; copied whole by assignment */
typedef struct {
    /* 0x00 */ u16 chars[6];
} playerName_t;                              /* size 0x0C */


extern u8 game_players[];           /* per player: bit 7 set for a player of the game (no controller); low nibble a character */
extern u8 game_mode;                /* low nibble the game mode; bits 4-5 a sub-mode, bits 6-7 its argument */
extern u8 game_numPlayers;          /* number of players */
extern u8 game_lastTeam;            /* index of the last team */
extern u8 game_stage;               /* the stage, index of ga_stages; a duel's opponent's character */
extern u8 D_8005C164;               /* set to 1 by the game reset (GA_ClearScores) */
extern u8 game_replay;              /* set when the results overlay plays again: gscode reopens team select */
extern u8 game_gamesWon[2];         /* per lane: games won */
extern u8 game_frame[4];            /* per player: current frame (10: the game is over) */
extern s16 game_frameScores[4][10];  /* per player and frame: score */
extern s8 game_secondRolls[4][10];  /* per player and frame: pins of the second roll (Score_DrawFrame) */
extern s8 game_firstRolls[4][10];   /* per player and frame: pins of the first roll (Score_DrawFrame) */
extern s8 D_800994B8[2];            /* per player of a duel: frames won */
extern s16 game_totals[];           /* per player: total score (Score_DrawTotal) */
extern u16 game_duelScores[2][3];   /* per player of a duel and game: score */
/* the scheduler thread reads it too, while the overlays write it */
// volatile: the game thread reads it with lbu and a separate andi 0xFF before both scene switches (Game_Thread)
extern volatile u8 game_scene;      /* the scene to run, SCENE_* (overlay.h); restored by the results overlay on leaving */
extern u8 game_leaveState;          /* 0, then 1 once an overlay asks to leave, 2 when it goes */
extern s16 game_fade;               /* alpha of the gray fill drawn over a scene change, 0 to 255 (Game_Thread) */
extern u32 game_demo;               /* demo the opening overlay plays next, 0 to 3 */
extern u8 game_demoCharacter;       /* counts the demos' characters */
extern playerName_t game_defaultNames[8];  /* per character: its default name */
extern playerName_t game_playerNames[];  /* per player: name */
extern u8 D_80068A50[];             /* per player (Score_DrawPlayerBoard) */
extern u8 game_playerDone[];        /* per player: set once the player's game is over (Score_DrawPlayerBoard) */
extern u8 game_frameFlags[4][10];   /* per player and frame: flags; in the par game, [0] holds each frame's par (GA_CountParThrow) */
extern u16 game_splits[4];          /* per player: splits this game (GA_IsSplit, by GA_ScoreStandard), summed into
                                       playerRecord_t.splits; the par game keeps the sum of its pars in [0] */
extern u16 game_strikes[4];         /* per player: strikes this game, counted up with game_strikeRun (GA_ScoreStandard) */
extern u16 game_spares[4];          /* per player: spares this game, counted when a frame ends after a non-run turn */
extern u8 game_strikeRun[4];        /* per player: strikes in a row (to 255), which the game modes read and reset (GA_ScoreBattle) */
extern u16 D_800A3050[4][12];       /* per player and frame: a pin mask, bit 9 - n for pin n */
extern u16 D_800F7C18[4][12];       /* per player and frame: a second pin mask, the same way */

/* The lane's floor height grid: a file (the stage's gaStage_t.unk_18). Name invented. */
typedef struct {
    /* 0x000 */ u16 numX;                  /* grid points along x */
    /* 0x002 */ u16 numZ;                  /* grid points along z */
    /* 0x004 */ u16 stepX;                 /* spacing of the points along x */
    /* 0x006 */ u16 stepZ;                 /* spacing of the points along z */
    /* 0x008 */ s16 originX;               /* x of the first point */
    /* 0x00A */ s16 originZ;               /* z of the first point */
    /* 0x00C */ s16 heights[2000];         /* by z, then x */
} floorGrid_t;                          /* size 0xFAC */

extern floorGrid_t game_floorGrids[2];  /* [0] the stage's; [1] a copy of the grid main loads at boot */

#endif
