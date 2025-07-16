#ifndef _NARC_UTILS_H
#define _NARC_UTILS_H

#include <vector>
#include <string>

#include "Globals.h"

// This code is entirelly based on KNarc by kr3nshaw (https://github.com/kr3nshaw/knarc)

int NarcUnpack(const string& path, const string& savePath);
int NarcUnpackSingle(const string& path, const string& savePath, u16 fileID);
int NarcUnpackBundle(const string& path, const string& savePath, const vector<u16>& fileIDs);
int NarcUnpackExclude(const string& path, const string& savePath, const vector<u16>& excludeIDs);

#endif // _NARC_UTILS_H