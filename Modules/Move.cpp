#include "Windows/Engine.h"

#include "Modules/Move.h"

#define MOVE_INPUT_INT(field, label, max, min) engine->InputInt(engine->moves, engine->project->move, field, &engine->project->move, engine->project->move, group, label, max, min)
#define MOVE_COMBO_BOX(field, label, _list) engine->ComboBox(engine->moves, engine->project->move, field, &engine->project->move, engine->project->move, group, label, _list)
#define MOVE_CHECK_BOX(field, label) engine->CheckBox(engine->moves, engine->project->move, field, &engine->project->move, engine->project->move, group, label)


Move::Move(Engine* const engine, u32 group) : Module(engine, group),
types(engine->GetTextFile(TYPE_NAME_FILE_ID))
{
}

ReturnState Move::RenderGUI()
{
	ImGui::Begin("Move");
	ImGui::Text("Index: " + to_string(engine->project->move));
	ImGui::Separator();

	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			ImGui::Text("Data");

			MOVE_COMBO_BOX(MOVE_TYPE, "Type", types);

			MOVE_COMBO_BOX(QUALITY, "Quality", &moveQuality);
			MOVE_COMBO_BOX(CATEGORY, "Category", &moveCategory);
			MOVE_INPUT_INT(POWER, "Power", 255, 0);
			MOVE_INPUT_INT(ACCURACY, "Accuracy", 101, 0);
			MOVE_INPUT_INT(BASE_PP, "Base PP", 255, 0);
			MOVE_INPUT_INT(PRIORITY, "Priority", 7, -7);
			MOVE_INPUT_INT(CRIT_STAGE, "Crit Stage", 6, 0);
			MOVE_INPUT_INT(FLINCH_RATE, "Flinch Rate", 100, 0);

			MOVE_INPUT_INT(RECOIL, "Recoil/Drain", 100, -100);
			MOVE_INPUT_INT(HEAL, "Heal", 100, -100);
			MOVE_COMBO_BOX(TARGET, "Target", &moveTarget);
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			ImGui::Text("Multi Hits");

			MOVE_INPUT_INT(HIT_MIN, "Minimum Hits", 255, 0);
			MOVE_INPUT_INT(HIT_MAX, "Maximum Hits", 255, 0);

			ImGui::Text("Apply Condition");

			MOVE_COMBO_BOX(CONDITION, "", &conditions);
			MOVE_INPUT_INT(CONDITION_CHANCE, "Chance", 100, 0);
			MOVE_INPUT_INT(CONDITION_DURATION, "Duration", 255, 0);

			MOVE_INPUT_INT(TURN_MIN, "Minimum Turns", 255, 0);
			MOVE_INPUT_INT(TURN_MIN, "Maximum Turns", 255, 0);

			ImGui::Text("Behavior");
			MOVE_INPUT_INT(BEHAVIOR, "", 0xFFFF, 0);
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();
		{
			ImGui::Text("On-Hit Stat Changes");
			for (u32 statChange = 0; statChange < 3; ++statChange)
			{
				ImGui::BeginGroup();
				{
					MOVE_COMBO_BOX(STAT_CHANGE_1 + statChange, "", &stats);
					MOVE_INPUT_INT(STAT_CHANGE_VOLUME_1 + statChange, "Volume", 6, -6);
					MOVE_INPUT_INT(STAT_CHANGE_CHANCE_1 + statChange, "Chance", 100, 0);
				}
				ImGui::EndGroup();
				ImGui::SameLine();
			}
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	{
		ImGui::Text("Flags");
		ImGui::BeginGroup();
		{
			MOVE_CHECK_BOX(IS_CONTACT, "Contact");
			MOVE_CHECK_BOX(IS_REQUIRES_CHARGE, "Charge");
			MOVE_CHECK_BOX(IS_RECHARGE, "Recharge");
			MOVE_CHECK_BOX(IS_BLOCKED_BY_PROTECT, "Blocked by Protect");
			MOVE_CHECK_BOX(IS_REFLECTED_BY_MAGIC_COAT, "Reflected by Magic Coat");
			MOVE_CHECK_BOX(IS_STOLEN_BY_SNATCH, "Stolen by Snatch");
			MOVE_CHECK_BOX(IS_COPIED_BY_MIRROR_MOVE, "Copied by Mirror Move");
			MOVE_CHECK_BOX(IS_PUNCH, "Punch");
			MOVE_CHECK_BOX(IS_SOUND, "Sound");
			MOVE_CHECK_BOX(IS_GROUNDED_BY_GRAVITY, "Grounded by Gravity");
			MOVE_CHECK_BOX(IS_DEFROST, "Defrost");
			MOVE_CHECK_BOX(IS_LONG_RANGE, "Long Range");
			MOVE_CHECK_BOX(IS_BLOCKED_BY_HEAL_BLOCK, "Blocked by Heal Block");
			MOVE_CHECK_BOX(IS_BYPASSES_SUBSTITUTE, "Bypasses Substitute");
			MOVE_CHECK_BOX(IS_UNKNOWN_14, "Powder");
			MOVE_CHECK_BOX(IS_UNKNOWN_15, "Bullet");
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			MOVE_CHECK_BOX(IS_UNKNOWN_16, "Bite");
			MOVE_CHECK_BOX(IS_UNKNOWN_17, "Pulse");
			MOVE_CHECK_BOX(IS_UNKNOWN_18, "Healing");
			MOVE_CHECK_BOX(IS_UNKNOWN_19, "Dance");
			MOVE_CHECK_BOX(IS_UNKNOWN_20, "Wind");
			MOVE_CHECK_BOX(IS_UNKNOWN_21, "Sharp");
			MOVE_CHECK_BOX(IS_UNKNOWN_22, "Protect");
			MOVE_CHECK_BOX(IS_UNKNOWN_23, "Unused 23");
			MOVE_CHECK_BOX(IS_UNKNOWN_24, "Unused 24");
			MOVE_CHECK_BOX(IS_UNKNOWN_25, "Unused 25");
			MOVE_CHECK_BOX(IS_UNKNOWN_26, "Unused 26");
			MOVE_CHECK_BOX(IS_UNKNOWN_27, "Unused 27");
			MOVE_CHECK_BOX(IS_UNKNOWN_28, "Unused 28");
			MOVE_CHECK_BOX(IS_UNKNOWN_29, "Unused 29");
			MOVE_CHECK_BOX(IS_UNKNOWN_30, "Unused 30");
			MOVE_CHECK_BOX(IS_UNKNOWN_31, "Unused 31");
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::End();
	return OK;
}
