#include "Data/PokemonData.h"

bool LoadPersonal(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	u32 currentByte = 0;

	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		data.at(BASE_HP + stat) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 type = 0; type < TYPE_COUNT; ++type)
		data.at(TYPE_1 + type) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(CATCH_RATE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(EVOLUTION_STAGE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u16 evYield = FileStreamReadUpdate<u16>(fileStream, currentByte);
	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		data.at(EV_HP + stat) = (int)((evYield >> (stat * 2)) & 0x0003);
	data.at(GROUNDED_SPRITE) = (int)((evYield & 0x1000) != 0);

	u16 expandedAbilBits[ABILITY_COUNT];
	for (u32 item = 0; item < WILD_ITEM_COUNT; ++item)
	{
		u16 itemValue = FileStreamReadUpdate<u16>(fileStream, currentByte);
		expandedAbilBits[item] = itemValue & 0xC000;
		// Remove the ability related bits from the item value
		itemValue -= expandedAbilBits[item];

		data.at(WILD_ITEM_50 + item) = (int)itemValue;
	}

	data.at(SEX_CHANCE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(EGG_HAPPINESS) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(BASE_HAPPINESS) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(GROWTH_RATE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 eggGroup = 0; eggGroup < EGG_GROUP_COUNT; ++eggGroup)
		data.at(EGG_GROUP_1 + eggGroup) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 ability = 0; ability < ABILITY_COUNT; ++ability)
	{
		u16 abilityValue = FileStreamReadUpdate<u8>(fileStream, currentByte);
		// Add the ability bits stored in the item slot to the ability value
		abilityValue += (expandedAbilBits[ability] >> 6);

		data.at(ABILITY_1 + ability) = (int)abilityValue;
	}

	data.at(ESCAPE_RATE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(FORM_OFFSET) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(FORM_SPRITE_OFFSET) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(FORM_COUNT) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	// Queue up the forms to be loaded concatenatedly
	if (data.at(FORM_OFFSET) != 0 && 
		data.at(FORM_COUNT) > 1 && 
		(u32)concatIndices.size() == 1)
	{
		for (u32 form = 1; form < (u32)data.at(FORM_COUNT); ++form)
			concatIndices.emplace_back((u32)data.at(FORM_OFFSET) + form);
	}

	u8 colorValue = FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(COLOR) = (int)(colorValue & 0x3F);
	data.at(SPRITE_FLIP) = (int)((colorValue & 0x40) >> 6);
	data.at(FORM_RARE) = (bool)((colorValue & 0x80) >> 7);

	data.at(BASE_EXP) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	data.at(HEIGHT) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(WEIGHT) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	const u32 maxBits = (u32)(sizeof(u32) * 8);

	for (u32 tmByte = 0; tmByte < TM_HM_COUNT; ++tmByte)
	{
		u32 tmFlags = FileStreamReadUpdate<u32>(fileStream, currentByte);
		u32 tmStart = TM_001 + (maxBits * tmByte);
		for (u32 tmBit = 0; tmBit < maxBits; ++tmBit)
			data.at(tmStart + tmBit) = (int)((tmFlags & (1 << tmBit)) != 0);
	}

	u32 tutorFlags = FileStreamReadUpdate<u32>(fileStream, currentByte);
	for (u32 tutorBit = 0; tutorBit < maxBits; ++tutorBit)
		data.at(GRASS_PLEDGE_TUTOR + tutorBit) = (int)((tutorFlags & (1 << tutorBit)) != 0);

	for (u32 shardByte = 0; shardByte < SHARD_TUTOR_COUNT; ++shardByte)
	{
		u32 shardFlags = FileStreamReadUpdate<u32>(fileStream, currentByte);
		u32 shardStart = RED_TUTOR_00 + (maxBits * shardByte);
		for (u32 shardBit = 0; shardBit < maxBits; ++shardBit)
			data.at(shardStart + shardBit) = (int)((shardFlags & (1 << shardBit)) != 0);
	}

	return true;
}
bool SavePersonal(const vector<int> data, FileStream& fileStream)
{
	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(BASE_HP + stat));

	for (u32 type = 0; type < TYPE_COUNT; ++type)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(TYPE_1 + type));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(CATCH_RATE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(EVOLUTION_STAGE));

	u16 evYield = 0;
	for (int stat = STAT_COUNT - 1; stat >= 0; --stat)
	{
		evYield |= (data.at(EV_HP + stat) & 0x0003);
		evYield = evYield << 2;
	}
	if (data.at(GROUNDED_SPRITE) != 0)
		evYield |= 0x1000;
	FileStreamPutBack<u16>(fileStream, evYield);

	for (u32 item = 0; item < WILD_ITEM_COUNT; ++item)
	{
		u16 itemValue = (u16)data.at(WILD_ITEM_50 + item);

		itemValue &= 0x3FFF;
		// Save the 2 most significant ability bits at the end of the item value
		itemValue += ((u16)data.at(ABILITY_1 + item) & 0x0300) << 6;

		FileStreamPutBack<u16>(fileStream, itemValue);
	}

	FileStreamPutBack<u8>(fileStream, (u8)data.at(SEX_CHANCE));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(EGG_HAPPINESS));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(BASE_HAPPINESS));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(GROWTH_RATE));

	for (u32 eggGroup = 0; eggGroup < EGG_GROUP_COUNT; ++eggGroup)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(EGG_GROUP_1 + eggGroup));

	for (u32 ability = 0; ability < ABILITY_COUNT; ++ability)
		FileStreamPutBack<u8>(fileStream, (u8)data.at(ABILITY_1 + ability));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(ESCAPE_RATE));

	FileStreamPutBack<u16>(fileStream, (u16)data.at(FORM_OFFSET));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(FORM_SPRITE_OFFSET));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FORM_COUNT));

	u8 color = (u8)data.at(COLOR) & 0x3F;
	if (data.at(SPRITE_FLIP) != 0)
		color |= 0x40;
	if (data.at(FORM_RARE) != 0)
		color |= 0x80;
	FileStreamPutBack<u8>(fileStream, color);

	FileStreamPutBack<u16>(fileStream, (u16)data.at(BASE_EXP));

	FileStreamPutBack<u16>(fileStream, (u16)data.at(HEIGHT));
	FileStreamPutBack<u16>(fileStream, (u16)data.at(WEIGHT));

	const u32 maxBits = (u32)(sizeof(u32) * 8);

	for (u32 tmByte = 0; tmByte < TM_HM_COUNT; ++tmByte)
	{
		u32 tmFlags = 0;
		u32 tmStart = TM_001 + (maxBits * tmByte);
		for (u32 tmBit = 0; tmBit < maxBits; ++tmBit)
			tmFlags |= ((data.at(tmStart + tmBit) & 0x1) << tmBit);

		FileStreamPutBack<u32>(fileStream, tmFlags);
	}

	u32 tutorFlags = 0;
	for (u32 tutorBit = 0; tutorBit < maxBits; ++tutorBit)
		tutorFlags |= ((data.at(GRASS_PLEDGE_TUTOR + tutorBit) & 0x1) << tutorBit);
	FileStreamPutBack<u32>(fileStream, tutorFlags);

	for (u32 shardByte = 0; shardByte < SHARD_TUTOR_COUNT; ++shardByte)
	{
		u32 shardFlags = 0;
		u32 shardStart = RED_TUTOR_00 + (maxBits * shardByte);
		for (u32 shardBit = 0; shardBit < maxBits; ++shardBit)
			shardFlags |= ((data.at(shardStart + shardBit) & 0x1) << shardBit);

		FileStreamPutBack<u32>(fileStream, shardFlags);
	}

	return true;
}

