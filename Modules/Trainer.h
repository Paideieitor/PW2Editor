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

	bool TeamMember(int trainerType, u32 slot);

	int TeamCount();

	void MakeNull(u32 slot);
	bool IsNull(u32 slot);

	void MakeDefault(u32 slot);

	bool Swap(u32 slot1, u32 slot2);
	bool Remove(u32 slot);
	bool Insert(u32 slot);

	u32 GetHiddenPower(u32 slot);

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
};

#endif // _TRAINER_MODULE_H


