#include "Data/EncounterData.h"

void GetEncounterSlot(u32 slotData, int& species, int& form, int& min, int& max)
{
	species = (int)(slotData & 0x000007FF);
	form = (int)((slotData & 0x0000F800) >> 11);
	min = (int)((slotData & 0x00FF0000) >> 16);
	max = (int)((slotData & 0xFF000000) >> 24);
}
u32 SetEncounterSlot(int species, int form, int min, int max)
{
	u32 data = 0;
	data |= species & 0x000007FF;
	data |= (form << 11) & 0x0000F800;
	data |= (min << 16) & 0x00FF0000;
	data |= (max << 24) & 0xFF000000;
	return data;
}

bool LoadEncounter(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
    UNUSED(concatIndices);

	u32 currentByte = 0;
	for (u32 season = SUMMER; season < SEASON_MAX; ++season)
	{
		if (FileStreamEnded(fileStream, currentByte))
			break;

		u32 idx = 0;
		for (; idx < ENCOUNTER_TYPE_MAX; ++idx)
			data.at(SEASON_START(season) + idx) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

		for (; idx < SEASON_SIZE; idx += ENCOUNTER_MAX)
		{
			u32 slotData = FileStreamReadUpdate<u32>(fileStream, currentByte);
			GetEncounterSlot(slotData, 
				data.at(SEASON_START(season) + idx), data.at(SEASON_START(season) + idx + 1),
				data.at(SEASON_START(season) + idx + 2), data.at(SEASON_START(season) + idx + 3));
		}
	}

	return true;
}
bool SaveEncounter(const vector<int> data, FileStream& fileStream)
{
	for (u32 season = SUMMER; season < SEASON_MAX; ++season)
	{
		u32 idx = 0;

		if (data.at(SEASON_START(season) + idx) == ENCOUNTER_NULL)
			break;

		for (; idx < ENCOUNTER_TYPE_MAX; ++idx)
			FileStreamPutBack<u8>(fileStream, (u8)data.at(SEASON_START(season) + idx));

		for (; idx < SEASON_SIZE; idx += ENCOUNTER_MAX)
		{
			u32 slotData = SetEncounterSlot(
				data.at(SEASON_START(season) + idx), data.at(SEASON_START(season) + idx + 1),
				data.at(SEASON_START(season) + idx + 2), data.at(SEASON_START(season) + idx + 3));
			FileStreamPutBack<u32>(fileStream, slotData);
		}
	}

	return true;
}

void EncounterData::GenerateSections(const vector<string>& narcPaths)
{
	flags = No_Flags;
	blockSize = ENCOUNTER_SIZE;
	nullValue = ENCOUNTER_NULL;

	sections.emplace_back(narcPaths.at(0), ENCOUNTER_RATE_START, ENCOUNTER_SIZE, LoadEncounter, SaveEncounter);
}
