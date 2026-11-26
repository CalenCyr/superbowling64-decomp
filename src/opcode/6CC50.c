/* 6CC50.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

/* ===== Data ===== */

/* Entry of each character code 0x10-0x94 (OP_ConvertPakChar). */
static u16 op_charEntries[0x85] = {
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5,
    0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD,
    0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5,
    0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x010E, 0x00FA, 0x010D, 0x0106,
    0x0116, 0x0114, 0x00E4, 0x0115, 0x00E5, 0x0123, 0x00E7, 0x0118,
    0x010F, 0x010A, 0x00AC, 0x00AE, 0x00AF, 0x008C, 0x008D, 0x008E,
    0x008F, 0x0090, 0x0088, 0x0089, 0x008A, 0x008B, 0x0086, 0x0087,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0091, 0x0092, 0x0093, 0x0094,
    0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C,
    0x009D, 0x009E, 0x009F, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4,
    0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9,
};

/* The images of the Controller Pak screen, ended by an id of -1. */
static opImage_t op_pakImages[] = {
    { 0x880, 64, 192, 0xFF },
    { 0x878, 176, 192, 0xFF },
    { 0x85B, 0, 8, 0xFF },
    { 0x85B, 64, 8, 0xFF },
    { 0x85B, 128, 8, 0xFF },
    { 0x85B, 192, 8, 0xFF },
    { 0x85B, 256, 8, 0xFF },
    { 0x927, 36, 56, 0xC8 },
    { 0x928, 36, 72, 0xC8 },
    { 0x928, 36, 90, 0xC8 },
    { 0x928, 36, 108, 0xC8 },
    { 0x928, 36, 126, 0xC8 },
    { 0x928, 36, 144, 0xC8 },
    { 0x901, 40, 168, 0xC8 },
    { 0x900, 178, 168, 0xC8 },
    { 0x8A2, 32, 12, 0xFF },
    { -1, 0, 0, 0xFF },
};

/* Unreferenced: two glyph strings for Txt_DrawGlyphs, glyphs 0-15 and 16-31, each ended by 0xFFFF. */
static u16 op_glyphsLow[17] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0xFFFF,
};
static u16 op_glyphsHigh[17] = {
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0xFFFF,
};

static u16 op_cursorImages[3] = { 0xAE0, 0xADF, 0xADE }; /* cursor images, a frame each */
static u8 op_pakStep = 0;                                  /* step of OP_PakScreen */
static u8 op_promptChoice = 1;                             /* choice of the first prompt */
static u8 op_cursorLine = 0;                               /* line of the cursor */
static u8 op_pakState = 0;                                 /* state of the screen */

/*
The unit's uninitialised state. File-scope statics: the assembler allocates them in .bss in definition order
with their own alignment, which the retail layout shows.
*/
static s32 op_pakMsg;                   /* OP_PakScreen's message to and from the controller thread */
static u16 op_numNotes;                 /* notes listed */
static u16 op_firstNote;                /* first note shown */
static u16 op_noteIndex[PAK_MAX_NOTES]; /* per note listed: its index on the pak */
static u16 op_notePages[PAK_MAX_NOTES]; /* per note listed: its pages */
static opNoteName_t op_noteNames[PAK_MAX_NOTES];
static s32 op_freeNotes;                /* free notes */
static s32 op_freePages;                /* free pages */

/*
================
OP_ConvertPakChar

Looks up a character code's entry in the table (codes 0x10-0x94); any other code gives 0x57.
================
*/
u16 OP_ConvertPakChar( u8 code ) {
    if ( (u8)( code - 0x10 ) >= 0x85 ) {
        return 0x57;
    }
    return op_charEntries[code - 0x10];
}

