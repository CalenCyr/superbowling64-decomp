#ifndef OVERLAY_H
#define OVERLAY_H

/*
overlay.h -- the code overlays the game thread (func_80026B44) loads at 0x80112350, one at a time: the
linker's bounds of each, and the entry points main calls in them
*/

#include "common.h"

/* values of D_8005C0B6: the scene, and the overlay file that runs it */
#define SCENE_GACODE        0           /* "GACODE  SYS": the game */
#define SCENE_CACODE        1           /* "CACODE  SYS" */
#define SCENE_PVCODE        2           /* "PVCODE  SYS" */
#define SCENE_OPCODE        3           /* "OPCODE  SYS": the opening */
#define SCENE_GSCODE        4           /* "GSCODE  SYS": the game select */
#define SCENE_MGCODE        5           /* "MGCODE  SYS" */
#define SCENE_RSCODE        6           /* "RSCODE  SYS": the results */

/* linker: where each overlay runs, and the end of its bss (its file end when it has none) */
extern u8 gacode_VRAM[];
extern u8 gacode_BSS_END[];
extern u8 rscode_VRAM[];
extern u8 rscode_BSS_END[];
extern u8 gscode_VRAM[];
extern u8 gscode_BSS_END[];
extern u8 cacode_VRAM[];
extern u8 cacode_BSS_END[];
extern u8 pvcode_VRAM[];
extern u8 pvcode_BSS_END[];
extern u8 mgcode_VRAM[];
extern u8 mgcode_BSS_END[];
extern u8 opcode_VRAM[];
extern u8 opcode_BSS_END[];

/* each overlay's init, run once after it is loaded, and its per-frame function; gacode's func_80112A74 ends the demo */
void func_80112350_gacode( void );
void func_801125A0_gacode( void );
void func_80112A74_gacode( void );
void RS_Init( void );
void RS_Frame( void );
void func_80112350_gscode( void );
s32 func_80112414_gscode( void );
void CA_Init( void );
void CA_Frame( void );
void PV_Init( void );
void PV_Frame( void );
void MG_Init( void );
void MG_Frame( void );
void OP_Init( void );
s32 OP_Frame( void );

#endif
