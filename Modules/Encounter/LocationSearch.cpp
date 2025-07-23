#include "Utils/StringUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Encounter/LocationSearch.h"

#define SEARCH_NAME_EVENT 0
#define SELECT_LOCATION_EVENT 1
#define CHANGE_NAME_EVENT 2
#define ADD_LOCATION_EVENT 3
#define REMOVE_LOCATION_EVENT 4

LocationSearch::LocationSearch(Engine* const engine, u32 group) : Module(engine, group, TEXT_NARC_PATH)
{
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

	return true;
}
