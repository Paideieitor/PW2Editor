#include <algorithm>

#include "System.h"

#include "Utils/FileUtils.h"
#include "Utils/NarcUtils.h"
#include "Utils/StringUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Module.h"
#include "Modules/MenuBar.h"
#include "Modules/Pokemon/PokemonSearch.h"
#include "Modules/Pokemon/PokemonForm.h"
#include "Modules/Pokemon/PokemonText.h"
#include "Modules/Pokemon/Personal.h"
#include "Modules/Pokemon/Learnset.h"
#include "Modules/Pokemon/Evolution.h"
#include "Modules/Pokemon/Child.h"
#include "Modules/Item/ItemSearch.h"
#include "Modules/Item/Item.h"
#include "Modules/Item/ItemText.h"
#include "Modules/Move/MoveSearch.h"
#include "Modules/Move/Move.h"
#include "Modules/Move/MoveText.h"

#include "Data/Project.h"

Engine::Engine(Project* const project) : project(project)
{
	string fsPath = project->path + PATH_SEPARATOR + FILESYSTEM_NAME;
	if (PathExists(fsPath) && 
		RemoveFolder(fsPath) == -1)
	{
		Log(CRITICAL, "Error removing temp forlder!");
		return;
	}

	if (!Start())
	{
		Log(CRITICAL, "Error starting Engine!");
		return;
	}

	string buildSettingsPath = ConcatPath(project->path, PATCH_SETTINGS_FILE);
	if (!PathExists(buildSettingsPath))
		InstallPatch(buildSettingsPath);

	// Engine
	modules.emplace_back(new MenuBar(this, ENGINE_GROUP));
	// Pokémon
	modules.emplace_back(new PokemonSearch(this, POKEMON_GROUP));
	modules.emplace_back(new PokemonForm(this, POKEMON_GROUP));
	modules.emplace_back(new PokemonText(this, POKEMON_GROUP));
	modules.emplace_back(new Personal(this, POKEMON_GROUP));
	modules.emplace_back(new Learnset(this, POKEMON_GROUP));
	modules.emplace_back(new Evolution(this, POKEMON_GROUP));
	modules.emplace_back(new Child(this, POKEMON_GROUP));
	// Items
	modules.emplace_back(new ItemSearch(this, ITEM_GROUP));
	modules.emplace_back(new Item(this, ITEM_GROUP));
	modules.emplace_back(new ItemText(this, ITEM_GROUP));
	// Moves
	modules.emplace_back(new MoveSearch(this, MOVE_GROUP));
	modules.emplace_back(new Move(this, MOVE_GROUP));
	modules.emplace_back(new MoveText(this, MOVE_GROUP));
}

Engine::~Engine()
{
	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
		delete modules[idx];

	for (u32 idx = 0; idx < (u32)reverseEvents.size(); ++idx)
		delete reverseEvents[idx].value;
}

