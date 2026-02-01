#include "System.h"

#include <cstring>

#include "Utils/KlangFormat.h"
#include "Utils/FileUtils.h"

#define KLANG_VAR_PREFIX "#define"
#define KLANG_IGNORE_PREFIX "#ifndef"

bool IsSeparator(char c)
{
	char separators[3] = { ' ', '\t', '\n' };
	for (u32 idx = 0; idx < 3u; ++idx)
		if (c == separators[idx])
			return true;
	return false;
}
u32 GetIndentation(const string& str)
{
	u32 indentation = 0;
	for (u32 idx = 0; idx < (u32)str.length(); ++idx)
	{
		switch (str[idx])
		{
		case ' ':
			++indentation;
			break;
		case '\t':
			indentation += 4;
			break;
		default:
			return indentation;
		}
	}
	return indentation;
}
bool KlangIsNumber(char c)
{
	char numbers[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-' };
	for (u32 idx = 0; idx < 11u; ++idx)
		if (c == numbers[idx])
			return true;
	return false;
}

bool KlangVar::SetValue(int value)
{
	if (this->value == value)
		return false;

	FILE* klangFile = fopen(this->path.c_str(), "r");
	if (!klangFile)
	{
		Log(WARNING, "Couldn't open KLANG file to edit (%s)", this->path.c_str());
		return false;
	}

	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	bool found = false;
	while (!feof(klangFile))
	{
		char line[1024];
		fgets(line, 1024, klangFile);
		string lineStr = line;

		int start = (int)lineStr.find(this->name);
		if (start >= 0 && !found)
		{
			this->value = value;

			for (start += (int)this->name.length(); start < (int)lineStr.length(); ++start)
				if (KlangIsNumber(lineStr[start]))
					break;
			int end = start + 1;
			for (; end < (int)lineStr.length(); ++end)
				if (!KlangIsNumber(lineStr[end]))
					break;

			lineStr.erase(start, end - start);
			lineStr.insert(start, to_string(value));

			found = true;
		}

		if (!feof(klangFile))
			FileStreamBufferWriteBack(fileStream, (u8*)lineStr.c_str(), (u32)lineStr.length());
	}
	fclose(klangFile);

	return SaveFileStream(fileStream, this->path.c_str());
}

bool Klang::Load(Klang& klang, const string& path)
{
	FILE* klangFile = fopen(path.c_str(), "r");
	if (!klangFile)
	{
		Log(WARNING, "Couldn't open KLANG file to load (%s)", path.c_str());
		return false;
	}

	klang = Klang();
	klang.path = path;

	string ignoreDefine;
	while (!feof(klangFile))
	{
		char line[1024];
		fgets(line, 1024, klangFile);
		string lineStr = line;

		int start = (int)lineStr.find(KLANG_VAR_PREFIX);
		if (start < 0)
		{
			if (!ignoreDefine.size())
			{
				start = (int)lineStr.find(KLANG_IGNORE_PREFIX);
				if (start < 0)
					continue;

				KlangVar var = GetKlangVar(lineStr, (u32)start + (u32)strlen(KLANG_IGNORE_PREFIX));
				ignoreDefine = var.name;
			}
			continue;
		}

		KlangVar var = GetKlangVar(lineStr, (u32)start + (u32)strlen(KLANG_VAR_PREFIX));
		if (var.name.empty() || var.name == ignoreDefine)
			continue;
		var.path = klang.path;

		if (!GetIndentation(lineStr))
		{
			var.parentIdx = -1;
			klang.vars.push_back(var);
		}
		else
		{
			var.parentIdx = (int)klang.vars.size() - 1;
			klang.vars.back().dependentVars.push_back(var);
		}
	}

	fclose(klangFile);
	return true;
}

KlangVar Klang::GetKlangVar(const string& line, u32 offset)
{
	string name;
	string value;

	u32 idx = offset;
	for (; idx < (u32)line.size(); ++idx)
	{
		if (IsSeparator(line[idx]))
		{
			if (name.empty())
				continue;
			else
				break;
		}
		name.push_back(line[idx]);
	}
	for (++idx; idx < (u32)line.size(); ++idx)
	{
		if (IsSeparator(line[idx]))
		{
			if (value.empty())
				continue;
			else
				break;
		}
		if (!KlangIsNumber(line[idx]))
			break;
		value.push_back(line[idx]);
	}

	if (value.empty())
		return KlangVar(string());

	KlangVar output(name);
	output.value = stoi(value);
	if (output.value != 0 && output.value != 1)
		return KlangVar(string());
		
	return output;
}

bool LoadKlang(Klang& klang, const string& path)
{
	return Klang::Load(klang, path);
}
