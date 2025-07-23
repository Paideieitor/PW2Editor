#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Item/Item.h"

Item::Item(Engine* const engine, u32 group) : Module(engine, group, ITEM_NARC_PATH)
{
}

Item::~Item()
{
}

ReturnState Item::RenderGUI()
{
	ImGui::Begin("Item");
	ItemData& itemData = engine->items[engine->project->selectedItemIdx];

	ImGui::BeginGroup();

	InputInt(itemData, "Price", PRICE, 65535);

	InputInt(itemData, "Held Effect", HELD_EFFECT, 255);
	InputInt(itemData, "Held Param", HELD_PARAM, 255);
	InputInt(itemData, "Fling Effect", FLING_EFFECT, 255);
	InputInt(itemData, "Fling Power", FLING_POWER, 255);
	ComboBox(itemData, "Natural Gift Type", engine->types, NATURAL_GIFT_TYPE);
	InputInt(itemData, "Natural Gift Power", NATURAL_GIFT_POWER, 255);
	InputInt(itemData, "Natural Gift Effect", NATURAL_GIFT_EFFECT, 255);

	CheckBox(itemData, "Flag 1", PACKED_FLAG_1);
	CheckBox(itemData, "Flag 2", PACKED_FLAG_2);
	InputInt(itemData, "Pocket Field", POCKET_FIELD, 6);

	InputInt(itemData, "Effect Field", EFFECT_FIELD, 255);
	InputInt(itemData, "Effect Battle", EFFECT_BATTLE, 255);
	CheckBox(itemData, "Has Battle Stats", HAS_BATTLE_STATS);
	InputInt(itemData, "Battle Pocket", BATTLE_POCKET, 255);
	CheckBox(itemData, "Consumable", CONSUMABLE);
	InputInt(itemData, "Sort Index", SORT_IDX, 255);

	ImGui::NewLine();

	InputInt(itemData, "Unknown 1", UNKNOWN_1, 255);
	InputInt(itemData, "Unknown 2", UNKNOWN_2, 255);

	if (ImGui::Button("Copy"))
		itemCopy = itemData;
	if (ImGui::Button("Paste"))
	{
		SAVE_CHECK_REVERSE_EVENT(ITEMDATA_MAX, itemData, itemCopy,
			engine->project->selectedItemIdx, itemData);
		itemData = itemCopy;
	}

	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	ImGui::Text("Heal Conditions");
	ImGui::BeginGroup();
	CheckBox(itemData, "Cure Sleep", CURE_SLEEP);
	CheckBox(itemData, "Cure Poison", CURE_POISON);
	CheckBox(itemData, "Cure Burn", CURE_BURN);
	CheckBox(itemData, "Cure Freeze", CURE_FREEZE);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	CheckBox(itemData, "Cure Paralysis", CURE_PARALYSIS);
	CheckBox(itemData, "Cure Confusion", CURE_CONFUSION);
	CheckBox(itemData, "Cure Infatuation", CURE_INFATUATION);
	CheckBox(itemData, "Cure Stat Drops", CURE_GSP);
	ImGui::EndGroup();

	ImGui::Text("Boost Effects");
	CheckBox(itemData, "Revive", BOOST_REVIVE);
	CheckBox(itemData, "Sacred Ash", BOOST_SACRED_ASH);
	CheckBox(itemData, "Rarecandy", BOOST_RARECANDY);
	CheckBox(itemData, "Evostone", BOOST_EVOSTONE);

	ImGui::BeginGroup();
	InputInt(itemData, "Boost Attack", BOOST_ATK, 255);
	InputInt(itemData, "Boost Defense", BOOST_DEF, 255);
	InputInt(itemData, "Boost Speed", BOOST_SPE, 255);
	InputInt(itemData, "Boost Sp.Attack", BOOST_SPA, 255);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	InputInt(itemData, "Boost Sp.Defense", BOOST_SPD, 255);
	InputInt(itemData, "Boost Accuracy", BOOST_ACC, 255);
	InputInt(itemData, "Boost Crit", BOOST_CRIT, 3);
	ImGui::EndGroup();

	CheckBox(itemData, "Boost PP", BOOST_PP);
	CheckBox(itemData, "Boost PP Max", BOOST_PP_MAX);

	CheckBox(itemData, "PP Replenish", PP_REPLENISH);
	CheckBox(itemData, "PP Replenish Full", PP_REPLENISH_FULL);
	CheckBox(itemData, "HP Replenish", HP_REPLENISH);
	ImGui::BeginGroup();
	CheckBox(itemData, "Add HP EVs", EV_ADD_HP);
	CheckBox(itemData, "Add Attack EVs", EV_ADD_ATK);
	CheckBox(itemData, "Add Defense EVs", EV_ADD_DEF);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	CheckBox(itemData, "Add Speed EVs", EV_ADD_SPE);
	CheckBox(itemData, "Add Sp.Attack EVs", EV_ADD_SPA);
	CheckBox(itemData, "Add Sp.Defense EVs", EV_ADD_SPD);
	ImGui::EndGroup();

	CheckBox(itemData, "Add EVs above 100", EV_ADD_ABOVE_100);
	CheckBox(itemData, "Add Friendship 1", FRIENDSHIP_ADD_1);
	CheckBox(itemData, "Add Friendship 2", FRIENDSHIP_ADD_2);
	CheckBox(itemData, "Add Friendship 3", FRIENDSHIP_ADD_3);

	ImGui::Text("Boost Values");
	ImGui::BeginGroup();
	InputInt(itemData, "HP EVs", ITEM_EV_HP, 255);
	InputInt(itemData, "Attack EVs", ITEM_EV_ATK, 255);
	InputInt(itemData, "Defense EVs", ITEM_EV_DEF, 255);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	InputInt(itemData, "Speed EVs", ITEM_EV_SPE, 255);
	InputInt(itemData, "Sp.Attack EVs", ITEM_EV_SPA, 255);
	InputInt(itemData, "Sp.Defense EVs", ITEM_EV_SPD, 255);
	ImGui::EndGroup();

	InputInt(itemData, "Heal Amount", HEAL_AMOUNT, 255);

	InputInt(itemData, "PP Gain", PP_GAIN, 255);

	InputInt(itemData, "Friendship 1", FRIENDSHIP_1, 255);
	InputInt(itemData, "Friendship 2", FRIENDSHIP_2, 255);
	InputInt(itemData, "Friendship 3", FRIENDSHIP_3, 255);
	
	ImGui::EndGroup();

	ImGui::End();
	return OK;
}