ReturnState Engine::RenderGUI()
{
	if (!quit.empty())
	{
		ReturnState exitState = OK;
		ImGui::Begin("The engine needs to shut down");
		ImGui::Text(quit);
		if (ImGui::Button("OK"))
			exitState = STOP;
		ImGui::End();
		return exitState;
	}

	if (GetCurrentPokemon() == nullptr)
		return ERROR;

	project->width = (u32)width;
	project->height = (u32)height;

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

void Engine::SetCurrentPokemon(u32 idx, u32 form)
{
	if (project->selectedPkmIdx != idx)
		form = 0;

	project->selectedPkmIdx = idx;
	project->selectedPkmForm = form;

	Pokemon* pkm = &(pokemon[idx]);
	if (project->selectedPkmForm > 0)
		pkm = &(pkm->forms[form - 1]);
	currentPkm = pkm;
}

void Engine::SetCurrentItem(u32 idx)
{
	project->selectedItemIdx = idx;
	currentItem = &(items[idx]);
}

void Engine::SetCurrentMove(u32 idx)
{
	project->selectedMoveIdx = idx;
	currentMove = &(moves[idx]);
}

void Engine::AddMove()
{
	moveNames.emplace_back(string("Move ") + to_string(moveNames.size()));
	u32 moveIdx = (u32)moveNames.size() - 1;

	if (moveIdx >= (u32)moves.size())
		moves.resize(moveIdx + 1);
	MoveData& moveData = moves[moveIdx];
	for (u32 idx = 0; idx < (u32)moveData.size(); ++idx)
		moveData[idx] = 0;

	if (moveIdx >= (u32)moveNamesMayus.size())
		moveNamesMayus.resize(moveIdx + 1);
	moveNamesMayus[moveIdx] = UpperCase(moveNames[moveIdx]);

	if (moveIdx * 3 >= (u32)moveUses.size())
		moveUses.resize((moveIdx + 1) * 3);
	string battlePrefix[] = { "", "The wild ", "The foe's " };
	string battleMessage;
	battleMessage.reserve(64);
	for (u32 idx = 0; idx < 3; ++idx)
	{
		battleMessage.clear();
		battleMessage = battlePrefix[idx] + u8"[Pokémon Nick(0)] used\n" + moveNames[moveIdx] + "!";
		moveUses[(moveIdx * 3) + idx] = battleMessage;
	}

	if (moveIdx >= (u32)moveDescriptions.size())
		moveDescriptions.resize((moveIdx + 1));
	moveDescriptions[moveIdx] = "";

	if (moveIdx >= (u32)moveAnims.size())
		moveAnims.resize((moveIdx + 1));
	LoadFileStream(moveAnims[moveIdx], MAKE_FILE_PATH(moveAnimPath, 1));

	SendGroupEvent(MOVE_GROUP);
}

void Engine::InstallPatch(string settingsPath)
{
	string toolsDir = DEPLOY_TOOLS_DIR;
	if (!PathExists(toolsDir))
		toolsDir = DEV_TOOLS_DIR;

	string buildSettings = "SET PROJECT_DIR=";
	buildSettings += project->ctrMapProjectPath + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET CTRMAP_DIR=";
	buildSettings += toolsDir + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET ARM_NONE_EABI_DIR=";
	buildSettings += ConcatPath(toolsDir, "Arm-None-Eabi\\bin\\");
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
#ifdef _WIN64
	builderPath = ConcatPath(builderPath, "x64");
#else
	builderPath = ConcatPath(builderPath, "x86");
#endif
	builderPath = ConcatPath(builderPath, "Release");
	return builderPath;
}

void Engine::BuildPatch()
{
	string builderPath = GetBuilderPath();
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		patchPath = DEV_PATCH_DIR;

	string command = PATH_FORMAT(ConcatPath(builderPath, PATCH_BUILDER_FILE)) + " ";
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

	string command = PATH_FORMAT(ConcatPath(builderPath, "PW2Builder.exe")) + " ";
	command += "-uninstall ";
	command += PATH_FORMAT(patchPath) + " ";
	command += "-custom-build ";
	command += PATH_FORMAT(project->path) + " ";
	command += "-keep-settings ";
	command += "-pause";

	system(PATH_FORMAT(command).c_str());
}

void LoadText(const string& narcPath, u32 fileID, const string& outputPath, vector<u16>& missingTextFiles)
{
	string ctrPath = MAKE_FILE_PATH(narcPath, fileID);
	if (!PathExists(ctrPath))
		missingTextFiles.emplace_back(fileID);
	else
		CopyFile(ctrPath, outputPath);
}
#define LOAD_TEXT(fileID, outputPath) LoadText(ctrTextNarcPath, fileID, outputPath, missingTextFiles)

bool Engine::LoadTextFiles()
{
	vector<u16> missingTextFiles;

	// The path where the text files are stored in the CTRMap project
	string ctrTextNarcPath = project->ctrMapProjectPath + PATH_SEPARATOR + CTRMAP_FILESYSTEM_PATH + PATH_SEPARATOR + TEXT_NARC_PATH;
	// The path to the text NARC file in the extracted ROM files
	string romTextNarcPath = project->romPath + PATH_SEPARATOR + ROM_FILESYSTEM_PATH + PATH_SEPARATOR + TEXT_NARC_PATH;
	// Create the path where the text files will be stored
	string textNarcPath = project->path + PATH_SEPARATOR + FILESYSTEM_NAME + PATH_SEPARATOR + TEXT_NARC_PATH;

	// If the file is not extracted in the CTRMap project
	// add all the needed text files to the NARC extraction list
	if (!PathExists(ctrTextNarcPath))
	{
		if (!PathExists(romTextNarcPath) || !IsFilePath(romTextNarcPath))
		{
			Log(CRITICAL, "Couldn't load project %s, game text NARC not found", project->name.c_str());
			return false;
		}
		
		missingTextFiles.emplace_back(PKM_NAME_FILE_ID);
		missingTextFiles.emplace_back(PKM_FORM_NAME_FILE_ID);
		missingTextFiles.emplace_back(PKM_DESCRIPTION_FILE_ID);
		missingTextFiles.emplace_back(PKM_TITLE_FILE_ID);

		missingTextFiles.emplace_back(TYPE_NAME_FILE_ID);

		missingTextFiles.emplace_back(ABILITY_NAME_FILE_ID);

		missingTextFiles.emplace_back(ITEM_NAME_FILE_ID);
		missingTextFiles.emplace_back(ITEM_DESCRIPTION_FILE_ID);
		missingTextFiles.emplace_back(ITEM_NAME_COLOR_FILE_ID);
		missingTextFiles.emplace_back(ITEM_NAME_PLURAL_FILE_ID);

		missingTextFiles.emplace_back(MOVE_NAME_FILE_ID);
		missingTextFiles.emplace_back(MOVE_NAME_MAYUS_FILE_ID);
		missingTextFiles.emplace_back(MOVE_USE_FILE_ID);
		missingTextFiles.emplace_back(MOVE_DESCRIPTION_FILE_ID);
	}

	// The file where the Pokémon text data is stored
	string pkmNamePath = MAKE_FILE_PATH(textNarcPath, PKM_NAME_FILE_ID);
	string formNamePath = MAKE_FILE_PATH(textNarcPath, PKM_FORM_NAME_FILE_ID);
	string pkmDescriptionPath = MAKE_FILE_PATH(textNarcPath, PKM_DESCRIPTION_FILE_ID);
	string pkmTitlePath = MAKE_FILE_PATH(textNarcPath, PKM_TITLE_FILE_ID);

	// The file where the Type text data is stored
	string typeNamePath = MAKE_FILE_PATH(textNarcPath, TYPE_NAME_FILE_ID);

	// The file where the Abilities text data is stored
	string abilityNamePath = MAKE_FILE_PATH(textNarcPath, ABILITY_NAME_FILE_ID);

	// The file where the Item text data is stored
	string itemNamePath = MAKE_FILE_PATH(textNarcPath, ITEM_NAME_FILE_ID);
	string itemDescriptionPath = MAKE_FILE_PATH(textNarcPath, ITEM_DESCRIPTION_FILE_ID);
	string itemColorPath = MAKE_FILE_PATH(textNarcPath, ITEM_NAME_COLOR_FILE_ID);
	string itemPluralPath = MAKE_FILE_PATH(textNarcPath, ITEM_NAME_PLURAL_FILE_ID);

	// The file where the Move text data is stored
	string moveNamePath = MAKE_FILE_PATH(textNarcPath, MOVE_NAME_FILE_ID);
	string moveNameMayusPath = MAKE_FILE_PATH(textNarcPath, MOVE_NAME_MAYUS_FILE_ID);
	string moveUsePath = MAKE_FILE_PATH(textNarcPath, MOVE_USE_FILE_ID);
	string moveDescriptionPath = MAKE_FILE_PATH(textNarcPath, MOVE_DESCRIPTION_FILE_ID);

	// If the missing files are not already set
	if (!missingTextFiles.size())
	{
		// Check if the files we need are already extracted in the CTRMap project
		// If they are not, add them to the missing files list
		LOAD_TEXT(PKM_NAME_FILE_ID, pkmNamePath);
		LOAD_TEXT(PKM_FORM_NAME_FILE_ID, formNamePath);
		LOAD_TEXT(PKM_DESCRIPTION_FILE_ID, pkmDescriptionPath);
		LOAD_TEXT(PKM_TITLE_FILE_ID, pkmTitlePath);

		LOAD_TEXT(TYPE_NAME_FILE_ID, typeNamePath);

		LOAD_TEXT(ABILITY_NAME_FILE_ID, abilityNamePath);

		LOAD_TEXT(ITEM_NAME_FILE_ID, itemNamePath);
		LOAD_TEXT(ITEM_DESCRIPTION_FILE_ID, itemDescriptionPath);
		LOAD_TEXT(ITEM_NAME_COLOR_FILE_ID, itemColorPath);
		LOAD_TEXT(ITEM_NAME_PLURAL_FILE_ID, itemPluralPath);

		LOAD_TEXT(MOVE_NAME_FILE_ID, moveNamePath);
		LOAD_TEXT(MOVE_NAME_MAYUS_FILE_ID, moveNameMayusPath);
		LOAD_TEXT(MOVE_USE_FILE_ID, moveUsePath);
		LOAD_TEXT(MOVE_DESCRIPTION_FILE_ID, moveDescriptionPath);
	}

	// Extract the files we are missing
	if (missingTextFiles.size())
	{
		if (!NarcUnpackBundle(romTextNarcPath, textNarcPath, missingTextFiles))
		{
			Log(CRITICAL, "Couldn't load project %s, game text NARC not found", project->name.c_str());
			return false;
		}
	}

	// Load relevant Pokémon text data
	LoadAlle5File(pkmNamePath, pkmNames);
	LoadAlle5File(formNamePath, formNames);
	LoadAlle5File(pkmDescriptionPath, pkmDescriptions);
	LoadAlle5File(pkmTitlePath, pkmTitles);
	// Load relevant Type text data
	LoadAlle5File(typeNamePath, types);
	// Load relevant Ability text data
	LoadAlle5File(abilityNamePath, abilities);
	// Load relevant Item text data
	LoadAlle5File(itemNamePath, itemNames);
	LoadAlle5File(itemDescriptionPath, itemDescriptions);
	LoadAlle5File(itemColorPath, itemColors);
	LoadAlle5File(itemPluralPath, itemPlurals);
	// Load relevant Move text data
	LoadAlle5File(moveNamePath, moveNames);
	LoadAlle5File(moveNameMayusPath, moveNamesMayus);
	LoadAlle5File(moveUsePath, moveUses);
	LoadAlle5File(moveDescriptionPath, moveDescriptions);

	return true;
}

u32 Engine::LoadDataNarc(const string& narcPath, string& outputPath)
{
	vector<u16> excludeFiles;

	// The path where the files are stored in the CTRMap project
	string ctrNarcPath = project->ctrMapProjectPath + PATH_SEPARATOR + CTRMAP_FILESYSTEM_PATH + PATH_SEPARATOR + narcPath;
	// The path to the NARC file in the extracted ROM
	string romNarcPath = project->romPath + PATH_SEPARATOR + ROM_FILESYSTEM_PATH + PATH_SEPARATOR + narcPath;
	// Create the path where the files will be stored
	outputPath = project->path + PATH_SEPARATOR + FILESYSTEM_NAME + PATH_SEPARATOR + narcPath;

	// If the NARC is partially extracted in the CTRMap project move
	// the extracted files and exclude them from the unpacking
	if (PathExists(ctrNarcPath))
	{
		vector<string> ctrFiles = GetFolderElementList(ctrNarcPath);
		for (u16 ctrIdx = 0; ctrIdx < ctrFiles.size(); ++ctrIdx)
		{
			string ctrFilePath = ctrNarcPath + PATH_SEPARATOR + ctrFiles[ctrIdx];
			if (!PathExists(ctrFilePath) || !IsFilePath(ctrFilePath))
			{
				Log(WARNING, "Couldn't find file %s in %s NARC in CTRMap", ctrFiles[ctrIdx].c_str(), narcPath.c_str());
				continue;
			}
			string engineFilePath = outputPath + PATH_SEPARATOR + ctrFiles[ctrIdx];
			// Copy the already extracted files from the CTRMap project
			if (!CopyFile(ctrFilePath, engineFilePath))
			{
				Log(WARNING, "Couldn't copy %s file in %s NARC", ctrFiles[ctrIdx].c_str(), narcPath.c_str());
				continue;
			}

			excludeFiles.emplace_back((u16)stoi(ctrFiles[ctrIdx]));
		}
	}

	// Create the NARC folder if necesary
	if (!PathExists(outputPath) && !CreateFolder(outputPath))
	{
		Log(CRITICAL, "Couldn't create %s NARC folder", narcPath.c_str());
		return 0;
	}

	if (!PathExists(romNarcPath) || !IsFilePath(romNarcPath))
	{
		Log(CRITICAL, "Couldn't load project %s, game text NARC not found", project->name.c_str());
		return 0;
	}

	// Extract the remaining files (excluding the ones found in the CTRMap project)
	int extractedFiles = NarcUnpackExclude(romNarcPath, outputPath, excludeFiles);
	if (extractedFiles < 0)
	{
		Log(CRITICAL, "Couldn't load project %s, data NARC not found", project->name.c_str());
		return 0;
	}

	return (u32)extractedFiles + (u32)excludeFiles.size();
}

bool Engine::LoadPokemon(Pokemon& pkm)
{
	string personalFilePath = MAKE_FILE_PATH(personalPath, pkm.dataIdx);
	if (!LoadPersonal(personal[pkm.dataIdx], personalFilePath))
	{
		Log(CRITICAL, u8"Pokémon %d lacks personal data", pkm.dataIdx);
		return false;
	}
	string learnsetFilePath = MAKE_FILE_PATH(learnsetPath, pkm.dataIdx);
	if (!LoadLearnset(learnset[pkm.dataIdx], learnsetFilePath))
	{
		Log(CRITICAL, u8"Pokémon %d lacks learnset data", pkm.dataIdx);
		return false;
	}
	string evolutionFilePath = MAKE_FILE_PATH(evolutionPath, pkm.dataIdx);
	if (!LoadEvolution(evolution[pkm.dataIdx], evolutionFilePath))
	{
		Log(CRITICAL, u8"Pokémon %d lacks evolution data", pkm.dataIdx);
		return false;
	}

	// Only non-PokéStar and non-Form Pokémon have a child file
	if (!pkm.pokeStudio && !PokemonIsForm(pkm))
	{
		string childFilePath = MAKE_FILE_PATH(childPath, pkm.idx);
		if (!LoadChild(child[pkm.dataIdx], childFilePath))
		{
			Log(CRITICAL, u8"Pokémon %d lacks child data", pkm.idx);
			return false;
		}
	}

	return true;
}

bool Engine::LoadPokemonData()
{
	u32 formCount = 0;
	// Store loaded data in the Pokémon array
	for (u32 pkmIdx = 0; pkmIdx < (u32)pkmNames.size(); ++pkmIdx)
	{
		Pokemon pkm;
		pkm.idx = pkmIdx;
		// Base form Pokémon share their Pokédex, PML and Text indexes
		pkm.dataIdx = pkmIdx;
		pkm.textIdx = pkmIdx;

		// Pokémon names over the Pokédex amount are PokéStudio enemies
		pkm.pokeStudio = (bool)!((u32)pkmIdx < project->pokemonCount);
		// PokéStudio enemies don't have Text index since they don't have a description
		if (pkm.pokeStudio)
			pkm.textIdx = 0;

		// Load the data of the Pokémon
		if (!LoadPokemon(pkm))
			return false;

		if (!pkm.pokeStudio)
		{
			// Load the data of the Pokémon forms
			for (u8 form = 0; form < personal[pkmIdx][FORM_COUNT] - 1; ++form)
			{
				Pokemon formPkm;
				formPkm.idx = pkmIdx;

				// Form Pokémon data index is specified in the personal data of the base Pokémon
				formPkm.dataIdx = personal[pkmIdx][FORM_OFFSET] + form;
				// Form Pokémon without data must use the data of the base Pokémon
				if (formPkm.dataIdx == form)
					formPkm.dataIdx = pkm.dataIdx;

				// Decription texts skip the PokéStudion Entries
				// textIdx 650 is empty so we need to add 1
				formPkm.textIdx = project->pokemonCount + formCount + 1;

				// Only non-PokéStudio Pokémon have forms
				formPkm.pokeStudio = false;

				++formCount;

				// Load the data of the Form Pokémon
				if (!LoadPokemon(formPkm))
					return false;

				pkm.forms.push_back(formPkm);
			}
		}
		pokemon.push_back(pkm);
	}
	if (project->selectedPkmIdx >= pokemon.size())
		SetCurrentPokemon(0, 0);
	else
		SetCurrentPokemon(project->selectedPkmIdx, project->selectedPkmForm);

	return true;
}

bool Engine::LoadItemData()
{
	// Store loaded data in the Item array
	for (u32 itemIdx = 0; itemIdx < (u32)itemNames.size(); ++itemIdx)
	{
		string itemFilePath = MAKE_FILE_PATH(itemPath, itemIdx);
		if (!LoadItem(items[itemIdx], itemFilePath))
		{
			Log(CRITICAL, u8"Item %d lacks item data", itemIdx);
			return false;
		}
	}
	if (project->selectedItemIdx >= items.size())
		SetCurrentItem(0);
	else
		SetCurrentItem(project->selectedItemIdx);

	return true;
}

bool Engine::LoadMoveData()
{
	// Store loaded data in the Move array
	for (u32 moveIdx = 0; moveIdx < (u32)moveNames.size(); ++moveIdx)
	{
		string moveFilePath = MAKE_FILE_PATH(movePath, moveIdx);
		if (moveIdx < (u32)moves.size() && !LoadMove(moves[moveIdx], moveFilePath))
		{
			Log(CRITICAL, u8"Move %d lacks move data", moveIdx);
			return false;
		}

		string moveAnimFilePath = MAKE_FILE_PATH(moveAnimPath, moveIdx);
		LoadFileStream(moveAnims[moveIdx], moveAnimFilePath);
	}
	if (project->selectedMoveIdx >= moves.size())
		SetCurrentMove(0);
	else
		SetCurrentMove(project->selectedMoveIdx);

	return true;
}

bool Engine::Start()
{
	// Extract the text data
	if (!LoadTextFiles())
		return false;

	// Extrat the data of every Pokémon
	int loadedFiles = LoadDataNarc(PERSONAL_NARC_PATH, personalPath);
	if (loadedFiles < 0)
		return false;
	personal.resize(loadedFiles);

	loadedFiles = LoadDataNarc(LEARNSET_NARC_PATH, learnsetPath);
	if (loadedFiles < 0)
		return false;
	learnset.resize(loadedFiles);

	loadedFiles = LoadDataNarc(EVOLUTION_NARC_PATH, evolutionPath);
	if (loadedFiles < 0)
		return false;
	evolution.resize(loadedFiles);

	loadedFiles = LoadDataNarc(CHILD_NARC_PATH, childPath);
	if (loadedFiles < 0)
		return false;
	child.resize(loadedFiles);

	if (!LoadPokemonData())
		return false;


	loadedFiles = LoadDataNarc(ITEM_NARC_PATH, itemPath);
	if (loadedFiles < 0)
		return false;
	items.resize(loadedFiles);

	if (!LoadItemData())
		return false;

	loadedFiles = LoadDataNarc(MOVE_NARC_PATH, movePath);
	if (loadedFiles < 0)
		return false;
	moves.resize(loadedFiles);

	loadedFiles = LoadDataNarc(MOVE_ANIM_NARC_PATH, moveAnimPath);
	if (loadedFiles < 0)
		return false;
	moveAnims.resize(loadedFiles);

	if (!LoadMoveData())
		return false;

	return true;
}

bool Engine::LoadPersonal(PersonalData& personalData, const string& file)
{
	PersonalReset(personalData);

	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;
	u32 currentByte = 0;

	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		personalData[(u32)BASE_HP + stat] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 type = 0; type < TYPE_COUNT; ++type)
		personalData[(u32)TYPE_1 + type] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	personalData[CATCH_RATE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	personalData[EVOLUTION_STAGE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u16 evYield = FileStreamReadUpdate<u16>(fileStream, currentByte);
	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		personalData[(u32)EV_HP + stat] = (u8)((evYield >> (stat * 2)) & 0x0003);
	personalData[GROUNDED_SPRITE] = (int)((evYield & 0x1000) != 0);

	u16 expandedAbilBits[ABILITY_COUNT];
	for (u32 item = 0; item < WILD_ITEM_COUNT; ++item)
	{
		u16 itemValue = FileStreamReadUpdate<u16>(fileStream, currentByte);
		expandedAbilBits[item] = itemValue & 0xC000;
		// Remove the ability related bits from the item value
		itemValue -= expandedAbilBits[item];

		personalData[(u32)WILD_ITEM_50 + item] = (int)itemValue;
	}

	personalData[SEX_CHANCE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	personalData[EGG_HAPPINESS] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	personalData[BASE_HAPPINESS] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	personalData[GROWTH_RATE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 eggGroup = 0; eggGroup < EGG_GROUP_COUNT; ++eggGroup)
		personalData[(u32)EGG_GROUP_1 + eggGroup] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 ability = 0; ability < ABILITY_COUNT; ++ability)
	{
		u16 abilityValue = FileStreamReadUpdate<u8>(fileStream, currentByte);
		// Add the ability bits stored in the item slot to the ability value
		abilityValue += (expandedAbilBits[ability] >> 6);

		personalData[(u32)ABILITY_1 + ability] = (int)abilityValue;
	}

	personalData[ESCAPE_RATE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	personalData[FORM_OFFSET] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	personalData[FORM_SPRITE_OFFSET] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	personalData[FORM_COUNT] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u8 colorValue = FileStreamReadUpdate<u8>(fileStream, currentByte);
	personalData[COLOR] = (int)(colorValue & 0x3F);
	personalData[SPRITE_FLIP] = (int)((colorValue & 0x40) >> 6);
	personalData[FORM_RARE] = (bool)((colorValue & 0x80) >> 7);

	personalData[BASE_EXP] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	personalData[WEIGHT] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	personalData[HEIGHT] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	for (u32 tmhm = 0; tmhm < TM_HM_COUNT; ++tmhm)
		personalData[(u32)TM_HM_1 + tmhm] = (int)FileStreamReadUpdate<u32>(fileStream, currentByte);

	personalData[TYPE_TUTORS] = (int)FileStreamReadUpdate<u32>(fileStream, currentByte);

	for (u32 specialTutor = 0; specialTutor < SPECIAL_TUTOR_COUNT; ++specialTutor)
		personalData[(u32)SPECIAL_TUTORS_1 + specialTutor] = FileStreamReadUpdate<u32>(fileStream, currentByte);

	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::SavePersonal(const PersonalData& personalData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	for (u32 stat = 0; stat < STAT_COUNT; ++stat)
		FileStreamPutBack<u8>(fileStream, (u8)personalData[(u32)BASE_HP + stat]);

	for (u32 type = 0; type < TYPE_COUNT; ++type)
		FileStreamPutBack<u8>(fileStream, (u8)personalData[(u32)TYPE_1 + type]);

	FileStreamPutBack<u8>(fileStream, (u8)personalData[CATCH_RATE]);
	FileStreamPutBack<u8>(fileStream, (u8)personalData[EVOLUTION_STAGE]);

	u16 evYield = 0;
	for (int stat = STAT_COUNT - 1; stat >= 0; --stat)
	{
		evYield |= (personalData[(u32)EV_HP + stat] & 0x0003);
		evYield = evYield << 2;
	}
	if (personalData[GROUNDED_SPRITE] != 0)
		evYield |= 0x1000;
	FileStreamPutBack<u16>(fileStream, evYield);

	for (u32 item = 0; item < WILD_ITEM_COUNT; ++item)
	{
		u16 itemValue = (u16)personalData[(u32)WILD_ITEM_50 + item];

		itemValue &= 0x3FFF;
		// Save the 2 most significant ability bits at the end of the item value
		itemValue += ((u16)personalData[(u32)ABILITY_1 + item] & 0x0300) << 6;

		FileStreamPutBack<u16>(fileStream, itemValue);
	}

	FileStreamPutBack<u8>(fileStream, (u8)personalData[SEX_CHANCE]);

	FileStreamPutBack<u8>(fileStream, (u8)personalData[EGG_HAPPINESS]);
	FileStreamPutBack<u8>(fileStream, (u8)personalData[BASE_HAPPINESS]);

	FileStreamPutBack<u8>(fileStream, (u8)personalData[GROWTH_RATE]);

	for (u32 eggGroup = 0; eggGroup < EGG_GROUP_COUNT; ++eggGroup)
		FileStreamPutBack<u8>(fileStream, (u8)personalData[(u32)EGG_GROUP_1 + eggGroup]);

	for (u32 ability = 0; ability < ABILITY_COUNT; ++ability)
		FileStreamPutBack<u8>(fileStream, (u8)personalData[(u32)ABILITY_1 + ability]);

	FileStreamPutBack<u8>(fileStream, (u8)personalData[ESCAPE_RATE]);

	FileStreamPutBack<u16>(fileStream, (u16)personalData[FORM_OFFSET]);
	FileStreamPutBack<u16>(fileStream, (u16)personalData[FORM_SPRITE_OFFSET]);
	FileStreamPutBack<u8>(fileStream, (u8)personalData[FORM_COUNT]);

	u8 color = (u8)personalData[COLOR] & 0x3F;
	if (personalData[SPRITE_FLIP] != 0)
		color |= 0x40;
	if (personalData[FORM_RARE] != 0)
		color |= 0x80;
	FileStreamPutBack<u8>(fileStream, color);

	FileStreamPutBack<u16>(fileStream, (u16)personalData[BASE_EXP]);

	FileStreamPutBack<u16>(fileStream, (u16)personalData[WEIGHT]);
	FileStreamPutBack<u16>(fileStream, (u16)personalData[HEIGHT]);

	for (u32 tmhm = 0; tmhm < TM_HM_COUNT; ++tmhm)
		FileStreamPutBack<u32>(fileStream, personalData[(u32)TM_HM_1 + tmhm]);

	FileStreamPutBack<u32>(fileStream, (u32)personalData[TYPE_TUTORS]);

	for (u32 specialTutor = 0; specialTutor < SPECIAL_TUTOR_COUNT; ++specialTutor)
		FileStreamPutBack<u32>(fileStream, personalData[(u32)SPECIAL_TUTORS_1 + specialTutor]);

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::LoadLearnset(LearnsetData& learnData, const string& file)
{
	LearnsetReset(learnData);

	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;
	u32 currentByte = 0;

	for (u32 learnIdx = 0; learnIdx < project->learnsetSize && learnIdx < LEARNSET_DATA_MAX; ++learnIdx)
	{
		u16 moveID = FileStreamReadUpdate<u16>(fileStream, currentByte);
		if (moveID == LEARNSET_NULL)
			break;
		u16 level = FileStreamReadUpdate<u16>(fileStream, currentByte);
		if (level == LEARNSET_NULL)
			break;

		learnData[learnIdx][LEARN_MOVE_ID] = (int)moveID;
		learnData[learnIdx][LEARN_LEVEL] = (int)level;
	}

	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::SaveLearnset(const LearnsetData& learnData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	u32 learnIdx = 0;
	vector<pair<u32, int>> orderedSet = Learnset::GetOrderedLearnset(learnData);
	for (; learnIdx < orderedSet.size(); ++learnIdx)
	{
		u32 idx = orderedSet[learnIdx].first;
		FileStreamPutBack<u16>(fileStream, (u16)learnData[idx][LEARN_MOVE_ID]);
		FileStreamPutBack<u16>(fileStream, (u16)learnData[idx][LEARN_LEVEL]);
	}
	if (learnIdx < project->learnsetSize)
	{
		FileStreamPutBack<u16>(fileStream, (u16)LEARNSET_NULL);
		FileStreamPutBack<u16>(fileStream, (u16)LEARNSET_NULL);
	}
		

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::LoadEvolution(EvolutionData& evoData, const string& file)
{
	EvolutionReset(evoData);

	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;
	u32 currentByte = 0;

	for (u32 evoIdx = 0; evoIdx < project->evolutionSize && evoIdx < EVOLUTION_DATA_MAX; ++evoIdx)
	{
		evoData[evoIdx][EVOLUTION_METHOD] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
		evoData[evoIdx][EVOLUTION_PARAM] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
		evoData[evoIdx][EVOLUTION_SPECIES] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	}

	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::SaveEvolution(const EvolutionData& evoData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	for (u32 evoIdx = 0; evoIdx < project->evolutionSize && evoIdx < EVOLUTION_DATA_MAX; ++evoIdx)
	{
		FileStreamPutBack<u16>(fileStream, (u16)evoData[evoIdx][EVOLUTION_METHOD]);
		FileStreamPutBack<u16>(fileStream, (u16)evoData[evoIdx][EVOLUTION_PARAM]);
		FileStreamPutBack<u16>(fileStream, (u16)evoData[evoIdx][EVOLUTION_SPECIES]);
	}

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::LoadChild(ChildData& childData, const string& file)
{
	childData = CHILD_NULL;

	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;

	childData = (int)FileStreamRead<u16>(fileStream, 0);

	ReleaseFileStream(fileStream);

	return true;
}

bool Engine::SaveChild(const ChildData& childData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	FileStreamPutBack<u16>(fileStream, (u16)childData);

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::LoadItem(ItemData& itemData, const string& file)
{
	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;

	u32 currentByte = 0;

	itemData[PRICE] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	itemData[HELD_EFFECT] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[HELD_PARAM] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[NATURAL_GIFT_EFFECT] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[FLING_EFFECT] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[FLING_POWER] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[NATURAL_GIFT_POWER] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	u16 packed = FileStreamReadUpdate<u16>(fileStream, currentByte);
	itemData[PACKED_FLAG_1] = (packed & 0x20) >> 5;
	itemData[PACKED_FLAG_2] = (packed & 0x40) >> 6;
	itemData[POCKET_FIELD] = (packed & 0x780) >> 7;
	itemData[NATURAL_GIFT_TYPE] = packed & 0x1F;

	itemData[EFFECT_FIELD] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[EFFECT_BATTLE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[HAS_BATTLE_STATS] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[BATTLE_POCKET] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[CONSUMABLE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	itemData[SORT_IDX] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	if (itemData[HAS_BATTLE_STATS] == 0)
	{
		for (int field = CURE_SLEEP; field < ITEMDATA_MAX; ++field)
			itemData[field] = 0;
	}
	else
	{
		u8 cureInflict = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		for (int field = CURE_SLEEP; field <= CURE_GSP; ++field)
		{
			u8 index = field - CURE_SLEEP;
			itemData[field] = (cureInflict & (0x1 << index)) >> index;
		}

		u32 boosts = FileStreamReadUpdate<u32>(fileStream, currentByte);
		for (int field = BOOST_REVIVE; field <= BOOST_EVOSTONE; ++field)
		{
			u32 index = field - BOOST_REVIVE;
			itemData[field] = (boosts & (0x1 << index)) >> index;
		}
		for (int field = BOOST_ATK; field <= BOOST_ACC; ++field)
		{
			u32 index = ((field - BOOST_ATK) + 1) * 4;
			itemData[field] = (boosts & (0xF << index)) >> index;
		}
		itemData[BOOST_CRIT] = (boosts & 0x30000000) >> 28;
		itemData[BOOST_PP] = (boosts & 0x40000000) >> 30;
		itemData[BOOST_PP_MAX] = (boosts & 0x80000000) >> 31;

		u16 functions = FileStreamReadUpdate<u16>(fileStream, currentByte);
		for (int field = PP_REPLENISH; field <= FRIENDSHIP_ADD_3; ++field)
		{
			u16 index = field - PP_REPLENISH;
			itemData[field] = (boosts & (0x1 << index)) >> index;
		}

		itemData[ITEM_EV_HP] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[ITEM_EV_ATK] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[ITEM_EV_DEF] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[ITEM_EV_SPE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[ITEM_EV_SPA] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[ITEM_EV_SPD] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

		itemData[HEAL_AMOUNT] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[PP_GAIN] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

		itemData[FRIENDSHIP_1] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[FRIENDSHIP_2] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[FRIENDSHIP_3] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

		itemData[UNKNOWN_1] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
		itemData[UNKNOWN_2] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	}

	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::SaveItem(const ItemData& itemData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	FileStreamPutBack<u16>(fileStream, (u16)itemData[PRICE]);

	FileStreamPutBack<u8>(fileStream, (u8)itemData[HELD_EFFECT]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[HELD_PARAM]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[NATURAL_GIFT_EFFECT]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[FLING_EFFECT]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[FLING_POWER]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[NATURAL_GIFT_POWER]);

	u16 packed = (itemData[PACKED_FLAG_1] << 5) & 0x20;
	packed |= (itemData[PACKED_FLAG_2] << 6) & 0x40;
	packed |= (itemData[POCKET_FIELD] << 7) & 0x780;
	packed |= itemData[NATURAL_GIFT_TYPE] & 0x1F;
	FileStreamPutBack<u16>(fileStream, packed);

	FileStreamPutBack<u8>(fileStream, (u8)itemData[EFFECT_FIELD]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[EFFECT_BATTLE]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[HAS_BATTLE_STATS]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[BATTLE_POCKET]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[CONSUMABLE]);
	FileStreamPutBack<u8>(fileStream, (u8)itemData[SORT_IDX]);

	if (itemData[HAS_BATTLE_STATS] == 0)
	{
		for (int field = CURE_SLEEP; field < ITEMDATA_MAX; ++field)
			FileStreamPutBack<u8>(fileStream, 0);
	}
	else
	{
		u8 cureInflict = 0;
		for (int field = CURE_SLEEP; field <= CURE_GSP; ++field)
		{
			u8 index = field - CURE_SLEEP;
			cureInflict |= (itemData[field] << index) & (0x1 << index);
		}
		FileStreamPutBack<u16>(fileStream, cureInflict);

		u32 boosts = 0;
		for (int field = BOOST_REVIVE; field <= BOOST_EVOSTONE; ++field)
		{
			u32 index = field - BOOST_REVIVE;
			boosts |= (itemData[field] << index) & (0x1 << index);
		}
		for (int field = BOOST_ATK; field <= BOOST_ACC; ++field)
		{
			u32 index = ((field - BOOST_ATK) + 1) * 4;
			boosts |= (itemData[field] << index) & (0xF << index);
		}
		boosts |= (itemData[BOOST_CRIT] << 28) & 0x30000000;
		boosts |= (itemData[BOOST_PP] << 30) & 0x40000000;
		boosts |= (itemData[BOOST_PP_MAX] << 31) & 0x80000000;
		FileStreamPutBack<u16>(fileStream, boosts);

		u16 functions = 0;
		for (int field = PP_REPLENISH; field <= FRIENDSHIP_ADD_3; ++field)
		{
			u16 index = field - PP_REPLENISH;
			functions |= (itemData[field] << index) & (0x1 << index);
		}
		FileStreamPutBack<u16>(fileStream, functions);

		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_HP]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_ATK]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_DEF]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_SPE]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_SPA]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[ITEM_EV_SPD]);

		FileStreamPutBack<u8>(fileStream, (u8)itemData[HEAL_AMOUNT]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[PP_GAIN]);

		FileStreamPutBack<u8>(fileStream, (u8)itemData[FRIENDSHIP_1]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[FRIENDSHIP_2]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[FRIENDSHIP_3]);

		FileStreamPutBack<u8>(fileStream, (u8)itemData[UNKNOWN_1]);
		FileStreamPutBack<u8>(fileStream, (u8)itemData[UNKNOWN_2]);
	}

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::LoadMove(MoveData& moveData, const string& file)
{
	FileStream fileStream;
	if (!LoadFileStream(fileStream, file))
		return false;

	u32 currentByte = 0;

	moveData[MOVE_TYPE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[QUALITY] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[CATEGORY] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[POWER] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[ACCURACY] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[BASE_PP] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[PRIORITY] = (int)FileStreamReadUpdate<char>(fileStream, currentByte);

	u8 hitMinMax = FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[HIT_MIN] = hitMinMax & 0x0F;
	moveData[HIT_MAX] = (hitMinMax & 0xF0) >> 4;

	moveData[CONDITION] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);
	moveData[CONDITION_CHANCE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[CONDITION_DURATION] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	moveData[TURN_MIN] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[TURN_MAX] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	moveData[CRIT_STAGE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[FLINCH_RATE] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	moveData[BEHAVIOR] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	moveData[RECOIL] = (int)FileStreamReadUpdate<char>(fileStream, currentByte);
	moveData[HEAL] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	moveData[TARGET] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	for (u32 statChange = 0; statChange < 3; ++statChange)
		moveData[STAT_CHANGE_1 + statChange] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		moveData[STAT_CHANGE_VOLUME_1 + statChange] = (int)FileStreamReadUpdate<char>(fileStream, currentByte);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		moveData[STAT_CHANGE_CHANCE_1 + statChange] = (int)FileStreamReadUpdate<u8>(fileStream, currentByte);

	moveData[PADDING] = (int)FileStreamReadUpdate<u16>(fileStream, currentByte);

	u32 flags = FileStreamReadUpdate<u32>(fileStream, currentByte);
	for (u32 moveFlag = 0; moveFlag < sizeof(u32) * 8; ++moveFlag)
		moveData[IS_CONTACT + moveFlag] = (flags & (1 << moveFlag)) != 0;

	ReleaseFileStream(fileStream);
	return true;
}

bool Engine::SaveMove(const MoveData& moveData, const string& file)
{
	FileStream fileStream;
	if (!LoadEmptyFileStream(fileStream))
		return false;

	FileStreamPutBack<u8>(fileStream, (u8)moveData[MOVE_TYPE]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[QUALITY]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[CATEGORY]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[POWER]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[ACCURACY]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[BASE_PP]);
	FileStreamPutBack<char>(fileStream, (char)moveData[PRIORITY]);

	u8 hitMinMax = (u8)moveData[HIT_MIN] + ((u8)moveData[HIT_MAX] << 4);
	FileStreamPutBack<u8>(fileStream, hitMinMax);

	FileStreamPutBack<u16>(fileStream, (u16)moveData[CONDITION]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[CONDITION_CHANCE]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[CONDITION_DURATION]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[TURN_MIN]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[TURN_MAX]);

	FileStreamPutBack<u8>(fileStream, (u8)moveData[CRIT_STAGE]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[FLINCH_RATE]);
	FileStreamPutBack<u16>(fileStream, (u16)moveData[BEHAVIOR]);
	FileStreamPutBack<char>(fileStream, (char)moveData[RECOIL]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[HEAL]);
	FileStreamPutBack<u8>(fileStream, (u8)moveData[TARGET]);

	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<u8>(fileStream, (u8)moveData[STAT_CHANGE_1 + statChange]);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<char>(fileStream, (char)moveData[STAT_CHANGE_VOLUME_1 + statChange]);
	for (u32 statChange = 0; statChange < 3; ++statChange)
		FileStreamPutBack<u8>(fileStream, (u8)moveData[STAT_CHANGE_CHANCE_1 + statChange]);

	FileStreamPutBack<u16>(fileStream, (u16)moveData[PADDING]);

	u32 flags = 0;
	for (u32 moveFlag = 0; moveFlag < sizeof(u32) * 8; ++moveFlag)
		if (moveData[IS_CONTACT + moveFlag])
			flags |= (1 << moveFlag);
	FileStreamPutBack<u32>(fileStream, flags);

	SaveFileStream(fileStream, file);
	ReleaseFileStream(fileStream);
	return true;
}

void Engine::HandleReverseEvent()
{
	if (!reverseEvents.size())
		return;

	Event& topEvent = reverseEvents.back();

	modules[topEvent.type]->HandleReverseEvent(&topEvent);
		
	delete topEvent.value;
	reverseEvents.pop_back();
}

void Engine::SendGroupEvent(u32 group)
{
	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
	{
		if (modules[idx]->group == group)
		{
			modules[idx]->HandleGroupEvent(nullptr);
		}
	}
}

void Engine::Save()
{
	for (u32 idx = 0; idx < (u32)saveEvents.size(); ++idx)
	{
		Event* saveEvent = &(saveEvents[idx]);
		modules[saveEvent->type]->HandleSaveEvent(saveEvent);
	}
	saveEvents.clear();

	SaveProjectSettings(*project);
}