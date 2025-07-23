#include "Utils/StringUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Encounter/LocationSearch.h"

#define SEARCH_NAME_EVENT 0
#define SELECT_LOCATION_EVENT 1
#define CHANGE_NAME_EVENT 2
#define ADD_LOCATION_EVENT 3
#define REMOVE_LOCATION_EVENT 4
#define ADVANCED_SEARCH_EVENT 5
#define CLEAR_SEARCH_EVENT 6

#define ENCOUNTER_EVENT(field) (7 + field)
#define GET_ENCOUNTER_FROM_EVENT(subType) (subType - ENCOUNTER_EVENT(0))

LocationSearch::LocationSearch(Engine* const engine, u32 group) : Module(engine, group, TEXT_NARC_PATH)
{
	ClearSearch();

	Search();
}

LocationSearch::~LocationSearch()
{
}

ReturnState LocationSearch::RenderGUI()
{
	if (selectedIdx >= 0 && engine->project->selectedLocationIdx != selectable[selectedIdx])
	{
		selectedIdx = 0;
		for (int idx = 0; idx < (int)engine->locations.size(); ++idx)
		{
			if (selectable[idx] == engine->project->selectedLocationIdx)
			{
				selectedIdx = idx;
				break;
			}
		}
	}

	ImGui::Begin(u8"Search Location");

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
			if (ImGui::Selectable(LABEL(engine->locationNames[selectable[idx]], idx), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_LOCATION_EVENT,
					make_pair(engine->project->selectedLocationIdx, selectedIdx),
					make_pair(selectable[idx], idx));

				engine->SetCurrentLocation(selectable[idx], engine->project->selectedZoneIdx);
				selectedIdx = idx;
			}
		}
		ImGui::EndListBox();
	}

	if (engine->PatchIsInstalled())
	{
		if (ImGui::Button("Add Location"))
		{
			SAVE_CHECK_REVERSE_EVENT(ADD_LOCATION_EVENT, 1, 0, 0, engine->locationNames);

			engine->locationNames.emplace_back("Location " + to_string(engine->locationNames.size()));
			engine->locations.emplace_back();
			Search();
		}

		ImGui::BeginDisabled(engine->project->selectedLocationIdx != (u32)engine->locations.size() - 1 ||
			!engine->locations.back().empty());
		if (ImGui::Button("Remove Last Location"))
		{
			engine->SetCurrentLocation((u32)engine->locations.size() - 2, 0);

			SAVE_CHECK_REVERSE_EVENT(REMOVE_LOCATION_EVENT, engine->locationNames.back(),
				string(), 0, engine->locationNames);

			engine->locationNames.erase(engine->locationNames.begin() + engine->locationNames.size() - 1);
			engine->locations.erase(engine->locations.begin() + engine->locations.size() - 1);
			Search();
		}
		if (engine->project->selectedLocationIdx != (u32)engine->locations.size() - 1)
		{
			ImGui::Text("Last location can't be removed!");
			ImGui::Text("It needs to be selected");
		}
		else if (!engine->locations.back().empty())
		{
			ImGui::Text("Last location can't be removed!");
			ImGui::Text("At least 1 zone references it");
		}
		ImGui::EndDisabled();
	}

	ImGui::Separator();

	if (ImGui::Button("Advanced Search"))
	{
		SIMPLE_REVERSE_EVENT(ADVANCED_SEARCH_EVENT, advancedSearchMenu, !advancedSearchMenu);
		advancedSearchMenu = !advancedSearchMenu;
	}

	if (ImGui::Button("Clear Search"))
	{
		pair<string, EncounterSlot> searchData = make_pair(searchName, encounterParams);
		ClearSearch();
		SIMPLE_REVERSE_EVENT(CLEAR_SEARCH_EVENT,
			make_pair(searchName, encounterParams), searchData);

		Search();
	}

	if (advancedSearchMenu)
		AdvancedSearchMenu();

	ImGui::Separator();

	if (selectedIdx >= 0 &&
		engine->project->selectedLocationIdx < (u32)engine->locationNames.size())
	{
		ImGui::Text("Name:");
		TextInput("##Name", &(engine->locationNames[engine->project->selectedLocationIdx]), CHANGE_NAME_EVENT, engine->locationNames);
	}

	ImGui::End();
	return OK;
}

void LocationSearch::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case SEARCH_NAME_EVENT: // Search Name
	{
		string* name = (string*)reverseEvent->value;
		searchName = *name;
		break;
	}
	case SELECT_LOCATION_EVENT: // Selected Location
	{
		pair<u32, int>* selected = (pair<u32, int>*)reverseEvent->value;
		engine->SetCurrentLocation(selected->first, engine->project->selectedZoneIdx);
		selectedIdx = selected->second;
		break;
	}
	case CHANGE_NAME_EVENT: // Change Name
	{
		string* value = (string*)reverseEvent->value;
		engine->locationNames[engine->project->selectedLocationIdx] = *value;
		break;
	}
	case ADD_LOCATION_EVENT: // Add Location
	{
		engine->locationNames.erase(engine->locationNames.begin() + engine->locationNames.size() - 1);
		engine->locations.erase(engine->locations.begin() + engine->locations.size() - 1);

		engine->SetCurrentLocation((u32)engine->locations.size() - 1, 0);
		break;
	}
	case REMOVE_LOCATION_EVENT: // Remove Last Location
	{
		string* value = (string*)reverseEvent->value;

		engine->locationNames.push_back(*value);
		engine->locations.emplace_back();
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
		pair<string, EncounterSlot>* searchData = (pair<string, EncounterSlot>*)reverseEvent->value;
		searchName = searchData->first;
		encounterParams = searchData->second;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;

		u32 field = GET_ENCOUNTER_FROM_EVENT(reverseEvent->subType);
		encounterParams[field] = *value;
		break;
	}
	}

	Search();
}

