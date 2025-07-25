#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/Trainer/TrainerSearch.h"

#include "Data/Project.h"

#define SEARCH_NAME_EVENT 0
#define SELECT_TRAINER_EVENT 1
#define ADVANCED_SEARCH_EVENT 2
#define CLEAR_SEARCH_EVENT 3

#define TRAINER_EVENT(field) (4 + field)
#define GET_TRAINER_FROM_EVENT(subType) (subType - TRAINER_EVENT(0))

TrainerSearch::TrainerSearch(Engine* const engine, u32 group) : Module(engine, group, " ")
{
	ClearSearch();

	Search();
}

TrainerSearch::~TrainerSearch()
{
}

ReturnState TrainerSearch::RenderGUI()
{
	ImGui::Begin(u8"Search Trainer");

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
			if (ImGui::Selectable(LABEL(engine->trainerNames[selectable[idx]], idx), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_TRAINER_EVENT,
					make_pair(engine->project->selectedTrainerIdx, selectedIdx),
					make_pair(selectable[idx], idx));

				engine->SetCurrentTrainer(selectable[idx]);
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
		pair<string, TrainerData> searchData = make_pair(searchName, trainerParams);
		ClearSearch();
		SIMPLE_REVERSE_EVENT(CLEAR_SEARCH_EVENT,
			make_pair(searchName, trainerParams), searchData);

		Search();
	}

	ImGui::End();
	return OK;
}

void TrainerSearch::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case SEARCH_NAME_EVENT: // Search Name
	{
		string* name = (string*)reverseEvent->value;
		searchName = *name;
		break;
	}
	case SELECT_TRAINER_EVENT: // Selected Trainer
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
		pair<string, TrainerData>* searchData = (pair<string, TrainerData>*)reverseEvent->value;
		searchName = searchData->first;
		trainerParams = searchData->second;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;

		u32 field = GET_TRAINER_FROM_EVENT(reverseEvent->subType);
		trainerParams[field] = *value;
		break;
	}
	}

	Search();
}

void TrainerSearch::ClearSearch()
{
	searchName.clear();

	TrainerReset(trainerParams);
}

void TrainerSearch::Search()
{
	// Reset the search data
	selectedIdx = -1;
	selectable.clear();

	for (u32 trIdx = 0; trIdx < (u32)engine->trainers.size(); ++trIdx)
	{
		if (SearchCheck(trIdx))
		{
			// Set the selected index if this is the selected Trainer
			if (trIdx == engine->project->selectedItemIdx)
				selectedIdx = (int)selectable.size();

			selectable.emplace_back(trIdx);
		}
	}
}

bool TrainerSearch::SearchCheck(u32 trIdx)
{
	const string& name = engine->trainerNames[trIdx];
	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			if (!IsNumber(searchName[0]) || trIdx != std::stoi(searchName))
				return false;
	}

	return SearchCheckTrainer(trIdx);
}

bool TrainerSearch::SearchCheckTrainer(u32 trIdx)
{
	for (u32 paramIdx = 0; paramIdx < (u32)TRAINERDATA_MAX; ++paramIdx)
	{
		int value = trainerParams[paramIdx];
		if (value == TRAINER_NULL)
			continue;

		if (engine->trainers[trIdx][paramIdx] != value)
			return false;
	}

	return true;
}

bool TrainerSearch::EnableButton(int* params, u32 field, int nullValue, u32 eventID)
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

bool TrainerSearch::ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID)
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
#define TRAINER_COMBO_BOX(label, items, field) ComboBox(label, items, trainerParams.data(), field, TRAINER_NULL, TRAINER_EVENT(field))

bool TrainerSearch::InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int value = params[field];
	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (maxValue >= 0)
		{
			if (value > maxValue)
				value = maxValue;
			if (value < 0)
				value = 0;
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
#define TRAINER_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, trainerParams.data(), field, TRAINER_NULL, TRAINER_EVENT(field))

bool TrainerSearch::CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID)
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
#define TRAINER_CHECK_BOX(label, field) CheckBox(label, trainerParams.data(), field, TRAINER_NULL, TRAINER_EVENT(field))

void TrainerSearch::AdvancedSearchMenu()
{
	bool search = false;
	ImGui::Begin("Trainer Advanced Search", &advancedSearchMenu);

	ImGui::BeginGroup();
	if (TRAINER_COMBO_BOX("Type", engine->trainerTypes, TRAINER_TYPE))
		search = true;
	if (TRAINER_COMBO_BOX("Class", engine->trainerClasses, TRAINER_CLASS))
		search = true;
	if (TRAINER_COMBO_BOX("Battle Type", engine->battleTypes, BATTLE_TYPE))
		search = true;
	
	if (TRAINER_INPUT_INT(u8"Pokémon Count", POKEMON_COUNT, 6))
		search = true;

	if (TRAINER_COMBO_BOX("Item 1", engine->itemNames, ITEM_1))
		search = true;
	if (TRAINER_COMBO_BOX("Item 2", engine->itemNames, ITEM_2))
		search = true;
	if (TRAINER_COMBO_BOX("Item 3", engine->itemNames, ITEM_3))
		search = true;
	if (TRAINER_COMBO_BOX("Item 4", engine->itemNames, ITEM_4))
		search = true;

	if (TRAINER_CHECK_BOX("Is Healer", IS_HEALER))
		search = true;

	if (TRAINER_INPUT_INT("Money", CASH, 255))
		search = true;

	if (TRAINER_COMBO_BOX("Post-Battle Item", engine->itemNames, POST_BATTLE_ITEM))
		search = true;
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("AI Flags");
	for (u32 flag = 0; flag < engine->trainerAIFlags.size(); ++flag)
		if (TRAINER_CHECK_BOX(engine->trainerAIFlags[flag].c_str(), flag + AI_NO_EFFECT_CHECK))
			search = true;
	ImGui::EndGroup();

	ImGui::End();
	if (search)
		Search();
}
