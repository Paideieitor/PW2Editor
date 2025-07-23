#ifndef _ENCOUNTER_DATA_H
#define _ENCOUNTER_DATA_H

#include <array>

enum EncounterSlotField
{
	ENCOUNTER_SPECIES,
	ENCOUNTER_FORM,
	ENCOUNTER_MIN_LEVEL,
	ENCOUNTER_MAX_LEVEL,
	ENCOUNTERSLOTDATA_MAX,
};

typedef std::array<int, ENCOUNTERSLOTDATA_MAX> EncounterSlot;

#define SINGLE_SIZE 12
#define DOUBLES_SIZE 12
#define SPECIAL_SIZE 12
#define SURF_SIZE 5
#define SURF_SPECIAL_SIZE 5
#define FISHING_SIZE 5
#define FISHING_SPECIAL_SIZE 5

enum EncounterField
{
	LAND_SINGLE_RATE = 0,
	LAND_DOUBLE_RATE = 1,
	LAND_SPECIAL_RATE = 2,
	SURF_RATE = 3,
	SURF_SPECIAL_RATE = 4,
	FISHING_RATE = 5,
	FISHING_SPECIAL_RATE = 6,
	UNKNOWN_RATE = 7,
	ENCOUNTER_SINGLE = 8,
	ENCOUNTER_DOUBLES = ENCOUNTER_SINGLE + SINGLE_SIZE,
	ENCOUNTER_SPECIAL = ENCOUNTER_DOUBLES + DOUBLES_SIZE,
	ENCOUNTER_SURF = ENCOUNTER_SPECIAL + SPECIAL_SIZE,
	ENCOUNTER_SURF_SPECIAL = ENCOUNTER_SURF + SURF_SIZE,
	ENCOUNTER_FISHING = ENCOUNTER_SURF_SPECIAL + SURF_SPECIAL_SIZE,
	ENCOUNTER_FISHING_SPECIAL = ENCOUNTER_FISHING + FISHING_SIZE,
	ENCOUNTERDATA_MAX = ENCOUNTER_FISHING_SPECIAL + FISHING_SPECIAL_SIZE,
};

typedef std::array<EncounterSlot, ENCOUNTERDATA_MAX> EncounterTable;

enum Season
{
	SUMMER,
	SPRING,
	FALL,
	WINTER,
	SEASON_MAX,
};

typedef std::array<EncounterTable, SEASON_MAX> EncounterData;

void EncounterReset(EncounterData& encounter);

void EncounterTableReset(EncounterTable& table);
bool EncounterTableIsEmpty(const EncounterTable& table);
void EncounterTableSetDefault(EncounterTable& table);

EncounterSlot GetEncounterSlot(u32 data);
u32 SetEncounterSlot(const EncounterSlot& slot);

#define ENCOUNTERDATA_NULL -1

#endif // _ENCOUNTER_DATA_H
