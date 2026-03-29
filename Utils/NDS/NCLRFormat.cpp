#include <cstring>

#include "System.h"

#include "Utils/NDS/NCLRFormat.h"

u32 BGR5ToRGBA(u16 bgr5)
{
    u32 rgba = 0;
    u8* colors = (u8*)&rgba;
    colors[0] = ((bgr5 & 0x001F) >> 0x00) * 8;
    colors[1] = ((bgr5 & 0x03E0) >> 0x05) * 8;
    colors[2] = ((bgr5 & 0x7C00) >> 0x0A) * 8;
    colors[3] = 0xFF;
    return rgba;
}

u16 RGBAToBGR5(u32 rgba)
{
    u16 bgr5 = 0;
    u8* colors = (u8*)&rgba;
    bgr5 |= ((colors[0] / 8) << 0x00) & 0x001F;
    bgr5 |= ((colors[1] / 8) << 0x05) & 0x03E0;
    bgr5 |= ((colors[2] / 8) << 0x0A) & 0x7C00;
    return bgr5;
}

bool LoadNCLR(const FileStream& fileStream, Palette& palette)
{
    memset(&palette, 0, sizeof(Palette));

    if (!AuthFileIdentifier(fileStream, 0x0, "RLCN"))
    {
        Log(WARNING, "Failed loading NCLR: Invalid file identifier!");
        return false;
    }

    // u16 endianCheck = FileStreamRead<u16>(fileStream, 0x4);
    // u16 constant = FileStreamRead<u16>(fileStream, 0x6);

    // File size
    FileStreamRead<u32>(fileStream, 0x8);
    // Header size
    FileStreamRead<u16>(fileStream, 0xC);
    palette.sectionCount = FileStreamRead<u16>(fileStream, 0xE); 

    u32 offset = 0x10;

    // Save in a buffer all the section data before the color section
    FileStream prev;
    LoadEmptyFileStream(prev);
    while (AuthFileIdentifierUpdate(fileStream, offset, "TTLP") == false)
    {
        FileStreamPutBack<u8>(prev, fileStream.data[offset]);
        ++offset;
    }

    // Section size
    FileStreamReadUpdate<u32>(fileStream, offset);

    u16 depth = FileStreamReadUpdate<u16>(fileStream, offset);
    palette.depth = depth;
    depth = 4; // Inconsistent depth value in Pokemon palettes (they always use bgr)
               // 2 unknown variables
    palette.unknown = FileStreamReadUpdate<u16>(fileStream, offset);
    FileStreamReadUpdate<u32>(fileStream, offset);
    u32 paletteSize = FileStreamReadUpdate<u32>(fileStream, offset);

    // Start of the data offset from the section size
    FileStreamReadUpdate<u32>(fileStream, offset);

    u32 colorSize = 0;
    switch (depth) {
        case 4:
            colorSize = 2;
            break;
        case 8:
            colorSize = 4;
            break;
        default:
            Log(WARNING, "Failed loading NCLR: Unsupported bit depth!");
            return false;
    }
    u32 paletteCount = paletteSize / (PALETTE_SIZE * colorSize);
    if (paletteCount > PALETTE_COUNT)
        paletteCount = PALETTE_COUNT;

    u32 currPalette = 0;
    palette.count = 1;
    for (u32 idx = 0; idx < paletteCount * PALETTE_SIZE; ++idx)
    {
        if (idx != 0 && idx % PALETTE_SIZE == 0) {
            ++currPalette;
            ++palette.count;
        }

        switch (depth) {
            case 4: {
                        u16 color = FileStreamReadUpdate<u16>(fileStream, offset);
                        palette.colors[idx] = BGR5ToRGBA(color);
                    }
                    break;
            case 8: {
                        u32 color = FileStreamReadUpdate<u32>(fileStream, offset);
                        palette.colors[idx] = color;
                    }
                    break;
        }
        ++palette.sizes[currPalette];
    }

    // Save in a buffer all the section data after the color section
    FileStream post;
    LoadEmptyFileStream(post);
    while (FileStreamEnded(fileStream, offset) == false)
    {
        FileStreamPutBack<u8>(post, fileStream.data[offset]);
        ++offset;
    }

    // Store the extra section data in the palette struct to add it later when saving
    palette.prevData = prev.data;
    palette.prevSize = prev.length;
    palette.postData = post.data;
    palette.postSize = post.length;
    return true;
}

bool SaveNCLR(FileStream& fileStream, const Palette& palette)
{
    LoadEmptyFileStream(fileStream);

    FileStreamBufferWriteBack(fileStream, (const u8*)"RLCN", 4);
    FileStreamPutBack<u16>(fileStream, 0xFEFF);
    FileStreamPutBack<u16>(fileStream, 0x0100);

    u32 fileSizeOffset = fileStream.length;
    FileStreamPutBack<u32>(fileStream, 0x0);
    FileStreamPutBack<u16>(fileStream, 0x10);
    FileStreamPutBack<u16>(fileStream, palette.sectionCount);

    if (palette.prevSize != 0)
        FileStreamBufferWriteBack(fileStream, palette.prevData, palette.prevSize);

    FileStreamBufferWriteBack(fileStream, (const u8*)"TTLP", 4);
    FileStreamPutBack<u32>(fileStream, ((palette.count * PALETTE_SIZE) * sizeof(u16)) + 0x18);
    FileStreamPutBack<u16>(fileStream, palette.depth);
    FileStreamPutBack<u16>(fileStream, palette.unknown);
    FileStreamPutBack<u32>(fileStream, 0x0);
    FileStreamPutBack<u32>(fileStream, ((palette.count * PALETTE_SIZE) * sizeof(u16)));
    FileStreamPutBack<u32>(fileStream, 0x10);

    for (u32 idx = 0; idx < palette.count * PALETTE_SIZE; ++idx)
    {
        u16 bgr5 = RGBAToBGR5(palette.colors[idx]);
        FileStreamPutBack<u16>(fileStream, bgr5);
    }

    if (palette.postSize != 0)
        FileStreamBufferWriteBack(fileStream, palette.postData, palette.postSize);

    FileStreamReplace<u32>(fileStream, fileSizeOffset, fileStream.length);
    
    return true;
}
