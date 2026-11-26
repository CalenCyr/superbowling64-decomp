/* 5E2B0.c -- game-select overlay: the character select */

#include "common.h"
#include "libultra.h"
#include "gscode/gscode.h"

/* ===== Data ===== */

/* the character select: [5] its title, ids 0-7 the portraits, id 9 the cursor */
static menuItem_t gs_selectItems[16] = {
    { -1, 0x85B, 0, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 64, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 128, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 192, 8, 0xFF, SLIDE_NONE },
    { -1, 0x85B, 256, 8, 0xFF, SLIDE_NONE },
    { -1, 0x92C, 72, 12, 0xFF, SLIDE_LEFT },
    { 0, 0x8BC, 32, 188, 0xFF, SLIDE_BOTTOM },
    { 1, 0x8BC, 64, 188, 0xFF, SLIDE_BOTTOM },
    { 2, 0x8BC, 96, 188, 0xFF, SLIDE_BOTTOM },
    { 3, 0x8BC, 128, 188, 0xFF, SLIDE_BOTTOM },
    { 4, 0x8BC, 160, 188, 0xFF, SLIDE_BOTTOM },
    { 5, 0x8BC, 192, 188, 0xFF, SLIDE_BOTTOM },
    { 6, 0x8BC, 224, 188, 0xFF, SLIDE_BOTTOM },
    { 7, 0x8BC, 256, 188, 0xFF, SLIDE_BOTTOM },
    { 9, 0x92D, 32, 180, 0xFF, SLIDE_BOTTOM },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* the character menu's name (id 8) and marker (id 10) */
static menuItem_t gs_nameItems[3] = {
    { 8, 0x916, 103, 152, 0xFF, SLIDE_BOTTOM },
    { 10, 0x914, 264, 114, 128, SLIDE_RIGHT },
    { -1, MENU_END, 0, 0, 0xFF, SLIDE_NONE },
};

/* per character: image of its name */
static s16 gs_nameImages[8] = {
    0x916, 0x919, 0x924, 0x91A, 0x918, 0x923, 0x915, 0x917,
};

/* per character: x of its name */
static s16 gs_nameX[8] = {
    103, 99, 108, 103, 122, 111, 109, 101,
};

/* images of the marker */
static s16 gs_markerImages[4] = {
    0x914, 0x913, 0x912, 0x911,
};

/* per character: its image in gs_markerImages */
static u8 gs_markerByCharacter[8] = {
    0x00, 0x03, 0x00, 0x03, 0x01, 0x01, 0x02, 0x02,
};

/* the scene of character 0's model (gs_modelScenes) */
static objParams_t gs_modelScene0[] = {
    { 1.0f, 0.0f, { 2056.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 430.0f, 1982.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 430.0f, 1982.0f, 196.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 416.0f, 1981.0f, -196.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 416.0f, 1981.0f, 392.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 416.0f, 1981.0f, -392.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 416.0f, 1981.0f, 588.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 369.0f, 1979.0f, 100.0f, 200.0f, 1770.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 369.0f, 1979.0f, -310.0f, 200.0f, 1770.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 369.0f, 1979.0f, 510.0f, 200.0f, 1770.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 369.0f, 1979.0f, -500.0f, 140.0f, 780.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 369.0f, 1979.0f, 670.0f, 140.0f, 780.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 385.0f, 1977.0f, 20.0f, 0.0f, 1375.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 385.0f, 1977.0f, 190.0f, 0.0f, 1375.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 385.0f, 1977.0f, -200.0f, 0.0f, 1375.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 385.0f, 1977.0f, 400.0f, 0.0f, 1375.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 362.0f, 1978.0f, 87.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1785.0f, 1966.0f, 100.0f, 0.0f, 1090.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1785.0f, 1966.0f, -290.0f, 0.0f, 1090.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1785.0f, 1966.0f, 490.0f, 0.0f, 1090.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1768.0f, 1966.0f, -290.0f, 0.0f, 1450.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1768.0f, 1966.0f, 100.0f, 0.0f, 1450.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1768.0f, 1966.0f, 490.0f, 0.0f, 1450.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1036.0f, 1966.0f, -100.0f, 0.0f, 1170.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1036.0f, 1966.0f, 300.0f, 0.0f, 1170.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1799.0f, 1966.0f, 100.0f, 0.0f, 1270.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1799.0f, 1966.0f, -290.0f, 0.0f, 1270.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1799.0f, 1966.0f, 490.0f, 0.0f, 1270.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 6.0f, 1966.0f, 500.0f, 20.0f, 1750.0f, 3.0f, 0.0f, 0.0f, 0.0f, 180.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 5's model (gs_modelScenes) */
static objParams_t gs_modelScene5[] = {
    { 1.0f, 0.0f, { 2122.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 1965.0f, 1977.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1944.0f, 2045.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1848.0f, 1978.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1848.0f, 1978.0f, 256.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, -3300.0f, 0.0f, -1700.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, -4000.0f, 0.0f, -200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, -5000.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, -5800.0f, 0.0f, 3000.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, 3550.0f, 0.0f, -1700.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, 4250.0f, 0.0f, -200.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, 5250.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1870.0f, 1966.0f, 6050.0f, 0.0f, 3000.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, -450.0f, 1200.0f, 6300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, -450.0f, 1200.0f, 7300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, -450.0f, 1200.0f, 8300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, 700.0f, 1200.0f, 6300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, 700.0f, 1200.0f, 7300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1859.0f, 1977.0f, 700.0f, 1200.0f, 8300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1829.0f, 1979.0f, -380.0f, 500.0f, -1700.0f, 3.0f, 1.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1829.0f, 1979.0f, 630.0f, 500.0f, -1700.0f, 3.0f, 1.0f, 1.0f, 0.0f, -30.0f } },
    { 0.0f, 5.0f, { 1907.0f, 2044.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 3's model (gs_modelScenes) */
static objParams_t gs_modelScene3[] = {
    { 1.0f, 0.0f, { 2058.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 451.0f, 1979.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 451.0f, 1979.0f, 255.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 507.0f, 1984.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 618.0f, 1991.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 465.0f, 1977.0f, 600.0f, 0.0f, 3200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 481.0f, 1977.0f, 2600.0f, 0.0f, 1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 481.0f, 1977.0f, -490.0f, 0.0f, 2200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 481.0f, 1977.0f, 600.0f, 5.0f, 3200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 476.0f, 1977.0f, 2600.0f, 0.0f, 1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 476.0f, 1977.0f, -490.0f, 0.0f, 2200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, 2200.0f, 0.0f, 1700.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, 100.0f, 0.0f, 1700.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, -1800.0f, 230.0f, 1200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, 900.0f, 0.0f, 1900.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, -1000.0f, 0.0f, 100.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, -1400.0f, 0.0f, -1000.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, -2400.0f, 170.0f, 300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, 1300.0f, 0.0f, 600.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, -2900.0f, 0.0f, -1200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, 3800.0f, 0.0f, -1100.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, -2800.0f, 170.0f, -2600.0f, 3.0f, 1.0f, 1.0f, 0.0f, 20.0f } },
    { 0.0f, 5.0f, { 585.0f, 1977.0f, 2500.0f, 220.0f, -2500.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 591.0f, 1966.0f, 2900.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, -4600.0f, 0.0f, -600.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, -4300.0f, 0.0f, 2100.0f, 3.0f, 1.0f, 1.0f, 0.0f, 105.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, -2400.0f, 0.0f, 4400.0f, 3.0f, 1.0f, 1.0f, 0.0f, 140.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, 4000.0f, 0.0f, 4300.0f, 3.0f, 1.0f, 1.0f, 0.0f, -120.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, 5200.0f, 0.0f, 1900.0f, 3.0f, 1.0f, 1.0f, 0.0f, -100.0f } },
    { 0.0f, 5.0f, { 574.0f, 1966.0f, 400.0f, 0.0f, 6000.0f, 3.0f, 1.0f, 1.0f, 0.0f, -180.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 6's model (gs_modelScenes) */
static objParams_t gs_modelScene6[] = {
    { 1.0f, 0.0f, { 2069.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 891.0f, 1966.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 891.0f, 1966.0f, 255.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 888.0f, 2000.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 949.0f, 2002.0f, 0.0f, -10.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, -90.0f, -180.0f, -220.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, -90.0f, -180.0f, -560.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, -90.0f, -180.0f, -900.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, -90.0f, -180.0f, -1240.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, 350.0f, -180.0f, -220.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, 350.0f, -180.0f, -560.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, 350.0f, -180.0f, -900.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 988.0f, 1966.0f, 350.0f, -180.0f, -1240.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 901.0f, 1966.0f, 130.0f, -180.0f, -140.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 894.0f, 1966.0f, 130.0f, -180.0f, -290.0f, 3.0f, 1.0f, 0.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 90.0f, -220.0f, -260.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 90.0f, -220.0f, -340.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 90.0f, -220.0f, -570.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 80.0f, -220.0f, -820.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 898.0f, 1966.0f, 130.0f, -180.0f, -480.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 901.0f, 1966.0f, 130.0f, -180.0f, -730.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 901.0f, 1966.0f, 130.0f, -180.0f, -930.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 898.0f, 1966.0f, 130.0f, -180.0f, -1180.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 894.0f, 1966.0f, 150.0f, -180.0f, -1370.0f, 3.0f, 1.0f, 0.0f, 0.0f, 75.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 100.0f, -220.0f, -1390.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, 175.0f, -220.0f, -1320.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 982.0f, 2003.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 977.0f, 1977.0f, 700.0f, 0.0f, 300.0f, 3.0f, 1.0f, 0.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 977.0f, 1977.0f, -1400.0f, 0.0f, 600.0f, 3.0f, 1.0f, 0.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 953.0f, 1966.0f, -120.0f, -220.0f, -570.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1015.0f, 1977.0f, 2000.0f, 15.0f, -550.0f, 3.0f, 1.0f, 0.0f, 0.0f, -10.0f } },
    { 0.0f, 5.0f, { 1015.0f, 1977.0f, -1800.0f, 15.0f, -550.0f, 3.0f, 1.0f, 0.0f, 0.0f, 10.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 1's model (gs_modelScenes) */
static objParams_t gs_modelScene1[] = {
    { 1.0f, 0.0f, { 2083.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 1318.0f, 2018.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 11.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, -145.0f, 0.0f, -520.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 11.0f, 0.0f, -1020.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, -145.0f, 0.0f, -1560.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, -14.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 146.0f, 0.0f, -510.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, -14.0f, 0.0f, -1030.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 146.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 112.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 271.0f, 0.0f, -520.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 112.0f, 0.0f, -1020.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1358.0f, 2020.0f, 271.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 406.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 246.0f, 0.0f, -510.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 406.0f, 0.0f, -1030.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1351.0f, 2019.0f, 246.0f, 0.0f, -1540.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1070.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 250.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -600.0f, 0.0f, -1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, -950.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, -130.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, 710.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, -1000.0f, 0.0f, 1550.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, -1000.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, -180.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, 660.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1333.0f, 1977.0f, 1550.0f, 0.0f, 1480.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1100.0f, 0.0f, -1000.0f, 3.0f, 0.0f, 1.0f, 0.0f, -92.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1200.0f, 0.0f, -740.0f, 3.0f, 0.0f, 1.0f, 0.0f, -83.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1450.0f, 0.0f, 1200.0f, 3.0f, 0.0f, 1.0f, 0.0f, -75.0f } },
    { 0.0f, 5.0f, { 1342.0f, 1966.0f, 1400.0f, 0.0f, 1100.0f, 3.0f, 0.0f, 1.0f, 0.0f, -93.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 150.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1200.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1350.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 0.0f, -800.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 1500.0f, -400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, 1450.0f, 1500.0f, 400.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1326.0f, 1966.0f, -850.0f, 0.0f, 1800.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -300.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -220.0f, 0.0f, -1300.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -850.0f, 1200.0f, -1300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 570.0f, 1100.0f, -1300.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 0.0f, 100.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 1200.0f, 600.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, 1450.0f, 1260.0f, 720.0f, 3.0f, 1.0f, 1.0f, 0.0f, -90.0f, 180.0f } },
    { 0.0f, 5.0f, { 1323.0f, 1977.0f, -850.0f, 0.0f, 1700.0f, 3.0f, 1.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -400.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -470.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -600.0f, 0.0f, -800.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -930.0f, 0.0f, -200.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -900.0f, 0.0f, -130.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -820.0f, 0.0f, -240.0f, 3.0f, 0.0f, 1.0f, 0.0f, 80.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -820.0f, 0.0f, 1400.0f, 3.0f, 0.0f, 1.0f, 0.0f, 80.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -770.0f, 0.0f, 1470.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 800.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 20.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 890.0f, 0.0f, -1300.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -100.0f, 0.0f, 2070.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, -170.0f, 0.0f, 2100.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1300.0f, 0.0f, 1500.0f, 3.0f, 0.0f, 1.0f, 0.0f, 30.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1500.0f, 0.0f, 980.0f, 3.0f, 0.0f, 1.0f, 0.0f, 45.0f } },
    { 0.0f, 5.0f, { 1336.0f, 1966.0f, 1470.0f, 0.0f, 910.0f, 3.0f, 0.0f, 1.0f, 0.0f, 26.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 7's model (gs_modelScenes) */
static objParams_t gs_modelScene7[] = {
    { 1.0f, 0.0f, { 2049.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 168.0f, 1967.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 201.0f, 1966.0f, 0.0f, 0.0f, 200.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 201.0f, 1966.0f, 0.0f, 0.0f, 800.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 201.0f, 1966.0f, 0.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 201.0f, 1966.0f, 0.0f, 0.0f, 2100.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 201.0f, 1966.0f, 650.0f, 0.0f, 2100.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 246.0f, 1969.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 246.0f, 1969.0f, 256.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 190.0f, 1966.0f, -1500.0f, -130.0f, -1400.0f, 3.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 190.0f, 1966.0f, -2700.0f, -130.0f, -700.0f, 3.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 190.0f, 1966.0f, -2700.0f, -130.0f, 300.0f, 3.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 190.0f, 1966.0f, -3000.0f, -130.0f, 2650.0f, 3.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1400.0f, 0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 0.0f, 40.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1500.0f, 0.0f, 400.0f, 3.0f, 0.0f, 1.0f, 0.0f, 120.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1480.0f, 0.0f, 790.0f, 3.0f, 0.0f, 1.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1480.0f, 0.0f, 1370.0f, 3.0f, 0.0f, 1.0f, 0.0f, 256.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1460.0f, 0.0f, 1700.0f, 3.0f, 0.0f, 1.0f, 0.0f, 40.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1300.0f, 0.0f, 2170.0f, 3.0f, 0.0f, 1.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1150.0f, 0.0f, 2400.0f, 3.0f, 0.0f, 1.0f, 0.0f, 120.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1600.0f, 0.0f, -380.0f, 3.0f, 0.0f, 1.0f, 0.0f, 80.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1480.0f, 0.0f, -630.0f, 3.0f, 0.0f, 1.0f, 0.0f, 40.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -1800.0f, 0.0f, -980.0f, 3.0f, 0.0f, 1.0f, 0.0f, 60.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -2000.0f, 0.0f, -1270.0f, 3.0f, 0.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 193.0f, 1966.0f, -2000.0f, 0.0f, -1570.0f, 3.0f, 0.0f, 1.0f, 0.0f, 180.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 4's model (gs_modelScenes) */
static objParams_t gs_modelScene4[] = {
    { 1.0f, 0.0f, { 2067.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 740.0f, 1977.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 779.0f, 1997.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 760.0f, 1984.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 760.0f, 1984.0f, 256.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 799.0f, 1966.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 802.0f, 1966.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f } },
    { 0.0f, 5.0f, { 802.0f, 1966.0f, 260.0f, 0.0f, -2250.0f, 3.0f, 1.0f, 1.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 794.0f, 1998.0f, 100.0f, -150.0f, 530.0f, 3.0f, 0.0f, 1.0f, 0.0f, 60.0f } },
    { 0.0f, 5.0f, { 794.0f, 1998.0f, 120.0f, -164.0f, -1100.0f, 3.0f, 0.0f, 1.0f, 0.0f, 240.0f } },
    { 0.0f, 5.0f, { 794.0f, 1998.0f, 60.0f, -127.0f, -1900.0f, 3.0f, 0.0f, 1.0f, 0.0f, 75.0f } },
    { -2.0f, 0.0f },
};

/* the scene of character 2's model (gs_modelScenes) */
static objParams_t gs_modelScene2[] = {
    { 1.0f, 0.0f, { 2089.0f, 224.0f, 120.0f, 48.0f, 54.0f } },
    { 0.0f, 5.0f, { 1531.0f, 2035.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1531.0f, 2035.0f, 255.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1670.0f, 2036.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1500.0f, 2034.0f, -800.0f, 1500.0f, 1200.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1500.0f, 2034.0f, 1100.0f, 1500.0f, 1200.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1500.0f, 2034.0f, -800.0f, 1500.0f, 300.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1500.0f, 2034.0f, 1100.0f, 1500.0f, 300.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1483.0f, 1977.0f, -860.0f, 365.0f, -310.0f, 3.0f, 1.0f } },
    { 0.0f, 5.0f, { 1483.0f, 1977.0f, 1165.0f, 365.0f, -310.0f, 3.0f, 1.0f, 0.0f, 0.0f, 180.0f } },
    { 0.0f, 5.0f, { 1483.0f, 1977.0f, -150.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 0.0f, 0.0f, 90.0f } },
    { 0.0f, 5.0f, { 1483.0f, 1977.0f, 400.0f, 0.0f, 1400.0f, 3.0f, 1.0f, 0.0f, 0.0f, 90.0f } },
    { -2.0f, 0.0f },
};

/* per character: the scene of its model */
static objParams_t * gs_modelScenes[8] = {
    gs_modelScene0, gs_modelScene1, gs_modelScene2, gs_modelScene3, gs_modelScene4, gs_modelScene5, gs_modelScene6, gs_modelScene7,
};

static u8 gs_cameraDelay = 30; /* frames before the camera turns */
static f32 gs_cameraAngle = 3.1415927f; /* angle of the camera about the model */

/* per character: its portrait image */
static s16 gs_portraitImages[8] = {
    0x922, 0x921, 0x920, 0x91F, 0x91E, 0x91D, 0x91C, 0x91B,
};

static u8 gs_slideStep = 30; /* slide step of the character menu */
static u8 gs_choice = 0; /* CHOICE_IN etc.; 4: the name entry */
static u8 gs_zoomFrame = 8; /* frame of the viewport's zoom, 0 to 15 */

/*
================
GS_CharacterSelect

One frame of the character select: the portraits of the unlocked characters, input, the viewport zooming in on the
highlighted character's model as the camera circles it, and the menu sliding in and out. Returns 1 once a character
is taken, 2 when backed out, else 0.
================
*/
s32 GS_CharacterSelect( void ) {
    s32 i;

    for ( i = 0; i < 8; i++ ) {
        if ( save_data.unlocks & ( 1 << i ) ) {
            Menu_SetItem( gs_selectItems, i, gs_portraitImages[i], MENU_NONE, MENU_NONE, MENU_NONE );
        }
    }

    if ( gs_choice == CHOICE_RUN ) {
        if ( cont_allPressed & ( A_BUTTON | Z_TRIG ) ) {
            Sound_PlaySe( SE_DECIDE, 0 );
            if ( save_data.rumble ) {
                GS_StartPakCheck();
                gs_choice = 4;
            } else {
                game_leaveState = 1;
                gs_choice = CHOICE_TAKEN;
            }
        }
        if ( cont_allPressed & B_BUTTON ) {
            Sound_PlaySe( 2, 0 );
            gs_choice = CHOICE_BACK;
        }
    }

    if ( gs_zoomFrame < 15 && gs_choice == CHOICE_RUN ) {
        gs_zoomFrame++;
        if ( gs_zoomFrame == 10 ) {
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            Obj_CreateAll( gs_classes, gs_modelScenes[game_stage] );
            gs_cameraDelay = 30;
            gs_cameraAngle = 3.1415927f;
            cam_cameras[0]->path->flags |= 4;
        }
        if ( gs_zoomFrame < 7 ) {
            cam_cameras[0]->width = 224.0f - gs_zoomFrame * 32;
            cam_cameras[0]->height = 120.0f - gs_zoomFrame * 17;
        } else if ( gs_zoomFrame >= 9 ) {
            cam_cameras[0]->width = ( gs_zoomFrame - 8 ) * 32.0f;
            cam_cameras[0]->height = ( gs_zoomFrame - 8 ) * 17.0f;
        } else {
            cam_cameras[0]->width = 32;
            cam_cameras[0]->height = 17;
        }
        cam_cameras[0]->left = 160 - cam_cameras[0]->width / 2;
        cam_cameras[0]->top = 114 - cam_cameras[0]->height / 2;
    }

    if ( gs_choice == CHOICE_RUN ) {
        if ( cont_allDpadRepeat & L_JPAD ) {
            if ( gs_zoomFrame == 15 ) {
                gs_zoomFrame = 0;
            } else if ( gs_zoomFrame >= 9 ) {
                gs_zoomFrame = 6;
            }
            do {
                game_stage--;
                if ( game_stage == 0xFF ) {
                    game_stage = 7;
                }
            } while ( !( save_data.unlocks & ( 1 << game_stage ) ) );
            Sound_PlaySe( SE_CURSOR, 0 );
        }
        if ( cont_allDpadRepeat & R_JPAD ) {
            if ( gs_zoomFrame == 15 ) {
                gs_zoomFrame = 0;
            } else if ( gs_zoomFrame >= 9 ) {
                gs_zoomFrame = 6;
            }
            do {
                game_stage++;
                if ( game_stage == 8 ) {
                    game_stage = 0;
                }
            } while ( !( save_data.unlocks & ( 1 << game_stage ) ) );
            Sound_PlaySe( SE_CURSOR, 0 );
        }
    }

    Menu_SetItem( gs_nameItems, 8, gs_nameImages[game_stage], gs_nameX[game_stage], MENU_NONE,
                   MENU_NONE );
    Menu_SetItem( gs_selectItems, 9, MENU_NONE, ( game_stage + 1 ) * 32, MENU_NONE, MENU_NONE );
    Menu_SetItem( gs_nameItems, 10, gs_markerImages[gs_markerByCharacter[game_stage]], MENU_NONE, MENU_NONE,
                   game_pulseAlpha );
    Draw_ClearScreen( 0xE0, 0xE0, 0xE0 );
    Draw_Image( 0xA58, 0, 0 );
    Menu_DrawItems( gs_selectItems, gs_slideStep );

    if ( gs_choice == CHOICE_RUN && gs_zoomFrame != 7 && gs_zoomFrame != 8 &&
         gs_zoomFrame != 9 ) {
        Draw_FillRect( cam_cameras[0]->left, cam_cameras[0]->top, cam_cameras[0]->width - 1,
                       cam_cameras[0]->height - 1, cam_cameras[0]->path->clearColor >> 16,
                       cam_cameras[0]->path->clearColor >> 8, cam_cameras[0]->path->clearColor, 0xFF );
        cam_cameras[0]->frame = -1;
        cam_cameras[0]->path->fovy = 30.0f;
        cam_cameras[0]->eye[0] = 0.0f;
        cam_cameras[0]->eye[1] = 100.0f;
        cam_cameras[0]->eye[2] = 0.0f;
        cam_cameras[0]->at[2] = cosf( gs_cameraAngle ) * 100.0f;
        cam_cameras[0]->at[0] = sinf( gs_cameraAngle ) * 100.0f;
        cam_cameras[0]->at[1] = cam_cameras[0]->eye[1];
        if ( gs_cameraDelay ) {
            gs_cameraDelay--;
        } else {
            gs_cameraAngle += 0.010471975511965976;
        }
        cam_cameras[0]->upPoint[2] = cam_cameras[0]->eye[2];
        cam_cameras[0]->upPoint[0] = cam_cameras[0]->eye[0];
        cam_cameras[0]->upPoint[1] = -100.0f;
        Obj_Frame();
    }

    Menu_DrawItems( gs_nameItems, gs_slideStep );

    switch ( gs_choice ) {
    case CHOICE_IN:
        gs_selectItems[5].slide = SLIDE_LEFT;
        gs_slideStep -= 2;
        if ( gs_slideStep == 0 ) {
            gs_choice = CHOICE_RUN;
            Game_StartPulse();
        }
        break;
    case CHOICE_TAKEN:
    case CHOICE_BACK:
        gs_selectItems[5].slide = SLIDE_RIGHT;
        gs_slideStep += 2;
        if ( gs_slideStep == 30 ) {
            if ( game_leaveState == 1 ) {
                if ( game_fade != 0xFF ) {
                    gs_slideStep = 28;
                    return 0;
                }
                game_leaveState = 2;
                game_fade = 0x107;
            }
            gs_zoomFrame = 8;
            Obj_FreeAll();
            Cache_FreeBelow( 0x825 );
            if ( gs_choice == CHOICE_TAKEN ) {
                gs_choice = CHOICE_IN;
                return 1;
            }
            gs_choice = CHOICE_IN;
            return 2;
        }
        break;
    case 4:
        if ( GS_PakCheck() ) {
            game_leaveState = 1;
            gs_choice = CHOICE_TAKEN;
        }
        break;
    }
    return 0;
}
