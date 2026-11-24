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

/* D_8005C9BC */
#define PROMPT_CLOSED       -1
#define PROMPT_RUNNING      0
#define PROMPT_OPENING      1
#define PROMPT_CLOSING      2

#define PROMPT_ZOOM_FRAMES  10          /* frames of the zoom in and out, the entries of D_8005C994 */


extern u8 D_8005C0E0;               /* saving is on */
extern u8 D_8005C990;               /* step of Save_SaveSettings */
extern s32 D_80064314;              /* message to and from the controller thread */
extern u8 D_800693E8[0x20];
extern u8 D_80066BA4[0x20];
extern s32 D_8005C9BC;              /* PROMPT_CLOSED etc. */
extern s32 D_80064310;              /* frame of the zoom, or of a PROMPT_TIMED prompt */
extern f32 D_8005C994[PROMPT_ZOOM_FRAMES];  /* scale of the prompt in each frame of its zoom */
extern u16 D_80066C44;              /* choice under the cursor */
extern u8 D_8005C9C4;               /* alpha of the blinking cursor, 0x40 to 0x80 */
extern s8 D_8005C9C5;               /* added to D_8005C9C4 each frame; turned at either end */
/* cursor rectangle of each choice, from the prompt's corner. Two entries each: the next bss objects
   (cont_held, D_800E4D6C, __osContLastCmd; unused bss after the heights) follow each; a three-choice
   prompt's third rectangle overruns them (OP_CheckSettings) */
extern u16 D_800E57E0[2];           /* x */
extern u16 D_800E4D68[2];           /* y */
extern s16 D_800DF590[2];           /* width */
extern s16 D_800E6538[2];           /* height */

s32 Save_ShowPrompt( s32 id, s32 mode );
s32 Save_Checksum( void );
s32 Save_SaveSettings( void );

#endif
