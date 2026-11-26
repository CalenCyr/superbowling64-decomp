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
Save_ShowPrompt

Shows the prompt image id, in the way of mode, for one frame: it zooms in, runs, then zooms out. Modes
PROMPT_CHOICE2_H to PROMPT_CHOICE3 move a blinking cursor over the choices, save_promptChoice, until A or Z.
Returns TRUE in the frame it has closed, or once open in mode PROMPT_OPEN.
================
*/
s32 Save_ShowPrompt( s32 id, s32 mode ) {
    f32 scale;
    s32 x;
    s32 y;
    s32 done;
    fileImage_t *image;

    done = FALSE;
    switch ( save_promptState ) {
    case PROMPT_CLOSED:
        save_promptFrame = 1;
        save_promptState = PROMPT_OPENING;
        break;
    case PROMPT_RUNNING:
        switch ( mode ) {
        case PROMPT_TIMED:
            if ( ++save_promptFrame == 90 || cont_allPressed != 0 ) {
                save_promptFrame = PROMPT_ZOOM_FRAMES;
                save_promptState = PROMPT_CLOSING;
            }
            break;
        case PROMPT_ANY_BUTTON:
            if ( cont_allPressed != 0 ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                save_promptState = PROMPT_CLOSING;
            }
            break;
        case PROMPT_OPEN:
            break;
        case PROMPT_CLOSE:
            save_promptState = PROMPT_CLOSING;
            break;
        case PROMPT_CHOICE2_H:
            if ( save_promptChoice == 0 && ( cont_allDpadRepeat & R_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 1;
            }
            if ( save_promptChoice == 1 && ( cont_allDpadRepeat & L_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 0;
            }
            if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                save_promptState = PROMPT_CLOSING;
            }
            break;
        case PROMPT_CHOICE2_V:
            if ( save_promptChoice == 0 && ( cont_allDpadRepeat & D_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 1;
            }
            if ( save_promptChoice == 1 && ( cont_allDpadRepeat & U_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 0;
            }
            if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                save_promptState = PROMPT_CLOSING;
            }
            break;
        case PROMPT_CHOICE3:
            if ( save_promptChoice == 0 && ( cont_allDpadRepeat & R_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 1;
            } else if ( save_promptChoice == 1 && ( cont_allDpadRepeat & L_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 0;
            } else if ( save_promptChoice == 1 && ( cont_allDpadRepeat & R_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 2;
            } else if ( save_promptChoice == 2 && ( cont_allDpadRepeat & L_JPAD ) ) {
                Sound_PlaySe( SE_CURSOR, 0 );
                save_promptChoice = 1;
            }
            if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                save_promptState = PROMPT_CLOSING;
            }
            break;
        case PROMPT_TIMED_LONG:
            if ( ++save_promptFrame == 270 || cont_allPressed != 0 ) {
                save_promptFrame = PROMPT_ZOOM_FRAMES;
                save_promptState = PROMPT_CLOSING;
            }
            break;
        }
        break;
    case PROMPT_OPENING:
        if ( ++save_promptFrame == PROMPT_ZOOM_FRAMES ) {
            save_promptState = PROMPT_RUNNING;
            save_promptChoice = 0;
            if ( mode == PROMPT_OPEN ) {
                done = TRUE;
            }
        }
        break;
    case PROMPT_CLOSING:
        if ( --save_promptFrame == 1 ) {
            save_promptState = PROMPT_CLOSED;
            done = TRUE;
        }
        break;
    }

    if ( save_promptFrame < PROMPT_ZOOM_FRAMES ) {
        scale = save_promptZoom[save_promptFrame];
    } else {
        scale = 1.0f;
    }
    if ( cache_files[id].refCount == 0 ) {
        Cache_GetFile( id, FILE_RAW );
    }
    image = cache_files[id].data;
    x = ( SCREEN_WIDTH - image->width * scale ) / 2.0f;
    y = ( 200 - image->height ) / 2;
    Draw_ImageScaled( id, x, y, 0xFF, scale, 1.0f );

    if ( save_promptState == PROMPT_RUNNING && mode >= PROMPT_CHOICE2_H ) {
        if ( mode < PROMPT_TIMED_LONG ) {
            Draw_FillRect( save_choiceX[save_promptChoice] + x, save_choiceY[save_promptChoice] + y,
                           save_choiceWidth[save_promptChoice], save_choiceHeight[save_promptChoice], 0, 0xFF, 0,
                           save_cursorAlpha );
            save_cursorAlpha += save_cursorStep;
            if ( save_cursorAlpha == 0x80 || save_cursorAlpha == 0x40 ) {
                save_cursorStep *= -1;
            }
        }
    }
    return done;
}

/*
================
Save_Checksum

Returns the checksum of the settings save_dataPtr: the sum of their bytes after the checksum word itself.
================
*/
s32 Save_Checksum( void ) {
    u8 *bytes;
    s32 sum;
    s32 i;

    bytes = (u8 *)save_dataPtr;
    sum = 0;
    for ( i = 4; i < 0x100; i++ ) {
        sum += bytes[i];
    }
    return sum;
}

/*
================
Save_SaveSettings

Runs one frame of saving the settings, over a dimmed screen: asks the controller thread to check the
Controller Pak, prompts, writes the settings with their checksum, and reports the result. Returns FALSE when
it is over, or when saving is off.
================
*/
s32 Save_SaveSettings( void ) {
    u8 *bytes;
    s32 sum;
    s32 i;

    if ( save_enabled == 0 ) {
        return FALSE;
    }
    Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
    switch ( save_step ) {
    case 0:
        save_contMsg = CONT_REQ_MOTOR_INIT;
        osSendMesg( &cont_requestQueue, &save_contMsg, OS_MESG_NOBLOCK );
        save_step = 1;
        break;
    case 1:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&save_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( cont_result == 0 ) {
                save_step = 2;
            } else {
                save_step = 3;
            }
        }
        break;
    case 2:
        if ( Save_ShowPrompt( 0x94C, PROMPT_TIMED ) ) {
            save_step = 10;
            save_promptChoice = 0;
        }
        break;
    case 3:
        save_contMsg = CONT_REQ_INIT_PAK;
        D_800E4FC8 = 0;
        osSendMesg( &cont_requestQueue, &save_contMsg, OS_MESG_NOBLOCK );
        save_step = 4;
        break;
    case 4:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&save_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( cont_result == 0 ) {
                if ( memcmp( D_800693E8, cont_pakPfs.id, sizeof( D_800693E8 ) ) == 0 ) {
                    save_step = 5;
                } else {
                    save_step = 8;
                }
            } else if ( cont_result == 1 ) {
                save_step = 10;
            } else {
                save_step = 11;
            }
        }
        break;
    case 5:
        bytes = (u8 *)save_dataPtr;
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
        save_contMsg = CONT_REQ_READ_WRITE_FILE;
        D_800E6578 = 1;
        osSendMesg( &cont_requestQueue, &save_contMsg, OS_MESG_NOBLOCK );
        save_step = 6;
        break;
    case 6:
        if ( osRecvMesg( &cont_replyQueue, (OSMesg *)&save_contMsg, OS_MESG_NOBLOCK ) != -1 ) {
            if ( cont_result == 0 ) {
                save_step = 7;
            } else if ( cont_result == 2 ) {
                save_step = 8;
            } else if ( cont_result == 1 ) {
                save_step = 10;
            } else {
                save_step = 11;
            }
        }
        Save_ShowPrompt( 0x95C, PROMPT_OPEN );
        break;
    case 7:
        if ( Save_ShowPrompt( 0x95C, PROMPT_CLOSE ) ) {
            save_step = 0;
            return FALSE;
        }
        break;
    case 8:
        if ( Save_ShowPrompt( 0x959, PROMPT_TIMED ) ) {
            save_step = 9;
        }
        break;
    case 9:
        if ( Save_ShowPrompt( 0x95A, PROMPT_ANY_BUTTON ) ) {
            save_step = 0;
        }
        break;
    case 10:
        save_choiceWidth[0] = 0x2E;
        save_choiceWidth[1] = 0x36;
        save_choiceHeight[0] = save_choiceHeight[1] = 0x10;
        save_choiceX[0] = 0x20;
        save_choiceY[0] = 0x2A;
        save_choiceX[1] = 0x6E;
        save_choiceY[1] = 0x2A;
        if ( Save_ShowPrompt( 0x95B, PROMPT_CHOICE2_H ) ) {
            if ( save_promptChoice == 0 ) {
                save_step = 9;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
                save_step = 0;
                return FALSE;
            }
        }
        break;
    case 11:
        save_choiceWidth[0] = save_choiceWidth[1] = 0x24;
        save_choiceHeight[0] = save_choiceHeight[1] = 0x10;
        save_choiceX[0] = 0x40;
        save_choiceY[0] = 0x2A;
        save_choiceX[1] = 0x66;
        save_choiceY[1] = 0x2A;
        if ( Save_ShowPrompt( 0x958, PROMPT_CHOICE2_H ) ) {
            if ( save_promptChoice == 0 ) {
                save_step = 9;
            }
            if ( save_promptChoice == 1 ) {
                save_promptChoice = 0;
                save_step = 0;
                return FALSE;
            }
        }
        break;
    }
    return TRUE;
}
