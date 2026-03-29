#include "Externals/lz11/lz11.h"

#include "System.h"

#include "Utils/NDS/NCGRFormat.h"

bool LoadNCGR(const FileStream& fileStream, Image& image, u32 flags)
{
    FileStream stream = {fileStream.data, fileStream.length, fileStream.capacity};

    memset(&image, 0, sizeof(Image));

    u8* decompData = nullptr;
    if ((flags & IMAGE_FLAG_DECOMP) && AuthFileIdentifier(stream, 0x0, "\x11"))
    {
        u32 decompSize = LZ11Decompress(stream.data, stream.length, &decompData);
        if (decompSize == 0)
        {
            Log(WARNING, "Failed decompressing NCGR!");
            return false;
        }

        stream.data = decompData;
        stream.length = decompSize;
        stream.capacity = decompSize;
    }

    if (!AuthFileIdentifier(stream, 0x0, "RGCN"))
    {
        Log(WARNING, "Failed loading NCGR: Invalid file identifier!");
        if (decompData) delete[] decompData;
        return false;
    }

    // File size
    FileStreamRead<u32>(stream, 0x8);
    // Header size
    FileStreamRead<u16>(stream, 0xC);
    u16 sectionCount = FileStreamRead<u16>(stream, 0xE); 

    u32 offset = 0x10;

    // Save in a buffer all the section data before the pixel section
    FileStream prev;
    LoadEmptyFileStream(prev);
    while (AuthFileIdentifierUpdate(stream, offset, "RAHC") == false)
    {
        FileStreamPutBack<u8>(prev, stream.data[offset]);
        ++offset;
    }

    // Section size
    FileStreamReadUpdate<u32>(stream, offset);
    u16 height = FileStreamReadUpdate<u16>(stream, offset);
    u16 width = FileStreamReadUpdate<u16>(stream, offset);
    u32 depth = FileStreamReadUpdate<u32>(stream, offset);

    // Unknown values
    image.unknown[0] = FileStreamReadUpdate<u16>(stream, offset);
    image.unknown[1] = FileStreamReadUpdate<u16>(stream, offset);

    u32 format = FileStreamReadUpdate<u32>(stream, offset);
    u32 dataSize = FileStreamReadUpdate<u32>(stream, offset);

    // Unknown value
    FileStreamReadUpdate<u32>(stream, offset);

    // Image data
    image.size = dataSize;
    image.data = new u8[image.size];
    FileStreamBufferReadUpdate(stream, offset, image.data, image.size);

    // Save in a buffer all the section data after the color section
    FileStream post;
    LoadEmptyFileStream(post);
    while (FileStreamEnded(stream, offset) == false)
    {
        FileStreamPutBack<u8>(post, stream.data[offset]);
        ++offset;
    }

    if (height == 0xFFFF && width == 0xFFFF)
    {
        if (flags & IMAGE_FLAG_PKM_ICON)
        {
            image.height = 8;
            image.width = 4;
        }
        else if (flags & IMAGE_FLAG_ITEM_ICON)
        {
            image.height = 4;
            image.width = 4;
        }
        else
        {
            image.height = 1;
            image.width = image.size / (TILE_SIZE * TILE_SIZE);
        }
    }
    else
    {
        image.height = height;
        image.width = width;
    }
    image.height *= TILE_SIZE;
    image.width *= TILE_SIZE;

    switch (depth)
    {
        case 0x3:
            image.depth = 4;
            break;
        case 0x4:
            image.depth = 8;
            break;
        default:
            Log(WARNING, "Failed loading NCGR: Invalid bit depth!");
            if (decompData) delete[] decompData;
            return false;
    }

    image.type = COLOR_TYPE_PALETTE;

    if (format == 0)
        image.format = FORMAT_HORIZONTAL;
    else
        image.format = FORMAT_LINEAR;
    image.originalFormat = image.format;

    // Correct formating
    if (image.format == FORMAT_HORIZONTAL)
    {
        if (flags & IMAGE_FLAG_DETANGLE) image.Detangle();
        image.ToLinear();
    }
    if (image.depth == 4) image.To8Bits();

    if (decompData) delete[] decompData;
    return true;
}

bool SaveNCGR(FileStream& fileStream, const Image& image, u32 flags)
{
    Image img(image);
    if (img.depth != 4) img.To4Bits();

    Log(INFO, "Format: %d | OG: %d", image.format, image.originalFormat);
    if (img.format != img.originalFormat)
    {
        switch (img.originalFormat)
        {
            case FORMAT_HORIZONTAL:
                Log(INFO, "TO HORI");
                img.ToHorizontal();
                break;
            case FORMAT_LINEAR:
                Log(INFO, "TO LINE");
                img.ToLinear();
                break;
        }
    }

    if (flags & IMAGE_FLAG_DETANGLE) img.Tangle();

    LoadEmptyFileStream(fileStream);

    FileStreamBufferWriteBack(fileStream, (const u8*)"RGCN", 4);
    FileStreamPutBack<u16>(fileStream, 0xFEFF);
    FileStreamPutBack<u16>(fileStream, 0x0101);

    u32 fileSizeOffset = fileStream.length;
    FileStreamPutBack<u32>(fileStream, 0x0);

    FileStreamPutBack<u16>(fileStream, 0x10);
    FileStreamPutBack<u16>(fileStream, 0x1);

    if (img.prevSize != 0)
        FileStreamBufferWriteBack(fileStream, img.prevData, img.prevSize);

    FileStreamBufferWriteBack(fileStream, (const u8*)"RAHC", 4);

    FileStreamPutBack<u32>(fileStream, img.size + 0x20);


    if (flags & IMAGE_FLAG_PKM_ICON || flags & IMAGE_FLAG_ITEM_ICON)
    {
        FileStreamPutBack<u16>(fileStream, 0xFFFF);
        FileStreamPutBack<u16>(fileStream, 0xFFFF);
    }
    else
    {
        FileStreamPutBack<u16>(fileStream, img.height / TILE_SIZE);
        FileStreamPutBack<u16>(fileStream, img.width / TILE_SIZE);
    }
    FileStreamPutBack<u32>(fileStream, 0x3);
    FileStreamPutBack<u16>(fileStream, img.unknown[0]);
    FileStreamPutBack<u16>(fileStream, img.unknown[1]);

    switch (img.format)
    {
        case FORMAT_NONE:
        case FORMAT_HORIZONTAL:
            FileStreamPutBack<u32>(fileStream, 0x0);
            break;
        case FORMAT_LINEAR:
            FileStreamPutBack<u32>(fileStream, 0x1);
            break;
    }

    FileStreamPutBack<u32>(fileStream, img.size);

    FileStreamPutBack<u32>(fileStream, 0x18);

    FileStreamBufferWriteBack(fileStream, img.data, img.size);
    delete[] img.data;

    if (img.postSize != 0)
        FileStreamBufferWriteBack(fileStream, img.postData, img.postSize);
    
    FileStreamReplace(fileStream, fileSizeOffset, fileStream.length); 

    if (flags & IMAGE_FLAG_DECOMP)
    {
        u8* compData = nullptr;
        u32 compSize = LZ11Compress(fileStream.data, fileStream.length, &compData);
        if (compSize == 0)
        {
            Log(WARNING, "Failed compressing NCGR!");
            return false;
        }
        delete[] fileStream.data;

        fileStream.data = compData;
        fileStream.length = compSize;
        fileStream.capacity = compSize;
    }

    return true;
}
