#ifndef DBGSHOT_H
#define DBGSHOT_H

/* dbgshot.h -- debug screenshots written to the development host (unit 13230) */

#include "common.h"

#define SCREENSHOT_WIDTH    320
#define SCREENSHOT_HEIGHT   240
#define SCREENSHOT_FILE_SIZE 0x38438   /* bytes written, as the header's bfSize: 2 more than the header and pixels */

/* The 54 bytes in front of a screenshot's pixels: a BMP file and info header, 24-bit, bottom-up. */
typedef struct {
    /* 0x00 */ u8 data[0x36];
} bmpHeader_t;

/* A screenshot as Dbg_ScreenShot writes it to the host. */
typedef struct {
    /* 0x00000 */ bmpHeader_t header;
    /* 0x00036 */ u8 pixels[SCREENSHOT_HEIGHT][SCREENSHOT_WIDTH][3];  /* blue, green, red */
} bmpFile_t;

extern bmpHeader_t dbg_shotHeader;  /* the header in front of each screenshot's pixels */
extern s16 dbg_shotNumber;          /* number of the next screenshot */

void Dbg_ScreenShot( void );

#endif
