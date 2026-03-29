#ifndef _PALETTE_H
#define _PALETTE_H

#include <cstring>

#include "Globals.h"

#define PALETTE_COUNT 3
#define PALETTE_SIZE 16

struct Palette
{
    Palette() : count(0)
    {
        memset(sizes, 0, PALETTE_COUNT);
        memset(colors, 0, PALETTE_COUNT * PALETTE_SIZE * sizeof(u32));
    }

    u8 count; // Amount of loaded palettes
    u8 sizes[PALETTE_COUNT]; // Amount of colors in each palette
    u32 colors[PALETTE_COUNT * PALETTE_SIZE]; // Color data

    u32 Color(u32 palIdx, u32 colorIdx) const { return colors[colorIdx + (palIdx * PALETTE_SIZE)]; }

    void Set(u32 palIdx, u32 colorIdx, u32 rgba) { colors[colorIdx + (palIdx * PALETTE_SIZE)] = rgba; }

    u16 sectionCount;
    u8* prevData;
    u32 prevSize;
    u8* postData;
    u32 postSize;

    u16 depth;
    u16 unknown;
};

#endif // _PALETTE_H
