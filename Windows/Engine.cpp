#include <algorithm>

#include "System.h"

#include "Utils/FileUtils.h"
#include "Utils/NarcUtils.h"
#include "Utils/StringUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Module.h"
#include "Modules/List.h"
#include "Modules/Pokemon.h"
#include "Modules/Item.h"
#include "Modules/Move.h"
#include "Modules/Trainer.h"
#include "Modules/Encounter.h"

#define NARC_PATH_TEMPLATE(a, b, c) (string(a) + PATH_SEPARATOR + b + PATH_SEPARATOR + c)
#define TEXT_NARC_PATH NARC_PATH_TEMPLATE("0", "0", "2")

enum DataNarc
{
	POKEMON_NARCS,
	ITEM_NARCS,
	MOVE_NARCS,
	TRAINER_NARCS,
	ZONE_NARCS,
	ENCOUNTER_NARCS,
};
struct NarcPath
{
	vector<string> paths[2];
};
const vector<NarcPath> narcPaths = {
	// POKEMONS -> PERSONAL, LEARNSET, EVOLUTION, CHILD
	{ vector<string>({NARC_PATH_TEMPLATE("0", "1", "6"), NARC_PATH_TEMPLATE("0", "1", "8"), NARC_PATH_TEMPLATE("0", "1", "9"), NARC_PATH_TEMPLATE("0", "2", "0"), NARC_PATH_TEMPLATE("0", "2", "0")}), vector<string>({NARC_PATH_TEMPLATE("0", "1", "6"), NARC_PATH_TEMPLATE("0", "1", "8"), NARC_PATH_TEMPLATE("0", "1", "9"), NARC_PATH_TEMPLATE("0", "2", "0"), NARC_PATH_TEMPLATE("0", "2", "0")}) },
	// ITEMS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "2", "4")}) , vector<string>({NARC_PATH_TEMPLATE("0", "2", "4")}) },
	// MOVES -> DATA, ANIMATIONS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "2", "1"), NARC_PATH_TEMPLATE("0", "6", "6")}) , vector<string>({NARC_PATH_TEMPLATE("0", "2", "1"), NARC_PATH_TEMPLATE("0", "6", "5")}) },
	// TRAINERS -> DATA, TEAMS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "9", "2"), NARC_PATH_TEMPLATE("0", "9", "3")}) , vector<string>({NARC_PATH_TEMPLATE("0", "9", "1"), NARC_PATH_TEMPLATE("0", "9", "2")}) },
	// ZONES
	{ vector<string>({NARC_PATH_TEMPLATE("0", "1", "2")}) , vector<string>({NARC_PATH_TEMPLATE("0", "1", "2")}) },
	// ENCOUNTERS
	{ vector<string>({NARC_PATH_TEMPLATE("1", "2", "6")}) , vector<string>({NARC_PATH_TEMPLATE("1", "2", "7")}) },
};

const vector<string> alternateVersions = {
	"POKEMON B2",
	"POKEMON W2",
};

const vector<string> validGameCodes = {
	"IRDO",
};

Engine::Engine(Project* const project) : project(project)
{
	Log(INFO, "Constructing engine");

	if (!LoadEngine())
	{
		Quit(CONSTRUCTOR_LOAD_QUIT);
	}

	Log(INFO, "    Constructing success");
}

Engine::~Engine()
{
	Log(INFO, "Destroying engine");

	ClearEngine();

	Log(INFO, "    Destroying success");
}

bool Engine::UsavedData()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
	{
		Data* data = datas[idx];
		if (data->HasSaveEvents())
			return true;
	}
	return false;
}

void Engine::Save()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
	{
		Data* data = datas[idx];
		data->Save(project);
	}

	SaveProjectSettings(*project);
}

bool Engine::PMCCheck()
{
	if (!enableBuilder)
		return false;
	return PathExists(PathConcat(project->ctrMapProjectDir, PMC_CHECK_PATH));
}

bool Engine::PatchIsInstalled()
{
	if (!PMCCheck())
		return false;
	return PathExists(PathConcat(project->path, PATCH_INSTALLED_FILE));
}

bool Engine::PW2CodeCheck()
{
#if _DEBUG
	return true;
#endif
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		return false;

	return PathExists(PathConcat(patchPath, ".git"));
}

