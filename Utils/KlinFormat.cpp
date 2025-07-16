#include <map>

#include "Log.h"

#include "Utils/KlinFormat.h"
#include "Utils/StringUtils.h"

#define KLIN_SEPARATOR ": "
#define KLIN_LIST_SEPARATOR ','

void LoadKlin(KlinHandler& handler, const string& path)
{
	FILE* klinFile = nullptr;
	fopen_s(&klinFile, path.c_str(), "r");
	if (!klinFile)
	{
		Log(WARNING, "Couldn't open KLIN file to load (%s)", path);
		return;
	}

	map<KlinValue, string>* klinMap = new map<KlinValue, string>();

	while (!feof(klinFile))
	{
		char line[1024];
		fgets(line, 1024, klinFile);
		string lineStr = line;

		for (u32 value = KLIN_NULL + 1; value < KLIN_MAX_VALUE; ++value)
		{
			if (lineStr.find(string(KlinParams[value].name) + KLIN_SEPARATOR) == 0)
			{
				string data = lineStr.substr(strlen(KlinParams[value].name) + sizeof(KLIN_SEPARATOR) - 1);
				data.pop_back();
				if (!data.size())
				{
					Log(WARNING, "Dataless KLIN %s value (%s)", KlinParams[value].name, path);
					continue;
				}

				klinMap->insert_or_assign((KlinValue)value, data);
				break;
			}
		}
	}
	fclose(klinFile);

	handler = klinMap;
}

void ReleaseKlin(KlinHandler& handler)
{
	delete handler;
	handler = nullptr;
}

