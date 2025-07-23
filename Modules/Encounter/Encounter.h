#ifndef _ENCOUNTER_H
#define _ENCOUNTER_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/EncounterData.h"

enum EncounterRateFunctions
{
	ENCOUNTER_RATE_12_SLOTS = 0,
	ENCOUNTER_RATE_5_SLOTS_1 = 1,
	ENCOUNTER_RATE_5_SLOTS_2 = 2,
	ENCOUNTER_RATE_EQUAL = 3, // used for swarms
};

struct EncounterTableInfo
{
	const string name = string();
	const u32 size = 0;
	const EncounterRateFunctions  rate = ENCOUNTER_RATE_12_SLOTS;
};

class Encounter : public Module
{
public:
	Encounter() = delete;
	Encounter(Engine* const engine, u32 group);
	~Encounter();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

	vector<EncounterTableInfo> tablesInfo = {
		{"Land Single", SINGLE_SIZE, ENCOUNTER_RATE_12_SLOTS},
		{"Land Doubles", DOUBLES_SIZE, ENCOUNTER_RATE_12_SLOTS},
		{"Land Special", SPECIAL_SIZE, ENCOUNTER_RATE_12_SLOTS},
		{"Surf", SURF_SIZE, ENCOUNTER_RATE_5_SLOTS_1},
		{"Surf Special", SURF_SPECIAL_SIZE, ENCOUNTER_RATE_5_SLOTS_1},
		{"Fishing", FISHING_SIZE, ENCOUNTER_RATE_5_SLOTS_2},
		{"Fishing Special", FISHING_SPECIAL_SIZE, ENCOUNTER_RATE_5_SLOTS_2},
	};
	vector<vector<string>> encounterRates = {
		{"20", "20", "10", "10", "10", "10", "  5", "  5", "  4", "  4", "  1", "  1"},
		{"60", "30", "  5", "  4", "  1"},
		{"40", "40", "15", "  4", "  1"},
		{"  0"},
	};

private:

	void ComboBox(EncounterData& encounterData, const char* label, const std::vector<std::string>& items, Season season, EncounterField field, EncounterSlotField slot);
	void InputInt(EncounterData& encounterData, const char* label, Season season, EncounterField field, EncounterSlotField slot, int maxValue);
	void CheckBox(EncounterData& encounterData, const char* label, Season season, EncounterField field, EncounterSlotField slot);

	EncounterTable encounterCopy = EncounterTable();
};

#endif // _ENCOUNTER_H
