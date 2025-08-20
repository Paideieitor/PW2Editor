#ifndef _TRAINER_MODULE_H
#define _TRAINER_MODULE_H

#include "Modules/Module.h"

class Trainer : public Module
{
public:
	Trainer() = delete;
	Trainer(Engine* const engine, u32 group);
	~Trainer() {}

	virtual ReturnState RenderGUI() override;

private:

	enum NatureStat
	{
		NATURE_STAT_HP,
		NATURE_STAT_ATK,
		NATURE_STAT_DEF,
		NATURE_STAT_SPA,
		NATURE_STAT_SPD,
		NATURE_STAT_SPE,
		NATURE_STAT_MAX,
	};

	bool TeamMember(int trainerType, u32 slot);

	int TeamCount();

	void MakeNull(u32 slot);
	bool IsNull(u32 slot);

	void MakeDefault(u32 slot);

	bool Swap(u32 slot1, u32 slot2);
	bool Remove(u32 slot);
	bool Insert(u32 slot);

	int GetHiddenPower(u32 slot);
	string GetStat(u32 slot, u32 baseStat, NatureStat stat);

	const vector<string>* const trainerClasses;
	const vector<string>* const pokemons;
	const vector<string>* const natures;
	const vector<string>* const abilities;
	const vector<string>* const items;
	const vector<string>* const moves;
	const vector<string>* const types;

	vector<string> trainerTypes = {
		"Simple",
		"Moves",
		"Item",
		"Good",
		// "Perfect", // included in the constructor if patches are enabled
	};
	const vector<string> battleTypes = {
		"Singles",
		"Doubles",
		"Triples",
		"Rotation",
	};
	const vector<string> pokemonSex = {
		"Random",
		"Male",
		"Female",
		"None",
	};
	const vector<string> abilitySlots = {
		"Random Ability",
		"Ability 1",
		"Ability 2",
		"Hidden Ability",
	};
	const vector<string> status = {
		"None",
		"Paralysis",
		"Sleep",
		"Freeze",
		"Burn",
		"Poison",
	};

	const char natureStats[125] = {
		0, 0, 0, 0, 0, 1, -1, 0, 0, 0, 1, 0, 0, 0, -1, 1, 0, -1, 0, 0, 1, 0, 0, -1,
		0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1, -1, 0, 0, 0, 1, 0,
		-1, 0, -1, 0, 0, 0, 1, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, -1, 0, 1, 0,
		0, 0, -1, 1, -1, 0, 1, 0, 0, 0, -1, 1, 0, 0, 0, 0, 1, 0, -1, 0, 0, 0, 0,
		0, 0, 0, 1, -1, 0, -1, 0, 0, 1, 0, 0, -1, 0, 1, 0, 0, 0, 0, 1, -1, 0, 0,
		-1, 1, 0, 0, 0, 0, 0, 0
	};
};

#endif // _TRAINER_MODULE_H


