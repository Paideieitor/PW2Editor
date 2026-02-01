#include "Windows/Engine.h"

#include "Modules/Pokemon.h"

#define POKEMON_INPUT_INT(field, label, max, min) engine->InputInt(engine->pokemons, GetProjectPokemon(*engine->project), field, &engine->project->pokemon, engine->project->pokemon, group, label, max, min)
#define POKEMON_COMBO_BOX(field, label, _list) engine->ComboBox(engine->pokemons, GetProjectPokemon(*engine->project), field, &engine->project->pokemon, engine->project->pokemon, group, label, _list)
#define POKEMON_CHECK_BOX(field, label) engine->CheckBox(engine->pokemons, GetProjectPokemon(*engine->project), field, &engine->project->pokemon, engine->project->pokemon, group, label)
#define POKEMON_LIST_BOX(firstField, label, _list) engine->ListBox(engine->pokemons, GetProjectPokemon(*engine->project), firstField, &engine->project->pokemon, engine->project->pokemon, group, label, _list)

#define POKEMON_GET_VALUE(field) engine->GetDataValue(engine->pokemons, GetProjectPokemon(*engine->project), field)
#define POKEMON_SET_VALUE(field, value) engine->SetDataValue(engine->pokemons, GetProjectPokemon(*engine->project), field, value)
#define POKEMON_START_ACTION engine->StartAction(&engine->project->pokemon, engine->project->pokemon, group)
#define POKEMON_END_ACTION engine->EndAction()

enum EvolutionMethod
{
	EVO_NONE = 0x0,
	EVO_FRIENDSHIP = 0x1,
	EVO_FRIENDSHIP_MORNING = 0x2,
	EVO_FRIENDSHIP_NIGHT = 0x3,
	EVO_LEVELUP = 0x4,
	EVO_TRADE = 0x5,
	EVO_TRADE_ITEM = 0x6,
	EVO_TRADE_MONS_SPECIFIC = 0x7,
	EVO_ITEM_USE = 0x8,
	EVO_LEVELUP_ATK_OVER_DEF = 0x9,
	EVO_LEVELUP_ATK_EQU_DEF = 0xA,
	EVO_LEVELUP_DEF_OVER_ATK = 0xB,
	EVO_LEVELUP_RANDOM_A = 0xC,
	EVO_LEVELUP_RANDOM_B = 0xD,
	EVO_LEVELUP_NINJASK = 0xE,
	EVO_LEVELUP_SHEDINJA = 0xF,
	EVO_LEVELUP_BEAUTY = 0x10,
	EVO_ITEM_USE_M = 0x11,
	EVO_ITEM_USE_F = 0x12,
	EVO_LEVELUP_ITEM_HELD_DAY = 0x13,
	EVO_LEVELUP_ITEM_HELD_NIGHT = 0x14,
	EVO_LEVELUP_MOVE = 0x15,
	EVO_LEVELUP_PARTYMONS = 0x16,
	EVO_LEVELUP_M = 0x17,
	EVO_LEVELUP_F = 0x18,
	EVO_LEVELUP_ELEC = 0x19,
	EVO_LEVELUP_MOSS = 0x1A,
	EVO_LEVELUP_ICE = 0x1B,
	EVO_LEVELUP_ELEC2 = 0x1C,
};

Pokemon::Pokemon(Engine* const engine, u32 group) : Module(engine, group),
pokemons(engine->GetTextFile(PKM_NAME_FILE_ID)), types(engine->GetTextFile(TYPE_NAME_FILE_ID)), 
abilities(engine->GetTextFile(ABILITY_NAME_FILE_ID)), items(engine->GetTextFile(ITEM_NAME_FILE_ID)),
moves(engine->GetTextFile(MOVE_NAME_FILE_ID))
{
}

