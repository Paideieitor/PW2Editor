#ifndef _NCGR_FORMAT_H
#define _NCGR_FORMAT_H

#include "Utils/FileUtils.h"
#include "Utils/Internal/Image.h"

bool LoadNCGR(const FileStream& fileStream, Image& image, u32 flags);
bool SaveNCGR(FileStream& fileStream, const Image& image, u32 flags);

#endif // _NCGR_FORMAT_H
