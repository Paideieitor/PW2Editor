#ifndef _ENCOUNTER_MODULE_H
#define _ENCOUNTER_MODULE_H

#include "Modules/Module.h"

class Encounter : public Module
{
public:
	Encounter() = delete;
	Encounter(Engine* const engine, u32 group);
	~Encounter() {}

	virtual ReturnState RenderGUI() override;

private:

	int Zones(u32 location);
	void Season(u32 encounterIdx, u32 season);
	u32 Table(u32 encounterIdx, u32 season, const string& label, u32 type, u32 ratesIdx, u32 firstSlot);

	bool HasSeasons(u32 encounterIdx);

	void FillTable(u32 encounterIdx, u32 season, int value);

	void Copy(u32 encounterIdx, u32 season);
	void Paste(u32 encounterIdx, u32 season);
	
	const vector<string>* const locations;
	const vector<string>* const pokemons;

	const vector<string> seasons = {
		"Summer",
		"Spring",
		"Autum",
		"Winter",
	};
	vector<vector<string>> rates = {
	{"20", "20", "10", "10", "10", "10", "  5", "  5", "  4", "  4", "  1", "  1"},
	{"60", "30", "  5", "  4", "  1"},
	{"40", "40", "15", "  4", "  1"},
	{"  0"},
	};

	int copied[SEASON_SIZE];
};

#endif // _ENCOUNTER_MODULE_H

