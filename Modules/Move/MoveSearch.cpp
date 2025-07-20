#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/Move/MoveSearch.h"

#include "Data/Project.h"

#define EMPTY_SEARCH_NAME " "

#define SEARCH_NAME_EVENT 0
#define SELECT_MOVE_EVENT 1
#define ADVANCED_SEARCH_EVENT 2
#define CLEAR_SEARCH_EVENT 3
#define SAVE_ANIM_EVENT 4

#define MOVE_EVENT(field) (5 + field)
#define GET_ITEM_FROM_EVENT(subType) (subType - MOVE_EVENT(0))

MoveSearch::MoveSearch(Engine* const engine, u32 group) : Module(engine, group, MOVE_ANIM_NARC_PATH)
{
	ClearSearch();

	Search();
}

MoveSearch::~MoveSearch()
{
}

ReturnState MoveSearch::RenderGUI()
{
	ImGui::Begin(u8"Search Move");

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
			if (ImGui::Selectable(LABEL(engine->moveNames[selectable[idx]], idx), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_MOVE_EVENT,
					make_pair(engine->project->selectedMoveIdx, selectedIdx),
					make_pair(selectable[idx], idx));

				engine->SetCurrentMove(selectable[idx]);
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
		pair<string, MoveData> searchData = make_pair(searchName, moveParams);
		ClearSearch();
		SIMPLE_REVERSE_EVENT(CLEAR_SEARCH_EVENT,
			make_pair(searchName, moveParams), searchData);

		Search();
	}

	//ImGui::Separator();
	//
	//if (ImGui::Button("Add Move"))
	//{
	//	engine->AddMove();
	//	Search();
	//
	//	u32 moveIdx = (u32)engine->moves.size() - 1;
	//	SIMPLE_REVERSE_EVENT(SELECT_MOVE_EVENT,
	//		make_pair(engine->project->selectedMoveIdx, selectedIdx),
	//		make_pair(moveIdx, -1));
	//
	//	engine->SetCurrentMove(moveIdx);
	//	selectedIdx = -1;
	//}

	ImGui::End();
	return OK;
}

void MoveSearch::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case SEARCH_NAME_EVENT: // Search Name
	{
		string* name = (string*)reverseEvent->value;
		searchName = *name;
		break;
	}
	case SELECT_MOVE_EVENT: // Selected Move
	{
		pair<u32, int>* selected = (pair<u32, int>*)reverseEvent->value;
		engine->SetCurrentMove(selected->first);
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
		pair<string, MoveData>* searchData = (pair<string, MoveData>*)reverseEvent->value;
		searchName = searchData->first;
		moveParams = searchData->second;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;

		u32 field = GET_ITEM_FROM_EVENT(reverseEvent->subType);
		moveParams[field] = *value;
		break;
	}
	}

	Search();
}

void MoveSearch::HandleSaveEvent(Event* saveEvent)
{
	SaveFileStream(engine->moveAnims[saveEvent->subType], MAKE_FILE_PATH(savePath, saveEvent->subType));
}

void MoveSearch::HandleGroupEvent(Event* groupEvent)
{
	engine->AddSaveEvent(moduleIdx, (u32)engine->moves.size() - 1, (u32)nullptr);
}

void MoveSearch::ClearSearch()
{
	searchName.clear();

	for (u32 paramIdx = 0; paramIdx < (u32)MOVEDATA_MAX; ++paramIdx)
		moveParams[paramIdx] = MOVEDATA_NULL;
}

void MoveSearch::Search()
{
	// Reset the search data
	selectedIdx = -1;
	selectable.clear();

	for (u32 moveIdx = 0; moveIdx < (u32)engine->moves.size(); ++moveIdx)
	{
		if (SearchCheck(moveIdx))
		{
			// Set the selected index if this is the selected Pokémon
			if (moveIdx == engine->project->selectedMoveIdx)
				selectedIdx = (int)selectable.size();

			selectable.emplace_back(moveIdx);
		}
	}
}

bool MoveSearch::SearchCheck(u32 itemIdx)
{
	const string& name = engine->moveNames[itemIdx];
	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			if (!IsNumber(searchName[0]) || itemIdx != std::stoi(searchName))
				return false;
	}

	return SearchCheckMove(itemIdx);
}

