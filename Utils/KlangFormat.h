#ifndef _KLANG_FORMAT_H
#define _KLANG_FORMAT_H

#include <vector>
#include <string>

#include "Globals.h"

class KlangVar
{
public:

	KlangVar() = delete;
	KlangVar(const string& name) : name(name) {}

	const string name;
	vector<KlangVar> dependentVars = vector<KlangVar>();

	int Value() const { return value; }
	bool SetValue(int value);

private:

	int value = 0;

	string path = string();
	int parentIdx = -1;

	friend class Klang;
};

class Klang
{
public:

	vector<KlangVar> vars = vector<KlangVar>();

private:

	string path = "";

	static bool Load(Klang& klang, const string& path);
	static KlangVar GetKlangVar(const string& line, u32 offset);

	friend bool LoadKlang(Klang& klang, const string& path);
};

bool LoadKlang(Klang& klang, const string& path);

#endif // _KLANG_FORMAT_H
