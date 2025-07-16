#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/PokemonSearch.h"

#include "Data/Project.h"

#define EMPTY_SEARCH_NAME " "

#define SEARCH_NAME_EVENT 0
#define SELECT_POKEMON_EVENT 1
#define SHOW_POKESTUDIO_EVENT 2
#define ADVANCED_SEARCH_EVENT 3
#define CLEAR_SEARCH_EVENT 4

#define PERSONAL_EVENT(field) (5 + field)
#define GET_PERSONAL_FROM_EVENT(subType) (subType - PERSONAL_EVENT(0))

#define LEARNSET_EVENT(field) (5 + PERSONAL_MAX + field)
#define GET_LEARNSET_FROM_EVENT(subType) (subType - LEARNSET_EVENT(0))

#define EVOLUTION_EVENT(field) (5 + PERSONAL_MAX + LEARNSET_MAX + field)
#define GET_EVOLUTION_FROM_EVENT(subType) (subType - EVOLUTION_EVENT(0))

#define CHILD_EVENT (5 + PERSONAL_MAX + LEARNSET_MAX + EVOLUTION_MAX)

u32 GetReverseEventType(u32 eventID, u32& field)
{
	if (eventID == CHILD_EVENT)
		return 4;

	field = GET_PERSONAL_FROM_EVENT(eventID);
	if (field < PERSONAL_MAX)
		return 1;

	field = GET_LEARNSET_FROM_EVENT(eventID);
	if (field < LEARNSET_MAX)
		return 2;

	field = GET_EVOLUTION_FROM_EVENT(eventID);
	if (field < EVOLUTION_MAX)
		return 3;

	return 0;
}

bool comparePairs(int a1, int a2, int b1, int b2) {
	int minA = (a1 < a2) ? a1 : a2;
	int maxA = (a1 > a2) ? a1 : a2;
	int minB = (b1 < b2) ? b1 : b2;
	int maxB = (b1 > b2) ? b1 : b2;

	return (minA == minB && maxA == maxB);
}

PokemonSearch::PokemonSearch(Engine* const engine, u32 group) : Module(engine, group, " ")
{
	ClearSearch();

	Search();
}

PokemonSearch::~PokemonSearch()
{
}

ReturnState PokemonSearch::RenderGUI()
{
	ImGui::Begin(u8"Search Pokémon");

	string name = searchName;
	if (ImGui::InputText("Search", &name))
	{
		SIMPLE_REVERSE_EVENT(SEARCH_NAME_EVENT, searchName, name);
		searchName = name;
		
		Search();
	}
		
	if (ImGui::BeginListBox(" ", ImVec2(ImGui::GetWindowWidth() - 15.0f, ImGui::GetWindowHeight() * 0.70f)))
	{
		for (int idx = 0; idx < selectable.size(); ++idx)
		{
			if (ImGui::Selectable(LABEL(engine->pkmNames[selectable[idx]->idx], idx), selectedIdx == idx))
			{
				SIMPLE_REVERSE_EVENT(SELECT_POKEMON_EVENT,
					make_pair(engine->project->selectedPkmIdx, selectedIdx),
					make_pair(selectable[idx]->idx, idx));

				engine->SetCurrentPokemon(selectable[idx]->idx, engine->project->selectedPkmForm);
				selectedIdx = idx;
			}
		}
		ImGui::EndListBox();
	}

	bool showPokeStudio = engine->project->showPokeStudio;
	if (ImGui::Checkbox(u8"Show PokéStudio", &showPokeStudio))
	{
		SIMPLE_REVERSE_EVENT(SHOW_POKESTUDIO_EVENT, engine->project->showPokeStudio, (u32)showPokeStudio);
		engine->project->showPokeStudio = (u32)showPokeStudio;

		Search();
	}

	if (ImGui::Button("Advanced Search"))
	{
		SIMPLE_REVERSE_EVENT(ADVANCED_SEARCH_EVENT, advancedSearchMenu, !advancedSearchMenu);
		advancedSearchMenu = !advancedSearchMenu;
	}
		
	if (advancedSearchMenu)
		AdvancedSearchMenu();

	if (ImGui::Button("Clear Search"))
	{
		pair<string, PersonalData> searchData = make_pair(searchName, personalParams);
		ClearSearch();
		SIMPLE_REVERSE_EVENT(CLEAR_SEARCH_EVENT,
			make_pair(searchName, personalParams), searchData);

		Search();
	}

	ImGui::End();
	return OK;
}

