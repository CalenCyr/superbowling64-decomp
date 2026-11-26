/* data.c -- main: shared globals of main's .data after 3D50's, in address order (no functions)

   Their users interleave (1060, 2DF0, BEF0, 12260, 13490 and the overlays), so they are defined in this data-only
   unit, after 3D50 in link order, and declared in the units' headers. */

#include "common.h"
#include "libultra.h"
#include "boot.h"
#include "dbgtext.h"
#include "filecache.h"
#include "game.h"
#include "gamesnd.h"
#include "input.h"
#include "save.h"
#include "opcode/opcode.h"

static u8 D_8005C0B0 = 0;               /* unreferenced */

/* ===== The scheduler's framebuffers, the cursor and the scene ===== */

// volatile: as declared in boot.h (read with lbu and a separate sign extension, once per use)
volatile s8 sched_bufferIndex = 0;
// volatile: as declared in boot.h (reloaded right after each store), as sched_shownFb
volatile s8 sched_nextFb = 0;
// volatile: as declared in boot.h
volatile s8 sched_shownFb = 0;
u8 sched_reset[2] = { 0 };
// volatile: as declared in game.h
volatile u8 game_scene = SCENE_OPCODE;
s16 game_cursorX = 160;
s16 game_cursorY = 120;
u8 cache_hostMode = 0;
u8 cache_hostFiles = 0;
u8 game_cursorSpeed = 1;
u8 game_leaveState = 0;
s16 game_fade = 0;

/* ===== The controller pak's game note ===== */

u16 cont_noteCompany = 0x364B;
u32 cont_noteGameCode = 0x4E425745;     /* "NBWE" */
u8 cont_noteGameName[16] = {
    0x2C, 0x2E, 0x29, 0x1E, 0x2B, 0x0F, 0x1B, 0x28, 0x30, 0x25, 0x22, 0x27, 0x20, 0x00, 0x00, 0x00,
};
u8 cont_noteExtName[4] = { 0 };
s32 cont_noteSize = 0x100;

/* ===== The game's setup ===== */

u8 save_enabled = 0;
u8 game_players[4] = { 0, 1, 2, 3 };
u8 op_nextMode = 0x0A;
s32 cont_demoMode = 0;
u32 game_demo = 3;
u8 game_demoCharacter = 0;
playerName_t game_defaultNames[8] = {
    { { 0x00B7, 0x00D2, 0x00DB, 0x00D8, 0x0057, 0xFFFF } },
    { { 0x00B0, 0x00D5, 0x00D9, 0x00D1, 0x00CA, 0xFFFF } },
    { { 0x00C1, 0x00D2, 0x00D7, 0x0057, 0x0057, 0xFFFF } },
    { { 0x00B5, 0x00CA, 0x00D7, 0x00D7, 0x00E2, 0xFFFF } },
    { { 0x00BF, 0x00CE, 0x00D0, 0x00D0, 0x00E2, 0xFFFF } },
    { { 0x00B3, 0x00D8, 0x00DD, 0x0057, 0x0057, 0xFFFF } },
    { { 0x00BA, 0x00CA, 0x00D7, 0x00DD, 0x00CA, 0xFFFF } },
    { { 0x00BC, 0x00D2, 0x00D4, 0x00D8, 0x0057, 0xFFFF } },
};
static s32 D_8005C158[2] = { 0 };     /* unreferenced */
u8 game_mode = 0;
u8 game_numPlayers = 1;
u8 game_lastTeam = 0;
u8 game_stage = 0;
u8 D_8005C164 = 0;
static s32 D_8005C168[2] = { 0 };     /* unreferenced */
static Lights1 D_8005C170 = gdSPDefLights1( 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, -80, 80, 0 );  /* unreferenced */
