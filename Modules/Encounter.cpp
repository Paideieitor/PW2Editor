#include "Windows/Engine.h"

#include "Modules/Encounter.h"

#define ZONE_GET_VALUE(idx, field) engine->GetDataValue(engine->zones, 0, ZONE_FIELD(idx, field))

#define ENCOUNTER_INPUT_INT(idx, field, label, max, min) engine->InputInt(engine->encounters, idx, field, &engine->project->location, engine->project->location, group, label, max, min)
#define ENCOUNTER_COMBO_BOX(idx, field, label, _list) engine->ComboBox(engine->encounters, idx, field, &engine->project->location, engine->project->location, group, label, _list)

#define ENCOUNTER_GET_VALUE(idx, field) engine->GetDataValue(engine->encounters, idx, field)
#define ENCOUNTER_SET_VALUE(idx, field, value) engine->SetDataValue(engine->encounters, idx, field, value)
#define ENCOUNTER_START_ACTION engine->StartAction(&engine->project->location, engine->project->location, group)
#define ENCOUNTER_END_ACTION engine->EndAction()

Encounter::Encounter(Engine* const engine, u32 group) : Module(engine, group),
locations(engine->GetTextFile(LOCATION_NAME_FILE_ID)), pokemons(engine->GetTextFile(PKM_NAME_FILE_ID))
{
	for (u32 entry = 0; entry < SEASON_SIZE; ++entry)
		copied[entry] = ENCOUNTER_NULL;
}

ReturnState Encounter::RenderGUI()
{
	int encounter = -1;
	u32 location = GetProjectLocation(*engine->project);

	ImGui::Begin("Zones");
	ImGui::BeginGroup();
	{
		if (location < locations->size())
		{
			encounter = Zones(location);
		}
	}
	ImGui::EndGroup();
	ImGui::End();

	ImGui::Begin("Encounters");

	if (encounter >= 0)
	{
		u32 encounterIdx = (u32)encounter;
		ImGui::Text(locations->at(location) +
			" - Zone: " + to_string(GetProjectZone(*engine->project)) +
			" - Encounter: " + to_string(encounterIdx));
		ImGui::Separator();

		ImGui::BeginGroup();
		{
			u32 seasonIdx = 0;
			if (HasSeasons(encounterIdx))
			{
				seasonIdx = GetProjectSeason(*engine->project);
				if (seasonIdx < 0 || seasonIdx >= SEASON_MAX)
				{
					seasonIdx = 0;
					SetProjectSeason(*engine->project, 0);
				}

				ImGui::SetNextItemWidth(150.0f);
				if (ImGui::TextInputComboBox("Season", &seasons, (int*)&seasonIdx))
				{
					SetProjectSeason(*engine->project, seasonIdx);
				}
				ImGui::SameLine();
				if (ImGui::Button("Remove Seasons"))
				{
					ENCOUNTER_START_ACTION;
					for (u32 season = 1; season < SEASON_MAX; ++season)
					{
						FillTable(encounterIdx, season, ENCOUNTER_NULL);
					}
					ENCOUNTER_END_ACTION;
				}
			}
			else
			{
				if (ImGui::Button("Add Seasons"))
				{
					ENCOUNTER_START_ACTION;
					for (u32 season = 1; season < SEASON_MAX; ++season)
					{
						FillTable(encounterIdx, season, 0);
					}
					ENCOUNTER_END_ACTION;
				}
			}

			Season(encounterIdx, seasonIdx);
		}
		ImGui::EndGroup();
	}
	ImGui::End();
	return OK;
}

int Encounter::Zones(u32 location)
{
	int encounter = -1;

	ImGui::Text("Zones");
	u32 zoneCount = engine->GetConcatDataCount(engine->zones, 0);
	if (ImGui::BeginListBox("##ZoneList", ImVec2(ImGui::GetWindowWidth() - 15.0f, ImGui::GetWindowHeight() * 0.90f)))
	{
		for (u32 zoneIdx = 0; zoneIdx < (int)zoneCount; ++zoneIdx)
		{
			int locationIdx = ZONE_GET_VALUE(zoneIdx, LOCATION_NAME_IDX);
			locationIdx = ZONE_LOCATION_NAME_IDX(locationIdx);
			if (location != locationIdx)
				continue;

			string zoneName = locations->at(location);
			zoneName += " - " + to_string(zoneIdx);

			int encounterIdx = ZONE_GET_VALUE(zoneIdx, ENCOUNTER_IDX);
			encounterIdx = ZONE_ENCOUNTER_IDX(encounterIdx);
			if (encounterIdx == ZONE_INVALID_ENCOUNTER_IDX)
			{
				ImGui::Text(zoneName);
				continue;
			}
			zoneName += " (" + to_string(encounterIdx) + ")";

			if (ImGui::Selectable(zoneName.c_str(), zoneIdx == GetProjectZone(*engine->project)))
			{
				SetProjectZone(*engine->project, zoneIdx);
			}

			if (zoneIdx == GetProjectZone(*engine->project))
				encounter = (int)encounterIdx;
		}
		ImGui::EndListBox();
	}

	return encounter;
}

