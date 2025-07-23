#include "Windows/Engine.h"

#include "Modules/Encounter/Zone.h"

#define SELECT_ZONE_EVENT 0
#define CHANGE_LOCATION 1

Zone::Zone(Engine* const engine, u32 group) : Module(engine, group, ZONE_NARC_PATH)
{
	prevLocation = engine->project->selectedLocationIdx;
}

Zone::~Zone()
{
}

ReturnState Zone::RenderGUI()
{
	Location& location = engine->locations[engine->project->selectedLocationIdx];

	if (prevLocation != engine->project->selectedLocationIdx)
	{
		selectedIdx = 0;
		for (int idx = 0; idx < (int)location.size(); ++idx)
		{
			if (location[idx] == engine->project->selectedZoneIdx)
			{
				selectedIdx = idx;
				break;
			}
		}
	}
	prevLocation = engine->project->selectedLocationIdx;

	ImGui::Begin("Zones");

	ImGui::Text("Zone List");
	if (ImGui::BeginListBox(" ", ImVec2(ImGui::GetWindowWidth() - 15.0f, ImGui::GetWindowHeight() * 0.70f)))
	{
		for (int idx = 0; idx < (int)location.size(); ++idx)
		{
			u32 zoneIdx = location[idx];
			if (zoneIdx >= (u32)engine->zones.size())
				continue;
			const ZoneData& zone = engine->zones[zoneIdx];
			u32 encounterIdx = ZONE_ENCOUNTER_IDX(zone);

			string encounterTable;
			if (encounterIdx == ZONE_INVALID_ENCOUNTER_IDX)
				encounterTable = "None";
			else if (encounterIdx >= (u32)engine->encounters.size())
				encounterTable = "Error";
			else
				encounterTable = to_string(encounterIdx);

			string zoneName = "Zone " + to_string(zoneIdx) + " (" + encounterTable + ")";
			if (ImGui::Selectable(zoneName.c_str(), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_ZONE_EVENT,
					make_pair(engine->project->selectedZoneIdx, selectedIdx),
					make_pair(zoneIdx, idx));

				engine->SetCurrentLocation(engine->project->selectedLocationIdx, zoneIdx);
				selectedIdx = idx;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::Separator();

	if (selectedIdx >= 0 && 
		!location.empty() &&
		engine->project->selectedZoneIdx < (u32)engine->zones.size())
	{
		ZoneData& zone = engine->zones[engine->project->selectedZoneIdx];

		string zoneName = "Zone " + to_string(engine->project->selectedZoneIdx);
		ImGui::Text(zoneName);

		LocationComboBox(zone);
	}
	ImGui::End();

	return OK;
}

void Zone::HandleReverseEvent(const Event* reverseEvent)
{
	ZoneData& zone = engine->zones[engine->project->selectedZoneIdx];

	switch (reverseEvent->subType)
	{
	case SELECT_ZONE_EVENT: // Selected Zone
	{
		pair<u32, int>* selected = (pair<u32, int>*)reverseEvent->value;
		engine->SetCurrentLocation(engine->project->selectedLocationIdx, selected->first);
		selectedIdx = selected->second;
		break;
	}
	case CHANGE_LOCATION:
	{
		int* previous = (int*)reverseEvent->value;
		int next = ZONE_LOCATION_NAME_IDX(zone);

		zone[LOCATION_NAME_INFO] = SET_ZONE_LOCATION_NAME_IDX(zone, *previous);
		SwapLocations(next, *previous);
		
		break;
	}
	}
}

void Zone::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	vector<ZoneData>* originalData = (vector<ZoneData>*)saveEvent->value;
	if (*originalData == engine->zones)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveZone(engine->zones, filePath);
}

void Zone::SwapLocations(int previous, int next)
{
	if (previous != next)
	{
		u32 zoneIdx = engine->project->selectedZoneIdx;

		Location& prevLoc = engine->locations[previous];
		for (u32 idx = 0; idx < (u32)prevLoc.size(); ++idx)
			if (prevLoc[idx] == zoneIdx)
				prevLoc.erase(prevLoc.begin() + idx);

		engine->locations[next].push_back(zoneIdx);

		engine->SetCurrentLocation((u32)next, 0);
		engine->SetCurrentLocation((u32)next, zoneIdx);
	}
}

void Zone::LocationComboBox(ZoneData& zoneData)
{
	ImGui::Text("Location:");
	int next = engine->project->selectedLocationIdx;

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox("##Location", engine->locationNames, &next))
	{
		int previous = engine->project->selectedLocationIdx;
		SAVE_CHECK_REVERSE_EVENT(CHANGE_LOCATION, previous, next,
			0, engine->zones);

		zoneData[LOCATION_NAME_INFO] = SET_ZONE_LOCATION_NAME_IDX(zoneData, next);

		SwapLocations(previous, next);
	}
}