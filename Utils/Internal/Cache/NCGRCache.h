#ifndef _NCGR_CACHE_H
#define _NCGR_CACHE_H

#include "Utils/Internal/Cache/Cache.h"
#include "Utils/Internal/Image.h"
#include "Utils/NDS/NarcUtils.h"
#include "Utils/NDS/NCGRFormat.h"

#include "Data/Project.h"

class NCGRCache : public Cache<Image>
{
public:
    NCGRCache(u32 capacity) : Cache<Image>(capacity) {}

protected:

    virtual bool LoadEntry(Image& image, string& dataStr, string keyStr, u64 keyVar, va_list args) override
    {
        Project* project = (Project*)va_arg(args, void*);
        u32 flags = (u32)va_arg(args, int);

        FileStream fileStream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, keyStr, keyVar);
        if (!fileStream.data)
            return false;

        bool output = true;
        if (!LoadNCGR(fileStream, image, flags))
            output = false;

        if (output)
        {
            dataStr = string("tmp") + PATH_SEPARATOR + GenerateDataStr(keyStr, keyVar);
            SaveFileStream(fileStream, dataStr);
        }

        ReleaseFileStream(fileStream);

        return output;
    }

    virtual void UnloadEntry(Image& image, const string& dataStr) override
    {
        if (image.data) delete[] image.data;
        if (!dataStr.empty()) RemoveFile(dataStr);
    }
};

#endif // _NCGR_CACHE_H
