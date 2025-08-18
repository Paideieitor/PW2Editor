#ifndef _ALLE5_FORMAT_H
#define _ALLE5_FORMAT_H

#include <string>
#include <vector>

#include "Globals.h"

#include "Utils/FileUtils.h"

// The text narc file code is adapted to C++ from Kaphotics xytext application 
// https://github.com/kwsch/xytext/blob/master/xytext/Form1.cs#L110

bool LoadAlle5Data(const FileStream& fileStream, vector<string>& lines);
bool LoadAlle5File(const string& path, vector<string>& lines);

bool SaveAlle5File(const string& path, const vector<string>& lines);

#endif // _ALLE5_FORMAT_H

