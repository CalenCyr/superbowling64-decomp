#ifndef SAVE_H
#define SAVE_H

/* save.h -- saving the settings, and the game's yes/no prompts (unit 12260) */

#include "common.h"
#include "libultra.h"

/* Save_ShowPrompt modes */
#define PROMPT_TIMED        0           /* closes after 90 frames, or at any button */
#define PROMPT_ANY_BUTTON   1           /* closes at any button */
#define PROMPT_OPEN         2           /* stays open */
#define PROMPT_CLOSE        3           /* closes */
#define PROMPT_CHOICE2_H    4           /* two choices, left and right */
#define PROMPT_CHOICE2_V    5           /* two choices, up and down */
#define PROMPT_CHOICE3      6           /* three choices, left to right */
#define PROMPT_TIMED_LONG   7           /* closes after 270 frames, or at any button */

/* save_promptState */
#define PROMPT_CLOSED       -1
#define PROMPT_RUNNING      0
#define PROMPT_OPENING      1
#define PROMPT_CLOSING      2

#define PROMPT_ZOOM_FRAMES  10          /* frames of the zoom in and out, the entries of save_promptZoom */


extern u8 save_enabled;             /* saving is on */
extern u8 save_pakId[0x20];          /* cont_pakPfs.id of the pak saving is enabled on */
extern s32 save_promptState;        /* PROMPT_CLOSED etc. */
extern u16 save_promptChoice;       /* choice under the cursor */
/* cursor rectangle of each choice, from the prompt's corner. Two entries each: the next bss objects
   (cont_held, scene_twoCycle, __osContLastCmd; unused bss after the heights) follow each; a three-choice
   prompt's third rectangle overruns them (OP_CheckSettings) */
extern u16 save_choiceX[2];         /* x */
extern u16 save_choiceY[2];         /* y */
extern s16 save_choiceWidth[2];     /* width */
extern s16 save_choiceHeight[2];    /* height */

s32 Save_ShowPrompt( s32 id, s32 mode );
s32 Save_Checksum( void );
s32 Save_SaveSettings( void );

#endif
