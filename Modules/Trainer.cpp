#include "Windows/Engine.h"

#include "Modules/Trainer.h"

#define TRAINER_INPUT_INT(field, label, max, min) engine->InputInt(engine->trainers, engine->project->trainer, field, &engine->project->trainer, engine->project->trainer, group, label, max, min)
#define TRAINER_COMBO_BOX(field, label, _list) engine->ComboBox(engine->trainers, engine->project->trainer, field, &engine->project->trainer, engine->project->trainer, group, label, _list)
#define TRAINER_CHECK_BOX(field, label) engine->CheckBox(engine->trainers, engine->project->trainer, field, &engine->project->trainer, engine->project->trainer, group, label)

#define TRAINER_GET_VALUE(field) engine->GetDataValue(engine->trainers, engine->project->trainer, field)
#define TRAINER_SET_VALUE(field, value) engine->SetDataValue(engine->trainers, engine->project->trainer, field, value)
#define TRAINER_START_ACTION engine->StartAction(&engine->project->item, engine->project->trainer, group)
#define TRAINER_END_ACTION engine->EndAction()

enum BattleTypes {
	SINGLES,
	DOUBLES,
	TRIPLES,
	ROTATION,
};

Trainer::Trainer(Engine* const engine, u32 group) : Module(engine, group),
trainerClasses(engine->GetTextFile(TRAINER_CLASS_FILE_ID)), pokemons(engine->GetTextFile(PKM_NAME_FILE_ID)), 
natures(engine->GetTextFile(NATURE_FILE_ID)), abilities(engine->GetTextFile(ABILITY_NAME_FILE_ID)), 
items(engine->GetTextFile(ITEM_NAME_FILE_ID)), moves(engine->GetTextFile(MOVE_NAME_FILE_ID)), 
types(engine->GetTextFile(TYPE_NAME_FILE_ID))
{
	if (engine->PatchIsInstalled())
		trainerTypes.emplace_back("Perfect");
}

