#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Trainer/TrainerTeam.h"

#define REMOVE_POKEMON_EVENT TEAM_SLOT(MAX_TEAM_SIZE, TRAINERPOKEMONDATA_MAX)
#define ADD_POKEMON_EVENT TEAM_SLOT(MAX_TEAM_SIZE, TRAINERPOKEMONDATA_MAX + 1)

TrainerTeam::TrainerTeam(Engine* const engine, u32 group) : Module(engine, group, TRAINER_TEAM_NARC_PATH)
{
}

TrainerTeam::~TrainerTeam()
{
}

ReturnState TrainerTeam::RenderGUI()
{
	ImGui::Begin("Trainer Team");
	TrainerTeamData& team = engine->trainerTeams[engine->project->selectedTrainerIdx];
	TrainerData& trainer = engine->trainers[engine->project->selectedTrainerIdx];

	TrainerType type = (TrainerType)trainer[TRAINER_TYPE];
	u32 pkmCount = trainer[POKEMON_COUNT];
	int slotToRemove = -1;
	for (u32 slot = 0; slot < pkmCount; ++slot)
	{
		ImGui::BeginGroup();

		ImGui::Separator();
		ImGui::Text(to_string(slot + 1));
		ImGui::Separator();
		ComboBox(team, slot, "Species", engine->pkmNames, TRAINER_SPECIES);
		ImGui::SameLine();
		InputInt(team, slot, "Form", TRAINER_FORM, 31);
		ImGui::Separator();

		ImGui::BeginGroup();

		InputInt(team, slot, "Level", TRAINER_LEVEL, 100);

		ComboBox(team, slot, "Sex", engine->pokemonSex, TRAINER_SEX);

		ImGui::BeginDisabled(type == SIMPLE_TRAINER || type == MOVE_TRAINER);
		ComboBox(team, slot, "Held Item", engine->itemNames, TRAINER_HELD_ITEM);
		ImGui::EndDisabled();

		ImGui::BeginDisabled(type == SIMPLE_TRAINER || type == ITEM_TRAINER);
		ImGui::Text("Moves");
		ComboBox(team, slot, "##Move1", engine->moveNames, TRAINER_MOVE_1);
		ComboBox(team, slot, "##Move2", engine->moveNames, TRAINER_MOVE_2);
		ComboBox(team, slot, "##Move3", engine->moveNames, TRAINER_MOVE_3);
		ComboBox(team, slot, "##Move4", engine->moveNames, TRAINER_MOVE_4);
		ImGui::EndDisabled();

		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();

		if (type != PERFECT_TRAINER)
		{
			ComboBox(team, slot, "##Ability", engine->abilitySlots, TRAINER_ABILITY);

			int species = team[TEAM_SLOT(slot, TRAINER_SPECIES)];
			int abilitySlot = team[TEAM_SLOT(slot, TRAINER_ABILITY)];
			PersonalData& personal = engine->personal[species];
			if (abilitySlot != 0)
			{
				int ability = personal[ABILITY_1 + (abilitySlot - 1)];
				ImGui::SameLine();
				ImGui::Text(engine->abilities[ability]);
			}
		}
		else
			ComboBox(team, slot, "Ability", engine->abilities, TRAINER_ABILITY);

		if (type != PERFECT_TRAINER)
		{
			InputInt(team, slot, "Difficulty", TRAINER_DIFFICULTY, 255);

			int difficulty = team[TEAM_SLOT(slot, TRAINER_DIFFICULTY)];
			ImGui::SameLine();
			ImGui::Text(string("-> IVs: ") + to_string(31 * difficulty / 255));
		}
		else
		{

		}

		ImGui::EndGroup();

		if (pkmCount > 1 && ImGui::Button(LABEL("Remove", slot)))
			slotToRemove = slot;

		ImGui::EndGroup();

		if (slot + 1 < pkmCount)
		{
			if (slot % 2 == 0)
			{
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(200.0f, 0.0f));
				ImGui::SameLine();
			}
			else
			{
				ImGui::NewLine();
				ImGui::NewLine();
			}
		}
	}
	if (slotToRemove >= 0)
	{
		TrainerTeamData previous = team;
		RemoveTrainerPokemon(team, (u32)slotToRemove);
		SAVE_CHECK_REVERSE_EVENT(REMOVE_POKEMON_EVENT, previous, team, engine->project->selectedTrainerIdx, previous);
		
		--trainer[POKEMON_COUNT];
	}
	if (pkmCount < MAX_TEAM_SIZE && ImGui::Button("Add"))
	{
		TrainerTeamData previous = team;
		InsertTrainerPokemon(team, pkmCount);
		SAVE_CHECK_REVERSE_EVENT(ADD_POKEMON_EVENT, previous, team, engine->project->selectedTrainerIdx, previous);
	
		++trainer[POKEMON_COUNT];
	}

	ImGui::End();
	return OK;
}

void TrainerTeam::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case REMOVE_POKEMON_EVENT:
	{
		TrainerTeamData* value = (TrainerTeamData*)reverseEvent->value;
		engine->trainerTeams[engine->project->selectedTrainerIdx] = *value;

		TrainerData& trainer = engine->trainers[engine->project->selectedTrainerIdx];
		++trainer[POKEMON_COUNT];
		break;
	}
	case ADD_POKEMON_EVENT:
	{
		TrainerTeamData* value = (TrainerTeamData*)reverseEvent->value;
		engine->trainerTeams[engine->project->selectedTrainerIdx] = *value;

		TrainerData& trainer = engine->trainers[engine->project->selectedTrainerIdx];
		--trainer[POKEMON_COUNT];
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;
		engine->trainerTeams[engine->project->selectedTrainerIdx][reverseEvent->subType] = *value;
		break;
	}
	}
	
}

void TrainerTeam::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	TrainerTeamData* originalData = (TrainerTeamData*)saveEvent->value;
	const TrainerTeamData& data = engine->trainerTeams[saveEvent->subType];
	if (*originalData == data)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveTrainerTeam(data, engine->trainers[saveEvent->subType], filePath);
}

void TrainerTeam::ComboBox(TrainerTeamData& team, u32 slot, const char* label, const std::vector<std::string>& items, TrainerPokemonField field)
{
	u32 realField = TEAM_SLOT(slot, field);

	int selected = team[realField];
	if (selected >= items.size())
	{
		InputInt(team, slot, label, field, 0xFFFF);
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(LABEL(label, (int)realField), items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(realField, team[realField], selected,
			engine->project->selectedTrainerIdx, team);
		team[realField] = selected;
	}
}

void TrainerTeam::InputInt(TrainerTeamData& team, u32 slot, const char* label, TrainerPokemonField field, int maxValue)
{
	u32 realField = TEAM_SLOT(slot, field);

	int value = team[realField];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)realField), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SAVE_CHECK_REVERSE_EVENT(realField, team[realField], value,
			engine->project->selectedTrainerIdx, team);
		team[realField] = value;
	}

}

void TrainerTeam::CheckBox(TrainerTeamData& team, u32 slot, const char* label, TrainerPokemonField field)
{
	u32 realField = TEAM_SLOT(slot, field);

	bool value = (bool)team[realField];
	if (ImGui::Checkbox(LABEL(label, (int)realField), &value))
	{
		SAVE_CHECK_REVERSE_EVENT(realField, team[realField], (int)value,
			engine->project->selectedTrainerIdx, team);
		team[realField] = (int)value;
	}
}
