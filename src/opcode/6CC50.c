/* 6CC50.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

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
        D_800E4FC8 = 0;
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
                op_notePages[n] = ( D_80066C00.note.file_size + 0xFF ) >> 8;
                for ( j = 0; j < 16; j++ ) {
                    op_noteNames[n].name[j] = OP_ConvertPakChar( D_80066C00.note.game_name[j] );
                }
                op_noteNames[op_numNotes].ext = OP_ConvertPakChar( D_80066C00.note.ext_name[0] );
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
                            memcpy( cont_deleteGameName, D_80066C00.note.game_name, 16 );
                            memcpy( cont_deleteExtName, D_80066C00.note.ext_name, 4 );
                            op_pakMsg = CONT_REQ_DELETE_FILE;
                            cont_deleteCompany = D_80066C00.note.company_code;
                            cont_deleteGameCode = D_80066C00.note.game_code;
                            osSendMesg( &cont_requestQueue, &op_pakMsg, OS_MESG_NOBLOCK );
                            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
                            if ( cont_result == 0 ) {
                                op_freeNotes++;
                                op_freePages += ( D_80066C00.note.file_size + 0xFF ) >> 8;
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