ReturnState Trainer::RenderGUI()
{
	ImGui::Begin("Trainer");
	ImGui::Text("Index: " + to_string(engine->project->trainer));
	ImGui::Separator();

	int teamCount = TeamCount();

	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			TRAINER_COMBO_BOX(TRAINER_FIELD(TRAINER_TYPE), "Type", &trainerTypes);
			TRAINER_COMBO_BOX(TRAINER_FIELD(TRAINER_CLASS), "Class", trainerClasses);
			TRAINER_COMBO_BOX(TRAINER_FIELD(BATTLE_TYPE), "Battle Type", &battleTypes);

			int pokemonCount = TRAINER_GET_VALUE(TRAINER_FIELD(POKEMON_COUNT));
			if (pokemonCount == teamCount)
				ImGui::Text(string(u8"Pokémon Count: ") + to_string(pokemonCount));
			else if (ImGui::Button("Fix Missmatch Count"))
			{
				TRAINER_START_ACTION;
				TRAINER_SET_VALUE(TRAINER_FIELD(POKEMON_COUNT), teamCount);
				TRAINER_END_ACTION;
			}

			ImGui::Text("Battle Items");
			TRAINER_COMBO_BOX(TRAINER_FIELD(ITEM_1), "", items);
			TRAINER_COMBO_BOX(TRAINER_FIELD(ITEM_2), "", items);
			TRAINER_COMBO_BOX(TRAINER_FIELD(ITEM_3), "", items);
			TRAINER_COMBO_BOX(TRAINER_FIELD(ITEM_4), "", items);

			ImGui::Text("Reward");
			TRAINER_INPUT_INT(TRAINER_FIELD(CASH), "Money", 0xFF, 0);
			TRAINER_COMBO_BOX(TRAINER_FIELD(POST_BATTLE_ITEM), "Item", items);
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();
		{
			ImGui::Text("AI Flags");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_NO_EFFECT_CHECK), "No effect check");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_EVALUATE_MOVES), "Evaluate moves");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_EXPERT), "Expert");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_SET_UP), "Set up");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_FIRST_RIVAL), "First rival");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_N_S_FINAL_BATTLE), "N's final Battle");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_BATON_PASS), "Baton Pass");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_DOUBLE_TRIPLE_BATTLE), "Double/Triple Battle");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_HP_BASED), "HP-Based");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_WEATHER), "Weather");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_DISRUPTION), "Disruption");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_ROAMING), "Roaming");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_SAFARI), "Safari");
			TRAINER_CHECK_BOX(TRAINER_FIELD(AI_TUTORIAL), "Tutorial");
			if (ImGui::Button("Best AI"))
			{
				TRAINER_START_ACTION;

				for (u32 flag = 0; flag < AI_UNUSED_31 - AI_NO_EFFECT_CHECK; ++flag)
					TRAINER_SET_VALUE(TRAINER_FIELD(AI_NO_EFFECT_CHECK + flag), 0);

				TRAINER_SET_VALUE(TRAINER_FIELD(AI_NO_EFFECT_CHECK), 1);
				TRAINER_SET_VALUE(TRAINER_FIELD(AI_EVALUATE_MOVES), 1);
				TRAINER_SET_VALUE(TRAINER_FIELD(AI_EXPERT), 1);

				int battleType = TRAINER_GET_VALUE(TRAINER_FIELD(BATTLE_TYPE));
				if (battleType == DOUBLES ||
					battleType == TRIPLES)
					TRAINER_SET_VALUE(TRAINER_FIELD(AI_DOUBLE_TRIPLE_BATTLE), 1);

				TRAINER_END_ACTION;
			}
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			ImGui::Text(u8"Pokémon");
			ImGui::SameLine();
			ImGui::BeginDisabled(teamCount >= TEAM_COUNT);
			if (ImGui::Button("Add"))
			{
				TRAINER_START_ACTION;

				if (Insert(teamCount))
				{
					++teamCount;
					TRAINER_SET_VALUE(TRAINER_FIELD(POKEMON_COUNT), teamCount);
				}

				TRAINER_END_ACTION;
			}
			ImGui::EndDisabled();
		}
		ImGui::EndGroup();

		ImGui::Dummy(ImVec2(30.0f, 0.0f));
		ImGui::SameLine();

		u32 actionSlot1 = TEAM_COUNT;
		u32 actionSlot2 = TEAM_COUNT;

		int trainerType = TRAINER_GET_VALUE(TRAINER_FIELD(TRAINER_TYPE));
		ImGui::BeginGroup();
		{
			ImGui::NewLine();

			for (u32 slot = 0; slot < TEAM_COUNT; ++slot)
			{
				ImGui::BeginGroup();
				{
					if (!TeamMember(trainerType, slot))
					{
						ImGui::EndGroup();
						break;
					}
				}

				ImGui::BeginDisabled(slot < 1 || IsNull(slot) || IsNull(slot - 1));
				if (ImGui::Button(LABEL("<", slot)))
				{
					actionSlot1 = slot;
					actionSlot2 = slot - 1;
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(slot >= TEAM_COUNT - 1 || IsNull(slot) || IsNull(slot + 1));
				if (ImGui::Button(LABEL(">", slot)))
				{
					actionSlot1 = slot;
					actionSlot2 = slot + 1;
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(teamCount <= 1);
				if (ImGui::Button(LABEL("Remove", slot)))
					actionSlot1 = slot;
				ImGui::EndDisabled();

				ImGui::EndGroup();

				if (slot + 1 < TEAM_COUNT)
				{
					if (slot != 2)
					{
						ImGui::SameLine();
						ImGui::Dummy(ImVec2(80.0f, 0.0f));
						ImGui::SameLine();
					}
					else
					{
						ImGui::NewLine();
						ImGui::NewLine();
					}
				}
			}
		}
		ImGui::EndGroup();

		if (actionSlot2 < TEAM_COUNT &&
			!IsNull(actionSlot1) &&
			!IsNull(actionSlot2))
		{
			TRAINER_START_ACTION;

			Swap(actionSlot1, actionSlot2);

			TRAINER_END_ACTION;
		}
		else if (actionSlot1 < TEAM_COUNT)
		{
			TRAINER_START_ACTION;

			if (Remove(actionSlot1))
			{
				--teamCount;
				TRAINER_SET_VALUE(TRAINER_FIELD(POKEMON_COUNT), teamCount);
			}

			TRAINER_END_ACTION;
		}
	}
	ImGui::EndGroup();

	ImGui::End();
	return OK;
}

