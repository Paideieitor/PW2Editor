#include <cstring>

#include "Utils/Internal/Image.h"

Image::Image(const Image& image) : size(image.size),  width(image.width), height(image.height), depth(image.depth), type(image.type), format(image.format), originalFormat(image.originalFormat), sectionCount(image.sectionCount), prevSize(image.prevSize), postSize(image.postSize) 
{
    data = new u8[size];
    memcpy(data, image.data, size);
 
    if (prevSize) memcpy(prevData, image.prevData, prevSize);
    if (postSize) memcpy(postData, image.postData, postSize);

    memcpy(unknown, image.unknown, sizeof(u16) * 2);
}

void Image::To8Bits()
{
	if (type != COLOR_TYPE_PALETTE)
		return;
	if (depth != 4)
		return;

	u8* buffer = new u8[size * 2];
	for (u32 i = 0; i < size; ++i)
	{
		u8 byte = data[i];// (m_bytes[i + 1] << 4);
#ifdef _WIN32
		buffer[(i * 2) + 1] = byte & 0x0F;
		buffer[(i * 2)] = (byte & 0xF0) >> 4;
#else
		buffer[(i * 2)] = byte & 0x0F;
		buffer[(i * 2) + 1] = (byte & 0xF0) >> 4;
#endif
	}

    if (data) { delete[] data; data = nullptr; }

	data = buffer;
	size = size * 2;
	depth = 8;
}

void Image::To4Bits()
{
    if (type != COLOR_TYPE_PALETTE)
        return;
    if (depth != 8)
        return;

    u8* buffer = new u8[size / 2];
    for (u32 i = 0; i < size; i+= 2)
    {
        u8 b1 = data[i];
        u8 b2 = data[i + 1];

#ifdef _WIN32
        buffer[i / 2] = ((b1 << 4) & 0xF0) + (b2 & 0x0F);
#else
        buffer[i / 2] = ((b2 << 4) & 0xF0) + (b1 & 0x0F);
#endif
    }

    if (data) { delete[] data; data = nullptr; }

    data = buffer;
    size = size / 2;
    depth = 4;
}

void Image::ToLinear()
{
	if (type != COLOR_TYPE_PALETTE)
		return;
	if (format != FORMAT_HORIZONTAL)
		return;
	if (depth != 4)
		return;

	u8* buffer = new u8[size];

	u32 widthInTiles = width / TILE_SIZE;
	u32 heightInTiles = height / TILE_SIZE;

	u32 tileWidth = TILE_SIZE;
	if (depth == 4)
		tileWidth /= 2;

	u32 tileCount = widthInTiles * heightInTiles;
	u32 tileSize = tileWidth * TILE_SIZE;

	u32 tileX = 0;
	u32 tileY = 0;
	for (u32 tile = 0; tile < tileCount; ++tile)
	{
		u32 currentByte = 0;
		for (u32 y = 0; y < TILE_SIZE; ++y)
			for (u32 x = 0; x < tileWidth; ++x)
			{
				u32 actx = (tileX * tileWidth);
				u32 acty = (tileY * (tileWidth * TILE_SIZE) * widthInTiles);
				
				buffer[acty + actx + ((tileWidth * widthInTiles) * y) + x] = data[(tile * tileSize) + currentByte];
				++currentByte;
			}

		++tileX;
		if (tileX >= widthInTiles)
		{
			tileX = 0;
			++tileY;
		}
	}

    if (data) { delete[] data; data = nullptr; }

	data = buffer;
	format = FORMAT_LINEAR;
}

void Image::ToHorizontal()
{
	if (type != COLOR_TYPE_PALETTE)
		return;
	if (format != FORMAT_LINEAR)
		return;
	if (depth != 4)
		return;

	u8* buffer = new u8[size];

    u32 dataWidth = width / 2;
    u32 widthTiles = width / TILE_SIZE;
    u32 heightTiles = height / TILE_SIZE;
    u32 tileWidth = TILE_SIZE / 2;

    u32 currentByte = 0;
    for (u32 tileY = 0; tileY < heightTiles; ++tileY)
        for (u32 tileX = 0; tileX < widthTiles; ++tileX)
            for (u32 y = 0; y < TILE_SIZE; ++y)
                for (u32 x = 0; x < tileWidth; ++x)
                {
                    u32 actualX = x + (tileX * tileWidth);
                    u32 actualY = y + (tileY * TILE_SIZE);
                    buffer[currentByte] = data[(actualY * dataWidth) + actualX];
                    ++currentByte;
                }

    if (data) { delete[] data; data = nullptr; }

    data = buffer;
    format = FORMAT_HORIZONTAL;
}