void Engine::Quit(const string& msg)
{
	quit = true;
	quitMessage = msg;
}

vector<string>* Engine::GetTextFile(u32 idx)
{
	unordered_map<u32, vector<string>>::iterator itr = textFiles.find(idx);
	if (itr == textFiles.end())
	{
		Log(CRITICAL, "Text file %d is not loaded", idx);
		return nullptr;
	}
	return &(itr->second);
}

u32 Engine::GetConcatDataCount(Data* data, u32 idx)
{
	return data->GetCount(project, idx);
}

u32 Engine::GetConcatDataIdx(Data* data, u32 idx, u32 concatIdx)
{
	return data->GetConcat(idx, concatIdx);
}

bool Engine::StartAction(u32* selectable, u32 selected, u32 group)
{
	if (actionIdx != -1)
	{
		Log(CRITICAL, "Can't start an action if there is already one in progress");
		return false;
	}

	actions.emplace_back(selectable, selected, group);
	actionIdx = (int)actions.size() - 1;
	return true;
}

bool Engine::EndAction()
{
	if (actionIdx == -1)
	{
		Log(CRITICAL, "Can't end an action if there is none in progress");
		return false;
	}
	actionIdx = -1;
	return true;
}

int Engine::GetDataValue(Data* data, u32 idx, u32 field)
{
	return data->GetValue(project, idx, field);
}

u32 Engine::SetDataValue(Data* data, u32 idx, u32 field, int value)
{
	if (actionIdx == -1)
	{
		Log(CRITICAL, "Can't set a data value outside of an action");
		return field;
	}

	return data->SetValue(idx, field, value, (u32)actionIdx);
}

void Engine::InputInt(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, int max, int min)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (ImGui::Button(LABEL(label + "Null", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, min);
			EndAction();
		}
		return;
	}

	if (value > max || value < min)
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (ImGui::Button(LABEL("Fix", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, min);
			EndAction();
		}
		return;
	}

	ImGui::SetNextItemWidth(40.0f);

	if (ImGui::InputInt(LABEL(label, (int)field), &value, 0) && ImGui::IsItemDeactivatedAfterEdit())
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;

		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, value);
		EndAction();
	}
}

void Engine::ComboBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, const std::vector<std::string>* const list)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (list && !list->empty())
		{
			if (ImGui::Button(LABEL(label + "Null", (int)field)))
			{
				StartAction(selectable, selected, group);
				SetDataValue(data, idx, field, 0);
				EndAction();
			}
		}
		else
		{
			ImGui::Text("No values");
		}
		return;
	}

	if (!list || value < 0 || value >= (int)list->size())
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (list && !list->empty())
		{
			if (ImGui::Button(LABEL("Fix", (int)field)))
			{
				StartAction(selectable, selected, group);
				SetDataValue(data, idx, field, 0);
				EndAction();
			}
		}
		else
		{
			ImGui::Text("No values");
		}
		
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(LABEL(label, (int)field), list, &value))
	{
		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, value);
		EndAction();
	}
}

void Engine::CheckBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (ImGui::Button(LABEL(label + "Null", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, 0);
			EndAction();
		}
		return;
	}

	if (value > 1 || value < 0)
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (ImGui::Button(LABEL("Fix", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, 0);
			EndAction();
		}
		return;
	}

	bool baalue = (bool)value; // Get it?
	if (ImGui::Checkbox(LABEL(label, (int)field), &baalue))
	{
		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, (int)baalue);
		EndAction();
	}
}

void Engine::ListBox(Data* data, u32 idx, u32 firstField, u32* selectable, u32 selected, u32 group, const string& label, const vector<string>* const list)
{
	ImGui::Text(label);
	if (ImGui::BeginListBox(string("##" + label).c_str(), ImVec2(150.0f, 200.0f)))
	{
		if (!list)
		{
			ImGui::Text("(Error): No values");
		}
		else
		{
			for (u32 listIdx = 0; listIdx < (u32)list->size(); ++listIdx)
				CheckBox(data, idx, firstField + listIdx, selectable, selected, group, list->at(listIdx));
		}
		ImGui::EndListBox();
	}
}

