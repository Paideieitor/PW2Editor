#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/Evolution.h"

Evolution::Evolution(Engine* const engine, u32 group) : Module(engine, group, EVOLUTION_NARC_PATH)
{
}

Evolution::~Evolution()
{
}

ReturnState Evolution::RenderGUI()
{
	ImGui::Begin("Evolution");
	EvolutionData& evolution = engine->evolution[engine->GetCurrentPokemon()->dataIdx];

	u32 evoCount = 0;
	for (u32 evoIdx = 0; evoIdx < (u32)evolution.size(); ++evoIdx)
	{
		int method = evolution[evoIdx][EVOLUTION_METHOD];
		if (method == EVOLUTION_NULL)
			continue;
		++evoCount;

		ComboBox(evolution, (string("##Method") + to_string(evoIdx)).c_str(),
			engine->evolutionMethods, evoIdx, EVOLUTION_METHOD);

		ImGui::SameLine();

		string paramLabel = string("##Param") + to_string(evoIdx);
		switch (method)
		{
		// Nothing
		case EVOLVE_FRIENDSHIP:
		case EVOLVE_FRIENDSHIP_MORNING:
		case EVOLVE_FRIENDSHIP_NIGHT:
		case EVOLVE_TRADE:
		case EVOLVE_TRADE_MONS_SPECIFIC:
		case EVOLVE_LEVELUP_ELEC:
		case EVOLVE_LEVELUP_MOSS:
		case EVOLVE_LEVELUP_ICE:
		case EVOLVE_LEVELUP_ELEC2:
			ImGui::Text("No Parameter");
			break;
		// Level
		case EVOLVE_LEVELUP:
		case EVOLVE_LEVELUP_ATK_OVER_DEF:
		case EVOLVE_LEVELUP_ATK_EQU_DEF:
		case EVOLVE_LEVELUP_DEF_OVER_ATK:
		case EVOLVE_LEVELUP_RANDOM_A:
		case EVOLVE_LEVELUP_RANDOM_B:
		case EVOLVE_LEVELUP_NINJASK:
		case EVOLVE_LEVELUP_SHEDINJA:
		case EVOLVE_LEVELUP_M:
		case EVOLVE_LEVELUP_F:
			InputInt(evolution, paramLabel.c_str(), evoIdx, 100);
			break;
		// Item
		case EVOLVE_TRADE_ITEM:
		case EVOLVE_ITEM_USE:
		case EVOLVE_LEVELUP_ITEM_HELD_DAY:
		case EVOLVE_LEVELUP_ITEM_HELD_NIGHT:
		case EVOLVE_ITEM_USE_M:
		case EVOLVE_ITEM_USE_F:
			ComboBox(evolution, paramLabel.c_str(), engine->itemNames, evoIdx, EVOLUTION_PARAM);
			break;
		// Beauty
		case EVOLVE_LEVELUP_BEAUTY:
			break;
		// Move
		case EVOLVE_LEVELUP_MOVE:
			ComboBox(evolution, paramLabel.c_str(), engine->moveNames, evoIdx, EVOLUTION_PARAM);
			break;
		// Pokémon
		case EVOLVE_LEVELUP_PARTYMONS:
			ComboBox(evolution, paramLabel.c_str(), engine->pkmNames, evoIdx, EVOLUTION_PARAM);
			break;
		}

		ImGui::SameLine();

		ComboBox(evolution, (string("##Species") + to_string(evoIdx)).c_str(),
			engine->pkmNames, evoIdx, EVOLUTION_SPECIES);

		ImGui::SameLine();

		if (ImGui::Button((string("X##") + to_string(evoIdx)).c_str()))
		{
			pair<u32, EvolutionMethod> value = make_pair(evoIdx, evolution[evoIdx]);
			pair<u32, EvolutionMethod> newValue = make_pair(evoIdx, EvolutionMethod({ EVOLUTION_NULL, EVOLUTION_NULL, EVOLUTION_NULL }));
			SAVE_CHECK_REVERSE_EVENT(EVOLUTION_MAX, value, newValue,
				engine->GetCurrentPokemon()->dataIdx, evolution);

			u32 lastIdx = (u32)evolution.size() - 1;
			for (; evoIdx < lastIdx; ++evoIdx)
			{
				evolution[evoIdx][EVOLUTION_METHOD] = evolution[evoIdx + 1][EVOLUTION_METHOD];
				evolution[evoIdx][EVOLUTION_PARAM] = evolution[evoIdx + 1][EVOLUTION_PARAM];
				evolution[evoIdx][EVOLUTION_SPECIES] = evolution[evoIdx + 1][EVOLUTION_SPECIES];
			}
			evolution[lastIdx][EVOLUTION_METHOD] = EVOLUTION_NULL;
			evolution[lastIdx][EVOLUTION_PARAM] = EVOLUTION_NULL;
			evolution[lastIdx][EVOLUTION_SPECIES] = EVOLUTION_NULL;
			
			ImGui::End();
			return OK;
		}
	}

	ImGui::BeginDisabled(evoCount >= engine->project->evolutionSize);
	if (ImGui::Button("Add Evolution"))
	{
		evolution[evoCount][EVOLUTION_METHOD] = 1;
		evolution[evoCount][EVOLUTION_PARAM] = 0;
		evolution[evoCount][EVOLUTION_SPECIES] = 0;
		SAVE_CHECK_REVERSE_EVENT(EVOLUTION_MAX + 1, evoCount, ~evoCount,
			engine->GetCurrentPokemon()->dataIdx, evolution);
	}
	ImGui::EndDisabled();

	ImGui::Text((string("Evolution Count: ") + to_string(evoCount) + "/" +
		to_string(engine->project->evolutionSize)));

	ImGui::End();
	return OK;
}

