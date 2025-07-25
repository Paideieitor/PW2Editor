#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Trainer/Trainer.h"

Trainer::Trainer(Engine* const engine, u32 group) : Module(engine, group, TRAINER_NARC_PATH)
{
}

Trainer::~Trainer()
{
}

ReturnState Trainer::RenderGUI()
{
	ImGui::Begin("Trainer");
	TrainerData& trainerData = engine->trainers[engine->project->selectedTrainerIdx];

	ImGui::BeginGroup();
	ComboBox(trainerData, "Type", engine->trainerTypes, TRAINER_TYPE);
	ComboBox(trainerData, "Class", engine->trainerClasses, TRAINER_CLASS);
	ComboBox(trainerData, "Battle Type", engine->battleTypes, BATTLE_TYPE);

	InputInt(trainerData, u8"Pokémon Count", POKEMON_COUNT, 6);

	ComboBox(trainerData, "Item 1", engine->itemNames, ITEM_1);
	ComboBox(trainerData, "Item 2", engine->itemNames, ITEM_2);
	ComboBox(trainerData, "Item 3", engine->itemNames, ITEM_3);
	ComboBox(trainerData, "Item 4", engine->itemNames, ITEM_4);

	InputInt(trainerData, "Money", CASH, 255);

	ComboBox(trainerData, "Post-Battle Item", engine->itemNames, POST_BATTLE_ITEM);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("AI Flags");
	for (u32 flag = 0; flag < engine->trainerAIFlags.size(); ++flag)
		CheckBox(trainerData, engine->trainerAIFlags[flag].c_str(), (TrainerField)(flag + AI_NO_EFFECT_CHECK));
	ImGui::EndGroup();

	ImGui::Text(to_string(engine->project->selectedTrainerIdx));

	ImGui::End();
	return OK;
}

void Trainer::HandleReverseEvent(const Event* reverseEvent)
{
	int* value = (int*)reverseEvent->value;
	engine->trainers[engine->project->selectedTrainerIdx][reverseEvent->subType] = *value;
}

void Trainer::HandleSaveEvent(Event* saveEvent)
{
	TrainerData* originalData = (TrainerData*)saveEvent->value;
	const TrainerData& data = engine->trainers[saveEvent->subType];
	if (*originalData == data)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveTrainer(data, filePath);
}

void Trainer::ComboBox(TrainerData& trainerData, const char* label, const std::vector<std::string>& items, TrainerField field)
{
	int selected = trainerData[field];
	if (selected >= items.size())
	{
		InputInt(trainerData, label, field, 0xFFFF);
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(field, trainerData[field], selected,
			engine->project->selectedItemIdx, trainerData);
		trainerData[field] = selected;
	}
}

void Trainer::InputInt(TrainerData& trainerData, const char* label, TrainerField field, int maxValue)
{
	int value = trainerData[field];
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

		SAVE_CHECK_REVERSE_EVENT(field, trainerData[field], value,
			engine->project->selectedItemIdx, trainerData);
		trainerData[field] = value;
	}
}

void Trainer::CheckBox(TrainerData& trainerData, const char* label, TrainerField field)
{
	bool value = (bool)trainerData[field];
	if (ImGui::Checkbox(label, &value))
	{
		SAVE_CHECK_REVERSE_EVENT(field, trainerData[field], (int)value,
			engine->project->selectedItemIdx, trainerData);
		trainerData[field] = (int)value;
	}
}