void PokemonSearch::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case SEARCH_NAME_EVENT: // Search Name
	{
		string* name = (string*)reverseEvent->value;
		searchName = *name;
		break;
	}
	case SELECT_POKEMON_EVENT: // Selected Pokémon
	{
		pair<u32, int>* selected = (pair<u32, int>*)reverseEvent->value;
		engine->SetCurrentPokemon(selected->first, engine->project->selectedPkmForm);
		selectedIdx = selected->second;
		break;
	}
	case SHOW_POKESTUDIO_EVENT: // Show PokéStudio
	{
		u32* value = (u32*)reverseEvent->value;
		engine->project->showPokeStudio = *value;
		break;
	}
	case ADVANCED_SEARCH_EVENT: // Advanced Search Menu
	{
		bool value = (bool*)reverseEvent->value;
		advancedSearchMenu = value;
		break;
	}
	case CLEAR_SEARCH_EVENT: // Clear Search Data
	{
		pair<string, PersonalData>* searchData = (pair<string, PersonalData>*)reverseEvent->value;
		searchName = searchData->first;
		personalParams = searchData->second;
		break;
	}
	default:
	{
		int* value = (int*)reverseEvent->value;

		u32 field = 0;
		u32 eventType = GetReverseEventType(reverseEvent->subType, field);
		switch (eventType)
		{
		case 1:
			personalParams[field] = *value;
			break;
		case 2:
			learnsetParams[field] = *value;
			break;
		case 3:
			evolutionParams[field] = *value;
			break;
		case 4:
			childParams = *value;
			break;
		}
		break;
	}
	}

	Search();
}

void PokemonSearch::ClearSearch()
{
	searchName.clear();

	for (u32 paramIdx = 0; paramIdx < (u32)PERSONAL_MAX; ++paramIdx)
		personalParams[paramIdx] = PERSONAL_NULL;

	for (u32 learnsetIdx = 0; learnsetIdx < (u32)LEARNSET_MAX; ++learnsetIdx)
		learnsetParams[learnsetIdx] = LEARNSET_NULL;

	for (u32 evolutionIdx = 0; evolutionIdx < (u32)EVOLUTION_MAX; ++evolutionIdx)
		evolutionParams[evolutionIdx] = EVOLUTION_NULL;

	childParams = CHILD_NULL;
}

void PokemonSearch::Search()
{
	// Reset the search data
	selectedIdx = -1;
	selectable.clear();

	for (u32 pkmIdx = 0; pkmIdx < (u32)engine->pokemon.size(); ++pkmIdx)
	{
		Pokemon& pkm = engine->pokemon[pkmIdx];
		if (SearchCheck(pkm))
		{
			// Set the selected index if this is the selected Pokémon
			if (pkm.idx == engine->project->selectedPkmIdx)
				selectedIdx = (int)selectable.size();
				
			selectable.emplace_back(&pkm);
		}
	}
}

bool PokemonSearch::SearchCheck(const Pokemon& pkm)
{
	if (!engine->project->showPokeStudio && pkm.pokeStudio)
		return false;

	const string& name = engine->pkmNames[pkm.idx];


	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			return false;
	}

	bool output = SearchCheckPokemon(pkm);
	// Exit the loop when the forms end or the output is no longer false
	for (u32 formIdx = 0; formIdx < (u32)pkm.forms.size() && !output; ++formIdx)
	{
		const Pokemon& formPkm = pkm.forms[formIdx];
		if (!PokemonFormHasData(formPkm))
			continue;

		output = SearchCheckPokemon(formPkm);
	}

	return output;
}

