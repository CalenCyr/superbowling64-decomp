/* 5AD50.c -- game-select overlay: the title of the game select */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"
#include "filecache.h"
#include "dbgtext.h"
#include "libc.h"

/*
================
GS_DrawTintedImage

Draws the image of file 0x960 tinted with the colour r, g, b, a: its pixels are three 4-bit intensity
textures behind the image's 16-colour palette, two 128x32 strips one above the other and a 32x64 strip on
their right, from (x - 8, y - 8).
================
*/
void GS_DrawTintedImage( s16 x, s16 y, u8 r, u8 g, u8 b, u8 a ) {
    D_8005C190 = DRAW2D_FILL; /* neither cached mode: the next image or text sets its mode up again */
    if ( D_800F80B0[0x960].refCount == 0 ) {
        Cache_GetFile( 0x960, FILE_RAW );
    }
    gDPSetCycleType( D_800F8094++, G_CYC_1CYCLE );
    gDPSetRenderMode( D_800F8094++, G_RM_XLU_SURF, G_RM_XLU_SURF2 );
    gDPSetCombineLERP( D_800F8094++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                       PRIMITIVE, 0 );
    gDPSetTextureLUT( D_800F8094++, G_TT_NONE );
    gDPSetPrimColor( D_800F8094++, 0, 0, r, g, b, a );

    gDPLoadTextureBlock_4b( D_800F8094++, ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE,
                            G_IM_FMT_I, 128, 32, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x - 8 ) << 2, ( y - 8 ) << 2, ( x + 120 ) << 2, ( y + 24 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPLoadTextureBlock_4b( D_800F8094++,
                            ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE + 128 * 32 / 2,
                            G_IM_FMT_I, 128, 32, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x - 8 ) << 2, ( y + 24 ) << 2, ( x + 120 ) << 2, ( y + 56 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPLoadTextureBlock_4b( D_800F8094++,
                            ( (fileImage_t *)D_800F80B0[0x960].data )->data + CI4_PALETTE_SIZE + 128 * 32,
                            G_IM_FMT_I, 32, 64, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD );
    gSPTextureRectangle( D_800F8094++, ( x + 120 ) << 2, ( y - 8 ) << 2, ( x + 152 ) << 2, ( y + 56 ) << 2,
                         G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10 );

    gDPPipeSync( D_800F8094++ );
    gDPSetTextureLUT( D_800F8094++, G_TT_RGBA16 );
}

/*
================
GS_NameEntry

One frame of the name entry of player gs_namePlayer: shows the grid of the character set and case, writes
the character under the cursor after the name, and moves the cursor. The C buttons change the case, B erases
a character (or backs out), A and Z take the character, or, on the row below the grid, the end.
Declared s32 but returns nothing (the caller ignores it): the original's return register is live at the
exit, which keeps the delay slots of the branches to it empty.
================
*/
s32 GS_NameEntry( void ) {
    s32 player;

    player = ( ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7 ) - 1;

    if ( gs_nameCursorY == -1 && gs_nameCursorX >= 4 && gs_nameCursorX <= 6 ) {
        gs_nameCharSet = gs_nameCursorX - 4;
    }
    func_8003893C( gs_nameEntryMenu, 0x17, 0x937, 0x30, 0x30, 0xFF );
    switch ( gs_nameCharSet ) {
    case 0:
        if ( gs_nameCharCase == 0 ) {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x93B, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x93A, 0x40, 0x58, 0xFF );
        }
        break;
    case 1:
        if ( gs_nameCharCase == 0 ) {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x939, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x938, 0x40, 0x58, 0xFF );
        }
        break;
    case 2:
        if ( gs_nameCharCase == 0 ) {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x93D, 0x40, 0x58, 0xFF );
        } else {
            func_8003893C( gs_nameEntryMenu, 0x18, 0x93C, 0x40, 0x58, 0xFF );
        }
        break;
    }

    if ( gs_nameCursorY == -1 ) {
        D_8009D098.unk_08[player].name[gs_nameLength] = 0xFFFF;
        if ( gs_nameCursorX == 7 ) {
            func_8003893C( gs_nameEntryMenu, 0x19, 0xA8C, 0xC0, 0x40, D_8009CFB0 );
        } else {
            func_8003893C( gs_nameEntryMenu, 0x19, 0xA8B, gs_nameCursorX * 16 + 0x3E, 0x3E, D_8009CFB0 );
        }
    } else {
        if ( gs_nameLength < 5 ) {
            D_8009D098.unk_08[player].name[gs_nameLength] =
                gs_nameEntryChars[gs_nameCursorY * 12 +
                                  ( ( gs_nameCharSet * 2 + gs_nameCharCase ) * 60 + gs_nameCursorX )];
            D_8009D098.unk_08[player].name[gs_nameLength + 1] = 0xFFFF;
        }
        func_8003893C( gs_nameEntryMenu, 0x19, 0xA8B, gs_nameCursorX * 16 + 0x3E, gs_nameCursorY * 16 + 0x56,
                       D_8009CFB0 );
    }

    if ( D_800E6758 & U_JPAD ) {
        Sound_PlaySe( 0, 0 );
        gs_nameCursorY--;
        if ( gs_nameCursorY == -1 ) {
            gs_gridExitColumn = gs_nameCursorX;
            gs_nameCursorX = 7;
        }
        if ( gs_nameCursorY == -2 ) {
            gs_nameCursorY = 4;
            gs_nameCursorX = gs_gridExitColumn;
        }
    }
    if ( D_800E6758 & D_JPAD ) {
        Sound_PlaySe( 0, 0 );
        gs_nameCursorY++;
        if ( gs_nameCursorY == 0 ) {
            gs_nameCursorX = gs_gridExitColumn;
        }
        if ( gs_nameCursorY == 5 ) {
            gs_nameCursorY = 0;
        }
    }
    if ( ( D_800E6758 & L_JPAD ) && gs_nameCursorY != -1 ) {
        Sound_PlaySe( 0, 0 );
        gs_nameCursorX--;
        if ( gs_nameCursorX == -1 ) {
            gs_nameCursorX = 11;
        }
    }
    if ( D_800E6758 & R_JPAD ) {
        if ( gs_nameCursorY == -1 ) {
            if ( gs_nameCursorX < 7 ) {
                gs_nameCursorX++;
                Sound_PlaySe( 0, 0 );
            }
        } else {
            Sound_PlaySe( 0, 0 );
            gs_nameCursorX++;
            if ( gs_nameCursorX == 12 ) {
                gs_nameCursorX = 0;
            }
        }
    }

    if ( ( D_800E4D62 & R_CBUTTONS ) && gs_nameCharCase == 0 ) {
        Sound_PlaySe( 0, 0 );
        gs_nameCharCase = 1;
    }
    if ( ( D_800E4D62 & L_CBUTTONS ) && gs_nameCharCase == 1 ) {
        Sound_PlaySe( 0, 0 );
        gs_nameCharCase = 0;
    }
    if ( D_800E4D62 & B_BUTTON ) {
        Sound_PlaySe( 2, 0 );
        if ( gs_nameLength != 0 ) {
            gs_nameLength--;
        } else {
            gs_nameEntryStep = 2;
            D_8009D098.unk_08[player].name[0] = 0xFFFF;
        }
    }
    if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
        if ( gs_nameCursorY == -1 ) {
            if ( gs_nameCursorX == 7 ) {
                if ( gs_nameLength == 0 ) {
                    D_8009D098.unk_08[player].name[0] = 0x57;
                    D_8009D098.unk_08[player].name[1] = 0xFFFF;
                }
                Sound_PlaySe( 6, 0 );
                if ( gs_namePlayer < D_8005C161 - 1 ) {
                    gs_namePlayer++;
                    gs_nameEntryStep = 1;
                } else {
                    gs_nameEntryStep = 5;
                }
            }
        } else {
            Sound_PlaySe( 5, 0 );
            gs_nameLength++;
            if ( gs_nameLength >= 5 ) {
                gs_nameLength = 5;
                gs_nameCursorY = -1;
                gs_nameCursorX = 7;
            }
        }
    }
}

