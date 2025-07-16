#ifndef _ENGINE_H
#define _ENGINE_H

#include "Utils/FileUtils.h"
#include "Utils/KlangFormat.h"

#include "Windows/Window.h"

#include "Data/Project.h"
#include "Data/Pokemon.h"
#include "Data/PersonalData.h"
#include "Data/LearnsetData.h"
#include "Data/EvolutionData.h"
#include "Data/ChildData.h"
#include "Data/ItemData.h"
#include "Data/MoveData.h"

class Module;

struct Event
{
	u32 type = 0; 
	u32 subType = 0;
	void* value = nullptr;
};

class Engine : public Window
{
public:

	Engine() = delete;
	Engine(Project* const project);
	~Engine();

	Project* const project;
	string personalPath = string();
	string learnsetPath = string();
	string evolutionPath = string();
	string childPath = string();
	string itemPath = string();
	string movePath = string();
	string moveAnimPath = string();

	vector<Pokemon> pokemon = vector<Pokemon>();

	vector<string> pkmNames = vector<string>();
	vector<string> formNames = vector<string>();
	vector<string> pkmDescriptions = vector<string>();
	vector<string> pkmTitles = vector<string>();

	vector<PersonalData> personal = vector<PersonalData>();
	vector<LearnsetData> learnset = vector<LearnsetData>();
	vector<EvolutionData> evolution = vector<EvolutionData>();
	vector<ChildData> child = vector<ChildData>();

	vector<ItemData> items = vector<ItemData>();

	vector<string> itemNames = vector<string>();
	vector<string> itemDescriptions = vector<string>();
	vector<string> itemColors = vector<string>();
	vector<string> itemPlurals = vector<string>();

	vector<MoveData> moves = vector<MoveData>();
	vector<FileStream> moveAnims = vector<FileStream>();

	vector<string> moveNames = vector<string>();
	vector<string> moveNamesMayus = vector<string>();
	vector<string> moveUses = vector<string>();
	vector<string> moveDescriptions = vector<string>();

	vector<string> types = vector<string>();
	vector<string> abilities = vector<string>();

