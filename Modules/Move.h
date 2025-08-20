#ifndef _MOVE_MODULE_H
#define _MOVE_MODULE_H

#include "Modules/Module.h"

class Move : public Module
{
public:
	Move() = delete;
	Move(Engine* const engine, u32 group);
	~Move() {}

	virtual ReturnState RenderGUI() override;

private:

	const vector<string>* const types;

	const vector<string> moveQuality = {
		"Damage",
		"Condition",
		"Stat change",
		"Heal",
		"Damage + Condition",
		"Condition + Stat change",
		"Damage + Stat change",
		"Damage + Self stat change",
		"Damage + Heal",
		"OHKO",
		"Field effect",
		"Side effect",
		"Force switch",
		"Uncategorized",
	};
	const vector<string> moveCategory = {
		"Status",
		"Physical",
		"Special",
	};
	const vector<string> moveTarget = {
		"Select Other",
		"Select Ally or User",
		"Select Ally",
		"Select Enemy",
		"All Other",
		"All Enemies",
		"All Allies",
		"User",
		"All",
		"Random Enemy",
		"Field",
		"Enemy Side",
		"Ally Side",
		"Unknown",
	};
	const vector<string> conditions = {
		"None",
		"Paralysis",
		"Sleep",
		"Freeze",
		"Burn",
		"Poison",
		"Confusion",
		"Attract",
		"Bind",
		"Nightmare",
		"Curse",
		"Taunt",
		"Torment",
		"Disable",
		"Yawn",
		"Healblock",
		"Gastroacid",
		"Foresight",
		"Leechseed",
		"Embargo",
		"Perishsong",
		"Ingrain",
		"Block",
		"Encore",
		"Roost",
		"Movelock",
		"Chargelock",
		"Choicelock",
		"Must Hit",
		"Lock-on",
		"Floating",
		"Knocked Down",
		"Telekinesis",
		"Skydrop",
		"Accuracy Up",
		"Aqua Ring",
		"All",
	};
	const vector<string> stats = {
		"Null",
		"Attack",
		"Defense",
		"Sp.Attack",
		"Sp.Defense",
		"Speed",
		"Accuracy",
		"Evasion",
	};
};

#endif // _MOVE_MODULE_H