void Evolution::HandleReverseEvent(const Event* reverseEvent)
{
	EvolutionData& evolution = engine->evolution[engine->GetCurrentPokemon()->dataIdx];
	switch (reverseEvent->subType)
	{
	case EVOLUTION_MAX: // Remove an Evolution Method
	{
		pair<u32, EvolutionMethod>* value = (pair<u32, EvolutionMethod>*)reverseEvent->value;
		
		for (int idx = (int)engine->project->evolutionSize - 1; idx > (int)value->first; --idx)
		{
			evolution[idx][EVOLUTION_METHOD] = evolution[idx - 1][EVOLUTION_METHOD];
			evolution[idx][EVOLUTION_PARAM] = evolution[idx - 1][EVOLUTION_PARAM];
			evolution[idx][EVOLUTION_SPECIES] = evolution[idx - 1][EVOLUTION_SPECIES];
		}

		evolution[value->first][EVOLUTION_METHOD] = value->second[EVOLUTION_METHOD];
		evolution[value->first][EVOLUTION_PARAM] = value->second[EVOLUTION_PARAM];
		evolution[value->first][EVOLUTION_SPECIES] = value->second[EVOLUTION_SPECIES];
		break;
	}
	case EVOLUTION_MAX + 1: // Add an Evolution Method
	{
		u32* idx = (u32*)reverseEvent->value;
		evolution[*idx][EVOLUTION_METHOD] = EVOLUTION_NULL;
		evolution[*idx][EVOLUTION_PARAM] = EVOLUTION_NULL;
		evolution[*idx][EVOLUTION_SPECIES] = EVOLUTION_NULL;
		break;
	}
	default: // Change an Evolution Method value
	{
		pair<u32, int>* value = (pair<u32, int>*)reverseEvent->value;
		evolution[value->first][reverseEvent->subType] = value->second;
		break;
	}
	}
}

void Evolution::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	EvolutionData* originalData = (EvolutionData*)saveEvent->value;
	const EvolutionData& data = engine->evolution[saveEvent->subType];
	if (*originalData == data)
		return;
	// Update the latest saved instance of the data for other changes
	*((EvolutionData*)saveEvent->value) = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveEvolution(data, filePath);
}

void Evolution::ComboBox(EvolutionData& evolution, const char* label, const std::vector<std::string>& items, u32 idx, EvolutionField field)
{
	int selected = evolution[idx][field];
	if (selected >= items.size())
		selected = 0;

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		pair<u32, int> value = make_pair(idx, evolution[idx][field]);
		pair<u32, int> newValue = make_pair(idx, selected);
		SAVE_CHECK_REVERSE_EVENT(field, value, newValue,
			engine->GetCurrentPokemon()->dataIdx, evolution);
		evolution[idx][field] = selected;
	}
}

void Evolution::InputInt(EvolutionData& evolution, const char* label, u32 idx, int maxValue)
{
	int data = evolution[idx][EVOLUTION_PARAM];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(label, &data) && ImGui::IsItemDeactivatedAfterEdit())
	{
		if (data > maxValue)
			data = maxValue;
		if (data < 0)
			data = 0;

		pair<u32, int> value = make_pair(idx, evolution[idx][EVOLUTION_PARAM]);
		pair<u32, int> newValue = make_pair(idx, data);
		SAVE_CHECK_REVERSE_EVENT(EVOLUTION_PARAM, value, newValue,
			engine->GetCurrentPokemon()->dataIdx, evolution);
		evolution[idx][EVOLUTION_PARAM] = data;
	}
}