ReturnState Engine::RenderGUI()
{
	if (quit)
	{
		if (quitMessage.empty())
			return STOP;

		ReturnState exitState = OK;
		ImGui::Begin("The engine needs to shut down");
		ImGui::Text(quitMessage);
		if (ImGui::Button("OK"))
			exitState = STOP;
		ImGui::End();
		return exitState;
	}

	project->width = (u32)width;
	project->height = (u32)height;

	static bool commandInput = false;
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		commandInput = true;
	if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
		commandInput = false;

	if (commandInput)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_S))
			Save();

		if (ImGui::IsKeyPressed(ImGuiKey_R))
		{
			Save();
			ReloadData();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Z))
			ReverseAction();

		for (u32 group = POKEMON_GROUP; group <= ENCOUNTER_GROUP; ++group)
			if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_0 + group)))
				project->group = group;
	}
	MenuBar();

	ReturnState moduleState = OK;
	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
	{
		if (modules[idx]->group == ENGINE_GROUP ||
			modules[idx]->group == project->group)
		{
			moduleState = modules[idx]->RenderGUI();
			if (moduleState != OK)
				break;
		}
	}

	return moduleState;
}

bool Engine::LoadEngine()
{
	Log(INFO, "Loading engine");

	// Only enable the Code Injection for sopported game codebases
	for (u32 codeIdx = 0; codeIdx < (u32)validGameCodes.size(); ++codeIdx)
	{
		if (project->gameCode == validGameCodes.at(codeIdx))
		{
			enableBuilder = true;
			break;
		}
	}

	if (PW2CodeCheck())
	{
		LoadPatchSettings();
	}

	const vector<u32> textFileIdexes = {
	ALL_TEXT_FILES
	};
	for (u32 textIdx = 0; textIdx < (u32)textFileIdexes.size(); ++textIdx)
	{
		u32 idx = textFileIdexes[textIdx];
		FileStream stream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, TEXT_NARC_PATH, idx);
		if (!stream.data)
		{
			Log(INFO, "    Failed loading text NARC file %d", idx);
			return false;
		}

		vector<string> text;
		LoadAlle5Data(stream, text);
		if (text.empty())
		{
			Log(INFO, "    Failed loading text data %d", idx);
			ReleaseFileStream(stream);
			return false;
		}
		ReleaseFileStream(stream);

		textFiles[idx] = text;
	}

	u32 gameVersion = 0;
	for (u32 versionIdx = 0; versionIdx < (u32)alternateVersions.size(); ++versionIdx)
	{
		if (project->game == alternateVersions.at(versionIdx))
		{
			gameVersion = 1;
			break;
		}
	}

	Log(INFO, "    Loading Data");
	AddData<PokemonData>(pokemons, narcPaths.at(POKEMON_NARCS).paths[gameVersion]);
	AddData<ItemData>(items, narcPaths.at(ITEM_NARCS).paths[gameVersion]);
	AddData<MoveData>(moves, narcPaths.at(MOVE_NARCS).paths[gameVersion]);
	AddData<TrainerData>(trainers, narcPaths.at(TRAINER_NARCS).paths[gameVersion]);
	AddData<ZoneData>(zones, narcPaths.at(ZONE_NARCS).paths[gameVersion]);
	AddData<EncounterData>(encounters, narcPaths.at(ENCOUNTER_NARCS).paths[gameVersion]);

	Log(INFO, "    Setting up PW2Code");
	string buildSettingsPath = PathConcat(project->path, PATCH_SETTINGS_FILE);
	if (!PathExists(buildSettingsPath))
		InstallPatch(buildSettingsPath);

	Log(INFO, "    Creating modules");
	modules.push_back(new List(this, POKEMON_GROUP, &project->pokemon, GetTextFile(PKM_NAME_FILE_ID)));
	modules.push_back(new Pokemon(this, POKEMON_GROUP));

	modules.push_back(new List(this, ITEM_GROUP, &project->item, GetTextFile(ITEM_NAME_FILE_ID)));
	modules.push_back(new Item(this, ITEM_GROUP));

	modules.push_back(new List(this, MOVE_GROUP, &project->move, GetTextFile(MOVE_NAME_FILE_ID)));
	modules.push_back(new Move(this, MOVE_GROUP));

	modules.push_back(new List(this, TRAINER_GROUP, &project->trainer, GetTextFile(TRAINER_NAME_FILE_ID)));
	modules.push_back(new Trainer(this, TRAINER_GROUP));

	modules.push_back(new List(this, ENCOUNTER_GROUP, &project->location, GetTextFile(LOCATION_NAME_FILE_ID)));
	modules.push_back(new Encounter(this, ENCOUNTER_GROUP));

	Log(INFO, "    Loading success");
	return true;
}