ReturnState Pokemon::RenderGUI()
{
	u32 currentPokemon = GetProjectPokemon(*engine->project);
	u32 currentForm = GetProjectForm(*engine->project);

	ImGui::Begin(u8"Pokémon");

	u32 formCount = engine->GetConcatDataCount(engine->pokemons, currentPokemon);
	if (formCount == 0)
	{
		ImGui::Text("No Data");
	}
	else if (ImGui::BeginTabBar("Form Tabs"))
	{
		bool rendered = false;
		for (u32 form = 0; form < formCount; ++form)
		{
			ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
			if (currentForm == form)
				flags |= ImGuiTabItemFlags_SetSelected;
		
			u32 formIdx = engine->GetConcatDataIdx(engine->pokemons, currentPokemon, form);
			bool tabOutput = ImGui::BeginTabItem(to_string(formIdx).c_str(), 0, flags);
			if (ImGui::IsItemClicked())
			{
				if (currentForm != form)
				{
					currentForm = form;
					SetProjectForm(*engine->project, currentForm);
				}
			}
		
			if (!rendered && currentForm == form)
			{
				rendered = true;

				ImGui::BeginGroup();
				{
					ImGui::Text("Personal");
					ImGui::BeginGroup();
					{
						ImGui::BeginGroup();
						{
							Personal(currentForm);
							Child(currentForm);
						}
						ImGui::EndGroup();
						ImGui::SameLine();
						Tutors(currentForm);
					}
					ImGui::EndGroup();
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Text("Learnset");
					Learnset(form);
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Text("Evolution");
					Evolution(form);
				}
				ImGui::EndGroup();
			}
		
			if (tabOutput)
				ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
	return OK;
}

void Pokemon::Personal(u32 form)
{
	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			ImGui::Text("Base Stats");
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_HP), "HP", 255, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_ATK), "Attack", 255, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_DEF), "Defense", 255, 0);
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_SPA), "Sp.Attack", 255, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_SPD), "Sp.Defense", 255, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_SPE), "Speed", 255, 0);
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Types");
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, TYPE_1), "", types);
			ImGui::SameLine();
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, TYPE_2), "", types);
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();
		{
			ImGui::Text("EVs");
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_HP), "HP", 3, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_ATK), "Attack", 3, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_DEF), "Defense", 3, 0);
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_SPA), "Sp.Attack", 3, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_SPD), "Sp.Defense", 3, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EV_SPE), "Speed", 3, 0);
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Abilities");
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, ABILITY_1), "", abilities);
			ImGui::SameLine();
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, ABILITY_2), "", abilities);
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, ABILITY_HIDDEN), "", abilities);
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Wild Items");
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, WILD_ITEM_50), "50%", items);
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, WILD_ITEM_5), "5%", items);
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, WILD_ITEM_1), "1%", items);
		}
		ImGui::EndGroup();
		
		ImGui::SameLine();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Egg Groups");
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, EGG_GROUP_1), "", &eggGroups);
			POKEMON_COMBO_BOX(PERSONAL_FIELD(form, EGG_GROUP_2), "", &eggGroups);
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Exp & Hapinness");
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_EXP), "Base Exp", 0xFFFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EGG_HAPPINESS), "Egg Happiness", 255, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, BASE_HAPPINESS), "Base Happiness", 255, 0);
		}
		ImGui::EndGroup();
		
		ImGui::SameLine();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Flags");
			POKEMON_CHECK_BOX(PERSONAL_FIELD(form, GROUNDED_SPRITE), "Grounded Sprite");
			POKEMON_CHECK_BOX(PERSONAL_FIELD(form, SPRITE_FLIP), "Sprite Flip");
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		{
			ImGui::Text("Data");
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, CATCH_RATE), "Catch Rate", 0xFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, SEX_CHANCE), "Sex Chance", 0xFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, GROWTH_RATE), "Growth Rate", 0xFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, ESCAPE_RATE), "Escape Rate", 0xFF, 0);
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, EVOLUTION_STAGE), "Evo Stage", 0xFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, COLOR), "Color", 0xFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, HEIGHT), "Height", 0xFFFF, 0);
			POKEMON_INPUT_INT(PERSONAL_FIELD(form, WEIGHT), "Weight", 0xFFFF, 0);
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
}

void Pokemon::Tutors(u32 form)
{
	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, TM_001), "TMs", &tms);
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, GRASS_PLEDGE_TUTOR), "Special Tutors", &specialTutors);
		ImGui::EndGroup();

		u32 shardIdx = 0;
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, RED_TUTOR_00), shardTutors.at(shardIdx), &redShardTutor);
		ImGui::EndGroup();
		ImGui::SameLine();
		shardIdx = 1;
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, BLUE_TUTOR_00), shardTutors.at(shardIdx), &blueShardTutor);
		ImGui::EndGroup();

		shardIdx = 2;
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, YELLOW_TUTOR_00), shardTutors.at(shardIdx), &yellowShardTutor);
		ImGui::EndGroup();
		ImGui::SameLine();
		shardIdx = 3;
		ImGui::BeginGroup();
		POKEMON_LIST_BOX(PERSONAL_FIELD(form, GREEN_TUTOR_00), shardTutors.at(shardIdx), &greenShardTutor);
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
}

