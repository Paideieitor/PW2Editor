#ifndef _NCLR_FORMAT_H
#define _NCLR_FORMAT_H

#include "Utils/FileUtils.h"
#include "Utils/Internal/Palette.h"

bool LoadNCLR(const FileStream& fileStream, Palette& palette);
bool SaveNCLR(FileStream& fileStream, const Palette& palette);

#endif // _NCLR_FORMAT_H
