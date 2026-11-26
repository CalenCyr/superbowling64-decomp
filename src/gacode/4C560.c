/* 4C560.c -- game overlay */

#include "common.h"
#include "libultra.h"
#include "ground.h"
#include "gacode/gacode.h"

/* ===== Data ===== */

static s32 ga_sceneryStarted = 0;
static s32 ga_frameCount = 0;          /* frame counter */
/* image files of GA_AnimateTextures's animated palettes, by animation frame */
static u16 D_8012C928_gacode[11] = { 0x0932, 0x0933, 0x0936, 0x0936, 0x0935, 0x0934, 0x0932, 0x0933, 0x0932, 0x0933, 0x0932 };
static u16 D_8012C940_gacode[11] = { 0x092E, 0x092F, 0x0931, 0x0930, 0x092E, 0x092E, 0x092E, 0x092F, 0x092E, 0x092F, 0x092E };
static u16 D_8012C958_gacode[2] = { 0x0A89, 0x0A88 };
static u16 D_8012C95C_gacode[2] = { 0x0A87, 0x0A86 };
static u16 D_8012C960_gacode[2] = { 0x0A85, 0x0A84 };
static u16 D_8012C964_gacode[9] = { 0x085A, 0x0859, 0x0858, 0x0857, 0x0856, 0x0857, 0x0856, 0x0857, 0x0856 };
/* image files of GA_FlickerLight's four palette slots, by brightness step */
static u16 D_8012C978_gacode[4] = { 0x08F9, 0x08FA, 0x08FB, 0x08FC };
static u16 D_8012C980_gacode[4] = { 0x08F1, 0x08F2, 0x08F3, 0x08F4 };
static u16 D_8012C988_gacode[4] = { 0x084D, 0x084E, 0x084F, 0x0850 };
static u16 D_8012C990_gacode[4] = { 0x08F5, 0x08F6, 0x08F7, 0x08F8 };
/* Unreferenced; 0x765 is in the range of the resource indices the unit patches (1905, 1906). */
static u16 D_8012C998_gacode[2] = { 0x0000, 0x0765 };
static u16 ga_flickerTimer = 1;         /* frames to the next flicker of GA_FlickerLight */
/* image files of GA_LoadLanePalettes, by its index */
static u16 ga_lanePaletteImages[11][4] = {
    { 0x0B55, 0x0B57, 0x0B56, 0x0B57 },
    { 0x0B52, 0x0B54, 0x0B53, 0x0B54 },
    { 0x0B4F, 0x0B51, 0x0B50, 0x0B51 },
    { 0x0B4C, 0x0B4E, 0x0B4D, 0x0B4E },
    { 0x0B46, 0x0B48, 0x0B47, 0x0B48 },
    { 0x0B42, 0x0B44, 0x0B43, 0x0B44 },
    { 0x0B3E, 0x0B40, 0x0B3F, 0x0B40 },
    { 0x0B3A, 0x0B3C, 0x0B3B, 0x0B3C },
    { 0x0B37, 0x0B39, 0x0B38, 0x0B39 },
    { 0x0B34, 0x0B36, 0x0B35, 0x0B36 },
    { 0x0B30, 0x0B32, 0x0B31, 0x0B32 },
};

/* The unit's uninitialised state: file-scope statics, which the assembler allocates in .bss in definition order
   (0x8012CDD0, padded to 16; the overlay's commons follow). */