bool LoadLearnset(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	u32 currentByte = 0;
	for (u32 idx = 0; idx < LEARNSET_MOVE_COUNT; ++idx)
	{
		u16 moveID = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
		if (moveID == 0xFFFF)
			break;

		u32 start = idx * LEARNSET_MAX;
		data.at(start + LEARN_MOVE_ID) = (int)moveID;
		data.at(start + LEARN_LEVEL) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	}

	return true;
}
bool SaveLearnset(const vector<int> data, FileStream& fileStream)
{
	for (u32 idx = 0; idx < LEARNSET_MOVE_COUNT; ++idx)
	{
		u32 start = idx * LEARNSET_MAX;
		int moveID = data.at(start + LEARN_MOVE_ID);
		if (moveID == POKEMON_NULL)
		{
			FileStreamPutBack<u32>(fileStream, (u32)0xFFFFFFFF);
			break;
		}

		FileStreamPutBack<u16>(fileStream, (u16)moveID);
		FileStreamPutBack<u16>(fileStream, (u16)data.at(start + LEARN_LEVEL));
	}

	return true;
}
bool OrderLearnset(vector<int>& data, u32 start, u32 size, u32& field)
#define LEARNSET_ORDER_NULL 0
#define LEARNSET_ORDER_BACKWARDS 1
#define LEARNSET_ORDER_FORWARD 2
{
	// No re-order if...
	// the move ID is changed
	if ((field - start) % 2 == 0)
		return false;

	u8 direction = LEARNSET_ORDER_NULL;

	int value = data.at(field);
	int nextValue = POKEMON_NULL;
	if (field + 2 < start + size)
		nextValue = data.at(field + 2);
	// If we are not at the back of the learnset...
	if (nextValue != POKEMON_NULL)
	{
		// and we erased the learn level
		// or the learn level is now bigger than the next learn level
		// move forward [->]
		if (value == POKEMON_NULL || value > nextValue)
			direction = LEARNSET_ORDER_FORWARD;
	}
	
	// If the learn level was not erased...
	if (value != POKEMON_NULL)
	{
		int prevValue = POKEMON_NULL;
		if (field >= 2)
		{
			prevValue = data.at(field - 2);
			if (prevValue == POKEMON_NULL)
				// If the previous learn level is null
				// move backwards [<-]
				direction = LEARNSET_ORDER_BACKWARDS;
		}
		// If we are not at the back of the learnset
		// and the learn level is now smaller than the previous learn level
		if (prevValue != POKEMON_NULL && value < prevValue)
			direction = LEARNSET_ORDER_BACKWARDS;
	}

	u32 originalField = field;
	switch (direction)
	{
	case LEARNSET_ORDER_NULL:
		break;
	case LEARNSET_ORDER_BACKWARDS:
		for (int idx = field - 2; field >= start; idx -= 2)
		{
			int prevLevel = data.at(idx);
			int currLevel = data.at(idx + 2);
			if (prevLevel != POKEMON_NULL && currLevel >= prevLevel)
				break;

			int prevMove = data.at(idx - 1);

			data.at(idx - 1) = data.at(idx + 1);
			data.at(idx) = currLevel;

			data.at(idx + 1) = prevMove;
			data.at(idx + 2) = prevLevel;

			// Update the new field position
			field -= 2;
		}
		break;
	case LEARNSET_ORDER_FORWARD:
		for (u32 idx = field + 2; idx < start + size; idx += 2)
		{
			int nextLevel = data.at(idx);
			int currLevel = data.at(idx - 2);
			if (nextLevel == POKEMON_NULL || 
				(currLevel != POKEMON_NULL && currLevel <= nextLevel))
				break;

			int nextMove = data.at(idx - 1);

			data.at(idx - 1) = data.at(idx - 3);
			data.at(idx) = currLevel;

			data.at(idx - 3) = nextMove;
			data.at(idx - 2) = nextLevel;

			// Update the new field position
			field += 2;
		}
		break;
	}
	return originalField != field;
}