bool Trainer::TeamMember(int trainerType, u32 slot)
{
	if (IsNull(slot))
		return false;

	ImGui::Text(to_string(slot + 1));
	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_SPECIES), "Species", pokemons);

			TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_FORM), "Form", 31, 0);
			if (trainerType == PERFECT_TRAINER)
			{
				ImGui::SameLine();
				TRAINER_CHECK_BOX(TEAM_FIELD(slot, TRAINER_SHINY), "Shiny");
			}

			TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_LEVEL), "Level", 100, 0);

			TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_SEX), "Sex", &pokemonSex);
		}
		ImGui::EndGroup();

		ImGui::BeginDisabled(trainerType == SIMPLE_TRAINER || trainerType == MOVE_TRAINER);
		{
			TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_HELD_ITEM), "Held Item", items);
		}
		ImGui::EndDisabled();

		ImGui::BeginDisabled(trainerType == SIMPLE_TRAINER || trainerType == ITEM_TRAINER);
		{
			ImGui::BeginGroup();
			{
				ImGui::BeginGroup();
				{
					ImGui::Text("Moves");
					for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
						TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_MOVE_1) + moveSlot, "", moves);
				}
				ImGui::EndGroup();
				if (trainerType == PERFECT_TRAINER)
				{
					ImGui::SameLine();
					ImGui::BeginGroup();
					{
						ImGui::Text("Max PP");
						for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
							TRAINER_CHECK_BOX(TEAM_FIELD(slot, TRAINER_MAX_PP_1) + moveSlot, "");
						if (ImGui::Button(LABEL("All", slot)))
						{
							TRAINER_START_ACTION;
							for (u32 moveSlot = 0; moveSlot < 4; ++moveSlot)
								TRAINER_SET_VALUE(TEAM_FIELD(slot, TRAINER_MAX_PP_1) + moveSlot, 1);
							TRAINER_END_ACTION;
						}
					}
					ImGui::EndGroup();
				}
			}
			ImGui::EndGroup();

			if (trainerType == PERFECT_TRAINER)
			{
				TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_NATURE), "Nature", natures);
				TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_HAPPINESS), "Happiness", 255, 0);
			}
		}
		ImGui::EndDisabled();
	}
	ImGui::EndGroup();

	ImGui::SameLine();


	ImGui::BeginGroup();
	{
		if (trainerType != PERFECT_TRAINER)
			TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_ABILITY_SLOT), "", &abilitySlots);
		else
			TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_ABILITY), "Ability", abilities);

		ImGui::BeginGroup();
		{
			if (trainerType != PERFECT_TRAINER)
			{
				int ivs = 31 * TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_DIFFICULTY)) / 255;

				TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_DIFFICULTY), "Difficulty", 255, 0);
				ImGui::SameLine();
				ImGui::Text(string("IVs: ") + to_string(ivs));
			}
			else
			{

				ImGui::BeginGroup();
				{
					ImGui::Text("IVs");
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_HP_IV), "", 31, 0);
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_ATK_IV), "", 31, 0);
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_DEF_IV), "", 31, 0);
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPA_IV), "", 31, 0);
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPD_IV), "", 31, 0);
					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPE_IV), "", 31, 0);
				}
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();
				{
					ImGui::Text("EVs");

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_HP_EV), "HP  ", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_HP, NATURE_STAT_HP));

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_ATK_EV), "ATK", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_ATK, NATURE_STAT_ATK));

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_DEF_EV), "DEF", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_DEF, NATURE_STAT_DEF));

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPA_EV), "SPA", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_SPA, NATURE_STAT_SPA));

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPD_EV), "SPD", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_SPD, NATURE_STAT_SPD));

					TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_SPE_EV), "SPE", 252, 0);
					ImGui::SameLine();
					ImGui::Text(GetStat(slot, BASE_SPE, NATURE_STAT_SPE));
				}
				ImGui::EndGroup();

				if (ImGui::Button(LABEL("Perfect IVs", slot)))
				{
					TRAINER_START_ACTION;
					for (u32 stat = 0; stat < STAT_COUNT; ++stat)
						TRAINER_SET_VALUE(TEAM_FIELD(slot, TRAINER_HP_IV) + stat, 31);
					TRAINER_END_ACTION;
				}
				ImGui::Text("Hidden Power: " + types->at(GetHiddenPower(slot)));
				TRAINER_COMBO_BOX(TEAM_FIELD(slot, TRAINER_STATUS), "Status", &status);
				TRAINER_INPUT_INT(TEAM_FIELD(slot, TRAINER_HP_PERCENT), "HP Percent", 100, 0);
			}
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	return true;
}

