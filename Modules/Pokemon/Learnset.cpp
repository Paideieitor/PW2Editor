#include <algorithm>

#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/Learnset.h"

Learnset::Learnset(Engine* const engine, u32 group) : Module(engine, group, LEARNSET_NARC_PATH)
{
}

Learnset::~Learnset()
{
}

ReturnState Learnset::RenderGUI()
{
	ImGui::Begin("Learnset");
	LearnsetData& learnset = engine->learnset[engine->GetCurrentPokemon()->dataIdx];

	ImGui::Text("Move										   Level");

	u32 moveIdx = 0;
	vector<pair<u32, int>> orderedSet = GetOrderedLearnset(learnset);
	for (; moveIdx < (u32)orderedSet.size(); ++moveIdx)
	{
		u32 idx = orderedSet[moveIdx].first;
		ComboBox(learnset, (string("##Move") + to_string(idx)).c_str(),
			engine->moveNames, idx);

		ImGui::SameLine();

		InputInt(learnset, (string("##Level") + to_string(idx)).c_str(),
			idx, 100);

		ImGui::SameLine();

		if (ImGui::Button((string("X##") + to_string(idx)).c_str()))
		{
			pair<u32, LearnMove> value = make_pair(idx, learnset[idx]);
			pair<u32, LearnMove> newValue = make_pair(idx, LearnMove({ LEARNSET_SKIP, LEARNSET_SKIP }));
			SAVE_CHECK_REVERSE_EVENT(LEARNSET_MAX, value, newValue,
				engine->GetCurrentPokemon()->dataIdx, learnset);

			learnset[idx][LEARN_MOVE_ID] = LEARNSET_SKIP;
			learnset[idx][LEARN_LEVEL] = LEARNSET_SKIP;
		}
	}

	ImGui::BeginDisabled(moveIdx >= engine->project->learnsetSize);
	if (ImGui::Button("Add Move"))
	{
		for (u32 addIdx = 0; addIdx < engine->project->learnsetSize; ++addIdx)
		{
			int& moveID = learnset[addIdx][LEARN_MOVE_ID];
			if (moveID == LEARNSET_SKIP || moveID == LEARNSET_NULL)
			{
				moveID = 0;
				learnset[addIdx][LEARN_LEVEL] = 0;

				SAVE_CHECK_REVERSE_EVENT(LEARNSET_MAX + 1, addIdx, ~addIdx,
					engine->GetCurrentPokemon()->dataIdx, learnset);

				break;
			}
		}
	}
	ImGui::EndDisabled();

	ImGui::Text((string("Move Count: ") + to_string(moveIdx) + "/" + 
		to_string(engine->project->learnsetSize)));

	ImGui::End();
	return OK;
}

void Learnset::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case LEARNSET_MAX: // Remove Learnset Move
	{
		pair<u32, LearnMove>* value = (pair<u32, LearnMove>*)reverseEvent->value;
		engine->learnset[engine->GetCurrentPokemon()->dataIdx][value->first][LEARN_MOVE_ID] = value->second[LEARN_MOVE_ID];
		engine->learnset[engine->GetCurrentPokemon()->dataIdx][value->first][LEARN_LEVEL] = value->second[LEARN_LEVEL];
		break;
	}
	case LEARNSET_MAX + 1: // Add a Learnset Move
	{
		u32* idx = (u32*)reverseEvent->value;
		engine->learnset[engine->GetCurrentPokemon()->dataIdx][*idx][LEARN_MOVE_ID] = LEARNSET_NULL;
		engine->learnset[engine->GetCurrentPokemon()->dataIdx][*idx][LEARN_LEVEL] = LEARNSET_NULL;
		break;
	}
	default: // Change a Learnset Move value
	{
		pair<u32, int>* value = (pair<u32, int>*)reverseEvent->value;
		engine->learnset[engine->GetCurrentPokemon()->dataIdx][value->first][reverseEvent->subType] = value->second;
		break;
	}
	}
}

void Learnset::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	LearnsetData* originalData = (LearnsetData*)saveEvent->value;
	const LearnsetData& data = engine->learnset[saveEvent->subType];
	if (*originalData == data)
		return;
	// Update the latest saved instance of the data for other changes
	*((LearnsetData*)saveEvent->value) = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveLearnset(data, filePath);
}

bool LearnsetOrderCmp(const pair<u32, int>& a, const pair<u32, int>& b)
{
	if (a.second < b.second)
		return true;
	return false;
}
vector<pair<u32, int>> Learnset::GetOrderedLearnset(const LearnsetData& learnset)
{
	vector<pair<u32, int>> output;
	for (u32 moveIdx = 0; moveIdx < (u32)learnset.size(); ++moveIdx)
	{
		int moveID = learnset[moveIdx][LEARN_MOVE_ID];
		if (moveID == LEARNSET_SKIP)
			continue;
		int level = learnset[moveIdx][LEARN_LEVEL];
		if (moveID == LEARNSET_NULL ||
			level == LEARNSET_NULL)
			break;

		output.emplace_back(make_pair(moveIdx, level));
	}
	sort(output.begin(), output.end(), LearnsetOrderCmp);

	return output;
}

void Learnset::ComboBox(LearnsetData& learnset, const char* label, const std::vector<std::string>& items, u32 idx)
{
	int selected = learnset[idx][LEARN_MOVE_ID];
	if (selected >= items.size())
		selected = 0;

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		pair<u32, int> value = make_pair(idx, learnset[idx][LEARN_MOVE_ID]);
		pair<u32, int> newValue = make_pair(idx, selected);
		SAVE_CHECK_REVERSE_EVENT(LEARN_MOVE_ID, value, newValue,
			engine->GetCurrentPokemon()->dataIdx, learnset);
		learnset[idx][LEARN_MOVE_ID] = selected;
	}
}

void Learnset::InputInt(LearnsetData& learnset, const char* label, u32 idx, int maxValue)
{
	int data = learnset[idx][LEARN_LEVEL];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(label, &data) && ImGui::IsItemDeactivatedAfterEdit())
	{
		if (data > maxValue)
			data = maxValue;
		if (data < 0)
			data = 0;

		pair<u32, int> value = make_pair(idx, learnset[idx][LEARN_LEVEL]);
		pair<u32, int> newValue = make_pair(idx, data);
		SAVE_CHECK_REVERSE_EVENT(LEARN_LEVEL, value, newValue,
			engine->GetCurrentPokemon()->dataIdx, learnset);
		learnset[idx][LEARN_LEVEL] = data;
	}
}
