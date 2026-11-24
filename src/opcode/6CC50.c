/* 6CC50.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

/*
================
func_80113130_opcode

Looks up a character code's entry in the table (codes 0x10-0x94); any other code gives 0x57.
================
*/
u16 func_80113130_opcode( u8 code ) {
    if ( (u8)( code - 0x10 ) >= 0x85 ) {
        return 0x57;
    }
    return op_charEntries[code - 0x10];
}

/*
================
func_80113168_opcode

Runs one frame of the Controller Pak screen: lists the pak's notes, and deletes the one picked once the player
confirms. Returns FALSE when the screen is over.
================
*/
s32 func_80113168_opcode( void ) {
    s32 i;
    s32 j;
    s32 n;
    u16 glyphs[8];

    func_8003068C( 0xE0, 0xE0, 0xE0 );
    switch ( op_pakStep ) {
    case 0:
        op_pakMsg = 6;
        D_800E4FC8 = 0;
        osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        op_numNotes = 0;
        for ( i = 0; i < PAK_MAX_NOTES; i++ ) {
            op_pakMsg = 10;
            D_800E6584 = i;
            osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
            osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                n = op_numNotes;
                op_noteIndex[n] = i;
                op_notePages[n] = ( D_80066C00.note.file_size + 0xFF ) >> 8;
                for ( j = 0; j < 16; j++ ) {
                    op_noteNames[n].name[j] = func_80113130_opcode( D_80066C00.note.game_name[j] );
                }
                op_noteNames[op_numNotes].ext = func_80113130_opcode( D_80066C00.note.ext_name[0] );
                op_noteNames[op_numNotes].end = 0xFFFF;
                op_numNotes++;
            } else if ( D_80066B8C != 5 ) {
                return FALSE;
            }
        }
        op_pakMsg = 3;
        osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        op_pakMsg = 4;
        op_freeNotes = D_80066C28 - D_800692D4;
        osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        op_freePages = D_8009944C / 256;
        op_firstNote = 0;
        op_pakStep = 2;
        break;
    case 2:
        func_8002DF3C( 0xA58, 0, 0 );
        if ( op_pakState == 0 ) {
            func_8002C950( op_promptChoice == 0 ? 0x40 : 0xB0, 0xC0, 0x50, 0x10, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
        }
        for ( i = 0; op_pakImages[i].id != -1; i++ ) {
            if ( op_pakImages[i].alpha ) {
                func_8002E4B0( op_pakImages[i].id, op_pakImages[i].x, op_pakImages[i].y,
                               op_pakImages[i].alpha, 1.0f, 1.0f );
            }
        }
        if ( op_firstNote != 0 ) {
            func_8002E4B0( 0x888, 0x98, 0x38, D_8009CFB0, 1.0f, 1.0f );
        }
        if ( op_numNotes - op_firstNote >= 6 ) {
            func_8002E4B0( 0x889, 0x98, 0xA4, D_8009CFB0, 1.0f, 1.0f );
        }
        func_80028EA8( &D_800F8094 );
        for ( i = 0; i < PAK_NOTES_SHOWN; i++ ) {
            if ( i + op_firstNote < op_numNotes ) {
                func_800292D4( glyphs, i + op_firstNote + 1, 2 );
                func_80029088( &D_800F8094, 0x28, 0x49 + i * 18, 8, glyphs );
                func_80029088( &D_800F8094, 0x3C, 0x49 + i * 18, 8, op_noteNames[i + op_firstNote].name );
                func_800292D4( glyphs, op_notePages[i + op_firstNote], 4 );
                func_80029088( &D_800F8094, 0xF9, 0x49 + i * 18, 8, glyphs );
            }
        }
        i = op_freeNotes < 10;
        func_800292D4( glyphs, op_freeNotes, 2 );
        func_80029088( &D_800F8094, 0x74 + i * 8, 0xA9, 8, glyphs );
        if ( op_freePages >= 100 ) {
            i = 0;
        } else if ( op_freePages >= 10 ) {
            i = 1;
        } else {
            i = 2;
        }
        func_800292D4( glyphs, op_freePages, 3 );
        func_80029088( &D_800F8094, 0xFE + i * 8, 0xA9, 8, glyphs );
        if ( D_8005C0BF == 0 ) {
            switch ( op_pakState ) {
            case 0:
                if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                    func_80037870( 1, 0 );
                    if ( op_promptChoice == 0 ) {
                        op_pakState = 1;
                    } else {
                        D_8005C0BF = 1;
                    }
                }
                if ( ( D_800E6758 & L_JPAD ) && op_promptChoice == 1 ) {
                    op_promptChoice = 0;
                    func_80037870( 0, 0 );
                }
                if ( ( D_800E6758 & R_JPAD ) && op_promptChoice == 0 ) {
                    op_promptChoice = 1;
                    func_80037870( 0, 0 );
                }
                break;
            case 1:
                if ( op_numNotes == 0 ) {
                    op_pakState = 0;
                    break;
                }
                if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                    func_80037870( 1, 0 );
                    op_pakState = 2;
                    D_80066C44 = 1;
                }
                if ( D_800E4D62 & B_BUTTON ) {
                    func_80037870( 2, 0 );
                    op_pakState = 0;
                }
                if ( D_800E6758 & U_JPAD ) {
                    if ( op_cursorLine != 0 ) {
                        func_80037870( 0, 0 );
                        op_cursorLine--;
                    } else if ( op_firstNote != 0 ) {
                        func_80037870( 0, 0 );
                        op_firstNote--;
                    }
                }
                if ( D_800E6758 & D_JPAD ) {
                    if ( op_cursorLine + op_firstNote < op_numNotes - 1 ) {
                        func_80037870( 0, 0 );
                        if ( op_cursorLine < PAK_NOTES_SHOWN - 1 ) {
                            op_cursorLine++;
                        } else {
                            op_firstNote++;
                        }
                    }
                }
                func_8002DF3C( op_cursorImages[D_8005B0F4 / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                func_80030058( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
                break;
            case 2:
                func_8002DF3C( op_cursorImages[D_8005B0F4 / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                func_80030058( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
                D_800DF590[0] = D_800DF590[1] = 0x24;
                D_800E6538[0] = D_800E6538[1] = 0x10;
                D_800E57E0[0] = 0x24;
                D_800E4D68[0] = 0x1E;
                D_800E57E0[1] = 0x4E;
                D_800E4D68[1] = 0x1E;
                func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                if ( func_80036E60( 0x94B, PROMPT_CHOICE2_H ) ) {
                    if ( D_80066C44 == 0 ) {
                        func_80037870( 8, 0 );
                        op_pakMsg = 10;
                        D_800E6584 = op_noteIndex[op_firstNote + op_cursorLine];
                        osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
                        osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
                        if ( D_80066B8C != 0 ) {
                            op_pakState = 3;
                        } else {
                            memcpy( D_800F7C78, D_80066C00.note.game_name, 16 );
                            memcpy( D_80066E10, D_80066C00.note.ext_name, 4 );
                            op_pakMsg = 1;
                            D_8009D090 = D_80066C00.note.company_code;
                            D_8009CF6C = D_80066C00.note.game_code;
                            osSendMesg( &D_800E6690, &op_pakMsg, OS_MESG_NOBLOCK );
                            osRecvMesg( &D_800F8098, (OSMesg *)&op_pakMsg, OS_MESG_BLOCK );
                            if ( D_80066B8C == 0 ) {
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
                    if ( D_80066C44 == 1 ) {
                        op_pakState = 1;
                    }
                }
                break;
            case 3:
                func_8002DF3C( op_cursorImages[D_8005B0F4 / 5 % 3], 0x12, op_cursorLine * 18 + 0x48 );
                func_80030058( 0x25, op_cursorLine * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
                func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                if ( func_80036E60( 0x94A, PROMPT_TIMED ) ) {
                    op_pakState = 1;
                }
                break;
            }
        }
        if ( D_8005C0BF == 1 && D_8005C0C0 == 0xFF ) {
            D_8005C0BF = 2;
            D_8005C0C0 = 0x107;
            return FALSE;
        }
        break;
    }
    return TRUE;
}
