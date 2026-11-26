/* 6C470.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

static u8 op_checkStep = 0; /* step of OP_CheckSettings */
static s32 op_contMsg;      /* message to and from the controller thread */

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
over: saving is then on (save_enabled) with the settings loaded, or off.
================
*/
s32 OP_CheckSettings( void ) {
    s32 *checksum;

    Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
    switch ( op_checkStep ) {
    case 0:
        op_contMsg = CONT_REQ_MOTOR_INIT;
        osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
        op_checkStep = 1;
        break;
    case 1:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( cont_result == 0 ) {
                op_checkStep = 2;
            } else {
                op_checkStep = 5;
            }
        }
        break;
    case 2:
        if ( Save_ShowPrompt( 0x94C, PROMPT_TIMED ) ) {
            op_checkStep = 10;
            save_promptChoice = 0;
        }
        break;
    case 3:
    case 4:
        break;
    case 5:
        op_contMsg = CONT_REQ_INIT_PAK;
        cont_pakPort = 0;
        osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
        op_checkStep = 6;
        break;
    case 6:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( cont_result == 0 ) {
                op_contMsg = CONT_REQ_FIND_FILE;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 20;
            } else if ( cont_result == 10 ) {
                op_checkStep = 25;
            } else if ( cont_result == 2 ) {
                op_checkStep = 5;
            } else if ( cont_result == 1 ) {
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
        save_choiceWidth[0] = save_choiceWidth[1] = 0x24;
        save_choiceHeight[0] = save_choiceHeight[1] = 0x10;
        save_choiceX[0] = 0x2E;
        save_choiceY[0] = 0x1C;
        save_choiceX[1] = 0x58;
        save_choiceY[1] = 0x1C;
        if ( Save_ShowPrompt( 0x950, PROMPT_CHOICE2_H ) ) {
            if ( save_promptChoice == 0 ) {
                save_enabled = 0;
                return FALSE;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
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
            if ( cont_result == 0 ) {
                op_contMsg = CONT_REQ_READ_WRITE_FILE;
                cont_pakWrite = 0;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 35;
            } else {
                op_checkStep = 21;
            }
        }
        break;
    case 21:
        save_choiceWidth[0] = 0x24;
        save_choiceHeight[0] = 0x10;
        save_choiceWidth[1] = 0x24;
        save_choiceHeight[1] = 0x10;
        save_choiceX[0] = 0x36;
        save_choiceY[0] = 0x44;
        save_choiceX[1] = 0x5E;
        save_choiceY[1] = 0x44;
        if ( Save_ShowPrompt( 0x94F, PROMPT_CHOICE2_H ) ) {
            if ( save_promptChoice == 0 ) {
                op_contMsg = CONT_REQ_ALLOCATE_FILE;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 22;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
                op_checkStep = 10;
            }
        }
        break;
    case 22:
        if ( Save_ShowPrompt( 0x955, PROMPT_CLOSE ) ) {
            osRecvMesg( &cont_replyQueue, (OSMesg *)&op_contMsg, OS_MESG_BLOCK );
            if ( cont_result == 0 ) {
                /* the checksum word through a raw view, as Save_SaveSettings does: a member store sinks past the
                   stores below */
                checksum = (s32 *)save_dataPtr;
                *checksum = Save_Checksum();
                op_contMsg = CONT_REQ_READ_WRITE_FILE;
                cont_pakWrite = 1;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
                op_checkStep = 24;
            } else if ( cont_result == 7 || cont_result == 8 ) {
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
            if ( cont_result == 0 ) {
                save_enabled = 1;
                memcpy( D_800693E8, cont_pakPfs.id, sizeof( D_800693E8 ) );
                return FALSE;
            }
            op_checkStep = 27;
        }
        break;
    case 25:
        /* BUG: the rectangle arrays hold two entries; the third choice's stores land on the next
           globals (__osContLastCmd and D_800DF595, cont_held[0], scene_twoCycle; the height lands in unused bss) */
        save_choiceWidth[0] = 0x34;
        save_choiceHeight[0] = 0xE;
        save_choiceWidth[1] = 0x34;
        save_choiceHeight[1] = 0x10;
        save_choiceWidth[2] = 0x7A;
        save_choiceHeight[2] = 0x1C;
        save_choiceX[0] = 0xA;
        save_choiceY[0] = 0x36;
        save_choiceX[1] = 0x44;
        save_choiceY[1] = 0x36;
        save_choiceX[2] = 0x78;
        save_choiceY[2] = 0x36;
        if ( Save_ShowPrompt( 0x954, PROMPT_CHOICE3 ) ) {
            if ( save_promptChoice == 0 ) {
                save_promptChoice = 0;
                op_checkStep = 29;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
                op_checkStep = 15;
            }
            if ( save_promptChoice == 2 ) {
                save_enabled = 0;
                return FALSE;
            }
        }
        break;
    case 29:
        save_choiceWidth[1] = 0x24;
        save_choiceWidth[0] = 0x24;
        save_choiceHeight[1] = 0x10;
        save_choiceHeight[0] = 0x10;
        save_choiceX[0] = 0x24;
        save_choiceY[0] = 0x32;
        save_choiceX[1] = 0x4E;
        save_choiceY[1] = 0x32;
        if ( Save_ShowPrompt( 0xA5B, PROMPT_CHOICE2_H ) ) {
            if ( save_promptChoice == 0 ) {
                op_checkStep = 26;
                save_promptChoice = 0;
                op_contMsg = CONT_REQ_REPAIR_ID;
                osSendMesg( &cont_requestQueue, &op_contMsg, OS_MESG_NOBLOCK );
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
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
           globals (__osContLastCmd and D_800DF595, cont_held[0], scene_twoCycle; the height lands in unused bss) */
        save_choiceWidth[0] = 0x34;
        save_choiceHeight[0] = 0xE;
        save_choiceWidth[1] = 0x34;
        save_choiceHeight[1] = 0x10;
        save_choiceWidth[2] = 0x7A;
        save_choiceHeight[2] = 0x1C;
        save_choiceX[0] = 0xA;
        save_choiceY[0] = 0x36;
        save_choiceX[1] = 0x44;
        save_choiceY[1] = 0x36;
        save_choiceX[2] = 0x78;
        save_choiceY[2] = 0x36;
        if ( Save_ShowPrompt( 0xB59, PROMPT_CHOICE3 ) ) {
            if ( save_promptChoice == 0 ) {
                save_promptChoice = 0;
                op_checkStep = 29;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
                op_checkStep = 15;
            }
            if ( save_promptChoice == 2 ) {
                save_enabled = 0;
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
            if ( cont_result == 0 && save_dataPtr->checksum == Save_Checksum() ) {
                save_enabled = 1;
                memcpy( D_800693E8, cont_pakPfs.id, sizeof( D_800693E8 ) );
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
