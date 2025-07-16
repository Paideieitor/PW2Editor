#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H

#include <vector>
#include <string>

#include "Globals.h"

/*
    Copyright 2014, Jason Kozak
    Copyright 2019, Tarmo Pikaro
    https://github.com/tapika/cutf

    Based on UTF8-CPP: Copyright 2006, Nemanja Trifunovicm
*/

std::wstring Utf8ToWide(const std::string& str);
std::string  WideToUtf8(const std::wstring& wstr);

vector<string> StringToStringVector(const string& input, char separator);
vector<u32> StringToU32Vector(const string& input, char separator);

string LowerCase(const string& input);
string UpperCase(const string& input);

bool IsNumber(char character);

#endif // _STRING_UTILS_H