int Trainer::TeamCount()
{
	u32 count = 0;
	for (; count < TEAM_COUNT; ++count)
		if (TRAINER_GET_VALUE(TEAM_FIELD(count, TRAINER_SPECIES)) == TRAINER_NULL)
			break;
	return count;
}

void Trainer::MakeNull(u32 slot)
{
	u32 start = TEAM_FIELD(slot, TRAINER_DIFFICULTY);
	for (u32 idx = 0; idx < TEAM_MAX; ++idx)
		TRAINER_SET_VALUE(start + idx, TRAINER_NULL);
}

bool Trainer::IsNull(u32 slot)
{
	return TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPECIES)) == TRAINER_NULL;
}

void Trainer::MakeDefault(u32 slot)
{
	u32 start = TEAM_FIELD(slot, TRAINER_DIFFICULTY);
	for (u32 idx = 0; idx < TEAM_MAX; ++idx)
		TRAINER_SET_VALUE(start + idx, 0);
}

bool Trainer::Swap(u32 slot1, u32 slot2)
{
	if (slot1 >= TEAM_COUNT || slot2 >= TEAM_COUNT)
		return false;

	u32 start1 = TEAM_FIELD(slot1, TRAINER_DIFFICULTY);
	u32 start2 = TEAM_FIELD(slot2, TRAINER_DIFFICULTY);
	for (u32 idx = 0; idx < TEAM_MAX; ++idx)
	{
		int temp = TRAINER_GET_VALUE(start1 + idx);
		TRAINER_SET_VALUE(start1 + idx, TRAINER_GET_VALUE(start2 + idx));
		TRAINER_SET_VALUE(start2 + idx, temp);
	}

	return true;
}

bool Trainer::Remove(u32 slot)
{
	if (slot >= TEAM_COUNT)
		return false;

	MakeNull(slot);
	for (u32 idx = slot; idx < TEAM_COUNT - 1; ++idx)
		Swap(idx, idx + 1);

	return true;
}

bool Trainer::Insert(u32 slot)
{
	if (slot >= TEAM_COUNT)
		return false;

	MakeDefault(TEAM_COUNT - 1);
	for (int idx = TEAM_COUNT - 2; idx >= (int)slot; --idx)
		Swap(idx, idx + 1);

	return true;
}

const u8 hpTypes[] = {
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
};
int Trainer::GetHiddenPower(u32 slot)
{
	int hpValue = (TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_HP_IV)) & 1) != 0;
	if ((TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_ATK_IV)) & 1) != 0)
		hpValue += 2;
	if ((TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_DEF_IV)) & 1) != 0)
		hpValue += 4;
	if ((TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPE_IV)) & 1) != 0)
		hpValue += 8;
	if ((TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPA_IV)) & 1) != 0)
		hpValue += 16;
	if ((TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPD_IV)) & 1) != 0)
		hpValue += 32;

	int idx = 15 * hpValue / 63;
	for (; idx > 16; idx -= 16) {}
	return hpTypes[idx];
}

string Trainer::GetStat(u32 slot, u32 baseStat, NatureStat stat)
{
	int ev = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPE_EV));
	int iv = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPE_IV));
	int level = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_LEVEL));

	int species = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_SPECIES));
	int form = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_FORM));
	int base = engine->GetDataValue(engine->pokemons, species, PERSONAL_FIELD(form, baseStat));

	int value = ((ev / 4 + iv + 2 * base) * level) / 100 + 5;

	int nature = TRAINER_GET_VALUE(TEAM_FIELD(slot, TRAINER_NATURE));
	char boost = 0;
	if (stat != NATURE_STAT_HP)
	{
		boost = natureStats[5 * nature - 1 + stat];
		if (boost == 1)
			value = (110 * value) / 100;
		else if (boost == -1)
			value = (90 * value) / 100;
	}
	else if (species == 292) // Shedinja
		value = 1;

	string output = '[' + to_string(value) + ']';
	if (boost == 1)
		output += " +";
	else if (boost == -1)
		output += " -";
	return output;
}
