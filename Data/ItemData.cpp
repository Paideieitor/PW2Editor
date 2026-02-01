#include "Data/ItemData.h"

bool LoadItem(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream)
{
    UNUSED(concatIndices);

	u32 currentByte = 0;

	data.at(PRICE) = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	data.at(HELD_EFFECT) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(HELD_PARAM) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(NATURAL_GIFT_EFFECT) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(FLING_EFFECT) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(FLING_POWER) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(NATURAL_GIFT_POWER) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u16 packed = FileStreamReadUpdate<u16>(fileStream, currentByte);
	data.at(USABLE_FLAG) = (packed & 0x20) >> 5;
	data.at(REGISTRABLE_FLAG) = (packed & 0x40) >> 6;
	data.at(POCKET_FIELD) = (packed & 0x780) >> 7;
	data.at(NATURAL_GIFT_TYPE) = packed & 0x1F;

	data.at(EFFECT_FIELD) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(EFFECT_BATTLE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(HAS_BATTLE_STATS) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(BATTLE_POCKET) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(CONSUMABLE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(SORT_IDX) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u8 cureInflict = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	for (int field = CURE_SLEEP; field <= CURE_GSP; ++field)
	{
		u8 index = field - CURE_SLEEP;
		data.at(field) = (cureInflict & (0x1 << index)) >> index;
	}

	u32 boosts = FileStreamReadUpdate<u32>(fileStream, currentByte);
	for (int field = BOOST_REVIVE; field <= BOOST_EVOSTONE; ++field)
	{
		u32 index = field - BOOST_REVIVE;
		data.at(field) = (boosts & (0x1 << index)) >> index;
	}
	for (int field = BOOST_ATK; field <= BOOST_ACC; ++field)
	{
		u32 index = ((field - BOOST_ATK) + 1) * 4;
		data.at(field) = (boosts & (0xF << index)) >> index;
	}
	data.at(BOOST_CRIT) = (boosts & 0x30000000) >> 28;
	data.at(BOOST_PP) = (boosts & 0x40000000) >> 30;
	data.at(BOOST_PP_MAX) = (boosts & 0x80000000) >> 31;

	u16 functions = FileStreamReadUpdate<u16>(fileStream, currentByte);
	for (int field = PP_REPLENISH; field <= FRIENDSHIP_ADD_3; ++field)
	{
		u16 index = field - PP_REPLENISH;
		data.at(field) = (functions & (0x1 << index)) >> index;
	}

	data.at(ITEM_EV_HP) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_EV_ATK) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_EV_DEF) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_EV_SPE) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_EV_SPA) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_EV_SPD) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(HEAL_AMOUNT) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(PP_GAIN) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(FRIENDSHIP_1) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(FRIENDSHIP_2) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(FRIENDSHIP_3) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	data.at(ITEM_UNUSED_1) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	data.at(ITEM_UNUSED_2) = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	return true;
}
bool SaveItem(const vector<int> data, FileStream& fileStream)
{
	FileStreamPutBack<u16>(fileStream, (u16)data.at(PRICE));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(HELD_EFFECT));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(HELD_PARAM));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(NATURAL_GIFT_EFFECT));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FLING_EFFECT));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FLING_POWER));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(NATURAL_GIFT_POWER));

	u16 packed = (data.at(USABLE_FLAG) << 5) & 0x20;
	packed |= (data.at(REGISTRABLE_FLAG) << 6) & 0x40;
	packed |= (data.at(POCKET_FIELD) << 7) & 0x780;
	packed |= data.at(NATURAL_GIFT_TYPE) & 0x1F;
	FileStreamPutBack<u16>(fileStream, packed);

	FileStreamPutBack<u8>(fileStream, (u8)data.at(EFFECT_FIELD));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(EFFECT_BATTLE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(HAS_BATTLE_STATS));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(BATTLE_POCKET));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(CONSUMABLE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(SORT_IDX));

	u8 cureInflict = 0;
	for (int field = CURE_SLEEP; field <= CURE_GSP; ++field)
	{
		u8 index = field - CURE_SLEEP;
		cureInflict |= (data.at(field) << index) & (0x1 << index);
	}
	FileStreamPutBack<u16>(fileStream, cureInflict);

	u32 boosts = 0;
	for (int field = BOOST_REVIVE; field <= BOOST_EVOSTONE; ++field)
	{
		u32 index = field - BOOST_REVIVE;
		boosts |= (data.at(field) << index) & (0x1 << index);
	}
	for (int field = BOOST_ATK; field <= BOOST_ACC; ++field)
	{
		u32 index = ((field - BOOST_ATK) + 1) * 4;
		boosts |= (data.at(field) << index) & (0xF << index);
	}
	boosts |= (data.at(BOOST_CRIT) << 28) & 0x30000000;
	boosts |= (data.at(BOOST_PP) << 30) & 0x40000000;
	boosts |= (data.at(BOOST_PP_MAX) << 31) & 0x80000000;
	FileStreamPutBack<u16>(fileStream, boosts);

	u16 functions = 0;
	for (int field = PP_REPLENISH; field <= FRIENDSHIP_ADD_3; ++field)
	{
		u16 index = field - PP_REPLENISH;
		functions |= (data.at(field) << index) & (0x1 << index);
	}
	FileStreamPutBack<u16>(fileStream, functions);

	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_HP));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_ATK));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_DEF));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_SPE));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_SPA));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_EV_SPD));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(HEAL_AMOUNT));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(PP_GAIN));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(FRIENDSHIP_1));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FRIENDSHIP_2));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(FRIENDSHIP_3));

	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_UNUSED_1));
	FileStreamPutBack<u8>(fileStream, (u8)data.at(ITEM_UNUSED_2));

	return true;
}

void ItemData::GenerateSections(const vector<string>& narcPaths)
{
	flags = No_Flags;
	blockSize = ITEM_SIZE;
	nullValue = ITEM_NULL;

	sections.emplace_back(narcPaths.at(0), ITEM_START, ITEM_SIZE, LoadItem, SaveItem);
}
