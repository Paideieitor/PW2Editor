#ifndef _NCLR_CACHE_H
#define _NCLR_CACHE_H

#include "Utils/Internal/Cache/Cache.h"
#include "Utils/Internal/Palette.h"
#include "Utils/NDS/NarcUtils.h"
#include "Utils/NDS/NCLRFormat.h"

#include "Data/Project.h"

class NCLRCache : public Cache<Palette>
{
public:
    NCLRCache(u32 capacity) : Cache<Palette>(capacity) {}

protected:

    virtual bool LoadEntry(Palette& palette, string& dataStr, string keyStr, u64 keyVar, va_list args) override
    {
        Project* project = (Project*)va_arg(args, void*);

        FileStream fileStream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, keyStr, keyVar);
        if (!fileStream.data)
            return false;

        bool output = true;
        if (!LoadNCLR(fileStream, palette))
            output = false;

        if (output)
        {
            dataStr = string("tmp") + PATH_SEPARATOR + GenerateDataStr(keyStr, keyVar);
            SaveFileStream(fileStream, dataStr);
        }

        ReleaseFileStream(fileStream);

        return output;
    }

    virtual void UnloadEntry(Palette& palette, const string& dataStr) override 
    {
        if (!dataStr.empty()) RemoveFile(dataStr);
    }
};

#endif // _NCLR_CACHE_H
