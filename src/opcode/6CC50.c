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
    return D_801140C0_opcode[code - 0x10];
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
    switch ( D_801142A2_opcode ) {
    case 0:
        D_80114370_opcode = 6;
        D_800E4FC8 = 0;
        osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        D_80114374_opcode = 0;
        for ( i = 0; i < PAK_MAX_NOTES; i++ ) {
            D_80114370_opcode = 10;
            D_800E6584 = i;
            osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
            if ( D_80066B8C == 0 ) {
                n = D_80114374_opcode;
                D_80114378_opcode[n] = i;
                D_80114398_opcode[n] = ( D_80066C00.note.file_size + 0xFF ) >> 8;
                for ( j = 0; j < 16; j++ ) {
                    D_801143B8_opcode[n].name[j] = func_80113130_opcode( D_80066C00.note.game_name[j] );
                }
                D_801143B8_opcode[D_80114374_opcode].ext = func_80113130_opcode( D_80066C00.note.ext_name[0] );
                D_801143B8_opcode[D_80114374_opcode].end = 0xFFFF;
                D_80114374_opcode++;
            } else if ( D_80066B8C != 5 ) {
                return FALSE;
            }
        }
        D_80114370_opcode = 3;
        osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        D_80114370_opcode = 4;
        D_801145F8_opcode = D_80066C28 - D_800692D4;
        osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
        osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
        if ( D_80066B8C != 0 ) {
            return FALSE;
        }
        D_801145FC_opcode = D_8009944C / 256;
        D_80114376_opcode = 0;
        D_801142A2_opcode = 2;
        break;
    case 2:
        func_8002DF3C( 0xA58, 0, 0 );
        if ( D_801142A5_opcode == 0 ) {
            func_8002C950( D_801142A3_opcode == 0 ? 0x40 : 0xB0, 0xC0, 0x50, 0x10, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
        }
        for ( i = 0; D_801141CC_opcode[i].id != -1; i++ ) {
            if ( D_801141CC_opcode[i].alpha ) {
                func_8002E4B0( D_801141CC_opcode[i].id, D_801141CC_opcode[i].x, D_801141CC_opcode[i].y,
                               D_801141CC_opcode[i].alpha, 1.0f, 1.0f );
            }
        }
        if ( D_80114376_opcode != 0 ) {
            func_8002E4B0( 0x888, 0x98, 0x38, D_8009CFB0, 1.0f, 1.0f );
        }
        if ( D_80114374_opcode - D_80114376_opcode >= 6 ) {
            func_8002E4B0( 0x889, 0x98, 0xA4, D_8009CFB0, 1.0f, 1.0f );
        }
        func_80028EA8( &D_800F8094 );
        for ( i = 0; i < PAK_NOTES_SHOWN; i++ ) {
            if ( i + D_80114376_opcode < D_80114374_opcode ) {
                func_800292D4( glyphs, i + D_80114376_opcode + 1, 2 );
                func_80029088( &D_800F8094, 0x28, 0x49 + i * 18, 8, glyphs );
                func_80029088( &D_800F8094, 0x3C, 0x49 + i * 18, 8, D_801143B8_opcode[i + D_80114376_opcode].name );
                func_800292D4( glyphs, D_80114398_opcode[i + D_80114376_opcode], 4 );
                func_80029088( &D_800F8094, 0xF9, 0x49 + i * 18, 8, glyphs );
            }
        }
        i = D_801145F8_opcode < 10;
        func_800292D4( glyphs, D_801145F8_opcode, 2 );
        func_80029088( &D_800F8094, 0x74 + i * 8, 0xA9, 8, glyphs );
        if ( D_801145FC_opcode >= 100 ) {
            i = 0;
        } else if ( D_801145FC_opcode >= 10 ) {
            i = 1;
        } else {
            i = 2;
        }
        func_800292D4( glyphs, D_801145FC_opcode, 3 );
        func_80029088( &D_800F8094, 0xFE + i * 8, 0xA9, 8, glyphs );
        if ( D_8005C0BF == 0 ) {
            switch ( D_801142A5_opcode ) {
            case 0:
                if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                    func_80037870( 1, 0 );
                    if ( D_801142A3_opcode == 0 ) {
                        D_801142A5_opcode = 1;
                    } else {
                        D_8005C0BF = 1;
                    }
                }
                if ( ( D_800E6758 & L_JPAD ) && D_801142A3_opcode == 1 ) {
                    D_801142A3_opcode = 0;
                    func_80037870( 0, 0 );
                }
                if ( ( D_800E6758 & R_JPAD ) && D_801142A3_opcode == 0 ) {
                    D_801142A3_opcode = 1;
                    func_80037870( 0, 0 );
                }
                break;
            case 1:
                if ( D_80114374_opcode == 0 ) {
                    D_801142A5_opcode = 0;
                    break;
                }
                if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                    func_80037870( 1, 0 );
                    D_801142A5_opcode = 2;
                    D_80066C44 = 1;
                }
                if ( D_800E4D62 & B_BUTTON ) {
                    func_80037870( 2, 0 );
                    D_801142A5_opcode = 0;
                }
                if ( D_800E6758 & U_JPAD ) {
                    if ( D_801142A4_opcode != 0 ) {
                        func_80037870( 0, 0 );
                        D_801142A4_opcode--;
                    } else if ( D_80114376_opcode != 0 ) {
                        func_80037870( 0, 0 );
                        D_80114376_opcode--;
                    }
                }
                if ( D_800E6758 & D_JPAD ) {
                    if ( D_801142A4_opcode + D_80114376_opcode < D_80114374_opcode - 1 ) {
                        func_80037870( 0, 0 );
                        if ( D_801142A4_opcode < PAK_NOTES_SHOWN - 1 ) {
                            D_801142A4_opcode++;
                        } else {
                            D_80114376_opcode++;
                        }
                    }
                }
                func_8002DF3C( D_8011429C_opcode[D_8005B0F4 / 5 % 3], 0x12, D_801142A4_opcode * 18 + 0x48 );
                func_80030058( 0x25, D_801142A4_opcode * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
                break;
            case 2:
                func_8002DF3C( D_8011429C_opcode[D_8005B0F4 / 5 % 3], 0x12, D_801142A4_opcode * 18 + 0x48 );
                func_80030058( 0x25, D_801142A4_opcode * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
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
                        D_80114370_opcode = 10;
                        D_800E6584 = D_80114378_opcode[D_80114376_opcode + D_801142A4_opcode];
                        osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
                        osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
                        if ( D_80066B8C != 0 ) {
                            D_801142A5_opcode = 3;
                        } else {
                            memcpy( D_800F7C78, D_80066C00.note.game_name, 16 );
                            memcpy( D_80066E10, D_80066C00.note.ext_name, 4 );
                            D_80114370_opcode = 1;
                            D_8009D090 = D_80066C00.note.company_code;
                            D_8009CF6C = D_80066C00.note.game_code;
                            osSendMesg( &D_800E6690, &D_80114370_opcode, OS_MESG_NOBLOCK );
                            osRecvMesg( &D_800F8098, (OSMesg *)&D_80114370_opcode, OS_MESG_BLOCK );
                            if ( D_80066B8C == 0 ) {
                                D_801145F8_opcode++;
                                D_801145FC_opcode += ( D_80066C00.note.file_size + 0xFF ) >> 8;
                                for ( i = D_801142A4_opcode + D_80114376_opcode; i < D_80114374_opcode - 1; i++ ) {
                                    D_80114378_opcode[i] = D_80114378_opcode[i + 1];
                                    D_80114398_opcode[i] = D_80114398_opcode[i + 1];
                                    D_801143B8_opcode[i] = D_801143B8_opcode[i + 1];
                                }
                                D_80114374_opcode--;
                                if ( D_801142A4_opcode + D_80114376_opcode == D_80114374_opcode ) {
                                    D_801142A4_opcode--;
                                }
                                D_801142A5_opcode = 1;
                            } else {
                                D_801142A5_opcode = 3;
                            }
                        }
                    }
                    if ( D_80066C44 == 1 ) {
                        D_801142A5_opcode = 1;
                    }
                }
                break;
            case 3:
                func_8002DF3C( D_8011429C_opcode[D_8005B0F4 / 5 % 3], 0x12, D_801142A4_opcode * 18 + 0x48 );
                func_80030058( 0x25, D_801142A4_opcode * 18 + 0x48, 0xF6, 0x11, 0xFF, 0, 0, D_8009CFB0 );
                func_8002FDE4( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
                if ( func_80036E60( 0x94A, PROMPT_TIMED ) ) {
                    D_801142A5_opcode = 1;
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
