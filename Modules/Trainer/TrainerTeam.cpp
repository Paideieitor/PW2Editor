#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Trainer/Trainer.h"
#include "Modules/Trainer/TrainerTeam.h"

#define REMOVE_POKEMON_EVENT TEAM_SLOT(MAX_TEAM_SIZE, TRAINERPOKEMONDATA_MAX)
#define ADD_POKEMON_EVENT TEAM_SLOT(MAX_TEAM_SIZE, TRAINERPOKEMONDATA_MAX + 1)

#define TRAINER_MODULE ((Trainer*)engine->modules[trainerModule])

TrainerTeam::TrainerTeam(Engine* const engine, u32 group, u32 trainerModule) :
	Module(engine, group, TRAINER_TEAM_NARC_PATH), trainerModule(trainerModule)
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
		ImGui::Text("Moves                                        Max PP");
		for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
		{
			ComboBox(team, slot, " ", engine->moveNames, (TrainerPokemonField)(TRAINER_MOVE_1 + moveSlot));
			if (type == PERFECT_TRAINER)
			{
				ImGui::SameLine();
				CheckBox(team, slot, " ", (TrainerPokemonField)(TRAINER_MAX_PP_1 + moveSlot));
			}
		}
		if (type == PERFECT_TRAINER)
		{
			ComboBox(team, slot, "Nature", engine->natures, TRAINER_NATURE);
			InputInt(team, slot, "Happiness", TRAINER_HAPPINESS, 255);
		}
		ImGui::EndDisabled();

		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();

		if (type != PERFECT_TRAINER)
		{
			ComboBox(team, slot, "##Ability", engine->abilitySlots, TRAINER_ABILITY_SLOT);

			int species = team[TEAM_SLOT(slot, TRAINER_SPECIES)];
			int abilitySlot = team[TEAM_SLOT(slot, TRAINER_ABILITY_SLOT)];
			PersonalData& personal = engine->personal[species];
			if (abilitySlot == 0)
				abilitySlot = 1;
			team[TEAM_SLOT(slot, TRAINER_ABILITY)] = personal[ABILITY_1 + (abilitySlot - 1)];

			if (team[TEAM_SLOT(slot, TRAINER_ABILITY_SLOT)] != 0)
			{
				ImGui::SameLine();
				ImGui::Text(engine->abilities[team[TEAM_SLOT(slot, TRAINER_ABILITY)]]);
			}
		}
		else
			ComboBox(team, slot, "Ability", engine->abilities, TRAINER_ABILITY);

		if (type != PERFECT_TRAINER)
		{
			InputInt(team, slot, "Difficulty", TRAINER_DIFFICULTY, 255);

			int ivs = 31 * team[TEAM_SLOT(slot, TRAINER_DIFFICULTY)] / 255;
			for (u32 iv = 0; iv < STAT_COUNT; ++iv)
				team[TEAM_SLOT(slot, TRAINER_HP_IV + iv)] = ivs;

			ImGui::SameLine();
			ImGui::Text(string("-> IVs: ") + to_string(ivs));
		}
		else
		{
			ImGui::Text("    IVs                EVs");
#define STAT_DISPLAY(text, iv, ev) ImGui::Text(text); ImGui::SameLine(); InputInt(team, slot, " ", iv, 31); ImGui::SameLine(); InputInt(team, slot, " ", ev, 255);
			STAT_DISPLAY("HP  ", TRAINER_HP_IV, TRAINER_HP_EV);
			STAT_DISPLAY("ATK", TRAINER_ATK_IV, TRAINER_ATK_EV);
			STAT_DISPLAY("DEF", TRAINER_DEF_IV, TRAINER_DEF_EV);
			STAT_DISPLAY("SPE", TRAINER_SPE_IV, TRAINER_SPE_EV);
			STAT_DISPLAY("SPA", TRAINER_SPA_IV, TRAINER_SPA_EV);
			STAT_DISPLAY("SPD", TRAINER_SPD_IV, TRAINER_SPD_EV);

			ComboBox(team, slot, "Status", status, TRAINER_STATUS);
			InputInt(team, slot, "HP Percent", TRAINER_HP_PERCENT, 100);
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
		
		TRAINER_MODULE->RemovePokemon();
	}
	if (pkmCount < MAX_TEAM_SIZE && ImGui::Button("Add"))
	{
		TrainerTeamData previous = team;
		InsertTrainerPokemon(team, pkmCount);
		SAVE_CHECK_REVERSE_EVENT(ADD_POKEMON_EVENT, previous, team, engine->project->selectedTrainerIdx, previous);
	
		TRAINER_MODULE->AddPokemon();
	}

	ImGui::End();
	return OK;
}

void TrainerTeam::HandleReverseEvent(const Event* reverseEvent)
{
	TrainerTeamData& team = engine->trainerTeams[engine->project->selectedTrainerIdx];
	switch (reverseEvent->subType)
	{
	case REMOVE_POKEMON_EVENT:
	{
		TrainerTeamData* value = (TrainerTeamData*)reverseEvent->value;
		team = *value;

		TrainerData& trainer = engine->trainers[engine->project->selectedTrainerIdx];
		++trainer[POKEMON_COUNT];
		break;
	}
	case ADD_POKEMON_EVENT:
	{
		TrainerTeamData* value = (TrainerTeamData*)reverseEvent->value;
		team = *value;

		TrainerData& trainer = engine->trainers[engine->project->selectedTrainerIdx];
		--trainer[POKEMON_COUNT];
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;
		team[reverseEvent->subType] = *value;

		u32 slot = 0;
		switch (GetFieldFromTeamSlot(reverseEvent->subType, &slot))
		{
		case TRAINER_DIFFICULTY:
		{
			int ivs = 31 * *value / 255;
			for (u32 iv = 0; iv < STAT_COUNT; ++iv)
				team[TEAM_SLOT(slot, TRAINER_HP_IV + iv)] = ivs;
			break;
		}
		case TRAINER_ABILITY_SLOT:
		{
			int species = team[TEAM_SLOT(slot, TRAINER_SPECIES)];
			int abilitySlot = team[TEAM_SLOT(slot, TRAINER_ABILITY_SLOT)];
			PersonalData& personal = engine->personal[species];
			if (abilitySlot == 0)
				abilitySlot = 1;
			team[TEAM_SLOT(slot, TRAINER_ABILITY)] = personal[ABILITY_1 + (abilitySlot - 1)];
			break;
		}
		}
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
