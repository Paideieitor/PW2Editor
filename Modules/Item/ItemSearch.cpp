#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/Item/ItemSearch.h"

#include "Data/Project.h"

#define EMPTY_SEARCH_NAME " "

#define SEARCH_NAME_EVENT 0
#define SELECT_ITEM_EVENT 1
#define ADVANCED_SEARCH_EVENT 2
#define CLEAR_SEARCH_EVENT 3

#define ITEM_EVENT(field) (4 + field)
#define GET_ITEM_FROM_EVENT(subType) (subType - ITEM_EVENT(0))

ItemSearch::ItemSearch(Engine* const engine, u32 group) : Module(engine, group, " ")
{
	ClearSearch();

	Search();
}

ItemSearch::~ItemSearch()
{
}

ReturnState ItemSearch::RenderGUI()
{
	ImGui::Begin(u8"Search Item");

	string name = searchName;
	if (ImGui::InputText("Search", &name))
	{
		SIMPLE_REVERSE_EVENT(SEARCH_NAME_EVENT, searchName, name);
		searchName = name;
		
		Search();
	}
		
	if (ImGui::BeginListBox(" ", ImVec2(ImGui::GetWindowWidth() - 15.0f, ImGui::GetWindowHeight() * 0.70f)))
	{
		for (int idx = 0; idx < selectable.size(); ++idx)
		{
			if (ImGui::Selectable(LABEL(engine->itemNames[selectable[idx]], idx), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_ITEM_EVENT,
					make_pair(engine->project->selectedItemIdx, selectedIdx),
					make_pair(selectable[idx], idx));

				engine->SetCurrentItem(selectable[idx]);
				selectedIdx = idx;
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Advanced Search"))
	{
		SIMPLE_REVERSE_EVENT(ADVANCED_SEARCH_EVENT, advancedSearchMenu, !advancedSearchMenu);
		advancedSearchMenu = !advancedSearchMenu;
	}
		
	if (advancedSearchMenu)
		AdvancedSearchMenu();

	if (ImGui::Button("Clear Search"))
	{
		pair<string, ItemData> searchData = make_pair(searchName, itemParams);
		ClearSearch();
		SIMPLE_REVERSE_EVENT(CLEAR_SEARCH_EVENT,
			make_pair(searchName, itemParams), searchData);

		Search();
	}

	ImGui::End();
	return OK;
}

void ItemSearch::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case SEARCH_NAME_EVENT: // Search Name
	{
		string* name = (string*)reverseEvent->value;
		searchName = *name;
		break;
	}
	case SELECT_ITEM_EVENT: // Selected Item
	{
		pair<u32, int>* selected = (pair<u32, int>*)reverseEvent->value;
		engine->SetCurrentItem(selected->first);
		selectedIdx = selected->second;
		break;
	}
	case ADVANCED_SEARCH_EVENT: // Advanced Search Menu
	{
		bool value = (bool*)reverseEvent->value;
		advancedSearchMenu = value;
		break;
	}
	case CLEAR_SEARCH_EVENT: // Clear Search Data
	{
		pair<string, ItemData>* searchData = (pair<string, ItemData>*)reverseEvent->value;
		searchName = searchData->first;
		itemParams = searchData->second;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;

		u32 field = GET_ITEM_FROM_EVENT(reverseEvent->subType);
		itemParams[field] = *value;
		break;
	}
	}

	Search();
}

void ItemSearch::ClearSearch()
{
	searchName.clear();

	for (u32 paramIdx = 0; paramIdx < (u32)ITEMDATA_MAX; ++paramIdx)
		itemParams[paramIdx] = ITEMDATA_NULL;
}

void ItemSearch::Search()
{
	// Reset the search data
	selectedIdx = -1;
	selectable.clear();

	for (u32 itemIdx = 0; itemIdx < (u32)engine->items.size(); ++itemIdx)
	{
		if (SearchCheck(itemIdx))
		{
			// Set the selected index if this is the selected Pokémon
			if (itemIdx == engine->project->selectedItemIdx)
				selectedIdx = (int)selectable.size();
				
			selectable.emplace_back(itemIdx);
		}
	}
}

bool ItemSearch::SearchCheck(u32 itemIdx)
{
	const string& name = engine->itemNames[itemIdx];
	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			if (!IsNumber(searchName[0]) || itemIdx != std::stoi(searchName))
				return false;
	}

	return SearchCheckItem(itemIdx);
}

bool ItemSearch::SearchCheckItem(u32 itemIdx)
{
	for (u32 paramIdx = 0; paramIdx < (u32)ITEMDATA_MAX; ++paramIdx)
	{
		int value = itemParams[paramIdx];
		if (value == ITEMDATA_NULL)
			continue;

		if (engine->items[itemIdx][paramIdx] != value)
			return false;
	}

	return true;
}

