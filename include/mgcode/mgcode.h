#ifndef MGCODE_MGCODE_H
#define MGCODE_MGCODE_H

/* mgcode.h -- the mgcode overlay (file MGCODE  SYS, ROM 0x6BA90, loaded at 0x80112350) */

#include "common.h"
#include "overlay.h"
#include "libultra.h"
#include "boot.h"
#include "input.h"
#include "object.h"
#include "dbgtext.h"
#include "gamesnd.h"
#include "menu.h"

#define MG_MAZE_SIZE        10          /* cells of the maze across and down */

extern objClass_t D_80112550_mgcode[];  /* the classes of the overlay's scene */
extern objParams_t D_801125B0_mgcode[]; /* the overlay's scene objects */
extern u8 D_80112730_mgcode[( 2 * MG_MAZE_SIZE + 1 ) * ( 2 * MG_MAZE_SIZE + 1 )];  /* the maze, walls and cells */

#endif
