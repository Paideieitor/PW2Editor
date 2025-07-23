#include "Globals.h"

#include "Data/EncounterData.h"

void EncounterReset(EncounterData& encounters)
{
	for (u32 season = 0; season < (u32)encounters.size(); ++season)
		EncounterTableReset(encounters[season]);
}

void EncounterTableReset(EncounterTable& table)
{
	for (u32 encounter = 0; encounter < (u32)table.size(); ++encounter)
		for (u32 slot = 0; slot < (u32)table[encounter].size(); ++slot)
			table[encounter][slot] = ENCOUNTERDATA_NULL;
}

bool EncounterTableIsEmpty(const EncounterTable& table)
{
	for (u32 rate = LAND_SINGLE_RATE; rate <= UNKNOWN_RATE; ++rate)
		if (table[rate][ENCOUNTER_SPECIES] == ENCOUNTERDATA_NULL)
			return true;
	return false;
}

void EncounterTableSetDefault(EncounterTable& table)
{
	for (u32 rate = LAND_SINGLE_RATE; rate <= UNKNOWN_RATE; ++rate)
		table[rate][ENCOUNTER_SPECIES] = 0;

	for (u32 encounter = ENCOUNTER_SINGLE; encounter < (u32)table.size(); ++encounter)
	{
		table[encounter][ENCOUNTER_SPECIES] = 1;
		table[encounter][ENCOUNTER_FORM] = 0;
		table[encounter][ENCOUNTER_MIN_LEVEL] = 1;
		table[encounter][ENCOUNTER_MAX_LEVEL] = 1;
	}
}

EncounterSlot GetEncounterSlot(u32 data)
{
	EncounterSlot slot = EncounterSlot();

	slot[ENCOUNTER_SPECIES] = (int)(data & 0x000007FF);
	slot[ENCOUNTER_FORM] = (int)((data & 0x0000F800) >> 11);
	slot[ENCOUNTER_MIN_LEVEL] = (int)((data & 0x00FF0000) >> 16);
	slot[ENCOUNTER_MAX_LEVEL] = (int)((data & 0xFF000000) >> 24);

	return slot;
}

u32 SetEncounterSlot(const EncounterSlot& slot)
{
	u32 data = 0;

	data |= slot[ENCOUNTER_SPECIES]& 0x000007FF;
	data |= (slot[ENCOUNTER_FORM] << 11) & 0x0000F800;
	data |= (slot[ENCOUNTER_MIN_LEVEL] << 16) & 0x00FF0000;
	data |= (slot[ENCOUNTER_MAX_LEVEL] << 24) & 0xFF000000;

	return data;
}