void Pokemon::Learnset(u32 form)
{
	u32 learnMoves = 0;
	ImGui::BeginGroup();
	{
		ImGui::Text("Level");
		for (u32 learnIdx = 0; learnIdx < LEARNSET_MOVE_COUNT; ++learnIdx)
		{
			u32 field = LEARNSET_FIELD(form, learnIdx, LEARN_LEVEL);
			int value = POKEMON_GET_VALUE(field);
			if (value == POKEMON_NULL)
			{
				if (ImGui::Button("Add##Learn"))
				{
					POKEMON_START_ACTION;
					u32 moveField = POKEMON_SET_VALUE(field, 0) - 1;
					POKEMON_SET_VALUE(moveField, 0);
					POKEMON_END_ACTION;
				}
				break;
			}
	
			POKEMON_INPUT_INT(field, "", 100, 0);
			++learnMoves;
		}
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		ImGui::Text("Move");
		for (u32 learnIdx = 0; learnIdx < learnMoves; ++learnIdx)
			POKEMON_COMBO_BOX(LEARNSET_FIELD(form, learnIdx, LEARN_MOVE_ID), "", moves);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		ImGui::Text(" ");
		for (u32 learnIdx = 0; learnIdx < learnMoves; ++learnIdx)
		{
			u32 field = LEARNSET_FIELD(form, learnIdx, LEARN_LEVEL);
			if (ImGui::Button(LABEL("X##Learn", learnIdx)))
			{
				POKEMON_START_ACTION;
				u32 moveField = POKEMON_SET_VALUE(field, POKEMON_NULL) - 1;
				POKEMON_SET_VALUE(moveField, POKEMON_NULL);
				POKEMON_END_ACTION;
			}
		}
	}
	ImGui::EndGroup();
}

void Pokemon::Evolution(u32 form)
{
	ImGui::BeginGroup();
	{
		ImGui::Text("Species");
		for (u32 evoIdx = 0; evoIdx < EVOLUTION_COUNT; ++evoIdx)
			POKEMON_COMBO_BOX(EVOLUTION_FIELD(form, evoIdx, EVOLUTION_SPECIES), "", pokemons);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		ImGui::Text("Method");
		for (u32 evoIdx = 0; evoIdx < EVOLUTION_COUNT; ++evoIdx)
			POKEMON_COMBO_BOX(EVOLUTION_FIELD(form, evoIdx, EVOLUTION_METHOD), "", &evolutionMethods);
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	{
		ImGui::Text("Parameter");
		for (u32 evoIdx = 0; evoIdx < EVOLUTION_COUNT; ++evoIdx)
		{
			int method = POKEMON_GET_VALUE(EVOLUTION_FIELD(form, evoIdx, EVOLUTION_METHOD));

			u32 field = EVOLUTION_FIELD(form, evoIdx, EVOLUTION_PARAM);
			switch (method)
			{
				// Nothing
			case EVO_FRIENDSHIP:
			case EVO_FRIENDSHIP_MORNING:
			case EVO_FRIENDSHIP_NIGHT:
			case EVO_TRADE:
			case EVO_TRADE_MONS_SPECIFIC:
			case EVO_LEVELUP_ELEC:
			case EVO_LEVELUP_MOSS:
			case EVO_LEVELUP_ICE:
			case EVO_LEVELUP_ELEC2:
				ImGui::Text("No Parameter");
				break;
				// Level
			case EVO_LEVELUP:
			case EVO_LEVELUP_ATK_OVER_DEF:
			case EVO_LEVELUP_ATK_EQU_DEF:
			case EVO_LEVELUP_DEF_OVER_ATK:
			case EVO_LEVELUP_RANDOM_A:
			case EVO_LEVELUP_RANDOM_B:
			case EVO_LEVELUP_NINJASK:
			case EVO_LEVELUP_SHEDINJA:
			case EVO_LEVELUP_M:
			case EVO_LEVELUP_F:
				POKEMON_INPUT_INT(field, "", 100, 0);
				break;
				// Item
			case EVO_TRADE_ITEM:
			case EVO_ITEM_USE:
			case EVO_LEVELUP_ITEM_HELD_DAY:
			case EVO_LEVELUP_ITEM_HELD_NIGHT:
			case EVO_ITEM_USE_M:
			case EVO_ITEM_USE_F:
				POKEMON_COMBO_BOX(field, "", items);
				break;
				// Beauty
			case EVO_LEVELUP_BEAUTY:
				break;
				// Move
			case EVO_LEVELUP_MOVE:
				POKEMON_COMBO_BOX(field, "", moves);
				break;
				// Pokémon
			case EVO_LEVELUP_PARTYMONS:
				POKEMON_COMBO_BOX(field, "", pokemons);
				break;
			}
		}
	}
	ImGui::EndGroup();
}

void Pokemon::Child(u32 form)
{
    UNUSED(form);

	ImGui::BeginGroup();
	ImGui::Text("Child");
	POKEMON_COMBO_BOX(CHILD_FIELD(0), "", pokemons);
	ImGui::EndGroup();
}

