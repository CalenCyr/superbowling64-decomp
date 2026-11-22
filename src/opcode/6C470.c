/* 6C470.c -- opcode overlay */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "save.h"
#include "opcode/opcode.h"

/*
================
func_80112950_opcode

Does nothing.
================
*/
void func_80112950_opcode( void ) {
}

/*
================
func_80112958_opcode

Runs one frame of the settings check at boot, over a dimmed screen: asks the controller thread for the
Controller Pak and the saved settings, and prompts to load, create or repair them. Returns FALSE when it is
over: saving is then on (D_8005C0E0) with the settings loaded, or off.
================
*/
s32 func_80112958_opcode( void ) {
    s32 *checksum;

    func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
    switch ( D_801140B0_opcode ) {
    case 0:
        D_80114360_opcode = 9;
        osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
        D_801140B0_opcode = 1;
        break;
    case 1:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                D_801140B0_opcode = 2;
            } else {
                D_801140B0_opcode = 5;
            }
        }
        break;
    case 2:
        if ( func_80036E60( 0x94C, PROMPT_TIMED ) ) {
            D_801140B0_opcode = 10;
            D_80066C44 = 0;
        }
        break;
    case 3:
    case 4:
        break;
    case 5:
        D_80114360_opcode = 6;
        D_800E4FC8 = 0;
        osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
        D_801140B0_opcode = 6;
        break;
    case 6:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                D_80114360_opcode = 2;
                osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
                D_801140B0_opcode = 20;
            } else if ( D_80066B8C == 10 ) {
                D_801140B0_opcode = 25;
            } else if ( D_80066B8C == 2 ) {
                D_801140B0_opcode = 5;
            } else if ( D_80066B8C == 1 ) {
                D_801140B0_opcode = 30;
            } else {
                D_801140B0_opcode = 28;
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
        if ( func_80036E60( 0x950, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_8005C0E0 = 0;
                return FALSE;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 15;
            }
        }
        break;
    case 11:
    case 12:
    case 13:
    case 14:
        break;
    case 15:
        if ( func_80036E60( 0x95A, PROMPT_ANY_BUTTON ) ) {
            D_801140B0_opcode = 0;
        }
        break;
    case 16:
    case 17:
    case 18:
    case 19:
        break;
    case 20:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                D_80114360_opcode = 5;
                D_800E6578 = 0;
                osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
                D_801140B0_opcode = 35;
            } else {
                D_801140B0_opcode = 21;
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
        if ( func_80036E60( 0x94F, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_80114360_opcode = 0;
                osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
                D_801140B0_opcode = 22;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 10;
            }
        }
        break;
    case 22:
        if ( func_80036E60( 0x955, PROMPT_CLOSE ) ) {
            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                /* the checksum word through a raw view, as func_80037410 does: a member store sinks past the stores below */
                checksum = (s32 *)D_801000B8;
                *checksum = func_800373DC();
                D_80114360_opcode = 5;
                D_800E6578 = 1;
                osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
                D_801140B0_opcode = 24;
            } else if ( D_80066B8C == 7 || D_80066B8C == 8 ) {
                D_801140B0_opcode = 23;
            } else {
                D_801140B0_opcode = 27;
            }
        }
        break;
    case 23:
        if ( func_80036E60( 0x94E, PROMPT_TIMED_LONG ) ) {
            D_801140B0_opcode = 10;
        }
        break;
    case 24:
        if ( func_80036E60( 0x95C, PROMPT_CLOSE ) ) {
            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                D_8005C0E0 = 1;
                memcpy( D_800693E8, D_80066BA4, sizeof( D_800693E8 ) );
                return FALSE;
            }
            D_801140B0_opcode = 27;
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
        if ( func_80036E60( 0x954, PROMPT_CHOICE3 ) ) {
            if ( D_80066C44 == 0 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 29;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 15;
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
        if ( func_80036E60( 0xA5B, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_801140B0_opcode = 26;
                D_80066C44 = 0;
                D_80114360_opcode = 7;
                osSendMesg( &D_800E6690, &D_80114360_opcode, OS_MESG_NOBLOCK );
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 15;
            }
        }
        break;
    case 26:
        if ( func_80036E60( 0x953, PROMPT_CLOSE ) ) {
            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_BLOCK );
            D_801140B0_opcode = 0;
        }
        break;
    case 27:
        if ( func_80036E60( 0x94D, PROMPT_TIMED ) ) {
            D_801140B0_opcode = 10;
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
        if ( func_80036E60( 0xB59, PROMPT_CHOICE3 ) ) {
            if ( D_80066C44 == 0 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 29;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_801140B0_opcode = 15;
            }
            if ( D_80066C44 == 2 ) {
                D_8005C0E0 = 0;
                return FALSE;
            }
        }
        break;
    case 30:
        if ( func_80036E60( 0x956, PROMPT_TIMED ) ) {
            D_801140B0_opcode = 10;
        }
        break;
    case 31:
    case 32:
    case 33:
    case 34:
        break;
    case 35:
        if ( func_80036E60( 0x951, PROMPT_CLOSE ) ) {
            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114360_opcode, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 && D_801000B8->unk_00 == func_800373DC() ) {
                D_8005C0E0 = 1;
                memcpy( D_800693E8, D_80066BA4, sizeof( D_800693E8 ) );
                return FALSE;
            }
            D_801140B0_opcode = 36;
        }
        break;
    case 36:
        if ( func_80036E60( 0x952, PROMPT_TIMED ) ) {
            D_801140B0_opcode = 10;
        }
        break;
    }
    return TRUE;
}
