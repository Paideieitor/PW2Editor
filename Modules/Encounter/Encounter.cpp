#include "Windows/Engine.h"

#include "Modules/Encounter/Encounter.h"

#define SELECTED_ENCOUNTER ZONE_ENCOUNTER_IDX(engine->zones[engine->project->selectedZoneIdx])

#define ENCOUNTER_EVENT(season, field, slot) (((season & 0x3) << 30) + ((slot & 0x3) << 28) + (field & 0x0FFFFFFF))
#define PASTE_EVENT ENCOUNTERDATA_MAX
#define CHANGE_SEASONS_EVENT ENCOUNTERDATA_MAX + 1

void GetEncouterEvent(u32 event, Season& season, EncounterField& field, EncounterSlotField& slot)
{
	season = (Season)((event >> 30) & 0x3);
	field = (EncounterField)(event & 0x0FFFFFFF);
	slot = (EncounterSlotField)((event >> 28) & 0x3);
}

Encounter::Encounter(Engine* const engine, u32 group) : Module(engine, group, ENCOUNTER_NARC_PATH)
{
	EncounterTableReset(encounterCopy);
}

Encounter::~Encounter()
{
}

#define COMBO_BOX(label, items, field, slot) ComboBox(encounterData, LABEL(label, field), items, (Season)season, (EncounterField)field, slot);
#define INPUT_INT(label, field, slot, maxValue) InputInt(encounterData, LABEL(label, field), (Season)season, (EncounterField)field, slot, maxValue);
#define CHECK_BOX(label, field, slot) CheckBox(encounterData, LABEL(label, field), (Season)season, (EncounterField)field, slot);
ReturnState Encounter::RenderGUI()
{
	ImGui::Begin("Encounters");

	u32 selectedEncounterIdx = SELECTED_ENCOUNTER;
	if (selectedEncounterIdx < engine->encounters.size())
	{
		EncounterData& encounterData = engine->encounters[selectedEncounterIdx];

		u32 seasonCount = 0;
		for (u32 season = SUMMER; season < SEASON_MAX; ++season)
		{
			EncounterTable& table = encounterData[season];
			if (EncounterTableIsEmpty(table))
				break;
			++seasonCount;
		}

		ImGui::BeginDisabled(seasonCount != 1);
		if (ImGui::Button("Add Season Encounters"))
		{
			EncounterData added = encounterData;
			for (u32 season = 1; season < SEASON_MAX; ++season)
				EncounterTableSetDefault(added[season]);

			u32 eventID = ENCOUNTER_EVENT(0, CHANGE_SEASONS_EVENT, 0);
			SAVE_CHECK_REVERSE_EVENT(eventID, encounterData, 
				added, SELECTED_ENCOUNTER, encounterData);

			encounterData = added;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(seasonCount == 1);
		if (ImGui::Button("Remove Season Encounters"))
		{
			EncounterData removed = encounterData;
			for (u32 season = 1; season < SEASON_MAX; ++season)
				EncounterTableReset(removed[season]);

			u32 eventID = ENCOUNTER_EVENT(0, CHANGE_SEASONS_EVENT, 0);
			SAVE_CHECK_REVERSE_EVENT(eventID, encounterData, 
				removed, SELECTED_ENCOUNTER, encounterData);

			encounterData = removed;
		}
		ImGui::EndDisabled();

		for (u32 season = SUMMER; season < seasonCount; ++season)
		{
			EncounterTable& table = encounterData[season];
			if (seasonCount != 1)
				ImGui::Text(engine->seasons[season]);

			u32 currentSlotIdx = ENCOUNTER_SINGLE;
			for (u32 tableIdx = 0; tableIdx < (u32)tablesInfo.size(); ++tableIdx)
			{
				ImGui::BeginGroup();
				ImGui::Text(tablesInfo[tableIdx].name);
				INPUT_INT("Rate", tableIdx, ENCOUNTER_SPECIES, 100)
				
				ImGui::Text(u8"        Species                                        Form                   Min.                       Max.");
				ImGui::BeginDisabled(table[tableIdx][ENCOUNTER_SPECIES] == 0);
				for (u32 slotIdx = 0; slotIdx < tablesInfo[tableIdx].size; ++slotIdx)
				{
					ImGui::Text(encounterRates[tablesInfo[tableIdx].rate][slotIdx]);
					ImGui::SameLine();
					COMBO_BOX("##Species", engine->pkmNames, currentSlotIdx, ENCOUNTER_SPECIES);
					ImGui::SameLine();
					INPUT_INT("##Form", currentSlotIdx, ENCOUNTER_FORM, 31);
					ImGui::SameLine();
					INPUT_INT("##MinLevel", currentSlotIdx, ENCOUNTER_MIN_LEVEL, 100);
					ImGui::SameLine();
					INPUT_INT("##MaxLevel", currentSlotIdx, ENCOUNTER_MAX_LEVEL, 100);
					++currentSlotIdx;
				}
				ImGui::EndDisabled();

				ImGui::EndGroup();

				if (tableIdx == 0 || tableIdx % 2 != 0)
					ImGui::SameLine();
			}

			if (ImGui::Button(LABEL("Copy", season)))
				encounterCopy = table;
			ImGui::SameLine();
			if (ImGui::Button(LABEL("Paste", season)))
			{
				u32 eventID = ENCOUNTER_EVENT(season, PASTE_EVENT, 0);
				SAVE_CHECK_REVERSE_EVENT(eventID, table,
					encounterCopy, SELECTED_ENCOUNTER, encounterData);

				table = encounterCopy;
			}

			ImGui::Separator();
		}
	}
	ImGui::End();
	return OK;
}

void Encounter::HandleReverseEvent(const Event* reverseEvent)
{
	Season season;
	EncounterField field;
	EncounterSlotField slot;
	GetEncouterEvent(reverseEvent->subType, season, field, slot);

	switch (field)
	{
	case PASTE_EVENT:
	{
		EncounterTable* value = (EncounterTable*)reverseEvent->value;
		engine->encounters[SELECTED_ENCOUNTER][season] = *value;
		break;
	}
	case CHANGE_SEASONS_EVENT:
	{
		EncounterData* value = (EncounterData*)reverseEvent->value;
		engine->encounters[SELECTED_ENCOUNTER] = *value;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;
		engine->encounters[SELECTED_ENCOUNTER][season][field][slot] = *value;
		break;
	}
	}
}

void Encounter::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	EncounterData* originalData = (EncounterData*)saveEvent->value;
	const EncounterData& data = engine->encounters[saveEvent->subType];
	if (*originalData == data)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveEncounter(data, filePath);
}

