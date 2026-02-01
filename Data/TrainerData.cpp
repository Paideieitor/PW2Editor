#include "TrainerData.h"

bool LoadTrainer(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
    UNUSED(concatIndices);

	u32 currentByte = 0;

	data.at(TRAINER_FIELD(TRAINER_TYPE)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(TRAINER_FIELD(TRAINER_CLASS)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TRAINER_FIELD(BATTLE_TYPE)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TRAINER_FIELD(POKEMON_COUNT)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(TRAINER_FIELD(ITEM_1)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(TRAINER_FIELD(ITEM_2)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(TRAINER_FIELD(ITEM_3)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(TRAINER_FIELD(ITEM_4)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	u32 AIFlags = (int)FileStreamReadUpdate<u32>(fileStream, currentByte);
	for (u32 flag = 0; flag < AI_UNUSED_31 - AI_NO_EFFECT_CHECK; ++flag)
		data.at(TRAINER_FIELD(AI_NO_EFFECT_CHECK + flag)) = (AIFlags & (1 << flag)) != 0;

	data.at(TRAINER_FIELD(IS_HEALER)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TRAINER_FIELD(CASH)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TRAINER_FIELD(POST_BATTLE_ITEM)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	return true;
}
bool SaveTrainer(const vector<int> data, FileStream& fileStream)
{
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(TRAINER_TYPE)));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(TRAINER_CLASS)));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(BATTLE_TYPE)));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(POKEMON_COUNT)));

	FileStreamPutBack<u16>(fileStream, (u16)data.at(TRAINER_FIELD(ITEM_1)));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(TRAINER_FIELD(ITEM_2)));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(TRAINER_FIELD(ITEM_3)));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(TRAINER_FIELD(ITEM_4)));

	u32 AIFlags = 0;
	for (u32 flag = 0; flag < AI_UNUSED_31 - AI_NO_EFFECT_CHECK; ++flag)
		AIFlags |= (data.at(TRAINER_FIELD(AI_NO_EFFECT_CHECK) + flag) << flag);
	FileStreamPutBack<u32>(fileStream, AIFlags);

	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(IS_HEALER)));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TRAINER_FIELD(CASH)));

	FileStreamPutBack<u16>(fileStream, (u16)data.at(TRAINER_FIELD(POST_BATTLE_ITEM)));

	return true;
}

void GetBaseTrainerTeamData(vector<int>& data, u32 slot, const FileStream& fileStream, u32& currentByte)
{
	data.at(TEAM_FIELD(slot, TRAINER_DIFFICULTY)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u8 abilAndSex = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(TEAM_FIELD(slot, TRAINER_SEX)) = abilAndSex & 0x0F;
	data.at(TEAM_FIELD(slot, TRAINER_ABILITY_SLOT)) = (abilAndSex & 0xF0) >> 4;

	data.at(TEAM_FIELD(slot, TRAINER_LEVEL)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(TEAM_FIELD(slot, TRAINER_SPECIES)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(TEAM_FIELD(slot, TRAINER_FORM)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	int ivs = 31 * data.at(TEAM_FIELD(slot, TRAINER_DIFFICULTY)) / 255;
	for (u32 iv = 0; iv < STAT_COUNT; ++iv)
		data.at(TEAM_FIELD(slot, TRAINER_HP_IV + iv)) = ivs;

	data.at(TEAM_FIELD(slot, TRAINER_HP_PERCENT)) = 100;
}
void SetBaseTrainerTeamData(const vector<int>& data, u32 slot, FileStream& fileStream)
{
	FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_DIFFICULTY)));

	u8 abilAndSex = data.at(TEAM_FIELD(slot, TRAINER_SEX)) & 0x0F;
	abilAndSex |= (data.at(TEAM_FIELD(slot, TRAINER_ABILITY_SLOT)) << 4) & 0xF0;
	FileStreamPutBack<u8>(fileStream, abilAndSex);

	FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_LEVEL)));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_SPECIES)));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_FORM)));
}