bool Engine::ClearEngine()
{
	Log(INFO, "Clearing engine");

	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		delete datas[idx];
	datas.clear();

	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
		delete modules[idx];
	modules.clear();

	Log(INFO, "    Clearing success");
	return true;
}

bool Engine::SaveData()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		datas.at(idx)->Save(project);

	return true;
}

bool Engine::ReverseAction()
{
	if (actions.empty())
		return false;

	const Action& action = actions.back();
	*action.selectable = action.selected;
	project->group = action.group;

	const u32 actionIdx = (u32)actions.size() - 1;
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		datas.at(idx)->ReverseAction(actionIdx);
	actions.erase(actions.begin() + actionIdx);

	return true;
}

bool Engine::ReloadData()
{
	if (!ClearEngine())
	{
		Quit(RELOAD_CLEAR_QUIT);
		return false;
	}
	if (!LoadEngine())
	{
		Quit(RELOAD_LOAD_QUIT);
		return false;
	}
	return true;
}

void Engine::LoadPatchSettings()
{
	string patchSettingsPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchSettingsPath))
		patchSettingsPath = DEV_PATCH_DIR;

	patchSettingsPath = PathConcat(patchSettingsPath, "settings.h");
	LoadKlang(patchSettings, patchSettingsPath);
}

void Engine::InstallPatch(string settingsPath)
{
	string toolsDir = DEPLOY_TOOLS_DIR;
	if (!PathExists(toolsDir))
		toolsDir = DEV_TOOLS_DIR;

	string buildSettings = "SET PROJECT_DIR=";
	buildSettings += project->ctrMapProjectDir + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET CTRMAP_DIR=";
	buildSettings += toolsDir + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET ARM_NONE_EABI_DIR=";
	buildSettings += PathConcat(toolsDir, string("Arm-None-Eabi") + PATH_SEPARATOR + "bin" + PATH_SEPARATOR);
	buildSettings += '\n';

	buildSettings += "SET JAVA_DIR=";

	FileStream file;
	LoadEmptyFileStream(file);

	FileStreamBufferWriteBack(file, (u8*)buildSettings.c_str(), (u32)buildSettings.size());

	SaveFileStream(file, settingsPath);
}

string GetBuilderPath()
{
	string builderPath = DEPLOY_BUILDER_DIR;
	if (PathExists(builderPath))
		return builderPath;

	builderPath = DEV_BUILDER_DIR;
	builderPath = PathConcat(builderPath, "x64");
	builderPath = PathConcat(builderPath, "Release");
	return builderPath;
}

void Engine::BuildPatch()
{
	string builderPath = GetBuilderPath();
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		patchPath = DEV_PATCH_DIR;

	string command = PATH_FORMAT(PathConcat(builderPath, PATCH_BUILDER_FILE)) + " ";
	command += "-rebuild ";
	command += PATH_FORMAT(patchPath) + " ";
	command += "-whitelist-all ";
	command += "-custom-build ";
	command += PATH_FORMAT(project->path) + " ";
	command += "-pause";

	system(PATH_FORMAT(command).c_str());
}

void Engine::UninstallPatch()
{
	string builderPath = GetBuilderPath();
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		patchPath = DEV_PATCH_DIR;

	string command = PATH_FORMAT(PathConcat(builderPath, "PW2Builder.exe")) + " ";
	command += "-uninstall ";
	command += PATH_FORMAT(patchPath) + " ";
	command += "-custom-build ";
	command += PATH_FORMAT(project->path) + " ";
	command += "-keep-settings ";
	command += "-pause";

	system(PATH_FORMAT(command).c_str());
}