bool LoadEvolution(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	u32 currentByte = 0;
	for (u32 idx = 0; idx < EVOLUTION_COUNT; ++idx)
	{
		u32 start = idx * EVOLUTION_MAX;
		data.at(start + EVOLUTION_METHOD) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
		data.at(start + EVOLUTION_PARAM) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
		data.at(start + EVOLUTION_SPECIES) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	}

	return true;
}
bool SaveEvolution(const vector<int> data, FileStream& fileStream)
{
	for (u32 idx = 0; idx < EVOLUTION_COUNT; ++idx)
	{
		u32 start = idx * EVOLUTION_MAX;
		FileStreamPutBack<u16>(fileStream, (u16)data.at(start + EVOLUTION_METHOD));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(start + EVOLUTION_PARAM));
		FileStreamPutBack<u16>(fileStream, (u16)data.at(start + EVOLUTION_SPECIES));
	}

	return true;
}

bool LoadChild(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
	data[CHILD] = (int)FileStreamRead<u16>(fileStream, 0);

	return true;
}
bool SaveChild(const vector<int> data, FileStream& fileStream)
{
	FileStreamPutBack<u16>(fileStream, (u16)data.at(CHILD));

	return true;
}

void PokemonData::GenerateSections(const vector<string>& narcPaths)
{
	flags = No_Flags;
	blockSize = POKEMON_SIZE;
	nullValue = POKEMON_NULL;

	sections.emplace_back(narcPaths.at(0), PERSONAL_START, PERSONAL_SIZE, LoadPersonal, SavePersonal);
	sections.emplace_back(narcPaths.at(1), LEARNSET_START, LEARNSET_SIZE, LoadLearnset, SaveLearnset, OrderLearnset);
	sections.emplace_back(narcPaths.at(2), EVOLUTION_START, EVOLUTION_SIZE, LoadEvolution, SaveEvolution);
	sections.emplace_back(narcPaths.at(3), CHILD_START, CHILD_SIZE, LoadChild, SaveChild);
}