void Encounter::ComboBox(EncounterData& encounterData, const char* label, const std::vector<std::string>& items, Season season, EncounterField field, EncounterSlotField slot)
{
	int selected = encounterData[season][field][slot];
	if (selected >= items.size())
	{
		InputInt(encounterData, label, season, field, slot, 0xFFFF);
		return;
	}

	u32 eventID = ENCOUNTER_EVENT(season, field, slot);

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(LABEL(label, (int)eventID), items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(eventID, encounterData[season][field][slot],
			selected, SELECTED_ENCOUNTER, encounterData);
		encounterData[season][field][slot] = selected;
	}
}

void Encounter::InputInt(EncounterData& encounterData, const char* label, Season season, EncounterField field, EncounterSlotField slot, int maxValue)
{
	u32 eventID = ENCOUNTER_EVENT(season, field, slot);

	int value = encounterData[season][field][slot];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)eventID), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SAVE_CHECK_REVERSE_EVENT(eventID, encounterData[season][field][slot],
			value, SELECTED_ENCOUNTER, encounterData);
		encounterData[season][field][slot] = value;
	}
}

void Encounter::CheckBox(EncounterData& encounterData, const char* label, Season season, EncounterField field, EncounterSlotField slot)
{
	u32 eventID = ENCOUNTER_EVENT(season, field, slot);

	bool value = (bool)encounterData[season][field][slot];
	if (ImGui::Checkbox(LABEL(label, (int)eventID), &value))
	{
		SAVE_CHECK_REVERSE_EVENT(eventID, encounterData[season][field][slot],
			(int)value, SELECTED_ENCOUNTER, encounterData);
		encounterData[season][field][slot] = (int)value;
	}
}