bool MoveSearch::SearchCheckMove(u32 itemIdx)
{
	for (u32 paramIdx = 0; paramIdx < (u32)MOVEDATA_MAX; ++paramIdx)
	{
		int value = moveParams[paramIdx];
		if (value == MOVEDATA_NULL)
			continue;

		if (engine->moves[itemIdx][paramIdx] != value)
			return false;
	}

	return true;
}

bool MoveSearch::EnableButton(int* params, u32 field, int nullValue, u32 eventID)
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

bool MoveSearch::ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int selected = params[field];
	if (params[field] == nullValue)
		selected = 0;
	if (selected >= items.size())
		return InputInt(label, 0xFFFF, params, field, nullValue, eventID, false);

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
#define MOVE_COMBO_BOX(label, items, field) ComboBox(label, items, moveParams.data(), field, MOVEDATA_NULL, MOVE_EVENT(field))

bool MoveSearch::InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID, bool allowNegative)
{
	bool output = false;

	int value = params[field];
	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (!allowNegative)
		{
			if (value < 0)
				value = 0;
		}
		else
		{
			if (value < -maxValue)
				value = -maxValue;
		}

		SIMPLE_REVERSE_EVENT(eventID, params[field], value);
		params[field] = value;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define MOVE_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, moveParams.data(), field, MOVEDATA_NULL, MOVE_EVENT(field), false)
#define MOVE_INPUT_SIGNED_INT(label, field, maxValue) InputInt(label, maxValue, moveParams.data(), field, MOVEDATA_NULL, MOVE_EVENT(field), true)

bool MoveSearch::CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID)
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
#define MOVE_CHECK_BOX(label, field) CheckBox(label, moveParams.data(), field, MOVEDATA_NULL, MOVE_EVENT(field))

