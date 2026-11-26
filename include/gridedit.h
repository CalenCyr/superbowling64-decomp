#ifndef GRIDEDIT_H
#define GRIDEDIT_H

/* gridedit.h -- unit 111A0: the debug editor of the floor grid (game_floorGrids[0], saved to the host as "ground.bin") */

#include "common.h"
#include "libultra.h"

/* gridEd_page: the editor's page, picked with R held and up or down */
#define GRIDEDIT_EXIT       0           /* Z leaves the editor */
#define GRIDEDIT_LOAD       1           /* Z reads "ground.bin" from the host */
#define GRIDEDIT_SAVE       2           /* Z writes "ground.bin" to the host */
#define GRIDEDIT_SIZE       3           /* the D-pad sets the point counts (clears the heights) */
#define GRIDEDIT_STEP       4           /* the D-pad sets the spacing */
#define GRIDEDIT_ORIGIN     5           /* the D-pad moves the origin */
#define GRIDEDIT_SELECT     6           /* the D-pad moves the cursor; Z marks points */
#define GRIDEDIT_HEIGHT     7           /* up and down raise and lower the marked points */
#define GRIDEDIT_PAGES      8

#define GRIDEDIT_POINTS     2000        /* points of floorGrid_t.heights */

extern u8 gridEd_active;            /* set: GridEd_Frame runs each frame; it clears it when done */

void GridEd_Frame( void );

#endif
