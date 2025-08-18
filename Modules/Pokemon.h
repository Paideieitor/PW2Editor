#ifndef _POKEMON_MODULE_H
#define _POKEMON_MODULE_H

#include "Modules/Module.h"

class Pokemon : public Module
{
public:
	Pokemon() = delete;
	Pokemon(Engine* const engine, u32 group);
	~Pokemon() {}

	virtual ReturnState RenderGUI() override;

private:

	void Personal(u32 form);
	void Tutors(u32 form);

	void Learnset(u32 form);

	void Evolution(u32 form);

	void Child(u32 form);

	const vector<string>* const pokemons;
	const vector<string>* const types;
	const vector<string>* const abilities;
	const vector<string>* const items;
	const vector<string>* const moves;

	const vector<string> eggGroups = {
		"None",
		"Monster",
		"Water 1",
		"Bug",
		"Flying",
		"Field",
		"Fairy",
		"Grass",
		"Human-Like",
		"Water 3",
		"Mineral",
		"Amorphous",
		"Water 2",
		"Ditto",
		"Dragon",
		"Unknown",
	};

	const vector<string> tms = {
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
	const vector<string> specialTutors = {
		"Grass Pledge",
		"Fire Pledge",
		"Water Pledge",
		"Frenzy Plant",
		"Blast Burn",
		"Hydro Cannon",
		"Draco Meteor",
	};

	const vector<string> shardTutors = {
		"Driftveil Tutor",
		"Lentimas Tutor",
		"Humilau Tutor",
		"Nacrene Tutor",
	};
	const vector<string> redShardTutor = {
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
	};
	const vector<string> blueShardTutor = {
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
	};
	const vector<string> yellowShardTutor = {
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
	};
	const vector<string> greenShardTutor = {
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
	};
	const vector<string> evolutionMethods = {
		"None",
		"Friendship",
		"Friendship Morning",
		"Friendship Night",
		"Level Up",
		"Trade",
		"Trade with Item",
		u8"Trade for Pokémon",
		"Item Use",
		"Level Up Atk. over Def.",
		"Level Up Atk. equal Def.",
		"Level Up Def. over Atk.",
		"Level Up Random A",
		"Level Up Random B",
		"Level Up Ninjask",
		"Level Up Shedinja",
		"Level Up Beauty",
		"Item Use Male",
		"Item Use Female",
		"Level Up with Item day",
		"Level Up with Item night",
		"Level Up with Move",
		u8"Level Up with Pokémon in Party",
		"Level Up Male",
		"Level Up Female",
		"Level Up Electric Cave",
		"Level Up Moss Rock",
		"Level Up Ice Rock",
		"Level Up Electric Cave 2",
	};
};

#endif // _POKEMON_MODULE_H