void LaunchCommand(const string& command)
{
	string buildScript = "@ECHO OFF\n";
	buildScript += command;
	buildScript += "\nPAUSE";

	FileStream fileStream;
	LoadEmptyFileStream(fileStream);
	FileStreamBufferWriteBack(fileStream, (u8*)buildScript.c_str(), (u32)buildScript.size());
	SaveFileStream(fileStream, TEMP_CMD);
	ReleaseFileStream(fileStream);

	system(TEMP_CMD);
}

void Engine::DownloadPW2Code()
{
	string patchPath = DEPLOY_PATCH_DIR;

	string command = "git clone --recursive ";
	command += PW2CODE_LINK;
	command += " " + PATH_FORMAT(patchPath);

	LaunchCommand(command);

	LoadPatchSettings();
}

void Engine::UpdatePW2Code()
{
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		return;

	string command = "cd ";
	command += PATH_FORMAT(patchPath) + '\n';
	command += "git fetch origin";
	command += '\n';
	command += "git reset --hard origin/main";

	LaunchCommand(command);

	LoadPatchSettings();
}

void PatchSetting(KlangVar& var, bool isChild)
{
	string text;
	if (isChild)
		text += "    ";
	text += var.name;

	ImGui::Text(text);
	ImGui::SameLine();

	bool value = (bool)var.Value();
	if (ImGui::Checkbox((string("##") + var.name).c_str(), &value))
		var.SetValue((int)value);
}

void Engine::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			ImGui::BeginDisabled(!UsavedData());
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				Save();
			ImGui::EndDisabled();

			if (ImGui::MenuItem("Save & Reload", "Ctrl+R"))
			{
				Save();
				ReloadData();
			}

			if (ImGui::MenuItem("Save & Exit"))
			{
				Save();
				Quit();
			}

			if (ImGui::MenuItem("Undo", "Ctrl+Z"))
				ReverseAction();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Editors"))
		{
			if (ImGui::MenuItem(u8"Pokémon", "Ctrl+1"))
				project->group = POKEMON_GROUP;
			if (ImGui::MenuItem("Items", "Ctrl+2"))
				project->group = ITEM_GROUP;
			if (ImGui::MenuItem("Moves", "Ctrl+3"))
				project->group = MOVE_GROUP;
			if (ImGui::MenuItem("Trainers", "Ctrl+4"))
				project->group = TRAINER_GROUP;
			if (ImGui::MenuItem("Encounters", "Ctrl+5"))
				project->group = ENCOUNTER_GROUP;
			ImGui::EndMenu();
		}

		ImGui::BeginDisabled(!enableBuilder || !PMCCheck());
		if (ImGui::BeginMenu("Patcher"))
		{
			if (!PW2CodeCheck())
			{
				if (ImGui::MenuItem("Download PW2Code"))
					DownloadPW2Code();
			}
			else
			{

				if (ImGui::MenuItem("Update PW2Code"))
					UpdatePW2Code();

				if (ImGui::MenuItem("Options"))
					patcherOptions = !patcherOptions;

				bool patchInstalled = PatchIsInstalled();

				string buildText = "Save + Build";
				if (patchInstalled)
					buildText = "Save + Rebuild";
				if (ImGui::MenuItem(buildText.c_str()))
				{
					Save();
					BuildPatch();
					ReloadData();
				}

				ImGui::BeginDisabled(!patchInstalled);
				if (ImGui::MenuItem("Save + Uninstall"))
				{
					Save();
					UninstallPatch();
					ReloadData();
				}
				ImGui::EndDisabled();
			}
			ImGui::EndMenu();
		}
		ImGui::EndDisabled();

		ImGui::EndMainMenuBar();
	}

	if (patcherOptions)
	{
		ImGui::Begin("Patcher Options", &patcherOptions);

		for (u32 setting = 0; setting < (u32)patchSettings.vars.size(); ++setting)
		{
			KlangVar& parent = patchSettings.vars[setting];
			PatchSetting(parent, false);

			ImGui::BeginDisabled(parent.Value() == 0);
			for (u32 child = 0; child < (u32)parent.dependentVars.size(); ++child)
			{
				KlangVar& dependent = parent.dependentVars[child];
				PatchSetting(dependent, true);
			}
			ImGui::EndDisabled();

			ImGui::Separator();
		}

		ImGui::End();
	}
}
