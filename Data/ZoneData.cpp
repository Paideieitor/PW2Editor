#include "Data/ZoneData.h"

bool LoadZone(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	u32 currentByte = 0;

	u32 currentZone = 0;
	while (!FileStreamEnded(fileStream, currentByte + 1))
	{
		data.emplace_back((int)FileStreamReadUpdate<u8>(fileStream, currentByte));  //MAP_TYPE,
		data.emplace_back((int)FileStreamReadUpdate<u8>(fileStream, currentByte));  //NPC_CACHE_IDX,

		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //AREA_ID,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //MATRIX_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //SCRIPTS_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //LEVEL_SCRIPTS_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //TEXTS_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //MUSIC_SPRING,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //MUSIC_SUMMER,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //MUSIC_AUTUM,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //MUSIC_WINTER,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //ENCOUNTER_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //ENTITIES_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //PARENT_ZONE_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //LOCATION_NAME_IDX,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //EVENT_FLAGS,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //BATTLE_TRANSITION_FLAGS,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //CAMERA_BOUNDS,
		data.emplace_back((int)FileStreamReadUpdate<u16>(fileStream, currentByte)); //ICON,

		data.emplace_back((int)FileStreamReadUpdate<u32>(fileStream, currentByte)); //FLY_X,
		data.emplace_back((int)FileStreamReadUpdate<u32>(fileStream, currentByte)); //FLY_Y,
		data.emplace_back((int)FileStreamReadUpdate<u32>(fileStream, currentByte)); //FLY_Z,

		if (currentZone != 0)
			concatIndices.emplace_back(currentZone);
		++currentZone;
	}

	return true;
}
bool SaveZone(const vector<int> data, FileStream& fileStream)
{
	u32 zoneCount = (u32)data.size() / ZONE_SIZE;
	for (u32 idx = 0; idx < zoneCount; ++idx)
	{
		FileStreamPutBack<u8>(fileStream, (u8)data.at(ZONE_FIELD(idx, MAP_TYPE)));
		FileStreamPutBack<u8>(fileStream, (u8)data.at(ZONE_FIELD(idx, NPC_CACHE_IDX)));

		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, NPC_CACHE_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, AREA_ID)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, MATRIX_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, SCRIPTS_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, LEVEL_SCRIPTS_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, TEXTS_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, MUSIC_SPRING)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, MUSIC_SUMMER)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, MUSIC_AUTUM)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, MUSIC_WINTER)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, ENCOUNTER_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, ENTITIES_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, PARENT_ZONE_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, LOCATION_NAME_IDX)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, EVENT_FLAGS)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, BATTLE_TRANSITION_FLAGS)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, CAMERA_BOUNDS)));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(ZONE_FIELD(idx, ICON)));

		FileStreamPutBack<u32>(fileStream, (u32)data.at(ZONE_FIELD(idx, FLY_X)));
		FileStreamPutBack<u32>(fileStream, (u32)data.at(ZONE_FIELD(idx, FLY_Y)));
		FileStreamPutBack<u32>(fileStream, (u32)data.at(ZONE_FIELD(idx, FLY_Z)));
	}

	return true;
}

void ZoneData::GenerateSections(const vector<string>& narcPaths)
{
	flags = Single_Block;
	blockSize = ZONE_SIZE;
	nullValue = ZONE_NULL;

	sections.emplace_back(narcPaths.at(0), ZONE_START, ZONE_SIZE, LoadZone, SaveZone);
}
