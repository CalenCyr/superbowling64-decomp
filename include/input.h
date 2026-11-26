#ifndef INPUT_H
#define INPUT_H

/* input.h -- controllers and Rumble Paks (unit 2DF0) */

#include "common.h"
#include "libultra.h"

#define NUM_PLAYERS     2           /* controller ports the game reads */

extern OSMesgQueue cont_siQueue;    /* serial interface events */
extern OSMesg cont_siMsgs;
extern u8 cont_siEvent[];           /* its address is the serial event's message */
extern u8 cont_standardMask;        /* bit per port with a standard controller */
extern OSContStatus cont_status[MAXCONTROLLERS];
extern OSPfs cont_rumblePfs[NUM_PLAYERS];
extern u8 cont_hasRumble[NUM_PLAYERS];   /* TRUE when the port has a Rumble Pak */
extern s16 cont_rumblePhase[NUM_PLAYERS];
extern u8 cont_rumble[NUM_PLAYERS];     /* rumble strength: 4 to 6 from gacode's hits; RUMBLE_REPROBE */
extern u8 cont_rumbleFrames[NUM_PLAYERS];   /* frames left of the cont_rumble strength */

#define RUMBLE_PERIOD   0xE0        /* cont_rumblePhase accumulates cont_rumble << 5; the motor runs each time it passes this */
#define RUMBLE_REPROBE  8           /* cont_rumble value: look for the Rumble Pak again and stop it */

/* per-port pad state, rebuilt each frame by Cont_ReadPads */
extern OSContPad cont_pads[MAXCONTROLLERS];
extern s8 cont_stickX[NUM_PLAYERS];          /* stick x, 0 inside the dead zone */
extern s8 cont_stickY[NUM_PLAYERS];          /* stick y, 0 inside the dead zone */
extern u16 cont_held[NUM_PLAYERS];           /* buttons held */
extern u16 cont_lastHeld[NUM_PLAYERS];       /* buttons held last frame */
extern u16 cont_pressed[NUM_PLAYERS];        /* buttons pressed this frame */
extern u16 cont_stickDpad[NUM_PLAYERS];      /* stick as D-pad bits */
extern u16 cont_lastStickDpad[NUM_PLAYERS];  /* the same, last frame */
extern u16 cont_dpadRepeat[NUM_PLAYERS];     /* D-pad presses, with auto-repeat */
extern s32 cont_holdFrames[NUM_PLAYERS];     /* frames a direction has been held */
extern s32 cont_noController;       /* set when port 1 has no controller */
/* both ports merged: port 1's value, or port 2's when port 1's is 0 */
extern u16 cont_allHeld;
extern u16 cont_allPressed;
extern u16 cont_allDpadRepeat;
extern s8 cont_allStickX;
extern s8 cont_allStickY;
extern s32 cont_demoMode;           /* a demo plays: Cont_ReadPads fakes its R presses */
extern u32 cont_demoFrames;         /* frames left of the demo; Start cuts it to 40 */
extern volatile s32 sched_frameCount;  /* frame counter (Main_Thread); volatile: it reloads it right after its own store */
/* requests to the controller thread (Cont_Thread), and its replies: the message points to the request code */
typedef enum {
    CONT_REQ_ALLOCATE_FILE,     /* allocate the game's note */
    CONT_REQ_DELETE_FILE,       /* delete the cont_delete* note */
    CONT_REQ_FIND_FILE,         /* find the game's note: cont_noteIndex */
    CONT_REQ_NUM_FILES,         /* cont_maxNotes, cont_usedNotes */
    CONT_REQ_FREE_BLOCKS,       /* cont_freeBytes */
    CONT_REQ_READ_WRITE_FILE,   /* note cont_noteIndex to or from *save_dataPtr (cont_pakWrite: PFS_READ, PFS_WRITE) */
    CONT_REQ_INIT_PAK,          /* open the pak of port cont_pakPort */
    CONT_REQ_REPAIR_ID,
    CONT_REQ_IS_PLUG,           /* cont_pakPlugged */
    CONT_REQ_MOTOR_INIT,        /* look for a Rumble Pak on port 1 (into cont_rumblePfs[1]) */
    CONT_REQ_FILE_STATE         /* the state of note cont_noteIndex: D_80066C00.note */
} contRequest_t;

extern OSMesgQueue cont_requestQueue;
extern OSMesgQueue cont_replyQueue;
extern s32 cont_result;             /* result of the last request */
extern s32 cont_pakPort;
extern s32 cont_noteIndex;          /* request 10: the note to read */
/* Main's D_80066C00 (0x24 bytes). Name invented. */
typedef struct {
    /* 0x00 */ OSPfsState note;            /* request 10: the state of the note read */
    /* 0x20 */ u8 unk_20[4];               /* per player: frames of the pin diagram's second page, 9 on */
} unk80066C00_t;                        /* size 0x24 */

extern unk80066C00_t D_80066C00;
extern s32 cont_maxNotes;           /* request 4: notes in all (with cont_usedNotes) */
extern s32 cont_usedNotes;          /* request 4: notes used */
extern s32 cont_freeBytes;          /* request 4: free bytes */
/* request 1: the note to delete */
extern u8 cont_deleteGameName[16];  /* game name */
extern u8 cont_deleteExtName[4];    /* extension */
extern u16 cont_deleteCompany;      /* company code */
extern u32 cont_deleteGameCode;     /* game code */
extern u8 cont_pakWrite;
/* the game's Controller Pak note */
extern OSPfs cont_pakPfs;           /* the pak the requests work on */
extern u16 cont_noteCompany;        /* company code */
extern u32 cont_noteGameCode;       /* game code, "NBWE" */
extern u8 cont_noteGameName[16];    /* game name */
extern u8 cont_noteExtName[4];      /* extension */
extern s32 cont_noteSize;           /* bytes */
extern u8 cont_pakPlugged;          /* bit per port with a pak (CONT_REQ_IS_PLUG) */
extern u8 cont_pollStatus;          /* nonzero: Cont_Thread queries the controller status every frame */

void Cont_Init( void );
void Rumble_Probe( s32 chan );
void Rumble_Start( s32 chan );
void Rumble_Stop( s32 chan );
void Cont_ReadPads( void );
void Cont_FindStandard( void );
void Cont_Thread( void *arg );

#endif