/*
================
GS_PlayerSetup

One frame of the player setup: the number of players (or teams), each player's team and handicap, the
name entry of a new team, and the prompts to clear a team's record and to insert a controller. Returns 1
once the setup is taken, 2 once it is backed out of, else 0.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: complete; three register choices differ: in case 0 the team searched for sits in a0 (retail
// a1, the count copied to a0), and in case 5 the human flag and the copy loop's team sit in a0 (retail v1).
// A u8 team fixes the copy loop but adds andi in case 0; a u8 flag adds andi to its tests.
// Permuter clue: holding the human flag in `j` (or `i`) instead of its own local cuts most of the case 5
// difference, so retail's flag likely shares a pseudo with another variable; a block-scoped flag, `team`, or
// another declaration order does not. Holding the flag in `j` and the copy loop's team in `j` too leaves only case
// 0 (sibling functions 5EC00/605D0 matched the same way: the original reuses its counters); case 0 still differs,
// and a `j` holding the searched team's per-player value there makes it worse.
s32 GS_PlayerSetup( void ) {
    u8 minPlayers;
    u8 maxPlayers;
    u8 kind;
    s32 i;
    s32 j;
    s32 team;
    s32 human;

    D_8005B191 = 1;
    Cont_FindStandard();
    func_8003893C( gs_setupMenu, 1, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_setupMenu, 2, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_setupMenu, 3, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_setupMenu, 4, MENU_NONE, MENU_NONE, MENU_NONE, 0 );

    if ( ( D_8005C160 & 0xF ) == 0 ) {
        switch ( ( D_8005C160 >> 4 ) & 3 ) {
        case 0:
            switch ( D_8005C160 >> 6 ) {
            case 0:
                minPlayers = 1;
                maxPlayers = 4;
                kind = 0;
                switch ( D_8005C161 ) {
                case 1:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x94, MENU_NONE, 0xFF );
                    break;
                case 2:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x80, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0xA8, MENU_NONE, 0xFF );
                    break;
                case 3:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x68, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0x94, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 3, 0x897, 0xC0, MENU_NONE, 0xFF );
                    break;
                case 4:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x58, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0x80, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 3, 0x897, 0xA8, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 4, 0x897, 0xD0, MENU_NONE, 0xFF );
                    break;
                }
                break;
            case 2:
                minPlayers = 2;
                maxPlayers = 4;
                kind = 1;
                switch ( D_8005C161 ) {
                case 2:
                    func_8003893C( gs_setupMenu, 1, 0x895, 0x8C, MENU_NONE, 0xFF );
                    break;
                case 4:
                    func_8003893C( gs_setupMenu, 1, 0x895, 0x73, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x895, 0xA5, MENU_NONE, 0xFF );
                    break;
                }
                break;
            case 1:
                minPlayers = 2;
                maxPlayers = 4;
                kind = 1;
                switch ( D_8005C161 ) {
                case 2:
                    func_8003893C( gs_setupMenu, 1, 0x896, 0x8C, MENU_NONE, 0xFF );
                    break;
                case 4:
                    func_8003893C( gs_setupMenu, 1, 0x896, 0x73, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x896, 0xA5, MENU_NONE, 0xFF );
                    break;
                }
                break;
            }
            break;
        case 1:
            switch ( D_8005C160 >> 6 ) {
            case 0:
                minPlayers = 1;
                maxPlayers = 4;
                kind = 0;
                switch ( D_8005C161 ) {
                case 1:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x94, MENU_NONE, 0xFF );
                    break;
                case 2:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x80, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0xA8, MENU_NONE, 0xFF );
                    break;
                case 3:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x68, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0x94, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 3, 0x897, 0xC0, MENU_NONE, 0xFF );
                    break;
                case 4:
                    func_8003893C( gs_setupMenu, 1, 0x897, 0x58, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 2, 0x897, 0x80, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 3, 0x897, 0xA8, MENU_NONE, 0xFF );
                    func_8003893C( gs_setupMenu, 4, 0x897, 0xD0, MENU_NONE, 0xFF );
                    break;
                }
                break;
            case 1:
                minPlayers = 1;
                maxPlayers = 2;
                kind = 4;
                func_8003893C( gs_setupMenu, 1, 0x897, 0x80, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 2, 0x897, 0xA8, MENU_NONE, 0xFF );
                break;
            }
            break;
        case 2:
            minPlayers = 1;
            maxPlayers = 4;
            kind = 2;
            switch ( D_8005C161 ) {
            case 1:
                func_8003893C( gs_setupMenu, 1, 0x897, 0x94, MENU_NONE, 0xFF );
                break;
            case 2:
                func_8003893C( gs_setupMenu, 1, 0x897, 0x80, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 2, 0x897, 0xA8, MENU_NONE, 0xFF );
                break;
            case 3:
                func_8003893C( gs_setupMenu, 1, 0x897, 0x68, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 2, 0x897, 0x94, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 3, 0x897, 0xC0, MENU_NONE, 0xFF );
                break;
            case 4:
                func_8003893C( gs_setupMenu, 1, 0x897, 0x58, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 2, 0x897, 0x80, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 3, 0x897, 0xA8, MENU_NONE, 0xFF );
                func_8003893C( gs_setupMenu, 4, 0x897, 0xD0, MENU_NONE, 0xFF );
                break;
            }
            break;
        }
    } else if ( ( D_8005C160 & 0xF ) == 3 ) {
        minPlayers = 2;
        maxPlayers = 2;
        kind = 4;
        D_8005C162 = 1;
        func_8003893C( gs_setupMenu, 1, 0x897, 0x80, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 2, 0x897, 0xA8, MENU_NONE, 0xFF );
    }

    gs_setupFrames++;
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    func_8003893C( gs_nameEntryMenu, 0x17, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_nameEntryMenu, 0x18, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_nameEntryMenu, 0x19, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_nameEntryMenu, 0x1A, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_nameEntryMenu, 0x1B, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_nameEntryMenu, 0x1C, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_setupMenu, 0, 0xA79, MENU_NONE, MENU_NONE, MENU_NONE );
    func_8003893C( gs_setupMenu, 5, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
    func_8003893C( gs_setupMenu, 9, 0x910, MENU_NONE, MENU_NONE, 0xFF );
    func_8003893C( gs_setupMenu, 0xD, 0x8FF, MENU_NONE, MENU_NONE, 0xFF );
    func_8003893C( gs_setupMenu, 6, MENU_NONE, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0xA, 0x910, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0xE, 0x8FF, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 7, MENU_NONE, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0xB, 0x910, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0xF, 0x8FF, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 8, MENU_NONE, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0xC, 0x910, MENU_NONE, MENU_NONE, 0x6F );
    func_8003893C( gs_setupMenu, 0x10, 0x8FF, MENU_NONE, MENU_NONE, 0x6F );

    if ( gs_setupState == CHOICE_RUN ) {
        switch ( gs_nameEntryStep ) {
        case 0:
            switch ( kind ) {
            case 0:
            case 2:
                GS_DrawTintedImage( 0x16, 0x31, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
                if ( ( D_800E6758 & L_JPAD ) && D_8005C161 >= 2 ) {
                    Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                    D_8005C161--;
                    if ( D_8005C162 == 1 && D_8005C161 == 1 ) {
                        D_8005C162 = 0;
                    }
                }
                if ( ( D_800E6758 & R_JPAD ) && D_8005C161 < 4 ) {
                    Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                    team = ( D_8005C0E4[D_8005C161] >> 4 ) & 7;
                    for ( i = 0; i < D_8005C161; i++ ) {
                        if ( ( ( D_8005C0E4[i] >> 4 ) & 7 ) != 0 && ( ( D_8005C0E4[i] >> 4 ) & 7 ) == team ) {
                            break;
                        }
                    }
                    if ( i != D_8005C161 ) {
                        D_8005C0E4[D_8005C161] &= 0x8F;
                    }
                    D_8005C161++;
                }
                break;
            case 1:
                GS_DrawTintedImage( 0x16, 0x31, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
                if ( ( D_800E6758 & L_JPAD ) && D_8005C161 == 4 ) {
                    Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                    D_8005C161 = 2;
                    D_8005C162 = 0;
                }
                if ( ( D_800E6758 & R_JPAD ) && D_8005C161 == 2 ) {
                    Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                    D_8005C161 = 4;
                }
                break;
            case 3:
            case 4:
                minPlayers = 2;
                maxPlayers = 2;
                D_8005C161 = 2;
                gs_nameEntryStep = 1;
                gs_namePlayer = 0;
                break;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                gs_nameEntryStep = 1;
                gs_namePlayer = 0;
            }
            if ( D_800E4D62 & B_BUTTON ) {
                Sound_PlaySe( 2, 0 );
                gs_setupState = CHOICE_BACK;
            }
            if ( minPlayers < D_8005C161 ) {
                func_8003893C( gs_nameEntryMenu, 0x17, 0x84C, 0xE, 0x4A, D_8009CFB0 );
            }
            if ( D_8005C161 < maxPlayers ) {
                func_8003893C( gs_nameEntryMenu, 0x18, 0x84B, 0x36, 0x4A, D_8009CFB0 );
            }
            break;

        case 1:
            if ( kind == 2 || kind == 3 || gs_namePlayer == 0 ) {
                gs_nameEntryStep = 2;
                break;
            }
            Draw_RoundedBox( gs_namePlayer * 62 + 0x2F, 0x78, 0x28, 2, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
            if ( D_8005C0E4[gs_namePlayer] & 0x80 ) {
                Draw_ImageScaled( 0x84C, gs_namePlayer * 62 + 0x1F, 0x71, D_8009CFB0, 1.0f, 1.0f );
            } else {
                Draw_ImageScaled( 0x84B, gs_namePlayer * 62 + 0x57, 0x71, D_8009CFB0, 1.0f, 1.0f );
            }
            if ( ( D_800E6758 & L_JPAD ) && ( D_8005C0E4[gs_namePlayer] & 0x80 ) ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                D_8005C0E4[gs_namePlayer] &= 0x7F;
            }
            if ( ( D_800E6758 & R_JPAD ) && !( D_8005C0E4[gs_namePlayer] & 0x80 ) ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                D_8005C0E4[gs_namePlayer] = 0x80;
                D_800DF5D0[gs_namePlayer] = 0;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                gs_nameEntryStep = 2;
            } else if ( D_800E4D62 & B_BUTTON ) {
                Sound_PlaySe( 2, 0 );
                if ( gs_namePlayer != 0 ) {
                    gs_namePlayer--;
                    if ( !( D_8005C0E4[gs_namePlayer] & 0x80 ) ) {
                        gs_nameEntryStep = 2;
                    }
                } else {
                    gs_nameEntryStep = 0;
                }
            }
            break;

        case 2:
            if ( D_8005C0E4[gs_namePlayer] & 0x80 ) {
                if ( gs_namePlayer < D_8005C161 - 1 ) {
                    gs_namePlayer++;
                    gs_nameEntryStep = 1;
                } else {
                    gs_nameEntryStep = 5;
                }
                break;
            }
            if ( D_800E6758 & L_JPAD ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                i = ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7;
                do {
                    i--;
                    if ( i == -1 ) {
                        i = 4;
                    }
                    if ( i == 0 ) {
                        break;
                    }
                    for ( j = 0; j < D_8005C161; j++ ) {
                        if ( i == ( ( D_8005C0E4[j] >> 4 ) & 7 ) ) {
                            break;
                        }
                    }
                } while ( j != D_8005C161 );
                D_8005C0E4[gs_namePlayer] = ( D_8005C0E4[gs_namePlayer] & 0x8F ) | ( i << 4 );
            }
            if ( D_800E6758 & R_JPAD ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                i = ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7;
                do {
                    i++;
                    if ( i == 5 ) {
                        i = 0;
                    }
                    if ( i == 0 ) {
                        break;
                    }
                    for ( j = 0; j < D_8005C161; j++ ) {
                        if ( i == ( ( D_8005C0E4[j] >> 4 ) & 7 ) ) {
                            break;
                        }
                    }
                } while ( j != D_8005C161 );
                D_8005C0E4[gs_namePlayer] = ( D_8005C0E4[gs_namePlayer] & 0x8F ) | ( i << 4 );
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                i = ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7;
                if ( i == 0 ) {
                    if ( gs_namePlayer < D_8005C161 - 1 ) {
                        gs_namePlayer++;
                        gs_nameEntryStep = 1;
                    } else {
                        gs_nameEntryStep = 5;
                    }
                } else if ( D_8009D098.unk_08[i - 1].name[0] == 0xFFFF ) {
                    gs_nameCharSet = 2;
                    gs_nameCursorX = 0;
                    gs_nameCursorY = 0;
                    gs_nameCharCase = 0;
                    gs_nameLength = 0;
                    gs_nameEntryStep = 4;
                } else {
                    gs_nameEntryStep = 3;
                    gs_teamPromptChoice = 0;
                }
            } else if ( D_800E4D62 & B_BUTTON ) {
                Sound_PlaySe( 2, 0 );
                if ( gs_namePlayer == 0 ) {
                    gs_nameEntryStep = 0;
                    if ( kind == 3 || kind == 4 ) {
                        gs_setupState = CHOICE_BACK;
                    }
                } else if ( kind < 2 ) {
                    gs_nameEntryStep = 1;
                } else {
                    gs_namePlayer--;
                    if ( !( D_8005C0E4[gs_namePlayer] & 0x80 ) ) {
                        gs_nameEntryStep = 2;
                    }
                }
            } else {
                Draw_RoundedBox( gs_namePlayer * 62 + 0x2B, 0x81, 0x30, 0x28, 0xE0, 0xE0, 0xE0, D_8009CFB0 );
                Draw_ImageScaled( 0x84C, gs_namePlayer * 62 + 0x1B, 0x95, D_8009CFB0, 1.0f, 1.0f );
                Draw_ImageScaled( 0x84B, gs_namePlayer * 62 + 0x5B, 0x95, D_8009CFB0, 1.0f, 1.0f );
                switch ( ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7 ) {
                case 1:
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x906, gs_namePlayer * 62 + 0x3F, 0x97, 0xFF );
                    break;
                case 2:
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x905, gs_namePlayer * 62 + 0x3F, 0x97, 0xFF );
                    break;
                case 3:
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x904, gs_namePlayer * 62 + 0x3F, 0x97, 0xFF );
                    break;
                case 4:
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x903, gs_namePlayer * 62 + 0x3F, 0x97, 0xFF );
                    break;
                }
            }
            break;

        case 3:
            if ( ( D_800E6758 & U_JPAD ) && gs_teamPromptChoice == 1 ) {
                gs_teamPromptChoice = 0;
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
            }
            if ( ( D_800E6758 & D_JPAD ) && gs_teamPromptChoice == 0 ) {
                gs_teamPromptChoice = 1;
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
            }
            func_8003893C( gs_nameEntryMenu, 0x17, 0x8FD, gs_namePlayer * 62 + 0x2B, 0x81, 0xFF );
            func_8003893C( gs_nameEntryMenu, 0x18, 0x8FE, gs_namePlayer * 62 + 0x2B, gs_teamPromptChoice * 16 + 0x81,
                           D_8009CFB0 );
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                Sound_PlaySe( SE_DECIDE, 0 );
                if ( gs_teamPromptChoice == 0 ) {
                    if ( gs_namePlayer < D_8005C161 - 1 ) {
                        gs_namePlayer++;
                        gs_nameEntryStep = 1;
                    } else {
                        gs_nameEntryStep = 5;
                    }
                } else {
                    gs_nameEntryStep = 9;
                }
            }
            if ( D_800E4D62 & B_BUTTON ) {
                Sound_PlaySe( 2, 0 );
                gs_nameEntryStep = 2;
            }
            break;

        case 9:
            D_800DF590[0] = D_800DF590[1] = 0x24;
            D_800E6538[0] = D_800E6538[1] = 0x10;
            D_800E57E0[0] = 0x24;
            D_800E4D68[0] = 0x1E;
            D_800E57E0[1] = 0x4E;
            D_800E4D68[1] = 0x1E;
            if ( gs_clearRecordAnswer != 0 ) {
                gs_clearRecordAnswer = 0;
                if ( D_80066C44 == 0 ) {
                    i = ( ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7 ) - 1;
                    memset( &D_8009D098.unk_08[i], 0, sizeof( D_8009D098.unk_08[i] ) );
                    D_8009D098.unk_08[i].low = 300;
                    memset( D_8009D098.unk_08[i].name, 0xFF, sizeof( D_8009D098.unk_08[i].name ) );
                    memset( &D_800E44E8[i], 0, sizeof( D_800E44E8[i] ) );
                    D_800E44E8[i].low = 300;
                    memset( D_800E44E8[i].name, 0xFF, sizeof( D_800E44E8[i].name ) );
                    gs_nameEntryStep = 2;
                } else {
                    gs_nameEntryStep = 3;
                }
            }
            break;

        case 4:
            GS_NameEntry();
            break;

        case 5:
            if ( D_8005C162 == 0 ) {
                Draw_RoundedBox( 0x3A, 0xBC, 0x64, 0x10, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
                if ( minPlayers < D_8005C161 ) {
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x84B, 0x9C, 0xBC, D_8009CFB0 );
                }
            } else {
                Draw_RoundedBox( 0xA4, 0xBC, 0x64, 0x10, 0xFF, 0xFF, 0xFF, D_8009CFB0 );
                if ( minPlayers < D_8005C161 ) {
                    func_8003893C( gs_nameEntryMenu, 0x17, 0x84C, 0x94, 0xBC, D_8009CFB0 );
                }
                switch ( D_8005C161 ) {
                case 2:
                    Draw_ImageScaled( 0xAE4, 0x2C, 0x5A, 0x99, 1.0f, 1.0f );
                    Draw_ImageScaled( 0x88E, 0x6A, 0x5A, 0x99, 1.0f, 1.0f );
                    break;
                case 3:
                    Draw_ImageScaled( 0xAE5, 0x2C, 0x5A, 0x99, 1.0f, 1.0f );
                    Draw_ImageScaled( 0x88E, 0xA8, 0x5A, 0x99, 1.0f, 1.0f );
                    break;
                case 4:
                    Draw_ImageScaled( 0xAE5, 0x2C, 0x5A, 0x99, 1.0f, 1.0f );
                    Draw_ImageScaled( 0x88F, 0xA8, 0x5A, 0x99, 1.0f, 1.0f );
                    break;
                }
            }
            if ( ( D_800E6758 & L_JPAD ) && D_8005C162 == 1 && minPlayers < D_8005C161 ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                D_8005C162 = 0;
            }
            if ( ( D_800E6758 & R_JPAD ) && D_8005C162 == 0 && minPlayers < D_8005C161 ) {
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                D_8005C162 = 1;
            }
            if ( D_800E4D62 & ( A_BUTTON | Z_TRIG ) ) {
                if ( D_8005C162 == 1 ) {
                    human = FALSE;
                    if ( kind < 2 ) {
                        switch ( D_8005C161 ) {
                        case 2:
                            if ( ( D_8005C0E4[1] >> 7 ) == 0 ) {
                                human = TRUE;
                            }
                            break;
                        case 3:
                            if ( ( D_8005C0E4[2] >> 7 ) == 0 ) {
                                human = TRUE;
                            }
                            break;
                        case 4:
                            if ( ( D_8005C0E4[2] >> 7 ) == 0 ) {
                                human = TRUE;
                            }
                            if ( ( D_8005C0E4[3] >> 7 ) == 0 ) {
                                human = TRUE;
                            }
                            break;
                        }
                    } else {
                        human = TRUE;
                    }
                    if ( human == TRUE ) {
                        if ( kind == 4 ) {
                            if ( ( D_8005C0E4[1] >> 7 ) == 0 && !( D_8005B190 & 2 ) ) {
                                Sound_PlaySe( 2, 0 );
                                gs_nameEntryStep = 7;
                                break;
                            }
                        } else if ( !( D_8005B190 & 2 ) ) {
                            Sound_PlaySe( 2, 0 );
                            gs_nameEntryStep = 6;
                            break;
                        }
                    }
                }
                Sound_PlaySe( SE_DECIDE, 0 );
                for ( i = 0; i < 4; i++ ) {
                    memcpy( D_800E44E8[i].name, D_8009D098.unk_08[i].name, sizeof( D_800E44E8[i].name ) );
                }
                for ( i = 0; i < D_8005C161; i++ ) {
                    team = ( D_8005C0E4[i] >> 4 ) & 7;
                    if ( team != 0 ) {
                        memcpy( &D_80068218[i], D_8009D098.unk_08[team - 1].name, sizeof( D_80068218[i] ) );
                    } else {
                        memset( &D_80068218[i], 0xFF, sizeof( D_80068218[i] ) );
                    }
                }
                gs_setupState = CHOICE_TAKEN;
            }
            if ( D_800E4D62 & B_BUTTON ) {
                Sound_PlaySe( 2, 0 );
                gs_nameEntryStep = 1;
                gs_namePlayer = 0;
            }
            break;

        case 6:
        case 7:
        case 8:
            break;

        }
    }

    if ( D_8005C0E4[0] & 0x80 ) {
        func_8003893C( gs_setupMenu, 9, 0xAB6, MENU_NONE, MENU_NONE, MENU_NONE );
    } else if ( D_8005C0E4[0] & 0x70 ) {
        if ( D_8009D098.unk_08[( ( D_8005C0E4[0] >> 4 ) & 7 ) - 1].name[0] == 0xFFFF ) {
            func_8003893C( gs_setupMenu, 0xD, 0x902, MENU_NONE, MENU_NONE, MENU_NONE );
        } else {
            func_8003893C( gs_setupMenu, 0xD, 0xA83, MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }
    if ( D_8005C161 >= 2 ) {
        func_8003893C( gs_setupMenu, 0, 0xA78, MENU_NONE, MENU_NONE, MENU_NONE );
        func_8003893C( gs_setupMenu, 6, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0xA, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0xE, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        if ( D_8005C0E4[1] & 0x80 ) {
            func_8003893C( gs_setupMenu, 0xA, 0xAB6, MENU_NONE, MENU_NONE, MENU_NONE );
        } else if ( D_8005C0E4[1] & 0x70 ) {
            if ( D_8009D098.unk_08[( ( D_8005C0E4[1] >> 4 ) & 7 ) - 1].name[0] == 0xFFFF ) {
                func_8003893C( gs_setupMenu, 0xE, 0x902, MENU_NONE, MENU_NONE, 0xFF );
            } else {
                func_8003893C( gs_setupMenu, 0xE, 0xA83, MENU_NONE, MENU_NONE, 0xFF );
            }
        }
    }
    if ( D_8005C161 >= 3 ) {
        func_8003893C( gs_setupMenu, 0, 0xA77, MENU_NONE, MENU_NONE, MENU_NONE );
        func_8003893C( gs_setupMenu, 7, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0xB, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0xF, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        if ( D_8005C0E4[2] & 0x80 ) {
            func_8003893C( gs_setupMenu, 0xB, 0xAB6, MENU_NONE, MENU_NONE, MENU_NONE );
        } else if ( D_8005C0E4[2] & 0x70 ) {
            if ( D_8009D098.unk_08[( ( D_8005C0E4[2] >> 4 ) & 7 ) - 1].name[0] == 0xFFFF ) {
                func_8003893C( gs_setupMenu, 0xF, 0x902, MENU_NONE, MENU_NONE, 0xFF );
            } else {
                func_8003893C( gs_setupMenu, 0xF, 0xA83, MENU_NONE, MENU_NONE, 0xFF );
            }
        }
    }
    if ( D_8005C161 >= 4 ) {
        func_8003893C( gs_setupMenu, 0, 0xA76, MENU_NONE, MENU_NONE, MENU_NONE );
        func_8003893C( gs_setupMenu, 8, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0xC, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0x10, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        if ( D_8005C0E4[3] & 0x80 ) {
            func_8003893C( gs_setupMenu, 0xC, 0xAB6, MENU_NONE, MENU_NONE, MENU_NONE );
        } else if ( D_8005C0E4[3] & 0x70 ) {
            if ( D_8009D098.unk_08[( ( D_8005C0E4[3] >> 4 ) & 7 ) - 1].name[0] == 0xFFFF ) {
                func_8003893C( gs_setupMenu, 0x10, 0x902, MENU_NONE, MENU_NONE, 0xFF );
            } else {
                func_8003893C( gs_setupMenu, 0x10, 0xA83, MENU_NONE, MENU_NONE, 0xFF );
            }
        }
    }
    if ( D_8005C162 == 0 ) {
        func_8003893C( gs_setupMenu, 0x11, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
        func_8003893C( gs_setupMenu, 0x12, MENU_NONE, MENU_NONE, MENU_NONE, 0x6F );
    } else {
        func_8003893C( gs_setupMenu, 0x11, MENU_NONE, MENU_NONE, MENU_NONE, 0x6F );
        func_8003893C( gs_setupMenu, 0x12, MENU_NONE, MENU_NONE, MENU_NONE, 0xFF );
    }
    func_800386E0( gs_setupMenu, gs_setupSlide );
    Txt_LoadGlyphSheet( &D_800F8094 );

    if ( gs_setupState == CHOICE_RUN ) {
        for ( i = 0; i < D_8005C161; i++ ) {
            team = ( D_8005C0E4[i] >> 4 ) & 7;
            if ( team != 0 && !( D_8005C0E4[i] & 0x80 ) && D_8009D098.unk_08[team - 1].name[0] != 0xFFFF ) {
                Txt_DrawGlyphs( &D_800F8094, i * 62 + 0x2F, 0x85, 8, D_8009D098.unk_08[team - 1].name );
            }
        }
        if ( D_8005C160 == 0 ) {
            for ( i = 0; i < D_8005C161; i++ ) {
                if ( D_800DF5D0[i] != 0 ) {
                    Draw_Image( 0x9CF, i * 62 + 0x2B, 0x99 );
                    if ( D_800DF5D0[i] / 10 != 0 ) {
                        Draw_Image( gs_handicapDigits[D_800DF5D0[i] / 10], i * 62 + 0x3F, 0xA1 );
                    }
                    Draw_Image( gs_handicapDigits[D_800DF5D0[i] % 10], i * 62 + 0x47, 0xA1 );
                }
            }
        }
        if ( gs_nameEntryStep == 2 && D_8005C160 == 0 && !( D_8005C0E4[gs_namePlayer] & 0x80 ) ) {
            Draw_Image( 0x9CF, gs_namePlayer * 62 + 0x2B, 0x99 );
            if ( D_800DF5D0[gs_namePlayer] / 10 != 0 ) {
                Draw_Image( gs_handicapDigits[D_800DF5D0[gs_namePlayer] / 10], gs_namePlayer * 62 + 0x3F,
                               0xA1 );
            }
            Draw_Image( gs_handicapDigits[D_800DF5D0[gs_namePlayer] % 10], gs_namePlayer * 62 + 0x47,
                           0xA1 );
            Draw_Image( 0x9D0, gs_namePlayer * 62 + 0x51, 0xA1 );
            Draw_Image( 0x9D1, gs_namePlayer * 62 + 0x2F, 0xA1 );
            if ( ( D_800E4D62 & R_CBUTTONS ) || ( ( D_800994BC & R_CBUTTONS ) && gs_cHeldFrames >= 9 ) ) {
                gs_cHeldFrames = 0;
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                Draw_Image( 0x9D2, gs_namePlayer * 62 + 0x51, 0xA1 );
                D_800DF5D0[gs_namePlayer] += 5;
                if ( D_800DF5D0[gs_namePlayer] >= 0x60 ) {
                    D_800DF5D0[gs_namePlayer] = 0;
                }
            }
            if ( ( D_800E4D62 & L_CBUTTONS ) || ( ( D_800994BC & L_CBUTTONS ) && gs_cHeldFrames >= 9 ) ) {
                gs_cHeldFrames = 0;
                Sound_PlaySe( SE_CURSOR, SE_NO_REUSE );
                Draw_Image( 0x9D3, gs_namePlayer * 62 + 0x2F, 0xA1 );
                D_800DF5D0[gs_namePlayer] -= 5;
                if ( D_800DF5D0[gs_namePlayer] < 0 ) {
                    D_800DF5D0[gs_namePlayer] = 0x5F;
                }
            }
            if ( D_800994BC & ( R_CBUTTONS | L_CBUTTONS ) ) {
                gs_cHeldFrames++;
            } else {
                gs_cHeldFrames = 0;
            }
        }
    }

    if ( gs_nameEntryStep == 4 || gs_nameEntryStep == 6 || gs_nameEntryStep == 7 || gs_nameEntryStep == 9 ) {
        Draw_FillRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0x80 );
        if ( gs_nameEntryStep == 9 ) {
            gs_clearRecordAnswer = Save_ShowPrompt( 0x94B, PROMPT_CHOICE2_H );
        }
    }
    func_8003893C( gs_setupMenu, 0x15, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    func_8003893C( gs_setupMenu, 0x16, MENU_NONE, MENU_NONE, MENU_NONE, 0 );
    for ( i = 0; i < 2; i++ ) {
        if ( ( ( D_800F7BE8[i].status ^ gs_rumbleStatus[i] ) & 3 ) & D_800F7BE8[i].status ) {
            if ( !D_800DF680[i] ) {
                D_800DF680[i] = 8;
            }
        }
    }
    if ( D_8009D098.unk_7B && gs_setupState == CHOICE_RUN && ( D_800E4FD0[0] || D_800E4FD0[1] ) ) {
        if ( D_800E4FD0[0] ) {
            func_8003893C( gs_setupMenu, 0x15, MENU_NONE, MENU_NONE, MENU_NONE, 0xA0 );
        }
        if ( D_800E4FD0[1] ) {
            func_8003893C( gs_setupMenu, 0x16, MENU_NONE, MENU_NONE, MENU_NONE, 0xA0 );
        }
        func_8003893C( gs_setupMenu, 0x14, gs_rumbleBlink[gs_setupFrames % 4], MENU_NONE, MENU_NONE,
                       MENU_NONE );
    }
    func_800386E0( gs_nameEntryMenu, gs_setupSlide );

    if ( gs_nameEntryStep == 4 ) {
        Txt_LoadGlyphSheet( &D_800F8094 );
        Txt_DrawGlyphs( &D_800F8094, 0x40, 0x40, 0xA,
                       D_8009D098.unk_08[( ( D_8005C0E4[gs_namePlayer] >> 4 ) & 7 ) - 1].name );
        if ( gs_nameLength < 5 ) {
            Draw_ImageScaled( 0xA5A, gs_nameLength * 10 + 0x40, 0x40, D_8009CFB0, 1.0f, 1.0f );
            if ( gs_nameLength + 1 < 5 ) {
                Draw_ImageScaled( 0xA5A, ( gs_nameLength + 1 ) * 10 + 0x40, 0x40, D_8009CFB0, 1.0f, 1.0f );
            }
        }
    }
    switch ( gs_nameEntryStep ) {
    case 6:
        if ( Save_ShowPrompt( 0x949, PROMPT_ANY_BUTTON ) ) {
            gs_nameEntryStep = 5;
        }
        break;
    case 7:
        if ( Save_ShowPrompt( 0x93F, PROMPT_ANY_BUTTON ) ) {
            gs_nameEntryStep = 5;
        }
        break;
    }

    switch ( gs_setupState ) {
    case CHOICE_IN:
        gs_setupMenu[5].slide = SLIDE_LEFT;
        gs_setupSlide -= 2;
        if ( gs_setupSlide == 0 ) {
            gs_setupState = CHOICE_RUN;
            func_80026B28();
            gs_rumbleStatus[1] = 0;
            gs_rumbleStatus[0] = 0;
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_setupMenu[5].slide = SLIDE_RIGHT;
        gs_setupSlide += 2;
        if ( gs_setupSlide == 30 ) {
            D_8005B191 = 0;
            if ( gs_setupState == CHOICE_TAKEN ) {
                gs_setupState = CHOICE_IN;
                return 1;
            }
            for ( i = 0; i < 4; i++ ) {
                D_8005C0E4[i] = 0;
                D_800DF5D0[i] = 0;
            }
            D_8005C163 = 0;
            gs_setupState = CHOICE_IN;
            return 2;
        }
        break;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/gscode/5AD50", GS_PlayerSetup);
#endif