	vector<string> eggGroups = {
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
		"Unknown"
	};
	vector<string> evolutionMethods = {
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
	vector<string> stats = {
		"Null",
		"Attack",
		"Defense",
		"Sp.Attack",
		"Sp.Defense",
		"Speed",
		"Accuracy",
		"Evasion",
	};
	vector<string> moveQuality = {
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
	vector<string> moveCategory = {
		"Status",
		"Physical",
		"Special",
	};
	vector<string> moveTarget = {
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
	vector<string> conditions = {
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

	bool commandInput = false;

	template<class T>
	void AddReverseEvent(u32 type, u32 subType, const T& value, const T& newValue, bool saveFlag)
	{
		if (value == newValue)
			return;

		Event reverseEvent = { type, subType, nullptr };
		reverseEvent.value = new T(value);

		reverseEvents.push_back(reverseEvent);
		if (reverseEvents.size() > project->maxEvents)
			reverseEvents.erase(reverseEvents.begin());
	}
	
	void HandleReverseEvent();

	template<class T>
	bool AddSaveEvent(u32 moduleIdx, u32 fileID, const T& originalValue)
	{
		// Discard save event if it has already been scheaduled
		for (u32 idx = 0; idx < (u32)saveEvents.size(); ++idx)
		{
			const Event& saveEvent = saveEvents[idx];
			if (saveEvent.type == moduleIdx &&
				saveEvent.subType == fileID)
				return false;
		}

		Event saveEvent = { moduleIdx, fileID, nullptr };
		saveEvent.value = new T(originalValue);
		saveEvents.push_back(saveEvent);
		return true;
	}

	bool SavePersonal(const PersonalData& personalData, const string& file);
	bool SaveLearnset(const LearnsetData& learnData, const string& file);
	bool SaveEvolution(const EvolutionData& evoData, const string& file);
	bool SaveChild(const ChildData& childData, const string& file);
	bool SaveItem(const ItemData& itemData, const string& file);
	bool SaveMove(const MoveData& moveData, const string& file);
	void Save();

	void SendGroupEvent(u32 group);

	void SetCurrentPokemon(u32 idx, u32 form);
	Pokemon* GetCurrentPokemon() { return currentPkm; }

	void SetCurrentItem(u32 idx);
	ItemData* GetCurrentItem() { return currentItem; }

	void SetCurrentMove(u32 idx);
	MoveData* GetCurrentMove() { return currentMove; }

	void AddMove();

protected:

	virtual ReturnState RenderGUI() override;

private:

	bool LoadTextFiles();
	u32 LoadDataNarc(const string& narcPath, string& outputPath);
	bool LoadPokemon(Pokemon& pkm);
	bool LoadPokemonData();
	bool LoadItemData();
	bool LoadMoveData();
	bool Start();

	bool LoadPersonal(PersonalData& personalData, const string& file);
	bool LoadLearnset(LearnsetData& learnData, const string& file);
	bool LoadEvolution(EvolutionData& evoData, const string& file);
	bool LoadChild(ChildData& childData, const string& file);
	bool LoadItem(ItemData& itemData, const string& file);
	bool LoadMove(MoveData& moveData, const string& file);

	vector<Module*> modules = vector<Module*>();
	vector<Event> reverseEvents = vector<Event>();
	vector<Event> saveEvents = vector<Event>();

	Pokemon* currentPkm = nullptr;
	ItemData* currentItem = nullptr;
	MoveData* currentMove = nullptr;

	friend class Module;
};

#define CTRMAP_FILESYSTEM_PATH "vfs\\data\\a"
#define CTRMAP_PATCHES_PATH "vfs\\data\\patches"
#define ROM_FILESYSTEM_PATH "data\\a"
#define FILESYSTEM_NAME "temp\\a"
#define KLANG_PATH "source\\settings.h"

#define TEXT_NARC_PATH "0\\0\\2"
#define PKM_NAME_FILE_ID 90
	#define PKM_FORM_NAME_FILE_ID 450
	#define PKM_NAME_COLOR_FILE_ID 483
	#define PKM_NAME_MAYUS_FILE_ID 486
	#define PKM_DESCRIPTION_FILE_ID 442
	#define PKM_TITLE_FILE_ID 464
#define MOVE_NAME_FILE_ID 403
	#define MOVE_NAME_MAYUS_FILE_ID 488
	#define MOVE_USE_FILE_ID 16
	#define MOVE_DESCRIPTION_FILE_ID 402
#define TYPE_NAME_FILE_ID 398
#define ABILITY_NAME_FILE_ID 374
#define ITEM_NAME_FILE_ID 64
	#define ITEM_DESCRIPTION_FILE_ID 63
	#define ITEM_NAME_COLOR_FILE_ID 481
	#define ITEM_NAME_PLURAL_FILE_ID 482

#define PERSONAL_NARC_PATH "0\\1\\6"
#define LEARNSET_NARC_PATH "0\\1\\8"
#define EVOLUTION_NARC_PATH "0\\1\\9"
#define CHILD_NARC_PATH "0\\2\\0"
#define MOVE_NARC_PATH "0\\2\\1"
#define ITEM_NARC_PATH "0\\2\\4"
#define MOVE_ANIM_NARC_PATH "0\\6\\5"

#define SEARCH_TEXT_SIZE 64

#define SIMPLE_REVERSE_EVENT(subType, value, newValue) engine->AddReverseEvent(moduleIdx, subType, value, newValue, false)
#define SAVE_CHECK_REVERSE_EVENT(subType, value, newValue, fileID, comparator) engine->AddReverseEvent(moduleIdx, subType, value, newValue, engine->AddSaveEvent(moduleIdx, fileID, comparator))

#endif // _ENGINE_H