void LocationSearch::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	vector<string>* originalData = (vector<string>*)saveEvent->value;
	if (*originalData == engine->locationNames)
		return;

	string filePath = MAKE_FILE_PATH(savePath, LOCATION_NAME_FILE_ID);
	SaveAlle5File(filePath, engine->locationNames);
}

void LocationSearch::TextInput(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputText(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}

bool LocationSearch::EnableButton(int* params, u32 field, int nullValue, u32 eventID)
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

bool LocationSearch::ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID)
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
#define ENCOUNTER_COMBO_BOX(label, items, field) ComboBox(label, items, encounterParams.data(), field, ENCOUNTERDATA_NULL, ENCOUNTER_EVENT(field))

bool LocationSearch::InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID)
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
#define ENCOUNTER_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, encounterParams.data(), field, ENCOUNTERDATA_NULL, ENCOUNTER_EVENT(field))

void LocationSearch::Search()
{
	// Reset the search data
	selectedIdx = -1;
	selectable.clear();

	for (u32 locationIdx = 0; locationIdx < (u32)engine->locations.size(); ++locationIdx)
	{
		if (SearchCheck(locationIdx))
		{
			// Set the selected index if this is the selected Pokémon
			if (locationIdx == engine->project->selectedLocationIdx)
				selectedIdx = (int)selectable.size();

			selectable.emplace_back(locationIdx);
		}
	}
}

bool LocationSearch::SearchCheck(u32 locationIdx)
{
	const string& name = engine->locationNames[locationIdx];
	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			if (!IsNumber(searchName[0]) || locationIdx != std::stoi(searchName))
				return false;
	}

	bool params = false;
	for (u32 idx = 0; idx < (u32)encounterParams.size(); ++idx)
		if (encounterParams[idx] != ENCOUNTERDATA_NULL)
			params = true;
	if (!params)
		return true;

	const Location& location = engine->locations[locationIdx];
	for (u32 idx = 0; idx < (u32)location.size(); ++idx)
	{
		const ZoneData& zone = engine->zones[location[idx]];
		u32 encounterIdx = ZONE_ENCOUNTER_IDX(zone);
		if (encounterIdx != ZONE_INVALID_ENCOUNTER_IDX)
			if (SearchCheckEncounter(encounterIdx))
				return true;
	}
	return false;
}

bool LocationSearch::SearchCheckEncounter(u32 encounterIdx)
{
	u32 season = SUMMER;
	u32 seasonCount = SEASON_MAX;
	if (encounterParams[ENCOUNTER_MAX_LEVEL] != ENCOUNTERDATA_NULL)
	{
		season = encounterParams[ENCOUNTER_MAX_LEVEL];
		seasonCount = season + 1;
	}

	bool looking = false;
	for (season; season < seasonCount; ++season)
	{
		const EncounterTable& table = engine->encounters[encounterIdx][season];
		for (u32 encounter = ENCOUNTER_SINGLE; encounter < ENCOUNTERDATA_MAX; ++encounter)
		{
			u32 params = 0;
			u32 matches = 0;
			if (encounterParams[ENCOUNTER_SPECIES] != ENCOUNTERDATA_NULL)
			{
				looking = true;
				++params;
				if (encounterParams[ENCOUNTER_SPECIES] == table[encounter][ENCOUNTER_SPECIES])
					++matches;
			}

			if (encounterParams[ENCOUNTER_FORM] != ENCOUNTERDATA_NULL)
			{
				looking = true;
				++params;
				if (encounterParams[ENCOUNTER_FORM] == table[encounter][ENCOUNTER_FORM])
					++matches;
			}

			if (encounterParams[ENCOUNTER_MIN_LEVEL] != ENCOUNTERDATA_NULL)
			{
				looking = true;
				++params;
				if (encounterParams[ENCOUNTER_MIN_LEVEL] >= table[encounter][ENCOUNTER_MIN_LEVEL] &&
					encounterParams[ENCOUNTER_MIN_LEVEL] <= table[encounter][ENCOUNTER_MAX_LEVEL])
					++matches;
			}

			if (matches == params)
				return true;
		}
	}

	return !looking;
}

void LocationSearch::ClearSearch()
{
	searchName.clear();

	EncounterSlotReset(encounterParams);
}

void LocationSearch::AdvancedSearchMenu()
{
	bool search = false;
	ImGui::Begin("Encounter Advanced Search", &advancedSearchMenu);

	if (ENCOUNTER_COMBO_BOX("Season", engine->seasons, ENCOUNTER_MAX_LEVEL))
		search = true;
	if (ENCOUNTER_COMBO_BOX("Species", engine->pkmNames, ENCOUNTER_SPECIES))
		search = true;
	if (ENCOUNTER_INPUT_INT("Form", ENCOUNTER_FORM, 31))
		search = true;
	if (ENCOUNTER_INPUT_INT("Level", ENCOUNTER_MIN_LEVEL, 100))
		search = true;

	ImGui::End();
	if (search)
		Search();
}