struct Chunk
{
	u8 x = 0;
	u8 y = 0;
	u8 width = 0;
	u8 height = 0;
};

u8 chunkWidths[4] = {8, 4, 2, 1};

void ProcessChunk(const Chunk processing, Chunk* const chunks, u32* const chunkCount)
{
	// Stop if there are no more chunks to process
	if (processing.width == 0)
		return;
	
	Chunk current = { processing.x, processing.y, 0, 0 };
	for (u32 j = 0; j < 4; ++j)
	{
		// Find the biggest possible width 
		if (processing.width >= chunkWidths[j])
		{
			current.width = chunkWidths[j];
	
			// Height can only be 1 size bigger than width
			if (j != 0)
				--j;
			for (; j < 4; ++j)
			{
				// Find the biggest possible heigth 
				if (processing.height >= chunkWidths[j])
				{
					current.height = chunkWidths[j];
					break;
				}
			}

			chunks[*chunkCount] = current;
			++*chunkCount;
	
			u8 remainingWidth = processing.width - current.width;
			if (remainingWidth)
				ProcessChunk({ (u8)(processing.x + current.width), processing.y, remainingWidth, current.height }, chunks, chunkCount);

			u8 remainingHeight = processing.height - current.height;
			if (remainingHeight)
				ProcessChunk({ processing.x, (u8)(processing.y + current.height), processing.width, remainingHeight }, chunks, chunkCount);
	
			break;
		}
	}
}

void Image::Detangle()
{
	if (type != COLOR_TYPE_PALETTE)
		return;
	if (format != FORMAT_HORIZONTAL)
		return;
	if (depth != 4 && depth != 8)
		return;

	u8* buffer = new u8[size];

	u32 widthInTiles = width / TILE_SIZE;
	u32 heightInTiles = height / TILE_SIZE;

	Chunk chunks[16];
	u32 chunkCount = 0;

	ProcessChunk({ 0, 0, (u8)widthInTiles ,(u8)heightInTiles }, chunks, &chunkCount);

	// u32 tileCount = widthInTiles * heightInTiles;

	u32 tileWidth = TILE_SIZE;
	if (depth == 4)
		tileWidth /= 2;
	u32 tileSize = tileWidth * TILE_SIZE;

	u32 currentTile = 0;
	for (u32 i = 0; i < chunkCount; ++i)
	{
		Chunk chunk = chunks[i];

		for (u32 y = 0; y < chunk.height; ++y)
			for (u32 x = 0; x < chunk.width; ++x)
			{
				u32 actx = chunk.x + x;
				u32 acty = chunk.y + y;
				memcpy(buffer + (acty * widthInTiles * tileSize) + (actx * tileSize), data + (currentTile * tileSize), tileSize);

				++currentTile;
			}
	}
	
    if (data) { delete[] data; data = nullptr; }

	data = buffer;
}

void Image::Tangle()
{
	if (type != COLOR_TYPE_PALETTE)
		return;
	if (format != FORMAT_HORIZONTAL)
		return;
	if (depth != 4 && depth != 8)
		return;

	u8* buffer = new u8[size];

    u32 widthInTiles = width / TILE_SIZE;
    u32 heightInTiles = height / TILE_SIZE;

    Chunk chunks[16];
    u32 chunkCount = 0;

    ProcessChunk({0, 0, (u8)widthInTiles, (u8)heightInTiles}, chunks, &chunkCount);

    u32 tileCount = widthInTiles * heightInTiles;

    u32 tileWidth = TILE_SIZE;
    if (depth == 4)
        tileWidth /= 2;
    u32 tileSize = tileWidth * TILE_SIZE;

    u32 currentTile = 0;
    for (u32 i = 0; i < chunkCount; ++i)
    {
        Chunk chunk = chunks[i];

        for (u32 y = 0; y < chunk.height; ++y)
            for (u32 x = 0; x < chunk.width; ++x)
            {
                u32 actx = chunk.x + x;
                u32 acty = chunk.y + y;
                memcpy(buffer + (currentTile * tileSize), data + (acty * widthInTiles * tileSize) + (actx * tileSize), tileSize);

                ++currentTile;
            }
    }

    if (data) { delete[] data; data = nullptr; }

    data = buffer;
}
