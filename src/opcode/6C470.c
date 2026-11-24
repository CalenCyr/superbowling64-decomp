/* 6C470.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

/*
================
OP_Null

Does nothing.
================
*/
void OP_Null( void ) {
}

/*
================
OP_CheckSettings

Runs one frame of the settings check at boot, over a dimmed screen: asks the controller thread for the
Controller Pak and the saved settings, and prompts to load, create or repair them. Returns FALSE when it is
over: saving is then on (D_8005C0E0) with the settings loaded, or off.
================
*/
s32 OP_CheckSettings( void ) {
    s32 *checksum;

    Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
    switch ( op_checkStep ) {
    case 0:
        op_contMsg = 9;
        osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
        op_checkStep = 1;
        break;
    case 1:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                op_checkStep = 2;
            } else {
                op_checkStep = 5;
            }
        }
        break;
    case 2:
        if ( Save_ShowPrompt( 0x94C, PROMPT_TIMED ) ) {
            op_checkStep = 10;
            D_80066C44 = 0;
        }
        break;
    case 3:
    case 4:
        break;
    case 5:
        op_contMsg = 6;
        D_800E4FC8 = 0;
        osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
        op_checkStep = 6;
        break;
    case 6:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                op_contMsg = 2;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 20;
            } else if ( D_80066B8C == 10 ) {
                op_checkStep = 25;
            } else if ( D_80066B8C == 2 ) {
                op_checkStep = 5;
            } else if ( D_80066B8C == 1 ) {
                op_checkStep = 30;
            } else {
                op_checkStep = 28;
            }
        }
        break;
    case 7:
    case 8:
    case 9:
        break;
    case 10:
        D_800DF590[0] = D_800DF590[1] = 0x24;
        D_800E6538[0] = D_800E6538[1] = 0x10;
        D_800E57E0[0] = 0x2E;
        D_800E4D68[0] = 0x1C;
        D_800E57E0[1] = 0x58;
        D_800E4D68[1] = 0x1C;
        if ( Save_ShowPrompt( 0x950, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_8005C0E0 = 0;
                return FALSE;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                op_checkStep = 15;
            }
        }
        break;
    case 11:
    case 12:
    case 13:
    case 14:
        break;
    case 15:
        if ( Save_ShowPrompt( 0x95A, PROMPT_ANY_BUTTON ) ) {
            op_checkStep = 0;
        }
        break;
    case 16:
    case 17:
    case 18:
    case 19:
        break;
    case 20:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                op_contMsg = 5;
                D_800E6578 = 0;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 35;
            } else {
                op_checkStep = 21;
            }
        }
        break;
    case 21:
        D_800DF590[0] = 0x24;
        D_800E6538[0] = 0x10;
        D_800DF590[1] = 0x24;
        D_800E6538[1] = 0x10;
        D_800E57E0[0] = 0x36;
        D_800E4D68[0] = 0x44;
        D_800E57E0[1] = 0x5E;
        D_800E4D68[1] = 0x44;
        if ( Save_ShowPrompt( 0x94F, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                op_contMsg = 0;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 22;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                op_checkStep = 10;
            }
        }
        break;
    case 22:
        if ( Save_ShowPrompt( 0x955, PROMPT_CLOSE ) ) {
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                /* the checksum word through a raw view, as Save_SaveSettings does: a member store sinks past the
                   stores below */
                checksum = (s32 *)save_dataPtr;
                *checksum = Save_Checksum();
                op_contMsg = 5;
                D_800E6578 = 1;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 24;
            } else if ( D_80066B8C == 7 || D_80066B8C == 8 ) {
                op_checkStep = 23;
            } else {
                op_checkStep = 27;
            }
        }
        break;
    case 23:
        if ( Save_ShowPrompt( 0x94E, PROMPT_TIMED_LONG ) ) {
            op_checkStep = 10;
        }
        break;
    case 24:
        if ( Save_ShowPrompt( 0x95C, PROMPT_CLOSE ) ) {
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                D_8005C0E0 = 1;
                memcpy( D_800693E8, D_80066BA4, sizeof( D_800693E8 ) );
                return FALSE;
            }
            op_checkStep = 27;
        }
        break;
    case 25:
        /* BUG: the rectangle arrays hold two entries; the third choice's stores land on the next
           globals (__osContLastCmd and D_800DF595, D_800E57E4[0], D_800E4D6C; the height lands in unused bss) */
        D_800DF590[0] = 0x34;
        D_800E6538[0] = 0xE;
        D_800DF590[1] = 0x34;
        D_800E6538[1] = 0x10;
        D_800DF590[2] = 0x7A;
        D_800E6538[2] = 0x1C;
        D_800E57E0[0] = 0xA;
        D_800E4D68[0] = 0x36;
        D_800E57E0[1] = 0x44;
        D_800E4D68[1] = 0x36;
        D_800E57E0[2] = 0x78;
        D_800E4D68[2] = 0x36;
        if ( Save_ShowPrompt( 0x954, PROMPT_CHOICE3 ) ) {
            if ( D_80066C44 == 0 ) {
                D_80066C44 = 0;
                op_checkStep = 29;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                op_checkStep = 15;
            }
            if ( D_80066C44 == 2 ) {
                D_8005C0E0 = 0;
                return FALSE;
            }
        }
        break;
    case 29:
        D_800DF590[1] = 0x24;
        D_800DF590[0] = 0x24;
        D_800E6538[1] = 0x10;
        D_800E6538[0] = 0x10;
        D_800E57E0[0] = 0x24;
        D_800E4D68[0] = 0x32;
        D_800E57E0[1] = 0x4E;
        D_800E4D68[1] = 0x32;
        if ( Save_ShowPrompt( 0xA5B, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                op_checkStep = 26;
                D_80066C44 = 0;
                op_contMsg = 7;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                op_checkStep = 15;
            }
        }
        break;
    case 26:
        if ( Save_ShowPrompt( 0x953, PROMPT_CLOSE ) ) {
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_BLOCK );
            op_checkStep = 0;
        }
        break;
    case 27:
        if ( Save_ShowPrompt( 0x94D, PROMPT_TIMED ) ) {
            op_checkStep = 10;
        }
        break;
    case 28:
        /* BUG: the rectangle arrays hold two entries; the third choice's stores land on the next
           globals (__osContLastCmd and D_800DF595, D_800E57E4[0], D_800E4D6C; the height lands in unused bss) */
        D_800DF590[0] = 0x34;
        D_800E6538[0] = 0xE;
        D_800DF590[1] = 0x34;
        D_800E6538[1] = 0x10;
        D_800DF590[2] = 0x7A;
        D_800E6538[2] = 0x1C;
        D_800E57E0[0] = 0xA;
        D_800E4D68[0] = 0x36;
        D_800E57E0[1] = 0x44;
        D_800E4D68[1] = 0x36;
        D_800E57E0[2] = 0x78;
        D_800E4D68[2] = 0x36;
        if ( Save_ShowPrompt( 0xB59, PROMPT_CHOICE3 ) ) {
            if ( D_80066C44 == 0 ) {
                D_80066C44 = 0;
                op_checkStep = 29;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                op_checkStep = 15;
            }
            if ( D_80066C44 == 2 ) {
                D_8005C0E0 = 0;
                return FALSE;
            }
        }
        break;
    case 30:
        if ( Save_ShowPrompt( 0x956, PROMPT_TIMED ) ) {
            op_checkStep = 10;
        }
        break;
    case 31:
    case 32:
    case 33:
    case 34:
        break;
    case 35:
        if ( Save_ShowPrompt( 0x951, PROMPT_CLOSE ) ) {
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 && save_dataPtr->checksum == Save_Checksum() ) {
                D_8005C0E0 = 1;
                memcpy( D_800693E8, D_80066BA4, sizeof( D_800693E8 ) );
                return FALSE;
            }
            op_checkStep = 36;
        }
        break;
    case 36:
        if ( Save_ShowPrompt( 0x952, PROMPT_TIMED ) ) {
            op_checkStep = 10;
        }
        break;
    }
    return TRUE;
}
