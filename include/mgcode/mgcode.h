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
#define MG_MAZE_BUF_SIZE    0x1000      /* bytes of the maze buffer: walls and cells of up to 31 x 31 cells */

extern objClass_t mg_classes[];         /* the classes of the overlay's scene */
extern objParams_t mg_sceneObjects[];   /* the overlay's scene objects */
extern u8 mg_maze[MG_MAZE_BUF_SIZE];    /* the maze, walls and cells */

#endif
