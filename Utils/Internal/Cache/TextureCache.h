#ifndef _TEXTURE_CACHE_H
#define _TEXTURE_CACHE_H

#include "Externals/glad/glad.h"
#include "Externals/imgui/imgui.h"

#include "Utils/Internal/Cache/NCGRCache.h"
#include "Utils/Internal/Cache/NCLRCache.h"

enum TextureType : u32
{
    TEXTURE_FULL,
    TEXTURE_RECT,
};

struct Texture
{
    ImTextureID texture;
    u32 width;
    u32 height;
    u32 flags;
};

class TextureCache : public Cache<Texture>
{
public:
    TextureCache(u32 capacity) : Cache<Texture>(capacity), palCache(capacity), imgCache(capacity) {}
    ~TextureCache()
    {
        palCache.Release();
        imgCache.Release();
    }

protected:
    virtual string GenKey(string keyStr, u64 keyVar, va_list args) override
    {
        Project* project = (Project*)va_arg(args, void*);
        TextureType type = (TextureType)va_arg(args, int);
        u32 flags = (u32)va_arg(args, int);
        u32 palIdx = (u32)va_arg(args, int);

        u32 imgFileIdx = keyVar & 0x00000000FFFFFFFF;
        u32 palFileIdx = (keyVar & 0xFFFFFFFF00000000) >> 32;

        string key = keyStr + "-" + to_string(imgFileIdx) + "-" + to_string(palFileIdx);
        key = key + "-" + to_string(palIdx);

        switch(type)
        {
            case TEXTURE_FULL:
                break;
            case TEXTURE_RECT:
                u32 x = (u32)va_arg(args, int); 
                u32 y = (u32)va_arg(args, int);
                u32 width = (u32)va_arg(args, int);
                u32 height = (u32)va_arg(args, int);
                
                key = key + "-" + to_string(x) + "-" + to_string(y);
                key = key + "-" + to_string(width) + "-" + to_string(height);
                break;
        }

        return key;
    }

    void GenerateTexture(Texture& texture, u16 x, u16 y, u16 width, u16 height, u32 flags, u32 palIdx, const Image& image, const Palette& palette)
    {
        GLuint glTexture = 0;
        glGenTextures(1, &glTexture);
        glBindTexture(GL_TEXTURE_2D, glTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        u32* pixels = new u32[width * height];
        for (u32 pY = y; pY < (y + height) && pY < image.height; ++pY)
        {
            u32 pos = pY * image.width;
            u32 pPos = (pY - y) * width;
            for (u32 pX = x; pX < (x + width) && pX < image.width; ++pX)
            {
                u32 colorIdx = image.data[pos + pX];
                if (colorIdx != 0 || !(flags & IMAGE_FLAG_TRANSPARENT_BG))
                    pixels[pPos + (pX - x)] = palette.Color(palIdx, colorIdx);
                else
                    pixels[pPos + (pX - x)] = 0;
            }
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] pixels;

        texture.texture = (ImTextureID)(uintptr_t)glTexture;
        texture.width = width;
        texture.height = height;
        texture.flags = flags;

    }

    virtual bool LoadEntry(Texture& texture, string& dataStr, string keyStr, u64 keyVar, va_list args) override
    {
        // return DebugPaletteRendering(texture, keyStr, keyVar, args); // uncomment to display the palettes

        Project* project = (Project*)va_arg(args, void*);
        TextureType type = (TextureType)va_arg(args, int);
        u32 flags = (u32)va_arg(args, int);
        u32 palIdx = (u32)va_arg(args, int);

        u32 imgFileIdx = keyVar & 0x00000000FFFFFFFF;
        u32 palFileIdx = (keyVar & 0xFFFFFFFF00000000) >> 32;

        Palette& palette = palCache.Get(keyStr, palFileIdx, project);
        if (palette.count == 0)
            return false;

        Image& image = imgCache.Get(keyStr, imgFileIdx, project, flags);
        if (!image.data || image.width == 0 || image.height == 0)
            return false;

        u16 x, y, width, height;
        switch(type)
        {
            case TEXTURE_FULL:
                x = 0;
                y = 0;
                width = image.width;
                height = image.height;
                break;
            case TEXTURE_RECT:
                x = (u32)va_arg(args, int); 
                y = (u32)va_arg(args, int);
                width = (u32)va_arg(args, int);
                height = (u32)va_arg(args, int);
                break;
        }
        GenerateTexture(texture, x, y, width, height, flags, palIdx, image, palette);

        return true;
    }

    virtual void UnloadEntry(Texture& texture, const string& dataStr) override
    {
        GLuint glTexture = (GLuint)texture.texture;
        glDeleteTextures(1, &glTexture);
    }


    virtual bool LoadEntryUpdate(Texture& texture, string& dataStr, string keyStr, u64 keyVar, va_list args)
    {
        Project* project = (Project*)va_arg(args, void*);
        TextureType type = (TextureType)va_arg(args, int);
        u32 flags = (u32)va_arg(args, int);
        u32 palIdx = (u32)va_arg(args, int);

        u32 imgFileIdx = keyVar & 0x00000000FFFFFFFF;
        u32 palFileIdx = (keyVar & 0xFFFFFFFF00000000) >> 32;

        Palette& palette = palCache.Update(keyStr, palFileIdx, project);
        if (palette.count == 0)
            return false;

        Image& image = imgCache.Update(keyStr, imgFileIdx, project, flags);
        if (!image.data || image.width == 0 || image.height == 0)
            return false;

        u16 x, y, width, height;
        switch(type)
        {
            case TEXTURE_FULL:
                x = 0;
                y = 0;
                width = image.width;
                height = image.height;
                break;
            case TEXTURE_RECT:
                x = (u32)va_arg(args, int); 
                y = (u32)va_arg(args, int);
                width = (u32)va_arg(args, int);
                height = (u32)va_arg(args, int);
                break;
        }
        GenerateTexture(texture, x, y, width, height, flags, palIdx, image, palette);

        return true;
    }

private:
    NCLRCache palCache;
    NCGRCache imgCache;

    bool DebugPaletteRendering(Texture& texture, string keyStr, u64 keyVar, va_list args)
    {
        Project* project = (Project*)va_arg(args, void*);
        TextureType type = (TextureType)va_arg(args, int);
        bool tangled = (bool)va_arg(args, int);
        u32 palIdx = (u32)va_arg(args, int);

        u32 imgFileIdx = keyVar & 0x00000000FFFFFFFF;
        u32 palFileIdx = (keyVar & 0xFFFFFFFF00000000) >> 32;
        
        Palette& palette = palCache.Get(keyStr, palFileIdx, project);
        if (palette.count == 0)
            return false;

        GLuint glTexture = 0;
        glGenTextures(1, &glTexture);
        glBindTexture(GL_TEXTURE_2D, glTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        u32* palPx = new u32[PALETTE_SIZE * palette.count];
        for (u32 idx = 0; idx < PALETTE_SIZE * palette.count; ++idx)
        {
            palPx[idx] = palette.colors[idx];
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PALETTE_SIZE, palette.count, 0, GL_RGBA, GL_UNSIGNED_BYTE, palPx);

        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] palPx;

        texture.texture = (ImTextureID)(uintptr_t)glTexture;
        texture.width = PALETTE_SIZE * 20;
        texture.height = palette.count * 20;

        return true;
    }
};

#endif // _TEXTURE_CACHE_H