bool ItemSearch::EnableButton(int* params, u32 field, int nullValue, u32 eventID)
{
	ImGui::SameLine();

	if (params[field] == nullValue)
	{
		if (ImGui::Button(LABEL("Enable", (int)eventID)))
		{
			SIMPLE_REVERSE_EVENT(eventID, params[field], 0);
			params[field] = 0;
			return true;
		}
	}
	else
	{
		if (ImGui::Button(LABEL("Disable", (int)eventID)))
		{
			SIMPLE_REVERSE_EVENT(eventID, params[field], nullValue);
			params[field] = nullValue;
			return true;
		}
	}
	return false;
}

bool ItemSearch::ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int selected = params[field];
	if (params[field] == nullValue)
		selected = 0;
	if (selected >= items.size())
		return InputInt(label, 0xFFFF, params, field, nullValue, eventID);

	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SIMPLE_REVERSE_EVENT(eventID, params[field], selected);
		params[field] = selected;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define ITEM_COMBO_BOX(label, items, field) ComboBox(label, items, itemParams.data(), field, ITEMDATA_NULL, ITEM_EVENT(field))

bool ItemSearch::InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int value = params[field];
	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SIMPLE_REVERSE_EVENT(eventID, params[field], value);
		params[field] = value;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define ITEM_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, itemParams.data(), field, ITEMDATA_NULL, ITEM_EVENT(field))