bool PokemonSearch::SearchCheckPokemon(const Pokemon& pkm)
{
	const PersonalData& personal = engine->personal[pkm.dataIdx];
	if (!SearchCheckPersonal(personal))
		return false;

	const LearnsetData& learnset = engine->learnset[pkm.dataIdx];
	if (!SearchCheckLearnset(learnset))
		return false;

	const EvolutionData& evolution = engine->evolution[pkm.dataIdx];
	if (!SearchCheckEvolution(evolution))
		return false;

	const ChildData& child = engine->child[pkm.idx];
	if (!SearchCheckChild(child))
		return false;

	return true;
}

bool PokemonSearch::SearchCheckPersonal(const PersonalData& personal)
{
	for (u32 paramIdx = 0; paramIdx < (u32)PERSONAL_MAX; ++paramIdx)
	{
		int value = personalParams[paramIdx];
		if (value == PERSONAL_NULL)
			continue;

		switch (paramIdx)
		{
		case TYPE_1:
		case EGG_GROUP_1:
		{
			int value2 = personalParams[paramIdx + 1];
			if (value2 == PERSONAL_NULL)
			{
				if (personal[paramIdx] != value && personal[paramIdx + 1] != value)
					return false;
			}
			else
			{
				if (!comparePairs(personal[paramIdx], personal[paramIdx + 1], value, value2))
					return false;
			}
			break;
		}
		case ABILITY_1:
		case WILD_ITEM_50:
			if (personal[paramIdx] != personalParams[paramIdx] &&
				personal[paramIdx + 1] != personalParams[paramIdx] &&
				personal[paramIdx + 2] != personalParams[paramIdx])
				return false;
			break;
		case CATCH_RATE:
		case EVOLUTION_STAGE:
		case GROUNDED_SPRITE:
		case SEX_CHANCE:
		case EGG_HAPPINESS:
		case BASE_HAPPINESS:
		case GROWTH_RATE:
		case ESCAPE_RATE:
		case FORM_COUNT:
		case FORM_RARE:
		case COLOR:
		case SPRITE_FLIP:
		case BASE_EXP:
		case HEIGHT:
		case WEIGHT:
			if (personal[paramIdx] != value)
				return false;
		}
	}

	return true;
}

bool PokemonSearch::SearchCheckLearnset(const LearnsetData& learnset)
{
	for (u32 moveIdx = 0; moveIdx < engine->project->learnsetSize; ++moveIdx)
	{
		bool match = true;
		for (u32 learnIdx = 0; learnIdx < (u32)LEARNSET_MAX; ++learnIdx)
		{
			int value = learnsetParams[learnIdx];

			if (value == LEARNSET_NULL)
				continue;
			if (learnset[moveIdx][learnIdx] != value)
			{
				match = false;
				break;
			}	
		}
		if (match)
			return true;
	}
	return false;
}

bool PokemonSearch::SearchCheckEvolution(const EvolutionData& evolution)
{
	for (u32 methodIdx = 0; methodIdx < engine->project->evolutionSize; ++methodIdx)
	{
		bool match = true;
		for (u32 evoIdx = 0; evoIdx < (u32)EVOLUTION_MAX; ++evoIdx)
		{
			int value = evolutionParams[evoIdx];

			if (value == EVOLUTION_NULL)
				continue;
			if (evolution[methodIdx][evoIdx] != value)
			{
				match = false;
				break;
			}
		}
		if (match)
			return true;
	}
	return false;
}

bool PokemonSearch::SearchCheckChild(const ChildData& child)
{
	if (childParams != CHILD_NULL && 
		child != childParams)
		return false;
	return true;
}

