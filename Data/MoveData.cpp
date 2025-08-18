#include "Data/MoveData.h"

bool LoadMove(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	u32 currentByte = 0;

	data.at(MOVE_TYPE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(QUALITY) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(CATEGORY) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(POWER) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ACCURACY) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(BASE_PP) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(PRIORITY) = (int)FileStreamReadUpdate<char>(fileStream, currentByte);

	u8 hitMinMax = FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(HIT_MIN) = hitMinMax & 0x0F;
	data.at(HIT_MAX) = (hitMinMax & 0xF0) >> 4;

	data.at(CONDITION) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(CONDITION_CHANCE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(CONDITION_DURATION) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(TURN_MIN) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TURN_MAX) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(CRIT_STAGE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(FLINCH_RATE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(BEHAVIOR) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	data.at(RECOIL) = (int)FileStreamReadUpdate<char>(fileStream, currentByte);
	data.at(HEAL) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TARGET) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 statChange = 0; statChange < 3; ++statChange)
		data.at(STAT_CHANGE_1 + statChange) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		data.at(STAT_CHANGE_VOLUME_1 + statChange) = (int)FileStreamReadUpdate<char>(fileStream, currentByte);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		data.at(STAT_CHANGE_CHANCE_1 + statChange) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(PADDING) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	u32 flags = FileStreamReadUpdate<u32>(fileStream, currentByte);
	for (u32 moveFlag = 0; moveFlag < sizeof(u32) * 8; ++moveFlag)
		data.at(IS_CONTACT + moveFlag) = (flags & (1 << moveFlag)) != 0;

	return true;
}
bool SaveMove(const vector<int> data, FileStream& fileStream)
{
	FileStreamPutBack<u8>(fileStream, (u8)data.at(MOVE_TYPE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(QUALITY));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(CATEGORY));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(POWER));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ACCURACY));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(BASE_PP));
	FileStreamPutBack<char>(fileStream, (char)data.at(PRIORITY));

	u8 hitMinMax = (u8)data.at(HIT_MIN) + ((u8)data.at(HIT_MAX) << 4);
	FileStreamPutBack<u8>(fileStream, hitMinMax);

	FileStreamPutBack<u16>(fileStream, (u16)data.at(CONDITION));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(CONDITION_CHANCE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(CONDITION_DURATION));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TURN_MIN));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TURN_MAX));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(CRIT_STAGE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FLINCH_RATE));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(BEHAVIOR));
	FileStreamPutBack<char>(fileStream, (char)data.at(RECOIL));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(HEAL));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TARGET));

	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(STAT_CHANGE_1 + statChange));
	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<char>(fileStream, (char)data.at(STAT_CHANGE_VOLUME_1 + statChange));
	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(STAT_CHANGE_CHANCE_1 + statChange));

	FileStreamPutBack<u16>(fileStream, (u16)data.at(PADDING));

	u32 flags = 0;
	for (u32 moveFlag = 0; moveFlag < sizeof(u32) * 8; ++moveFlag)
		if (data.at(IS_CONTACT + moveFlag))
			flags |= (1 << moveFlag);
	FileStreamPutBack<u32>(fileStream, flags);

	return true;
}

void MoveData::GenerateSections(const vector<string>& narcPaths)
{
	flags = No_Flags;
	blockSize = MOVE_SIZE;
	nullValue = MOVE_NULL;

	sections.emplace_back(narcPaths.at(0), MOVE_START, MOVE_SIZE, LoadMove, SaveMove);
}