bool ItemSearch::CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	bool value = params[field] == -1 ? false : (bool)params[field];

	ImGui::BeginDisabled(params[field] == nullValue);
	if (ImGui::Checkbox(label, &value))
	{
		SIMPLE_REVERSE_EVENT(eventID, params[field], (int)value);
		params[field] = (int)value;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define ITEM_CHECK_BOX(label, field) CheckBox(label, itemParams.data(), field, ITEMDATA_NULL, ITEM_EVENT(field))

void ItemSearch::AdvancedSearchMenu()
{
	bool search = false;
	ImGui::Begin("Item Advanced Search", &advancedSearchMenu);

	ImGui::BeginGroup();

	if (ITEM_INPUT_INT("Price", PRICE, 65535))
		search = true;

	if (ITEM_INPUT_INT("Held Effect", HELD_EFFECT, 255))
		search = true;
	if (ITEM_INPUT_INT("Held Param", HELD_PARAM, 255))
		search = true;
	if (ITEM_INPUT_INT("Fling Effect", FLING_EFFECT, 255))
		search = true;
	if (ITEM_INPUT_INT("Fling Power", FLING_POWER, 255))
		search = true;
	if (ITEM_COMBO_BOX("Natural Gift Type", engine->types, NATURAL_GIFT_TYPE))
		search = true;
	if (ITEM_INPUT_INT("Natural Gift Power", NATURAL_GIFT_POWER, 255))
		search = true;
	if (ITEM_INPUT_INT("Natural Gift Effect", NATURAL_GIFT_EFFECT, 255))
		search = true;

	if (ITEM_CHECK_BOX("Flag 1", PACKED_FLAG_1))
		search = true;
	if (ITEM_CHECK_BOX("Flag 2", PACKED_FLAG_2))
		search = true;
	if (ITEM_INPUT_INT("Pocket Field", POCKET_FIELD, 6))
		search = true;

	if (ITEM_INPUT_INT("Effect Field", EFFECT_FIELD, 255))
		search = true;
	if (ITEM_INPUT_INT("Effect Battle", EFFECT_BATTLE, 255))
		search = true;
	if (ITEM_CHECK_BOX("Has Battle Stats", HAS_BATTLE_STATS))
		search = true;
	if (ITEM_INPUT_INT("Battle Pocket", BATTLE_POCKET, 255))
		search = true;
	if (ITEM_CHECK_BOX("Has Battle Stats", HAS_BATTLE_STATS))
		search = true;
	if (ITEM_CHECK_BOX("Consumable", CONSUMABLE))
		search = true;
	if (ITEM_INPUT_INT("Sort Index", SORT_IDX, 255))
		search = true;

	ImGui::NewLine();

	if (ITEM_INPUT_INT("Unknown 1", UNKNOWN_1, 255))
		search = true;
	if (ITEM_INPUT_INT("Unknown 1", UNKNOWN_2, 255))
		search = true;

	ImGui::EndGroup();



	ImGui::SameLine();

	ImGui::BeginGroup();

	ImGui::Text("Heal Conditions");
	ImGui::BeginGroup();
	if (ITEM_CHECK_BOX("Cure Sleep", CURE_SLEEP))
		search = true;
	if (ITEM_CHECK_BOX("Cure Poison", CURE_POISON))
		search = true;
	if (ITEM_CHECK_BOX("Cure Burn", CURE_BURN))
		search = true;
	if (ITEM_CHECK_BOX("Cure Freeze", CURE_FREEZE))
		search = true;
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (ITEM_CHECK_BOX("Cure Paralysis", CURE_PARALYSIS))
		search = true;
	if (ITEM_CHECK_BOX("Cure Confusion", CURE_CONFUSION))
		search = true;
	if (ITEM_CHECK_BOX("Cure Infatuation", CURE_INFATUATION))
		search = true;
	if (ITEM_CHECK_BOX("Cure Stat Drops", CURE_GSP))
		search = true;
	ImGui::EndGroup();
	
	ImGui::Text("Boost Effects");
	if (ITEM_CHECK_BOX("Revive", BOOST_REVIVE))
		search = true;
	if (ITEM_CHECK_BOX("Sacred Ash", BOOST_SACRED_ASH))
		search = true;
	if (ITEM_CHECK_BOX("Rarecandy", BOOST_RARECANDY))
		search = true;
	if (ITEM_CHECK_BOX("Evostone", BOOST_EVOSTONE))
		search = true;

	ImGui::BeginGroup();
	if (ITEM_INPUT_INT("Boost Attack", BOOST_ATK, 255))
		search = true;
	if (ITEM_INPUT_INT("Boost Defense", BOOST_DEF, 255))
		search = true;
	if (ITEM_INPUT_INT("Boost Speed", BOOST_SPE, 255))
		search = true;
	if (ITEM_INPUT_INT("Boost Sp.Attack", BOOST_SPA, 255))
		search = true;
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (ITEM_INPUT_INT("Boost Sp.Defense", BOOST_SPD, 255))
		search = true;
	if (ITEM_INPUT_INT("Boost Accuracy", BOOST_ACC, 255))
		search = true;
	if (ITEM_INPUT_INT("Boost Crit", BOOST_CRIT, 3))
		search = true;
	ImGui::EndGroup();

	if (ITEM_CHECK_BOX("Boost PP", BOOST_PP))
		search = true;
	if (ITEM_CHECK_BOX("Boost PP Max", BOOST_PP_MAX))
		search = true;

	if (ITEM_CHECK_BOX("PP Replenish", PP_REPLENISH))
		search = true;
	if (ITEM_CHECK_BOX("PP Replenish Full", PP_REPLENISH_FULL))
		search = true;
	if (ITEM_CHECK_BOX("HP Replenish", HP_REPLENISH))
		search = true;
	ImGui::BeginGroup();
	if (ITEM_CHECK_BOX("Add HP EVs", EV_ADD_HP))
		search = true;
	if (ITEM_CHECK_BOX("Add Attack EVs", EV_ADD_ATK))
		search = true;
	if (ITEM_CHECK_BOX("Add Defense EVs", EV_ADD_DEF))
		search = true;
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (ITEM_CHECK_BOX("Add Speed EVs", EV_ADD_SPE))
		search = true;
	if (ITEM_CHECK_BOX("Add Sp.Attack EVs", EV_ADD_SPA))
		search = true;
	if (ITEM_CHECK_BOX("Add Sp.Defense EVs", EV_ADD_SPD))
		search = true;
	ImGui::EndGroup();

	if (ITEM_CHECK_BOX("Add EVs above 100", EV_ADD_ABOVE_100))
		search = true;
	if (ITEM_CHECK_BOX("Add Friendship 1", FRIENDSHIP_ADD_1))
		search = true;
	if (ITEM_CHECK_BOX("Add Friendship 2", FRIENDSHIP_ADD_2))
		search = true;
	if (ITEM_CHECK_BOX("Add Friendship 3", FRIENDSHIP_ADD_3))
		search = true;

	ImGui::Text("Boost Values");
	ImGui::BeginGroup();
	if (ITEM_INPUT_INT("HP EVs", ITEM_EV_HP, 255))
		search = true;
	if (ITEM_INPUT_INT("Attack EVs", ITEM_EV_ATK, 255))
		search = true;
	if (ITEM_INPUT_INT("Defense EVs", ITEM_EV_DEF, 255))
		search = true;
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (ITEM_INPUT_INT("Speed EVs", ITEM_EV_SPE, 255))
		search = true;
	if (ITEM_INPUT_INT("Sp.Attack EVs", ITEM_EV_SPA, 255))
		search = true;
	if (ITEM_INPUT_INT("Sp.Defense EVs", ITEM_EV_SPD, 255))
		search = true;
	ImGui::EndGroup();

	if (ITEM_INPUT_INT("Heal Amount", HEAL_AMOUNT, 255))
		search = true;

	if (ITEM_INPUT_INT("PP Gain", PP_GAIN, 255))
		search = true;

	if (ITEM_INPUT_INT("Friendship 1", FRIENDSHIP_1, 255))
		search = true;
	if (ITEM_INPUT_INT("Friendship 2", FRIENDSHIP_2, 255))
		search = true;
	if (ITEM_INPUT_INT("Friendship 3", FRIENDSHIP_3, 255))
		search = true;

	ImGui::EndGroup();

	ImGui::End();
	if (search)
		Search();
}