void MoveSearch::AdvancedSearchMenu()
{
	bool search = false;
	ImGui::Begin("Move Advanced Search", &advancedSearchMenu);

	ImGui::BeginGroup();
	ImGui::BeginGroup();

	ImGui::Text("Move Data");

	if (MOVE_COMBO_BOX("Type", engine->types, MOVE_TYPE))
		search = true;

	if (MOVE_COMBO_BOX("Quality", engine->moveQuality, QUALITY))
		search = true;
	if (MOVE_COMBO_BOX("Category", engine->moveCategory, CATEGORY))
		search = true;
	if (MOVE_INPUT_INT("Power", POWER, 255))
		search = true;
	if (MOVE_INPUT_INT("Accuracy", ACCURACY, 101))
		search = true;
	if (MOVE_INPUT_INT("Base PP", BASE_PP, 255))
		search = true;
	if (MOVE_INPUT_SIGNED_INT("Priority", PRIORITY, 7))
		search = true;
	if (MOVE_INPUT_INT("Crit Stage", CRIT_STAGE, 6))
		search = true;
	if (MOVE_INPUT_INT("Flinch Rate", FLINCH_RATE, 100))
		search = true;

	if (MOVE_INPUT_SIGNED_INT("Recoil/Drain", RECOIL, 100))
		search = true;
	if (MOVE_INPUT_INT("Heal", HEAL, 255))
		search = true;
	if (MOVE_COMBO_BOX("Target", engine->moveTarget, TARGET))
		search = true;

	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	ImGui::Text("Multi Hits");

	if (MOVE_INPUT_INT("Minimum Hits", HIT_MIN, 255))
		search = true;
	if (MOVE_INPUT_INT("Maximum Hits", HIT_MAX, 255))
		search = true;
	
	ImGui::Text("Apply Condition");

	if (MOVE_COMBO_BOX("##Condition", engine->conditions, CONDITION))
		search = true;
	if (MOVE_INPUT_INT("Condition Chance", CONDITION_CHANCE, 10))
		search = true;
	if (MOVE_INPUT_INT("Condition Duration", CONDITION_DURATION, 255))
		search = true;

	if (MOVE_INPUT_INT("Minimum Turns", TURN_MIN, 255))
		search = true;
	if (MOVE_INPUT_INT("Maximum Turns", TURN_MAX, 255))
		search = true;

	ImGui::Text("Behavior");

	if (MOVE_INPUT_INT("##Animation ID", BEHAVIOR, 65535))
		search = true;

	ImGui::EndGroup();

	ImGui::BeginGroup();

	for (u32 statChange = 0; statChange < 3; ++statChange)
	{
		ImGui::BeginGroup();
		ImGui::Text((string("Stat Change ") + std::to_string(statChange)));
		if (MOVE_COMBO_BOX((string("##")+ std::to_string(statChange)).c_str(), engine->stats, STAT_CHANGE_1 + statChange))
			search = true;
		if (MOVE_INPUT_SIGNED_INT((string("Volume") + "##" + std::to_string(statChange)).c_str(), STAT_CHANGE_VOLUME_1 + statChange, 6))
			search = true;
		if (MOVE_INPUT_INT((string("Chance") + "##" + std::to_string(statChange)).c_str(), STAT_CHANGE_CHANCE_1 + statChange, 100))
			search = true;
		ImGui::EndGroup();
		ImGui::SameLine();
	}

	ImGui::EndGroup();
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Flags");

	ImGui::BeginGroup();
	if (MOVE_CHECK_BOX("Contact", IS_CONTACT))
		search = true;
	if (MOVE_CHECK_BOX("Charge", IS_REQUIRES_CHARGE))
		search = true;
	if (MOVE_CHECK_BOX("Recharge", IS_RECHARGE))
		search = true;
	if (MOVE_CHECK_BOX("Blocked by Protect", IS_BLOCKED_BY_PROTECT))
		search = true;
	if (MOVE_CHECK_BOX("Reflected by Magic Coat", IS_REFLECTED_BY_MAGIC_COAT))
		search = true;
	if (MOVE_CHECK_BOX("Stolen by Snatch", IS_STOLEN_BY_SNATCH))
		search = true;
	if (MOVE_CHECK_BOX("Copied by Mirror Move", IS_COPIED_BY_MIRROR_MOVE))
		search = true;
	if (MOVE_CHECK_BOX("Punch", IS_PUNCH))
		search = true;
	if (MOVE_CHECK_BOX("Sound", IS_SOUND))
		search = true;
	if (MOVE_CHECK_BOX("Grounded by Gravity", IS_GROUNDED_BY_GRAVITY))
		search = true;
	if (MOVE_CHECK_BOX("Defrost", IS_DEFROST))
		search = true;
	if (MOVE_CHECK_BOX("Long Range", IS_LONG_RANGE))
		search = true;
	if (MOVE_CHECK_BOX("Blocked by Heal Block", IS_BLOCKED_BY_HEAL_BLOCK))
		search = true;
	if (MOVE_CHECK_BOX("Bypasses Substitute", IS_BYPASSES_SUBSTITUTE))
		search = true;
	if (MOVE_CHECK_BOX("Powder", IS_UNKNOWN_14))
		search = true;
	if (MOVE_CHECK_BOX("Bullet", IS_UNKNOWN_15))
		search = true;
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	if (MOVE_CHECK_BOX("Bite", IS_UNKNOWN_16))
		search = true;
	if (MOVE_CHECK_BOX("Pulse", IS_UNKNOWN_17))
		search = true;
	if (MOVE_CHECK_BOX("Healing", IS_UNKNOWN_18))
		search = true;
	if (MOVE_CHECK_BOX("Dance", IS_UNKNOWN_19))
		search = true;
	if (MOVE_CHECK_BOX("Wind", IS_UNKNOWN_20))
		search = true;
	if (MOVE_CHECK_BOX("Sharp", IS_UNKNOWN_21))
		search = true;
	if (MOVE_CHECK_BOX("Unused 22", IS_UNKNOWN_22))
		search = true;
	if (MOVE_CHECK_BOX("Unused 23", IS_UNKNOWN_23))
		search = true;
	if (MOVE_CHECK_BOX("Unused 24", IS_UNKNOWN_24))
		search = true;
	if (MOVE_CHECK_BOX("Unused 25", IS_UNKNOWN_25))
		search = true;
	if (MOVE_CHECK_BOX("Unused 26", IS_UNKNOWN_26))
		search = true;
	if (MOVE_CHECK_BOX("Unused 27", IS_UNKNOWN_27))
		search = true;
	if (MOVE_CHECK_BOX("Unused 28", IS_UNKNOWN_28))
		search = true;
	if (MOVE_CHECK_BOX("Unused 29", IS_UNKNOWN_29))
		search = true;
	if (MOVE_CHECK_BOX("Unused 30", IS_UNKNOWN_30))
		search = true;
	if (MOVE_CHECK_BOX("Unused 31", IS_UNKNOWN_31))
		search = true;
	ImGui::EndGroup();

	ImGui::EndGroup();

	ImGui::End();
	if (search)
		Search();
}