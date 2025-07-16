#ifndef _PERSONAL_H
#define _PERSONAL_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/PersonalData.h"

struct Pokemon;

class Personal : public Module
{
public:
	Personal() = delete;
	Personal(Engine* const engine, u32 group);
	~Personal();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	vector<string> tms = {
		"TM01 Hone Claws",
		"TM02 Dragon Claw",
		"TM03 Psyshock",
		"TM04 Calm Mind",
		"TM05 Roar",
		"TM06 Toxic",
		"TM07 Hail",
		"TM08 Bulk Up",
		"TM09 Venoshock",
		"TM10 Hidden Power",
		"TM11 Sunny Day",
		"TM12 Taunt",
		"TM13 Ice Beam",
		"TM14 Blizzard",
		"TM15 Hyper Beam",
		"TM16 Light Screen",
		"TM17 Protect",
		"TM18 Rain Dance",
		"TM19 Telekinesis",
		"TM20 Safeguard",
		"TM21 Frustration",
		"TM22 SolarBeam",
		"TM23 Smack Down",
		"TM24 Thunderbolt",
		"TM25 Thunder",
		"TM26 Earthquake",
		"TM27 Return",
		"TM28 Dig",
		"TM29 Psychic",
		"TM30 Shadow Ball",
		"TM31 Brick Break",
		"TM32 Double Team",
		"TM33 Reflect",
		"TM34 Sludge Wave",
		"TM35 Flamethrower",
		"TM36 Sludge Bomb",
		"TM37 Sandstorm",
		"TM38 Fire Blast",
		"TM39 Rock Tomb",
		"TM40 Aerial Ace",
		"TM41 Torment",
		"TM42 Facade",
		"TM43 Flame Charge",
		"TM44 Rest",
		"TM45 Attract",
		"TM46 Thief",
		"TM47 Low Sweep",
		"TM48 Round",
		"TM49 Echoed Voice",
		"TM50 Overheat",
		"TM51 Ally Switch",
		"TM52 Focus Blast",
		"TM53 Energy Ball",
		"TM54 False Swipe",
		"TM55 Scald",
		"TM56 Fling",
		"TM57 Charge Beam",
		"TM58 Sky Drop",
		"TM59 Incinerate",
		"TM60 Quash",
		"TM61 Will-O-Wisp",
		"TM62 Acrobatics",
		"TM63 Embargo",
		"TM64 Explosion",
		"TM65 Shadow Claw",
		"TM66 Payback",
		"TM67 Retaliate",
		"TM68 Giga Impact",
		"TM69 Rock Polish",
		"TM70 Flash",
		"TM71 Stone Edge",
		"TM72 Volt Switch",
		"TM73 Thunder Wave",
		"TM74 Gyro Ball",
		"TM75 Swords Dance",
		"TM76 Struggle Bug",
		"TM77 Psych Up",
		"TM78 Bulldoze",
		"TM79 Frost Breath",
		"TM80 Rock Slide",
		"TM81 X-Scissor",
		"TM82 Dragon Tail",
		"TM83 Work Up",
		"TM84 Poison Jab",
		"TM85 Dream Eater",
		"TM86 Grass Knot",
		"TM87 Swagger",
		"TM88 Pluck",
		"TM89 U-turn",
		"TM90 Substitute",
		"TM91 Flash Cannon",
		"TM92 Trick Room",
		"TM93 Wild Charge",
		"TM94 Rock Smash",
		"TM95 Snarl",
		"HM01 Cut",
		"HM02 Fly",
		"HM03 Surf",
		"HM04 Strength",
		"HM05 Waterfall",
		"HM06 Dive"
	};
	vector<string> typeTutors = {
		"Grass Pledge",
		"Fire Pledge",
		"Water Pledge",
		"Frenzy Plant",
		"Blast Burn",
		"Hydro Cannon",
		"Draco Meteor",
	};
	vector<pair<string, vector<string>>> tutors = {
		{"Driftveil Tutor",{
		"Bug Bite",
		"Covet",
		"Super Fang",
		"Dual Chop",
		"Signal Beam",
		"Iron Head",
		"Seed Bomb",
		"Drill Run",
		"Bounce",
		"Low Kick",
		"Gunk Shot",
		"Uproar",
		"Thunder Punch",
		"Fire Punch",
		"Ice Punch",
	}},
		{"Lentimas Tutor",{
		"Magic Coat",
		"Block",
		"Earth Power",
		"Foul Play",
		"Gravity",
		"Magnet Rise",
		"Iron Defense",
		"Last Resort",
		"Super Power",
		"Electroweb",
		"Icy Wind",
		"Aqua Tail",
		"Dark Pulse",
		"Zen Headbutt",
		"Dragon Pulse",
		"Hyper Voice",
		"Iron Tail",
	}},
		{"Humilau Tutor",{
		"Bind",
		"Snore",
		"Knock Off",
		"Synthesis",
		"Heat Wave",
		"Role Play",
		"Heal Bell",
		"Tailwind",
		"Sky Attack",
		"Pain Split",
		"Giga Drain",
		"Drain Punch",
		"Roost",
	}},
		{"Nacrene Tutor",{
		"Gastro Acid",
		"Worry Seed",
		"Spite",
		"After You",
		"Helping Hand",
		"Trick",
		"Magic Room",
		"Wonder Room",
		"Endeavor",
		"Outrage",
		"Recycle",
		"Snatch",
		"Stealth Rock",
		"Sleep Talk",
		"Skill Swap"
	}},
	};

	bool moveMenu = false;

	void ComboBox(PersonalData& personal, const char* label, const std::vector<std::string>& items, PersonalField field);
	void InputInt(PersonalData& personal, const char* label, PersonalField field, int maxValue);
	void CheckBox(PersonalData& personal, const char* label, PersonalField field);
	void BitCheckBox(PersonalData& personal, const char* label, PersonalField field, u32 mask);
};

#endif // _PERSONAL_H