void Encounter::Season(u32 encounterIdx, u32 season)
{
	u32 startField = ENCOUNTER_RATE(season, ENCOUNTER_TYPE_MAX);
	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Land Singles", LAND_SINGLES, 0, startField);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Dummy({ 30.0f, 0.0f });
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Land Doubles", LAND_DOUBLES, 0, startField);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Dummy({ 30.0f, 0.0f });
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Land Special", LAND_SPECIAL, 0, startField);
	}
	ImGui::EndGroup();

	ImGui::NewLine();

	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Surf", SURF, 1, startField);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Dummy({ 30.0f, 0.0f });
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Surf Special", SURF_SPECIAL, 1, startField);
	}
	ImGui::EndGroup();

	ImGui::NewLine();

	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Fishing", FISHING, 2, startField);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Dummy({ 30.0f, 0.0f });
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		startField = Table(encounterIdx, season, "Fishing Special", FISHING_SPECIAL, 2, startField);
	}
	ImGui::EndGroup();

	if (ImGui::Button("Copy"))
	{
		Copy(encounterIdx, season);
	}
	ImGui::SameLine();
	if (ImGui::Button("Paste"))
	{
		ENCOUNTER_START_ACTION;
		Paste(encounterIdx, season);
		ENCOUNTER_END_ACTION;
	}
}

u32 Encounter::Table(u32 encounterIdx, u32 season, const string& label, u32 type, u32 ratesIdx, u32 firstField)
{
	ImGui::Text(label);
	ImGui::SameLine();
	ENCOUNTER_INPUT_INT(encounterIdx, ENCOUNTER_RATE(season, type), "Rate", 0xFF, 0);

	const vector<string>& rate = rates.at(ratesIdx);
	u32 field = firstField;
	ImGui::BeginGroup();
	{
		ImGui::Text("        Species");
		for (u32 idx = 0; idx < (u32)rate.size(); ++idx)
		{
			ImGui::Text(rate.at(idx));
			ImGui::SameLine();
			ENCOUNTER_COMBO_BOX(encounterIdx, field, "", pokemons);
			field += ENCOUNTER_MAX;
		}
	}
	ImGui::EndGroup();
	ImGui::SameLine();

	field = firstField + 1;
	ImGui::BeginGroup();
	{
		ImGui::Text("Form");
		for (u32 idx = 0; idx < (u32)rate.size(); ++idx)
		{
			ENCOUNTER_INPUT_INT(encounterIdx, field, "", 31, 0);
			field += ENCOUNTER_MAX;
		}
	}
	ImGui::EndGroup();
	ImGui::SameLine();

	field = firstField + 2;
	ImGui::BeginGroup();
	{
		ImGui::Text("Min.");
		for (u32 idx = 0; idx < (u32)rate.size(); ++idx)
		{
			ENCOUNTER_INPUT_INT(encounterIdx, field, "", 100, 0);
			field += ENCOUNTER_MAX;
		}
	}
	ImGui::EndGroup();
	ImGui::SameLine();

	field = firstField + 3;
	ImGui::BeginGroup();
	{
		ImGui::Text("Max.");
		for (u32 idx = 0; idx < (u32)rate.size(); ++idx)
		{
			ENCOUNTER_INPUT_INT(encounterIdx, field, "", 100, 0);
			field += ENCOUNTER_MAX;
		}
	}
	ImGui::EndGroup();

	// Set the returning field to the first field in the next encounter table
	return field - ENCOUNTER_MAX + 1;
}

bool Encounter::HasSeasons(u32 encounterIdx)
{
	return ENCOUNTER_GET_VALUE(encounterIdx, ENCOUNTER_RATE(1, LAND_SINGLES)) != ENCOUNTER_NULL;
}

void Encounter::FillTable(u32 encounterIdx, u32 season, int value)
{
	u32 start = SEASON_START(season);
	for (u32 entry = 0; entry < SEASON_SIZE; ++entry)
		ENCOUNTER_SET_VALUE(encounterIdx, start + entry, value);
}

void Encounter::Copy(u32 encounterIdx, u32 season)
{
	u32 start = SEASON_START(season);
	for (u32 entry = 0; entry < SEASON_SIZE; ++entry)
		copied[entry] = ENCOUNTER_GET_VALUE(encounterIdx, start + entry);
}

void Encounter::Paste(u32 encounterIdx, u32 season)
{
	if (copied[0] == ENCOUNTER_NULL)
		return;

	u32 start = SEASON_START(season);
	for (u32 entry = 0; entry < SEASON_SIZE; ++entry)
		ENCOUNTER_SET_VALUE(encounterIdx, start + entry, copied[entry]);
}
