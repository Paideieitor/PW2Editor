#ifndef _POKEMON_SEARCH_H
#define _POKEMON_SEARCH_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/Pokemon.h"
#include "Data/PersonalData.h"
#include "Data/LearnsetData.h"
#include "Data/EvolutionData.h"
#include "Data/ChildData.h"

class PokemonSearch : public Module
{
public:
	PokemonSearch() = delete;
	PokemonSearch(Engine* const engine, u32 group);
	~PokemonSearch();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override {}

private:

	void ClearSearch();

	void Search();
	bool SearchCheck(const Pokemon& pkm);
	bool SearchCheckPokemon(const Pokemon& pkm);
	bool SearchCheckPersonal(const PersonalData& personal);
	bool SearchCheckLearnset(const LearnsetData& learnset);
	bool SearchCheckEvolution(const EvolutionData& evolution);
	bool SearchCheckChild(const ChildData& child);

	bool EnableButton(int* params, u32 field, int nullValue, u32 eventID);
	bool ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID);
	bool InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID);
	bool CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID);
	void AdvancedSearchMenu();

	int selectedIdx = -1;
	vector<Pokemon*> selectable = vector<Pokemon*>();

	string searchName = string();
	PersonalData personalParams;
	LearnMove learnsetParams;
	EvolutionMethod evolutionParams;
	ChildData childParams;

	bool advancedSearchMenu = false;
};

#endif // _POKEMON_SEARCH_H
