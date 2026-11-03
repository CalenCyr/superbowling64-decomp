/* 12260.c -- saving the settings, and the game's yes/no prompts */

#include "common.h"
#include "libultra.h"
#include "libc.h"
#include "boot.h"
#include "filecache.h"
#include "gamesnd.h"
#include "input.h"
#include "sprite.h"
#include "save.h"

/*
================
func_80036E60

Shows the prompt image id, in the way of mode, for one frame: it zooms in, runs, then zooms out. Modes
PROMPT_CHOICE2_H to PROMPT_CHOICE3 move a blinking cursor over the choices, D_80066C44, until A or Z.
Returns TRUE in the frame it has closed, or once open in mode PROMPT_OPEN.
================
*/
s32 func_80036E60( s32 id, s32 mode ) {
    f32 scale;
    s32 x;
    s32 y;
    s32 done;
    fileImage_t *image;

    done = FALSE;
    switch ( D_8005C9BC ) {
    case PROMPT_CLOSED:
        D_80064310 = 1;
        D_8005C9BC = PROMPT_OPENING;
        break;
    case PROMPT_RUNNING:
        switch ( mode ) {
        case PROMPT_TIMED:
            if ( ++D_80064310 == 90 || D_800E4D62 != 0 ) {
                D_80064310 = PROMPT_ZOOM_FRAMES;
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        case PROMPT_ANY_BUTTON:
            if ( D_800E4D62 != 0 ) {
                func_80037870( SE_DECIDE, 0 );
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        case PROMPT_OPEN:
            break;
        case PROMPT_CLOSE:
            D_8005C9BC = PROMPT_CLOSING;
            break;
        case PROMPT_CHOICE2_H:
            if ( D_80066C44 == 0 && ( D_800E6758 & R_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 1;
            }
            if ( D_80066C44 == 1 && ( D_800E6758 & L_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 0;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                func_80037870( SE_DECIDE, 0 );
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        case PROMPT_CHOICE2_V:
            if ( D_80066C44 == 0 && ( D_800E6758 & D_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 1;
            }
            if ( D_80066C44 == 1 && ( D_800E6758 & U_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 0;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                func_80037870( SE_DECIDE, 0 );
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        case PROMPT_CHOICE3:
            if ( D_80066C44 == 0 && ( D_800E6758 & R_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 1;
            } else if ( D_80066C44 == 1 && ( D_800E6758 & L_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 0;
            } else if ( D_80066C44 == 1 && ( D_800E6758 & R_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 2;
            } else if ( D_80066C44 == 2 && ( D_800E6758 & L_JPAD ) ) {
                func_80037870( SE_CURSOR, 0 );
                D_80066C44 = 1;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                func_80037870( SE_DECIDE, 0 );
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        case PROMPT_TIMED_LONG:
            if ( ++D_80064310 == 270 || D_800E4D62 != 0 ) {
                D_80064310 = PROMPT_ZOOM_FRAMES;
                D_8005C9BC = PROMPT_CLOSING;
            }
            break;
        }
        break;
    case PROMPT_OPENING:
        if ( ++D_80064310 == PROMPT_ZOOM_FRAMES ) {
            D_8005C9BC = PROMPT_RUNNING;
            D_80066C44 = 0;
            if ( mode == PROMPT_OPEN ) {
                done = TRUE;
            }
        }
        break;
    case PROMPT_CLOSING:
        if ( --D_80064310 == 1 ) {
            D_8005C9BC = PROMPT_CLOSED;
            done = TRUE;
        }
        break;
    }

    if ( D_80064310 < PROMPT_ZOOM_FRAMES ) {
        scale = D_8005C994[D_80064310];
    } else {
        scale = 1.0f;
    }
    if ( D_800F80B0[id].refCount == 0 ) {
        func_80030C04( id, FILE_RAW );
    }
    image = D_800F80B0[id].data;
    x = ( SCREEN_WIDTH - image->width * scale ) / 2.0f;
    y = ( 200 - image->height ) / 2;
    func_8002E4B0( id, x, y, 0xFF, scale, 1.0f );

    if ( D_8005C9BC == PROMPT_RUNNING && mode >= PROMPT_CHOICE2_H ) {
        if ( mode < PROMPT_TIMED_LONG ) {
            func_8002FDE4( D_800E57E0[D_80066C44] + x, D_800E4D68[D_80066C44] + y, D_800DF590[D_80066C44],
                           D_800E6538[D_80066C44], 0, 0xFF, 0, D_8005C9C4 );
            D_8005C9C4 += D_8005C9C5;
            if ( D_8005C9C4 == 0x80 || D_8005C9C4 == 0x40 ) {
                D_8005C9C5 *= -1;
            }
        }
    }
    return done;
}

/*
================
func_800373DC

Returns the checksum of the settings D_801000B8: the sum of their bytes after the checksum word itself.
================
*/
s32 func_800373DC( void ) {
    u8 *bytes;
    s32 sum;
    s32 i;

    bytes = (u8 *)D_801000B8;
    sum = 0;
    for ( i = 4; i < 0x100; i++ ) {
        sum += bytes[i];
    }
    return sum;
}

/*
================
func_80037410

Runs one frame of saving the settings, over a dimmed screen: asks the controller thread to check the
Controller Pak, prompts, writes the settings with their checksum, and reports the result. Returns FALSE when
it is over, or when saving is off.
================
*/
s32 func_80037410( void ) {
    u8 *bytes;
    s32 sum;
    s32 i;

    if ( D_8005C0E0 == 0 ) {
        return FALSE;
    }
    func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
    switch ( D_8005C990 ) {
    case 0:
        D_80064314 = 9;
        osSendMesg( &D_800E6690, &D_80064314, OS_MESG_NOBLOCK );
        D_8005C990 = 1;
        break;
    case 1:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80064314, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                D_8005C990 = 2;
            } else {
                D_8005C990 = 3;
            }
        }
        break;
    case 2:
        if ( func_80036E60( 0x94C, PROMPT_TIMED ) ) {
            D_8005C990 = 10;
            D_80066C44 = 0;
        }
        break;
    case 3:
        D_80064314 = 6;
        D_800E4FC8 = 0;
        osSendMesg( &D_800E6690, &D_80064314, OS_MESG_NOBLOCK );
        D_8005C990 = 4;
        break;
    case 4:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80064314, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                if ( memcmp( D_800693E8, D_80066BA4, sizeof( D_800693E8 ) ) == 0 ) {
                    D_8005C990 = 5;
                } else {
                    D_8005C990 = 8;
                }
            } else if ( D_80066B8C == 1 ) {
                D_8005C990 = 10;
            } else {
                D_8005C990 = 11;
            }
        }
        break;
    case 5:
        bytes = (u8 *)D_801000B8;
        sum = 0;
        for ( i = 4; i < 0x100; i++ ) {
            sum += bytes[i];
        }
        /*
        The checksum word goes back through the same raw view of the settings. A store to the struct member
        (settings->unk_00) is not what the original wrote: gcc 2.7.2 lets a struct-member store move past the
        global stores below (MEM_IN_STRUCT_P), and retail keeps it first.
        */
        *(s32 *)bytes = sum;
        D_80064314 = 5;
        D_800E6578 = 1;
        osSendMesg( &D_800E6690, &D_80064314, OS_MESG_NOBLOCK );
        D_8005C990 = 6;
        break;
    case 6:
        if ( osRecvMesg( &D_800F8098, (OSMesg *)&D_80064314, OS_MESG_NOBLOCK ) != -1 ) {
            if ( D_80066B8C == 0 ) {
                D_8005C990 = 7;
            } else if ( D_80066B8C == 2 ) {
                D_8005C990 = 8;
            } else if ( D_80066B8C == 1 ) {
                D_8005C990 = 10;
            } else {
                D_8005C990 = 11;
            }
        }
        func_80036E60( 0x95C, PROMPT_OPEN );
        break;
    case 7:
        if ( func_80036E60( 0x95C, PROMPT_CLOSE ) ) {
            D_8005C990 = 0;
            return FALSE;
        }
        break;
    case 8:
        if ( func_80036E60( 0x959, PROMPT_TIMED ) ) {
            D_8005C990 = 9;
        }
        break;
    case 9:
        if ( func_80036E60( 0x95A, PROMPT_ANY_BUTTON ) ) {
            D_8005C990 = 0;
        }
        break;
    case 10:
        D_800DF590[0] = 0x2E;
        D_800DF590[1] = 0x36;
        D_800E6538[0] = D_800E6538[1] = 0x10;
        D_800E57E0[0] = 0x20;
        D_800E4D68[0] = 0x2A;
        D_800E57E0[1] = 0x6E;
        D_800E4D68[1] = 0x2A;
        if ( func_80036E60( 0x95B, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_8005C990 = 9;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_8005C990 = 0;
                return FALSE;
            }
        }
        break;
    case 11:
        D_800DF590[0] = D_800DF590[1] = 0x24;
        D_800E6538[0] = D_800E6538[1] = 0x10;
        D_800E57E0[0] = 0x40;
        D_800E4D68[0] = 0x2A;
        D_800E57E0[1] = 0x66;
        D_800E4D68[1] = 0x2A;
        if ( func_80036E60( 0x958, PROMPT_CHOICE2_H ) ) {
            if ( D_80066C44 == 0 ) {
                D_8005C990 = 9;
            }
            if ( D_80066C44 == 1 ) {
                D_80066C44 = 0;
                D_8005C990 = 0;
                return FALSE;
            }
        }
        break;
    }
    return TRUE;
}
