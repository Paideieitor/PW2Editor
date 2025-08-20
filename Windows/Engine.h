#ifndef _ENGINE_H
#define _ENGINE_H

#include <unordered_map>

#include "Utils/FileUtils.h"
#include "Utils/KlangFormat.h"

#include "Windows/Window.h"

#include "Data/PokemonData.h"
#include "Data/ItemData.h"
#include "Data/MoveData.h"
#include "Data/TrainerData.h"
#include "Data/ZoneData.h"
#include "Data/EncounterData.h"

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
#define ITEM_NAME_FILE_ID 64
	#define ITEM_DESCRIPTION_FILE_ID 63
	#define ITEM_NAME_COLOR_FILE_ID 481
	#define ITEM_NAME_PLURAL_FILE_ID 482
#define LOCATION_NAME_FILE_ID 109
#define TRAINER_NAME_FILE_ID 382
	#define TRAINER_CLASS_FILE_ID 383

#define TYPE_NAME_FILE_ID 398
#define ABILITY_NAME_FILE_ID 374
#define NATURE_FILE_ID 27

#define ALL_TEXT_FILES PKM_NAME_FILE_ID, PKM_FORM_NAME_FILE_ID, PKM_NAME_COLOR_FILE_ID, PKM_NAME_MAYUS_FILE_ID, \
PKM_DESCRIPTION_FILE_ID, PKM_TITLE_FILE_ID, MOVE_NAME_FILE_ID, MOVE_NAME_MAYUS_FILE_ID, MOVE_USE_FILE_ID, \
MOVE_DESCRIPTION_FILE_ID, ITEM_NAME_FILE_ID, ITEM_DESCRIPTION_FILE_ID, ITEM_NAME_COLOR_FILE_ID, \
ITEM_NAME_PLURAL_FILE_ID, LOCATION_NAME_FILE_ID, TRAINER_NAME_FILE_ID, TRAINER_CLASS_FILE_ID, TYPE_NAME_FILE_ID, \
ABILITY_NAME_FILE_ID, NATURE_FILE_ID,

class Module;

class Engine : public Window
{
public:

	Engine() = delete;
	Engine(Project* const project);
	~Engine();

	bool UsavedData();
	void Save();

	bool PMCCheck();
	bool PatchIsInstalled();

	bool PW2CodeCheck();

	void Quit(const string& msg = string());

	vector<string>* GetTextFile(u32 idx);

	u32 GetConcatDataCount(Data* data, u32 idx);
	u32 GetConcatDataIdx(Data* data, u32 idx, u32 concatIdx);

	bool StartAction(u32* selectable, u32 selected, u32 group);
	bool EndAction();

	int GetDataValue(Data* data, u32 idx, u32 field);
	u32 SetDataValue(Data* data, u32 idx, u32 field, int value);

	void InputInt(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, int max, int min);
	void ComboBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, const std::vector<std::string>* const list);
	void CheckBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label);
	void ListBox(Data* data, u32 idx, u32 firstField, u32* selectable, u32 selected, u32 group, const string& label, const vector<string>* const list);

	Project* const project;
	bool commandInput = false;

	PokemonData* pokemons;
	ItemData* items;
	MoveData* moves;
	TrainerData* trainers;
	ZoneData* zones;
	EncounterData* encounters;

protected:

	virtual ReturnState RenderGUI() override;

private:

	struct Action
	{
		Action() : selectable(nullptr), selected(0), group(0) {}
		Action(u32* selectable, u32 selected, u32 group) :
			selectable(selectable), selected(selected), group(group) {}
		Action(const Action& other) :
			selectable(other.selectable), selected(other.selected), group(other.group) {}
		const Action& operator=(const Action& other)
		{
			this->selectable = other.selectable;
			this->selected = other.selected;
			this->group = other.group;
			return *this;
		}

		u32* selectable;
		u32 selected;
		u32 group;
	};

	template<class T>
	void AddData(T*& data, const vector<string>& narcPaths)
	{
		data = new T;
		data->GenerateSections(narcPaths);
		datas.push_back(data);
	}

	bool LoadEngine();
	bool ClearEngine();

	bool SaveData();
	bool ReverseAction();
	bool ReloadData();

	void LoadPatchSettings();
	void InstallPatch(string settingsPath);
	void BuildPatch();
	void UninstallPatch();

	void DownloadPW2Code();
	void UpdatePW2Code();

	void MenuBar();

	bool quit = false;
	string quitMessage = string();

	unordered_map<u32, vector<string>> textFiles;

	vector<Data*> datas = vector<Data*>();
	vector<Module*> modules = vector<Module*>();

	vector<Action> actions = vector<Action>();
	int actionIdx = -1;

	bool enableBuilder = false;
	bool patcherOptions = false;
	Klang patchSettings;
};

#define SEARCH_TEXT_SIZE 64

#define SIMPLE_REVERSE_EVENT(subType, value, newValue) engine->AddReverseEvent(moduleIdx, subType, value, newValue, false)
#define SAVE_CHECK_REVERSE_EVENT(subType, value, newValue, fileID, comparator) engine->AddReverseEvent(moduleIdx, subType, value, newValue, engine->AddSaveEvent(moduleIdx, fileID, comparator))

#define PMC_CHECK_PATH "vfs\\overlay\\overlay_0344.bin"

#define DEV_TOOLS_DIR "..\\Tools"
#define DEV_BUILDER_DIR ".."
#define DEV_PATCH_DIR "..\\Code\\PW2Code"

#define DEPLOY_TOOLS_DIR "Patcher\\Tools"
#define DEPLOY_BUILDER_DIR "Patcher"
#define DEPLOY_PATCH_DIR "Patcher\\PW2Code"

#define PW2CODE_LINK "https://github.com/Paideieitor/PW2Code"

#define PATCH_BUILDER_FILE "PW2Builder.exe"

#define PATCH_SETTINGS_FILE "Builder\\buildSettings.txt"
#define PATCH_INSTALLED_FILE "Builder\\install.log"

#define CONSTRUCTOR_LOAD_QUIT "Engine could not load"
#define RELOAD_CLEAR_QUIT "Failed to clear data"
#define RELOAD_LOAD_QUIT "Failed to load data"

#ifdef _WIN32
#define TEMP_CMD "temp.bat"
#else
#define TEMP_CMD "temp.sh"
#endif

#endif // _ENGINE_H