void SaveKlin(KlinHandler& handler, const string& path, bool release)
{
	if (!handler)
	{
		Log(WARNING, "Can't save a null KLIN handler (%s)", path);
		return;
	}

	FILE* klinFile = nullptr;
	fopen_s(&klinFile, path.c_str(), "w");
	if (!klinFile)
	{
		Log(WARNING, "Couldn't open KLIN file to save (%s)", path);
		return;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;

	for (u32 value = KLIN_NULL + 1; value < KLIN_MAX_VALUE; ++value)
	{
		map<KlinValue, string>::iterator itr = klinMap->find((KlinValue)value);
		if (itr != klinMap->end())
			fprintf(klinFile, "%s%s%s\n", KlinParams[itr->first].name, KLIN_SEPARATOR, itr->second.c_str());
	}
	fclose(klinFile);

	if (release)
		ReleaseKlin(handler);
}

string GetKlinValue(const KlinHandler& handler, const KlinValue value)
{
	if (!handler)
	{
		Log(WARNING, "Can't get a %s value from a null KLIN handler", KlinParams[value].name);
		return string();
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;
	map<KlinValue, string>::iterator itr = klinMap->find((KlinValue)value);
	if (itr != klinMap->end())
		return itr->second;

	//Log(INFO, "KLIN handler does not contain %s value", KlinParams[value].name);
	return string();
}

bool GetKlinValueString(const KlinHandler& handler, const KlinValue value, string& output)
{
	if (KlinParams[value].type != KLIN_STRING)
	{
		Log(WARNING, "%s value is not a string type", KlinParams[value].name);
		return false;
	}

	string klinOutput = GetKlinValue(handler, value);
	if (!klinOutput.size())
		return 0;

	output = klinOutput;
	return true;
}

bool GetKlinValueU32(const KlinHandler& handler, const KlinValue value, u32& output)
{
	if (KlinParams[value].type != KLIN_U32)
	{
		Log(WARNING, "%s value is not a u32 type", KlinParams[value].name);
		return false;
	}

	string klinOutput = GetKlinValue(handler, value);
	if (!klinOutput.size())
		return false;

	output = (u32)stoi(klinOutput);
	return true;
}

vector<string> GetKlinList(const KlinHandler& handler, const KlinValue value)
{
	if (!handler)
	{
		Log(WARNING, "Can't get a %s value from a null KLIN handler", KlinParams[value].name);
		return vector<string>();
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;
	map<KlinValue, string>::iterator itr = klinMap->find((KlinValue)value);
	if (itr != klinMap->end())
		return StringToStringVector(itr->second, KLIN_LIST_SEPARATOR);
		
	//Log(INFO, "KLIN handler does not contain %s list", KlinParams[value].name);
	return vector<string>();
}

bool GetKlinListString(const KlinHandler& handler, const KlinValue value, vector<string>& output)
{
	if (KlinParams[value].type != KLIN_STRING_LIST)
	{
		Log(WARNING, "%s value is not a string list type", KlinParams[value].name);
		return false;
	}

	vector<string> klinOutput = GetKlinList(handler, value);
	if (!klinOutput.size())
		return false;

	output = klinOutput;
	return true;
}

bool GetKlinListU32(const KlinHandler& handler, const KlinValue value, vector<u32>& output)
{
	if (KlinParams[value].type != KLIN_U32_LIST)
	{
		Log(WARNING, "%s value is not a string list type", KlinParams[value].name);
		return false;
	}

	vector<string> klinOutput = GetKlinList(handler, value);
	if (!klinOutput.size())
		return false;

	for (u32 idx = 0; idx < (u32)klinOutput.size(); ++idx)
		output.emplace_back((u32)stoi(klinOutput[idx]));
	return true;
}

bool SetKlinValueString(const KlinHandler& handler, const KlinValue value, const string& param)
{
	if (!handler)
	{
		Log(WARNING, "Can't set a %s value from a null KLIN handler", KlinParams[value].name);
		return false;
	}

	if (KlinParams[value].type != KLIN_STRING)
	{
		Log(WARNING, "%s value is not a string type", KlinParams[value].name);
		return false;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;
	klinMap->insert_or_assign((KlinValue)value, param);
	return true;
}

bool SetKlinValueU32(const KlinHandler& handler, const KlinValue value, const u32 param)
{
	if (!handler)
	{
		Log(WARNING, "Can't set a %s value from a null KLIN handler", KlinParams[value].name);
		return false;
	}

	if (KlinParams[value].type != KLIN_U32)
	{
		Log(WARNING, "%s value is not a u32 type", KlinParams[value].name);
		return false;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;
	klinMap->insert_or_assign((KlinValue)value, to_string(param));
	return true;
}

bool SetKlinListString(const KlinHandler& handler, const KlinValue value, const vector<string>& param)
{
	if (!handler)
	{
		Log(WARNING, "Can't set a %s value from a null KLIN handler", KlinParams[value].name);
		return false;
	}

	if (KlinParams[value].type != KLIN_STRING_LIST)
	{
		Log(WARNING, "%s value is not a string list type", KlinParams[value].name);
		return false;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;

	string input;
	for (u32 idx = 0; idx < (u32)param.size(); ++idx)
		input += param[idx] + KLIN_LIST_SEPARATOR;
	klinMap->insert_or_assign((KlinValue)value, input);
	return true;
}

bool SetKlinListU32(const KlinHandler& handler, const KlinValue value, const vector<u32>& param)
{
	if (!handler)
	{
		Log(WARNING, "Can't set a %s value from a null KLIN handler", KlinParams[value].name);
		return false;
	}

	if (KlinParams[value].type != KLIN_U32_LIST)
	{
		Log(WARNING, "%s value is not a string list type", KlinParams[value].name);
		return false;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;

	string input;
	for (u32 idx = 0; idx < (u32)param.size(); ++idx)
		input += to_string(param[idx]) + KLIN_LIST_SEPARATOR;
	klinMap->insert_or_assign((KlinValue)value, input);
	return true;
}

void DeleteKlinValue(const KlinHandler& handler, const KlinValue value)
{
	if (!handler)
	{
		Log(WARNING, "Can't delete a %s value from a null KLIN handler", KlinParams[value].name);
		return;
	}

	map<KlinValue, string>* klinMap = (map<KlinValue, string>*)handler;
	klinMap->erase((KlinValue)value);
}