void Item::HandleReverseEvent(const Event* reverseEvent)
{
	if (reverseEvent->subType == ITEMDATA_MAX) // Item Data Paste
	{
		ItemData* value = (ItemData*)reverseEvent->value;
		engine->items[engine->project->selectedItemIdx] = *value;
		return;
	}

	int* value = (int*)reverseEvent->value;
	engine->items[engine->project->selectedItemIdx][reverseEvent->subType] = *value;
}

void Item::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	ItemData* originalData = (ItemData*)saveEvent->value;
	const ItemData& data = engine->items[saveEvent->subType];
	if (*originalData == data)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveItem(data, filePath);
}

void Item::ComboBox(ItemData& itemData, const char* label, const std::vector<std::string>& items, ItemField field)
{
	int selected = itemData[field];
	if (selected >= items.size())
	{
		InputInt(itemData, label, field, 0xFFFF);
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(field, itemData[field], selected,
			engine->project->selectedItemIdx, itemData);
		itemData[field] = selected;
	}
}

void Item::InputInt(ItemData& itemData, const char* label, ItemField field, int maxValue)
{
	int value = itemData[field];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SAVE_CHECK_REVERSE_EVENT(field, itemData[field], value,
			engine->project->selectedItemIdx, itemData);
		itemData[field] = value;
	}
}

void Item::CheckBox(ItemData& itemData, const char* label, ItemField field)
{
	bool value = (bool)itemData[field];
	if (ImGui::Checkbox(label, &value))
	{
		SAVE_CHECK_REVERSE_EVENT(field, itemData[field], (int)value,
			engine->project->selectedItemIdx, itemData);
		itemData[field] = (int)value;
	}
}
