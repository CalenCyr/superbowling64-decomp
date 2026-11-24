/* 4C560.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "ground.h"
#include "gacode/gacode.h"

/*
================
GA_CountFrame

Counts a frame; in mode 3, gives resources 1673 and 1674 the game_pulseAlpha value in their byte 7.
Declared inline: retail repeats its body in each caller below, and still has this out-of-line copy, which
nothing calls (gcc 2.7 emits an extern inline function).
================
*/
inline void GA_CountFrame( void ) {
    ga_frameCount++;
    if ( ( game_mode & 0xF ) == 3 ) {
        ( (unkImage_t *)D_800F80B0[1673].data )->unk_07 = game_pulseAlpha;
        ( (unkImage_t *)D_800F80B0[1674].data )->unk_07 = game_pulseAlpha;
    }
}

/*
================
GA_SetPlayerObjectFlags

With set nonzero, gives each player's scene object (140 on) the flag bits of the low nibble of its
game_players entry.
================
*/
void GA_SetPlayerObjectFlags( s32 set ) {
    s32 i;

    if ( !set ) {
        return;
    }
    for ( i = 0; i < game_numPlayers; i++ ) {
        switch ( game_players[i] & 0xF ) {
        case 1:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1400;
            break;
        case 7:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1C00;
            break;
        case 0:
        case 2:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1000;
            break;
        case 4:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x1E00;
            break;
        case 3:
        case 6:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x3000;
            break;
        case 5:
            ( (sceneObject_t *)D_800EA120[140 + i] )->flags |= 0x2800;
            break;
        }
    }
}

/*
================
func_80122470_gacode
================
*/
void func_80122470_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 0 );
    }
    GA_CountFrame();
}

/*
================
func_801224F8_gacode

Like func_80122BD8_gacode, then on every other frame puts a 0x300 effect near each lane's ball while that
ball's state is 1 (the second lane only when game_lastTeam is 1).
================
*/
void func_801224F8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 1 );
    }
    GA_CountFrame();
    if ( D_800DF595[0] == 1 && ( ga_frameCount & 1 ) ) {
        Scene_StartPuff( 0x300, D_800E6560[0][0] + guRandom() % 20 - 10.0f,
                       D_800E6560[0][1] - 10.0f + guRandom() % 20 - 10.0f, D_800E6560[0][2] + 10.0f );
    }
    if ( D_800DF595[1] == 1 && game_lastTeam == 1 && ( ga_frameCount & 1 ) ) {
        Scene_StartPuff( 0x300, D_800E6560[1][0] + guRandom() % 20 - 10.0f,
                       D_800E6560[1][1] - 10.0f + guRandom() % 20 - 10.0f, D_800E6560[1][2] + 10.0f );
    }
}

/*
================
GA_AnimateTextures

Animated textures: loads the frame images on the first frame and saves the texels of resources 1523 and 1524,
then points the palettes of seven resources at the images of the current frame of D_8005B0F4. Every other
64 frames, resources 1523 and 1524 show images 2682 and 2698 instead of their own texels.
================
*/
void GA_AnimateTextures( void ) {
    s32 i;

    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 0 );
        Cache_GetFile( 2682, FILE_RAW );
        Cache_GetFile( 2698, FILE_RAW );
        ga_savedTexels[0] = ( (fileRefB_t *)D_800F80B0[1523].data )->texels;
        ga_savedTexels[1] = ( (fileRefB_t *)D_800F80B0[1524].data )->texels;
        for ( i = 0; i < 11; i++ ) {
            Cache_GetFile( D_8012C928_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C940_gacode[i], FILE_RAW );
        }
        for ( i = 0; i < 2; i++ ) {
            Cache_GetFile( D_8012C958_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C95C_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C960_gacode[i], FILE_RAW );
        }
        for ( i = 0; i < 9; i++ ) {
            Cache_GetFile( D_8012C964_gacode[i], FILE_RAW );
        }
    }
    ( (fileRefB_t *)D_800F80B0[1648].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C964_gacode[D_8005B0F4 / 20 % 9]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1528].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C95C_gacode[D_8005B0F4 / 64 & 1]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1529].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C960_gacode[D_8005B0F4 / 64 & 1]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1496].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C958_gacode[D_8005B0F4 / 2 & 1]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1663].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C928_gacode[D_8005B0F4 / 20 % 11]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1664].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C940_gacode[D_8005B0F4 / 20 % 11]].data )->palette;
    if ( D_8005B0F4 & 0x40 ) {
        ( (fileRefB_t *)D_800F80B0[1523].data )->texels = ( (unkImage_t *)D_800F80B0[2682].data )->palette;
        ( (fileRefB_t *)D_800F80B0[1524].data )->texels = ( (unkImage_t *)D_800F80B0[2698].data )->palette;
    } else {
        ( (fileRefB_t *)D_800F80B0[1523].data )->texels = ga_savedTexels[0];
        ( (fileRefB_t *)D_800F80B0[1524].data )->texels = ga_savedTexels[1];
    }
    GA_CountFrame();
}

/*
================
func_80122BD8_gacode
================
*/
void func_80122BD8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 1 );
    }
    GA_CountFrame();
}

/*
================
func_80122C60_gacode
================
*/
void func_80122C60_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 0 );
    }
    GA_CountFrame();
}