bool LoadTeam(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
    UNUSED(concatIndices);

	u32 currentByte = 0;

	int trainerType = data.at(TRAINER_FIELD(TRAINER_TYPE));

	u32 slot = 0;
	while (slot < TEAM_COUNT && !FileStreamEnded(fileStream, currentByte))
	{
		for (u32 idx = 0; idx < TEAM_MAX; ++idx)
			data.at(TEAM_FIELD(slot, TRAINER_DIFFICULTY) + idx) = 0;

		switch (trainerType)
		{
		case SIMPLE_TRAINER:
			GetBaseTrainerTeamData(data, slot, fileStream, currentByte);
			break;
		case MOVE_TRAINER:
			GetBaseTrainerTeamData(data, slot, fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_MOVE_1)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_2)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_3)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_4)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			break;
		case ITEM_TRAINER:
			GetBaseTrainerTeamData(data, slot, fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			break;
		case GOOD_TRAINER:
			GetBaseTrainerTeamData(data, slot, fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_MOVE_1)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_2)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_3)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_MOVE_4)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			break;
		case PERFECT_TRAINER:

			u32 genetic = (int)FileStreamReadUpdate<u32>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_SEX)) = genetic >> 30;
			for (u32 iv = 0; iv < STAT_COUNT; ++iv)
				data.at(TEAM_FIELD(slot, TRAINER_HP_IV + iv)) = (genetic & (0x1F << (5 * iv))) >> (5 * iv);

			data.at(TEAM_FIELD(slot, TRAINER_SPECIES)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_FORM)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_LEVEL)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

			for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
			{
				u16 moveData = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

				data.at(TEAM_FIELD(slot, TRAINER_MOVE_1 + moveSlot)) = moveData & 0x7FFF;
				data.at(TEAM_FIELD(slot, TRAINER_MAX_PP_1 + moveSlot)) = ((moveData & 0x8000) != 0);
			}

			data.at(TEAM_FIELD(slot, TRAINER_ABILITY)) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

			for (u32 ev = 0; ev < STAT_COUNT; ++ev)
				data.at(TEAM_FIELD(slot, TRAINER_HP_EV + ev)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

			data.at(TEAM_FIELD(slot, TRAINER_NATURE)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_HAPPINESS)) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

			u16 state = FileStreamReadUpdate<u16>(fileStream, currentByte);
			data.at(TEAM_FIELD(slot, TRAINER_STATUS)) = (int)((state & 0x0380) >> 7);
			data.at(TEAM_FIELD(slot, TRAINER_HP_PERCENT)) = (int)(state & 0x007F);
			data.at(TEAM_FIELD(slot, TRAINER_SHINY)) = (int)((state & 0x0400) != 0);

			break;
		}
		++slot;
	}

	return true;
}
bool SaveTeam(const vector<int> data, FileStream& fileStream)
{
	int trainerType = data.at(TRAINER_FIELD(TRAINER_TYPE));

	u32 slot = 0;
	while (slot < TEAM_COUNT && data.at(TEAM_FIELD(slot, TRAINER_LEVEL)) != TRAINER_NULL)
	{
		switch (trainerType)
		{
		case SIMPLE_TRAINER:
			SetBaseTrainerTeamData(data, slot, fileStream);
			break;
		case MOVE_TRAINER:
			SetBaseTrainerTeamData(data, slot, fileStream);

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_1)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_2)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_3)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_4)));
			break;
		case ITEM_TRAINER:
			SetBaseTrainerTeamData(data, slot, fileStream);

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)));
			break;
		case GOOD_TRAINER:
			SetBaseTrainerTeamData(data, slot, fileStream);

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)));

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_1)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_2)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_3)));
			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_MOVE_4)));
			break;
		case PERFECT_TRAINER:
			u32 genetic = data.at(TEAM_FIELD(slot, TRAINER_SEX)) << 30;
			for (u32 iv = 0; iv < STAT_COUNT; ++iv)
				genetic |= (data.at(TEAM_FIELD(slot, TRAINER_HP_IV + iv)) & 0x1F) << (5 * iv);
			FileStreamPutBack<u32>(fileStream, genetic);

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_SPECIES)));
			FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_FORM)));
			FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_LEVEL)));

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_HELD_ITEM)));

			for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
			{
				u16 moveData = data.at(TEAM_FIELD(slot, TRAINER_MOVE_1 + moveSlot));

				if (data.at(TEAM_FIELD(slot, TRAINER_MAX_PP_1 + moveSlot)))
					moveData |= 0x8000;
				FileStreamPutBack<u16>(fileStream, moveData);
			}

			FileStreamPutBack<u16>(fileStream, (u16)data.at(TEAM_FIELD(slot, TRAINER_ABILITY)));

			for (u32 ev = 0; ev < STAT_COUNT; ++ev)
				FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_HP_EV + ev)));

			FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_NATURE)));
			FileStreamPutBack<u8>(fileStream, (u8)data.at(TEAM_FIELD(slot, TRAINER_HAPPINESS)));

			u16 state = (u16)data.at(TEAM_FIELD(slot, TRAINER_HP_PERCENT));
			state |= (data.at(TEAM_FIELD(slot, TRAINER_STATUS)) << 7) & 0x0380;
			if (data.at(TEAM_FIELD(slot, TRAINER_SHINY)))
				state += 0x0400;
			FileStreamPutBack<u16>(fileStream, state);
			break;
		}
		++slot;
	}

	return true;
}

void TrainerData::GenerateSections(const vector<string>& narcPaths)
{
	flags = Full_Access;
	blockSize = TRAINER_TEAM_SIZE;
	nullValue = TRAINER_NULL;

	sections.emplace_back(narcPaths.at(0), TRAINER_START, TRAINER_SIZE, LoadTrainer, SaveTrainer);
	sections.emplace_back(narcPaths.at(1), TEAM_START, TEAM_SIZE, LoadTeam, SaveTeam);
}
