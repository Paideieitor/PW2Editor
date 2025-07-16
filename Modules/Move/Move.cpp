#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Move/Move.h"

Move::Move(Engine* const engine, u32 group) : Module(engine, group, MOVE_NARC_PATH)
{
}

Move::~Move()
{
}

ReturnState Move::RenderGUI()
{
	ImGui::Begin("Move");
	MoveData& moveData = engine->moves[engine->project->selectedMoveIdx];

	ImGui::BeginGroup();
	ImGui::BeginGroup();

	ImGui::Text("Move Data");

	ComboBox(moveData, "Type", engine->types, MOVE_TYPE);

	ComboBox(moveData, "Quality", engine->moveQuality, QUALITY);
	ComboBox(moveData, "Category", engine->moveCategory, CATEGORY);
	InputInt(moveData, "Power", POWER, 255);
	InputInt(moveData, "Accuracy", ACCURACY, 101);
	InputInt(moveData, "Base PP", BASE_PP, 255);
	InputInt(moveData, "Priority", PRIORITY, 7, true);
	InputInt(moveData, "Crit Stage", CRIT_STAGE, 6);
	InputInt(moveData, "Flinch Rate", FLINCH_RATE, 100);

	InputInt(moveData, "Recoil/Drain", RECOIL, 100, true);
	InputInt(moveData, "Heal", HEAL, 255);
	ComboBox(moveData, "Target", engine->moveTarget, TARGET);

	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	ImGui::Text("Multi Hits");

	InputInt(moveData, "Minimum Hits", HIT_MIN, 255);
	InputInt(moveData, "Maximum Hits", HIT_MAX, 255);

	ImGui::Text("Apply Condition");

	ComboBox(moveData, "##Condition", engine->conditions, CONDITION);
	InputInt(moveData, "Condition Chance", CONDITION_CHANCE, 100);
	InputInt(moveData, "Condition Duration", CONDITION_DURATION, 255);

	InputInt(moveData, "Minimum Turns", TURN_MIN, 255);
	InputInt(moveData, "Maximum Turns", TURN_MAX, 255);

	ImGui::Text("Behavior");

	InputInt(moveData, "##Animation ID", BEHAVIOR, 65535);

	ImGui::EndGroup();

	ImGui::BeginGroup();

	for (u32 statChange = 0; statChange < 3; ++statChange)
	{
		ImGui::BeginGroup();
		ImGui::Text((string("Stat Change ") + std::to_string(statChange)));
		ComboBox(moveData, (string("##") + std::to_string(statChange)).c_str(), engine->stats, (MoveField)(STAT_CHANGE_1 + statChange));
		InputInt(moveData, (string("Volume") + "##" + std::to_string(statChange)).c_str(), (MoveField)(STAT_CHANGE_VOLUME_1 + statChange), 6, true);
		InputInt(moveData, (string("Chance") + "##" + std::to_string(statChange)).c_str(), (MoveField)(STAT_CHANGE_CHANCE_1 + statChange), 100);
		ImGui::EndGroup();
		ImGui::SameLine();
	}

	ImGui::EndGroup();
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Flags");

	ImGui::BeginGroup();
	CheckBox(moveData, "Contact", IS_CONTACT);
	CheckBox(moveData, "Charge", IS_REQUIRES_CHARGE);
	CheckBox(moveData, "Recharge", IS_RECHARGE);
	CheckBox(moveData, "Blocked by Protect", IS_BLOCKED_BY_PROTECT);
	CheckBox(moveData, "Reflected by Magic Coat", IS_REFLECTED_BY_MAGIC_COAT);
	CheckBox(moveData, "Stolen by Snatch", IS_STOLEN_BY_SNATCH);
	CheckBox(moveData, "Copied by Mirror Move", IS_COPIED_BY_MIRROR_MOVE);
	CheckBox(moveData, "Punch", IS_PUNCH);
	CheckBox(moveData, "Sound", IS_SOUND);
	CheckBox(moveData, "Grounded by Gravity", IS_GROUNDED_BY_GRAVITY);
	CheckBox(moveData, "Defrost", IS_DEFROST);
	CheckBox(moveData, "Long Range", IS_LONG_RANGE);
	CheckBox(moveData, "Blocked by Heal Block", IS_BLOCKED_BY_HEAL_BLOCK);
	CheckBox(moveData, "Bypasses Substitute", IS_BYPASSES_SUBSTITUTE);
	CheckBox(moveData, "Powder", IS_UNKNOWN_14);
	CheckBox(moveData, "Bullet", IS_UNKNOWN_15);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	CheckBox(moveData, "Bite", IS_UNKNOWN_16);
	CheckBox(moveData, "Pulse", IS_UNKNOWN_17);
	CheckBox(moveData, "Healing", IS_UNKNOWN_18);
	CheckBox(moveData, "Dance", IS_UNKNOWN_19);
	CheckBox(moveData, "Wind", IS_UNKNOWN_20);
	CheckBox(moveData, "Sharp", IS_UNKNOWN_21);
	CheckBox(moveData, "Unused 22", IS_UNKNOWN_22);
	CheckBox(moveData, "Unused 23", IS_UNKNOWN_23);
	CheckBox(moveData, "Unused 24", IS_UNKNOWN_24);
	CheckBox(moveData, "Unused 25", IS_UNKNOWN_25);
	CheckBox(moveData, "Unused 26", IS_UNKNOWN_26);
	CheckBox(moveData, "Unused 27", IS_UNKNOWN_27);
	CheckBox(moveData, "Unused 28", IS_UNKNOWN_28);
	CheckBox(moveData, "Unused 29", IS_UNKNOWN_29);
	CheckBox(moveData, "Unused 30", IS_UNKNOWN_30);
	CheckBox(moveData, "Unused 31", IS_UNKNOWN_31);

	ImGui::EndGroup();

	ImGui::EndGroup();

	ImGui::End();
	return OK;
}