/*
================
GA_FlickerLight

Flickering light: loads the four palette images of every brightness step on the first frame, then counts down
to a random flicker; in its last five frames the brightness is random (100 to 254), else 100. The brightness
sets lights 0 and 1 and picks the palettes of resources 1903 to 1906.
================
*/
void GA_FlickerLight( void ) {
    s32 i;
    u8 color[3];
    u8 level;
    s32 step;              /* brightness step, 0 to 3 */

    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 1 );
        for ( i = 0; i < 4; i++ ) {
            Cache_GetFile( D_8012C978_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C980_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C988_gacode[i], FILE_RAW );
            Cache_GetFile( D_8012C990_gacode[i], FILE_RAW );
        }
    }
    if ( --ga_flickerTimer == 0 ) {
        ga_flickerTimer = guRandom() % 140 + 10;
    }
    if ( --ga_flickerSoundTimer == 0 ) {
        Sound_PlaySe( 70, 3 );
    }
    if ( ga_flickerTimer < 6 ) {
        if ( ga_flickerTimer == 1 ) {
            ga_flickerSoundTimer = 30;
        }
        level = guRandom() % 155 + 100;
    } else {
        level = 100;
    }
    color[0] = color[1] = color[2] = level;
    Cam_SetAmbient( 0, color );
    Cam_SetAmbient( 1, color );
    step = ( level - 100 ) / 39;
    ( (fileRefB_t *)D_800F80B0[1903].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C988_gacode[step]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1904].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C978_gacode[step]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1905].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C980_gacode[step]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[1906].data )->texels =
        ( (unkImage_t *)D_800F80B0[D_8012C990_gacode[step]].data )->palette;
    GA_CountFrame();
}

/*
================
func_80123004_gacode

Scrolls the texture of resource 883 by one texel step in s, then puts 0x301 effects near each lane's ball
while its state is 3: with two lanes on frames 0 (first lane) and 2 (second lane) of every four, with one lane
on every other frame.
================
*/
void func_80123004_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 1 );
    }
    ( (fileRefB_t *)D_800F80B0[883].data )->scroll[0]--;
    GA_CountFrame();
    if ( game_lastTeam == 1 ) {
        if ( D_800DF595[0] == 3 && ( ga_frameCount & 3 ) == 0 ) {
            Scene_StartPuff( 0x301, D_800E6560[0][0] + guRandom() % 10 - 5.0f,
                           D_800E6560[0][1] + 15.0f + guRandom() % 10 - 5.0f, D_800E6560[0][2] + 10.0f );
        }
        if ( D_800DF595[1] == 3 && ( ga_frameCount & 3 ) == 2 ) {
            Scene_StartPuff( 0x301, D_800E6560[1][0] + guRandom() % 10 - 5.0f,
                           D_800E6560[1][1] + 15.0f + guRandom() % 20 - 10.0f, D_800E6560[1][2] + 10.0f );
        }
    } else {
        if ( D_800DF595[0] == 3 && ( ga_frameCount & 1 ) == 0 ) {
            Scene_StartPuff( 0x301, D_800E6560[0][0] + guRandom() % 10 - 5.0f,
                           D_800E6560[0][1] + 15.0f + guRandom() % 10 - 5.0f, D_800E6560[0][2] + 10.0f );
        }
    }
}

/*
================
func_801233A8_gacode
================
*/
void func_801233A8_gacode( void ) {
    if ( D_8012C920_gacode == 0 ) {
        D_8012C920_gacode = 1;
        GA_SetPlayerObjectFlags( 1 );
    }
    GA_CountFrame();
}

/*
================
GA_LoadLanePalettes

Loads the image files of ga_lanePaletteImages[index] and points the palettes of the first four parts' second
files of the lane's scene object (100 or 120) at them; the first part takes file 1, or file 0 when alt is set.
================
*/
void GA_LoadLanePalettes( s32 lane, s32 alt, s32 index ) {
    sceneObject_t *obj;

    if ( lane == 0 ) {
        obj = D_800EA120[100];
    } else {
        obj = D_800EA120[120];
    }
    Cache_GetFile( ga_lanePaletteImages[index][2], FILE_RAW );
    Cache_GetFile( ga_lanePaletteImages[index][3], FILE_RAW );
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[1].material].data )->texels =
        ( (unkImage_t *)D_800F80B0[ga_lanePaletteImages[index][3]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[2].material].data )->texels =
        ( (unkImage_t *)D_800F80B0[ga_lanePaletteImages[index][2]].data )->palette;
    ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[3].material].data )->texels =
        ( (unkImage_t *)D_800F80B0[ga_lanePaletteImages[index][2]].data )->palette;
    if ( alt == 0 ) {
        Cache_GetFile( ga_lanePaletteImages[index][1], FILE_RAW );
        ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[0].material].data )->texels =
            ( (unkImage_t *)D_800F80B0[ga_lanePaletteImages[index][1]].data )->palette;
    } else {
        Cache_GetFile( ga_lanePaletteImages[index][0], FILE_RAW );
        ( (fileRefB_t *)D_800F80B0[obj->model->parts[0]->refs[0].material].data )->texels =
            ( (unkImage_t *)D_800F80B0[ga_lanePaletteImages[index][0]].data )->palette;
    }
}
