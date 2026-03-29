#ifndef _IMAGE_H
#define _IMAGE_H

#include "Globals.h"

#define TILE_SIZE 8

enum ColorType : u8 {
    COLOR_TYPE_NONE = 0xFF,
    COLOR_TYPE_GRAY = 0,
    COLOR_TYPE_RGB = 2,
    COLOR_TYPE_PALETTE = 3,
    COLOR_TYPE_GRAY_ALPHA = 4,
    COLOR_TYPE_RGBA = 6,
};

enum ImageFormat : u8 {
    FORMAT_NONE = 0,
    FORMAT_HORIZONTAL = 1, // 8x8 tiles
    FORMAT_LINEAR = 2,
};

enum ImageFlags
{
    IMAGE_FLAG_DETANGLE  = 1 << 0,
    IMAGE_FLAG_DECOMP = 1 << 1,
    IMAGE_FLAG_TRANSPARENT_BG = 1 << 2,
    IMAGE_FLAG_PKM_ICON = 1 << 3,
    IMAGE_FLAG_ITEM_ICON = 1 << 4,
};

struct Image {

    Image() : size(0), data(nullptr), width(0), height(0), depth(0), type(COLOR_TYPE_NONE), format(FORMAT_NONE) {}

    Image(const Image& image);

    u32 size; // in bytes
    u8* data;

    u32 width; // in pixels
    u32 height; // in pixels

    u8 depth;
    ColorType type;
    ImageFormat format;

    void To8Bits();
    void To4Bits();

    void ToLinear();
    void ToHorizontal();

    void Detangle();
    void Tangle();

    u16 sectionCount;
    u8* prevData;
    u32 prevSize;
    u8* postData;
    u32 postSize;

    ImageFormat originalFormat;
    u16 unknown[2];
};

#endif // _IMAGE_H