bool PokemonSearch::EnableButton(int* params, u32 field, int nullValue, u32 eventID)
{
	ImGui::SameLine();

	if (params[field] == nullValue)
	{
		if (ImGui::Button(LABEL("Enable", (int)eventID)))
		{
			SIMPLE_REVERSE_EVENT(eventID, params[field], 0);
			params[field] = 0;
			return true;
		}
	}
	else
	{
		if (ImGui::Button(LABEL("Disable", (int)eventID)))
		{
			SIMPLE_REVERSE_EVENT(eventID, params[field], nullValue);
			params[field] = nullValue;
			return true;
		}
	}
	return false;
}

bool PokemonSearch::ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int selected = params[field];
	if (params[field] == nullValue)
		selected = 0;
	if (selected >= items.size())
		return InputInt(label, 0xFFFF, params, field, nullValue, eventID);

	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SIMPLE_REVERSE_EVENT(eventID, params[field], selected);
		params[field] = selected;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define PERSONAL_COMBO_BOX(label, items, field) ComboBox(label, items, personalParams.data(), field, PERSONAL_NULL, PERSONAL_EVENT(field))
#define LEARNSET_COMBO_BOX(label, items, field) ComboBox(label, items, learnsetParams.data(), field, LEARNSET_NULL, LEARNSET_EVENT(field))
#define EVOLUTION_COMBO_BOX(label, items, field) ComboBox(label, items, evolutionParams.data(), field, EVOLUTION_NULL, EVOLUTION_EVENT(field))
#define CHILD_COMBO_BOX(label, items) ComboBox(label, items, &childParams, 0, CHILD_NULL, CHILD_EVENT)

bool PokemonSearch::InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	int value = params[field];
	ImGui::BeginDisabled(params[field] == nullValue);
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SIMPLE_REVERSE_EVENT(eventID, params[field], value);
		params[field] = value;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define PERSONAL_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, personalParams.data(), field, PERSONAL_NULL, PERSONAL_EVENT(field))
#define LEARNSET_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, learnsetParams.data(), field, LEARNSET_NULL, LEARNSET_EVENT(field))
#define EVOLUTION_INPUT_INT(label, field, maxValue) InputInt(label, maxValue, evolutionParams.data(), field, EVOLUTION_NULL, EVOLUTION_EVENT(field))

bool PokemonSearch::CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID)
{
	bool output = false;

	bool value = params[field] == -1 ? false : (bool)params[field];

	ImGui::BeginDisabled(params[field] == nullValue);
	if (ImGui::Checkbox(label, &value))
	{
		SIMPLE_REVERSE_EVENT(eventID, params[field], (int)value);
		params[field] = (int)value;
		output = true;
	}
	ImGui::EndDisabled();

	if (EnableButton(params, field, nullValue, eventID))
		output = true;

	return output;
}
#define PERSONAL_CHECK_BOX(label, field) CheckBox(label, personalParams.data(), field, PERSONAL_NULL, PERSONAL_EVENT(field))