void Move::HandleReverseEvent(const Event* reverseEvent)
{
	int* value = (int*)reverseEvent->value;
	engine->moves[engine->project->selectedMoveIdx][reverseEvent->subType] = *value;
}

void Move::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	MoveData* originalData = (MoveData*)saveEvent->value;
	const MoveData& data = engine->moves[saveEvent->subType];
	if (*originalData == data)
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveMove(data, filePath);
}

void Move::HandleGroupEvent(Event* groupEvent)
{
	engine->AddSaveEvent(moduleIdx, (u32)engine->moves.size() - 1, (u32)nullptr);
}

void Move::ComboBox(MoveData& moveData, const char* label, const std::vector<std::string>& items, MoveField field)
{
	int selected = moveData[field];
	if (selected >= items.size())
	{
		InputInt(moveData, label, field, 0xFFFF);
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(field, moveData[field], selected,
			engine->project->selectedMoveIdx, moveData);
		moveData[field] = selected;
	}
}

void Move::InputInt(MoveData& moveData, const char* label, MoveField field, int maxValue, bool allowNegative)
{
	int value = moveData[field];
	ImGui::SetNextItemWidth(80.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (!allowNegative)
		{
			if (value < 0)
				value = 0;
		}
		else
		{
			if (value < -maxValue)
				value = -maxValue;
		}

		SAVE_CHECK_REVERSE_EVENT(field, moveData[field], value,
			engine->project->selectedMoveIdx, moveData);
		moveData[field] = value;
	}
}

void Move::CheckBox(MoveData& moveData, const char* label, MoveField field)
{
	bool value = (bool)moveData[field];
	if (ImGui::Checkbox(label, &value))
	{
		SAVE_CHECK_REVERSE_EVENT(field, moveData[field], (int)value,
			engine->project->selectedMoveIdx, moveData);
		moveData[field] = (int)value;
	}
}