/*
================
OP_PakScreen

Runs one frame of the Controller Pak screen: lists the pak's notes, and deletes the one picked once the player
confirms. Returns FALSE when the screen is over.
================
*/
s32 OP_PakScreen( void ) {
    s32 i;
    s32 j;
    s32 n;
    u16 glyphs[8];

    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    switch ( op_pakStep ) {
    case 0:
        op_pakMsg = CONT_REQ_INIT_PAK;
        cont_pakPort = 0;
        osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( cont_result != 0 ) {
            return FALSE;
        }
        op_numNotes = 0;
        for ( i = 0; i < PAK_MAX_NOTES; i++ ) {
            op_pakMsg = CONT_REQ_FILE_STATE;
            cont_noteIndex = i;
            osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
            if ( cont_result == 0 ) {
                n = op_numNotes;
                op_noteIndex[n] = i;
                op_notePages[n] = ( cont_noteState.file_size + 0xFF ) >> 8;
                for ( j = 0; j < 16; j++ ) {
                    op_noteNames[n].name[j] = OP_ConvertPakChar( cont_noteState.game_name[j] );
                }
                op_noteNames[op_numNotes].ext = OP_ConvertPakChar( cont_noteState.ext_name[0] );
                op_noteNames[op_numNotes].end = 0xFFFF;
                op_numNotes++;
            } else if ( cont_result != 5 ) {
                return FALSE;
            }
        }
        op_pakMsg = CONT_REQ_NUM_FILES;
        osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( cont_result != 0 ) {
            return FALSE;
        }
        op_pakMsg = CONT_REQ_FREE_BLOCKS;
        op_freeNotes = cont_maxNotes - cont_usedNotes;
        osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( cont_result != 0 ) {
            return FALSE;
        }
        op_freePages = cont_freeBytes / 256;
        op_firstNote = 0;
        op_pakStep = 2;
        break;
    case 2:
        Draw_Image( 0xA58, 0, 0 );
        if ( op_pakState == 0 ) {
            Draw_RoundedBox( op_promptChoice == 0 ? 0x40 : 0xB0, 0xC0, 0x50, 0x10, 0xFF, 0xFF, 0xFF, game_pulseAlpha );
        }
        for ( i = 0; op_pakImages[i].id != -1; i++ ) {
            if ( op_pakImages[i].alpha ) {
                Draw_ImageScaled( op_pakImages[i].id, op_pakImages[i].x, op_pakImages[i].y,
                               op_pakImages[i].alpha, 1.0f, 1.0f );
            }
        }
        if ( op_firstNote != 0 ) {
            Draw_ImageScaled( 0x888, 0x98, 0x38, game_pulseAlpha, 1.0f, 1.0f );
        }
        if ( op_numNotes - op_firstNote >= 6 ) {
            Draw_ImageScaled( 0x889, 0x98, 0xA4, game_pulseAlpha, 1.0f, 1.0f );
        }
        Txt_LoadGlyphSheet( &game_gfx );
        for ( i = 0; i < PAK_NOTES_SHOWN; i++ ) {
            if ( i + op_firstNote < op_numNotes ) {
                Txt_NumberToGlyphs( glyphs, i + op_firstNote + 1, 2 );
                Txt_DrawGlyphs( &game_gfx, 0x28, 0x49 + i * 18, 8, glyphs );
                Txt_DrawGlyphs( &game_gfx, 0x3C, 0x49 + i * 18, 8, op_noteNames[i + op_firstNote].name );
                Txt_NumberToGlyphs( glyphs, op_notePages[i + op_firstNote], 4 );
                Txt_DrawGlyphs( &game_gfx, 0xF9, 0x49 + i * 18, 8, glyphs );
            }
        }
        i = op_freeNotes < 10;
        Txt_NumberToGlyphs( glyphs, op_freeNotes, 2 );
        Txt_DrawGlyphs( &game_gfx, 0x74 + i * 8, 0xA9, 8, glyphs );
        if ( op_freePages >= 100 ) {
            i = 0;
        } else if ( op_freePages >= 10 ) {
            i = 1;
        } else {
            i = 2;
        }
        Txt_NumberToGlyphs( glyphs, op_freePages, 3 );
        Txt_DrawGlyphs( &game_gfx, 0xFE + i * 8, 0xA9, 8, glyphs );
        if ( game_leaveState == 0 ) {
            switch ( op_pakState ) {
            case 0:
                if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
                    Sound_PlaySe( 1, 0 );
                    if ( op_promptChoice == 0 ) {
                        op_pakState = 1;
                    } else {
                        game_leaveState = 1;
                    }
                }
                if ( ( cont_allDpadRepeat & L_JPAD ) && op_promptChoice == 1 ) {
                    op_promptChoice = 0;
                    Sound_PlaySe( 0, 0 );
                }
                if ( ( cont_allDpadRepeat & R_JPAD ) && op_promptChoice == 0 ) {
                    op_promptChoice = 1;
                    Sound_PlaySe( 0, 0 );
                }
                break;
            case 1:
                if ( op_numNotes == 0 ) {
                    op_pakState = 0;
                    break;
                }
                if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
                    Sound_PlaySe( 1, 0 );
                    op_pakState = 2;
                    save_promptChoice = 1;
                }
                if ( cont_allPressed & B_BUTTON ) {
                    Sound_PlaySe( 2, 0 );
                    op_pakState = 0;
                }
                if ( cont_allDpadRepeat & U_JPAD ) {
                    if ( op_cursorLine != 0 ) {
                        Sound_PlaySe( 0, 0 );
                        op_cursorLine--;
                    } else if ( op_firstNote != 0 ) {
                        Sound_PlaySe( 0, 0 );
                        op_firstNote--;
                    }
                }
                if ( cont_allDpadRepeat & D_JPAD ) {
                    if ( op_cursorLine + op_firstNote < op_numNotes - 1 ) {
                        Sound_PlaySe( 0, 0 );
                        if ( op_cursorLine < PAK_NOTES_SHOWN - 1 ) {
                            op_cursorLine++;
                        } else {
                            op_firstNote++;
                        }
                    }
                }
                Draw_Image( op_cursorImages[sched_frameCount / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                Draw_RectOutline( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, game_pulseAlpha );
                break;
            case 2:
                Draw_Image( op_cursorImages[sched_frameCount / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                Draw_RectOutline( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, game_pulseAlpha );
                save_choiceWidth[0] = save_choiceWidth[1] = 0x24;
                save_choiceHeight[0] = save_choiceHeight[1] = 0x10;
                save_choiceX[0] = 0x24;
                save_choiceY[0] = 0x1E;
                save_choiceX[1] = 0x4E;
                save_choiceY[1] = 0x1E;
                Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                if ( Save_ShowPrompt( 0x94B, PROMPT_CHOICE2_H ) ) {
                    if ( save_promptChoice == 0 ) {
                        Sound_PlaySe( 8, 0 );
                        op_pakMsg = CONT_REQ_FILE_STATE;
                        cont_noteIndex = op_noteIndex[op_firstNote + op_cursorLine];
                        osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
                        osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
                        if ( cont_result != 0 ) {
                            op_pakState = 3;
                        } else {
                            memcpy( cont_deleteGameName, cont_noteState.game_name, 16 );
                            memcpy( cont_deleteExtName, cont_noteState.ext_name, 4 );
                            op_pakMsg = CONT_REQ_DELETE_FILE;
                            cont_deleteCompany = cont_noteState.company_code;
                            cont_deleteGameCode = cont_noteState.game_code;
                            osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
                            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
                            if ( cont_result == 0 ) {
                                op_freeNotes++;
                                op_freePages += ( cont_noteState.file_size + 0xFF ) >> 8;
                                for ( i = op_cursorLine + op_firstNote; i < op_numNotes - 1; i++ ) {
                                    op_noteIndex[i] = op_noteIndex[i + 1];
                                    op_notePages[i] = op_notePages[i + 1];
                                    op_noteNames[i] = op_noteNames[i + 1];
                                }
                                op_numNotes--;
                                if ( op_cursorLine + op_firstNote == op_numNotes ) {
                                    op_cursorLine--;
                                }
                                op_pakState = 1;
                            } else {
                                op_pakState = 3;
                            }
                        }
                    }
                    if ( save_promptChoice == 1 ) {
                        op_pakState = 1;
                    }
                }
                break;
            case 3:
                Draw_Image( op_cursorImages[sched_frameCount / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                Draw_RectOutline( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, game_pulseAlpha );
                Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                if ( Save_ShowPrompt( 0x94A, PROMPT_TIMED ) ) {
                    op_pakState = 1;
                }
                break;
            }
        }
        if ( game_leaveState == 1 && game_fade == 0xFF ) {
            game_leaveState = 2;
            game_fade = 0x107;
            return FALSE;
        }
        break;
    }
    return TRUE;
}