void PokemonSearch::AdvancedSearchMenu()
{
	bool search = false;
	ImGui::Begin(u8"Pokémon Advanced Search", &advancedSearchMenu);

	ImGui::BeginGroup();
	ImGui::Text("Personal");

	if (PERSONAL_COMBO_BOX("Type 1", engine->types, TYPE_1))
		search = true;
	ImGui::BeginDisabled(personalParams[TYPE_1] == PERSONAL_NULL);
	if (personalParams[TYPE_1] == PERSONAL_NULL)
		personalParams[TYPE_2] = PERSONAL_NULL;
	if (PERSONAL_COMBO_BOX("Type 2", engine->types, TYPE_2))
		search = true;
	ImGui::EndDisabled();

	if (PERSONAL_INPUT_INT("Catch Rate", CATCH_RATE, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Evo Stage", EVOLUTION_STAGE, 255))
		search = true;

	if (PERSONAL_CHECK_BOX("Grounded Sprite", GROUNDED_SPRITE))
		search = true;

	if (PERSONAL_COMBO_BOX("Wild Item", engine->itemNames, WILD_ITEM_50))
		search = true;

	if (PERSONAL_INPUT_INT("Sex Chance", SEX_CHANCE, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Egg Happiness", EGG_HAPPINESS, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Base Happiness", BASE_HAPPINESS, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Growth Rate", GROWTH_RATE, 255))
		search = true;

	if (PERSONAL_COMBO_BOX("Egg Group 1", engine->eggGroups, EGG_GROUP_1))
		search = true;
	ImGui::BeginDisabled(personalParams[EGG_GROUP_1] == PERSONAL_NULL);
	if (personalParams[EGG_GROUP_1] == PERSONAL_NULL)
		personalParams[EGG_GROUP_2] = PERSONAL_NULL;
	if (PERSONAL_COMBO_BOX("Egg Group 2", engine->eggGroups, EGG_GROUP_2))
		search = true;
	ImGui::EndDisabled();

	if (PERSONAL_COMBO_BOX("Ability", engine->abilities, ABILITY_1))
		search = true;

	if (PERSONAL_INPUT_INT("Escape Rate", ESCAPE_RATE, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Form Count", FORM_COUNT, 255))
		search = true;

	if (PERSONAL_CHECK_BOX("Rare Form", FORM_RARE))
		search = true;

	if (PERSONAL_INPUT_INT("Color", COLOR, 63))
		search = true;

	if (PERSONAL_CHECK_BOX("Sprite Flip", SPRITE_FLIP))
		search = true;

	if (PERSONAL_INPUT_INT("Base Exp", BASE_EXP, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Height", HEIGHT, 255))
		search = true;
	if (PERSONAL_INPUT_INT("Weight", WEIGHT, 255))
		search = true;
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();

	ImGui::BeginGroup();
	ImGui::Text("Leanset");

	if (LEARNSET_COMBO_BOX("Move", engine->moveNames, LEARN_MOVE_ID))
		search = true;
	if (LEARNSET_INPUT_INT("Level", LEARN_LEVEL, 100))
		search = true;

	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Text("Evolution");

	if (EVOLUTION_COMBO_BOX("Method", engine->evolutionMethods, EVOLUTION_METHOD))
	{
		evolutionParams[EVOLUTION_PARAM] = EVOLUTION_NULL;
		search = true;
	}

	switch (evolutionParams[EVOLUTION_METHOD])
	{
	// Nothing
	case EVOLUTION_NULL:
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
		if (EVOLUTION_INPUT_INT("Level##Evolution", EVOLUTION_PARAM, 100))
			search = true;
		break;
		// Item
	case EVOLVE_TRADE_ITEM:
	case EVOLVE_ITEM_USE:
	case EVOLVE_LEVELUP_ITEM_HELD_DAY:
	case EVOLVE_LEVELUP_ITEM_HELD_NIGHT:
	case EVOLVE_ITEM_USE_M:
	case EVOLVE_ITEM_USE_F:
		if (EVOLUTION_COMBO_BOX("Item##Evolution", engine->itemNames, EVOLUTION_PARAM))
			search = true;
		break;
		// Beauty
	case EVOLVE_LEVELUP_BEAUTY:
		break;
		// Move
	case EVOLVE_LEVELUP_MOVE:
		if (EVOLUTION_COMBO_BOX("Move##Evolution", engine->moveNames, EVOLUTION_PARAM))
			search = true;
		break;
		// Pokémon
	case EVOLVE_LEVELUP_PARTYMONS:
		if (EVOLUTION_COMBO_BOX("Species##Evolution", engine->pkmNames, EVOLUTION_PARAM))
			search = true;
		break;
	}

	if (EVOLUTION_COMBO_BOX("Evolved Species", engine->pkmNames, EVOLUTION_SPECIES))
		search = true;
	
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Text("Child");

	if (CHILD_COMBO_BOX("Species##Child", engine->pkmNames))
		search = true;

	ImGui::EndGroup();

	ImGui::EndGroup();

	ImGui::End();
	if (search)
		Search();
}