static void *ga_savedTexels[2];         /* texels of resources 1523 and 1524, saved by GA_AnimateTextures */
static u16 ga_flickerSoundTimer;        /* frames to GA_FlickerLight's flicker sound */

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
        ( (unkImage_t *)cache_files[1673].data )->unk_07 = game_pulseAlpha;
        ( (unkImage_t *)cache_files[1674].data )->unk_07 = game_pulseAlpha;
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
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x1400;
            break;
        case 7:
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x1C00;
            break;
        case 0:
        case 2:
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x1000;
            break;
        case 4:
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x1E00;
            break;
        case 3:
        case 6:
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x3000;
            break;
        case 5:
            ( (sceneObject_t *)obj_scene[140 + i] )->flags |= 0x2800;
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
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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
then points the palettes of seven resources at the images of the current frame of sched_frameCount. Every other
64 frames, resources 1523 and 1524 show images 2682 and 2698 instead of their own texels.
================
*/
void GA_AnimateTextures( void ) {
    s32 i;

    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
        GA_SetPlayerObjectFlags( 0 );
        Cache_GetFile( 2682, FILE_RAW );
        Cache_GetFile( 2698, FILE_RAW );
        ga_savedTexels[0] = ( (fileRefB_t *)cache_files[1523].data )->texels;
        ga_savedTexels[1] = ( (fileRefB_t *)cache_files[1524].data )->texels;
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
    ( (fileRefB_t *)cache_files[1648].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C964_gacode[sched_frameCount / 20 % 9]].data )->palette;
    ( (fileRefB_t *)cache_files[1528].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C95C_gacode[sched_frameCount / 64 & 1]].data )->palette;
    ( (fileRefB_t *)cache_files[1529].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C960_gacode[sched_frameCount / 64 & 1]].data )->palette;
    ( (fileRefB_t *)cache_files[1496].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C958_gacode[sched_frameCount / 2 & 1]].data )->palette;
    ( (fileRefB_t *)cache_files[1663].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C928_gacode[sched_frameCount / 20 % 11]].data )->palette;
    ( (fileRefB_t *)cache_files[1664].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C940_gacode[sched_frameCount / 20 % 11]].data )->palette;
    if ( sched_frameCount & 0x40 ) {
        ( (fileRefB_t *)cache_files[1523].data )->texels = ( (unkImage_t *)cache_files[2682].data )->palette;
        ( (fileRefB_t *)cache_files[1524].data )->texels = ( (unkImage_t *)cache_files[2698].data )->palette;
    } else {
        ( (fileRefB_t *)cache_files[1523].data )->texels = ga_savedTexels[0];
        ( (fileRefB_t *)cache_files[1524].data )->texels = ga_savedTexels[1];
    }
    GA_CountFrame();
}

/*
================
func_80122BD8_gacode
================
*/
void func_80122BD8_gacode( void ) {
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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

    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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
    ( (fileRefB_t *)cache_files[1903].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C988_gacode[step]].data )->palette;
    ( (fileRefB_t *)cache_files[1904].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C978_gacode[step]].data )->palette;
    ( (fileRefB_t *)cache_files[1905].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C980_gacode[step]].data )->palette;
    ( (fileRefB_t *)cache_files[1906].data )->texels =
        ( (unkImage_t *)cache_files[D_8012C990_gacode[step]].data )->palette;
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
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
        GA_SetPlayerObjectFlags( 1 );
    }
    ( (fileRefB_t *)cache_files[883].data )->scroll[0]--;
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
    if ( ga_sceneryStarted == 0 ) {
        ga_sceneryStarted = 1;
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
        obj = obj_scene[100];
    } else {
        obj = obj_scene[120];
    }
    Cache_GetFile( ga_lanePaletteImages[index][2], FILE_RAW );
    Cache_GetFile( ga_lanePaletteImages[index][3], FILE_RAW );
    ( (fileRefB_t *)cache_files[obj->model->parts[0]->refs[1].material].data )->texels =
        ( (unkImage_t *)cache_files[ga_lanePaletteImages[index][3]].data )->palette;
    ( (fileRefB_t *)cache_files[obj->model->parts[0]->refs[2].material].data )->texels =
        ( (unkImage_t *)cache_files[ga_lanePaletteImages[index][2]].data )->palette;
    ( (fileRefB_t *)cache_files[obj->model->parts[0]->refs[3].material].data )->texels =
        ( (unkImage_t *)cache_files[ga_lanePaletteImages[index][2]].data )->palette;
    if ( alt == 0 ) {
        Cache_GetFile( ga_lanePaletteImages[index][1], FILE_RAW );
        ( (fileRefB_t *)cache_files[obj->model->parts[0]->refs[0].material].data )->texels =
            ( (unkImage_t *)cache_files[ga_lanePaletteImages[index][1]].data )->palette;
    } else {
        Cache_GetFile( ga_lanePaletteImages[index][0], FILE_RAW );
        ( (fileRefB_t *)cache_files[obj->model->parts[0]->refs[0].material].data )->texels =
            ( (unkImage_t *)cache_files[ga_lanePaletteImages[index][0]].data )->palette;
    }
}
