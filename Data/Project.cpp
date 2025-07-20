#include "System.h"

#include "Utils/FileUtils.h"
#include "Utils/KlinFormat.h"

#include "Data/Project.h"

ReturnState LoadProjectSettings(Project& project, const string& name)
{
	project.name = name;

	project.path = string(PROJECTS_PATH) + PATH_SEPARATOR + project.name;
	if (IsFilePath(project.path))
	{
		Log(WARNING, "Project %s is not a folder", project.name.c_str());
		return ERROR;
	}

	project.settingsPath = project.path + PATH_SEPARATOR + SETTINGS_NAME + KLIN_TERMINATION;
	if (!PathExists(project.settingsPath))
	{
		Log(WARNING, "Could not find project file (%s)", project.settingsPath.c_str());
		return ERROR;
	}

	KlinHandler klin;
	LoadKlin(klin, project.settingsPath);
	if (!klin)
	{
		Log(WARNING, "Could not load project file (%s)", project.settingsPath.c_str());
		return STOP;
	}

	if (!GetKlinValueU32(klin, KLIN_PROJECT_ORDER, project.order))
	{
		Log(WARNING, "Could not find project order in project file (%s)", project.settingsPath.c_str());
		project.order = ~0;
	}

	if (!GetKlinValueString(klin, KLIN_CTRMAP_PROJECT_PATH, project.ctrMapProjectPath))
	{
		Log(WARNING, "Could not find project ctrmap path in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	if (!GetKlinValueString(klin, KLIN_ROM_PATH, project.romPath))
	{
		Log(WARNING, "Could not find project rom path in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	GetKlinValueU32(klin, KLIN_WINDOW_WIDTH, project.width);
	GetKlinValueU32(klin, KLIN_WINDOW_HEIGHT, project.height);
	GetKlinValueString(klin, KLIN_FONT, project.font);
	GetKlinValueU32(klin, KLIN_FONT_SIZE, project.fontSize);

	GetKlinValueU32(klin, KLIN_LEARNSET_SIZE, project.learnsetSize);
	GetKlinValueU32(klin, KLIN_EVOLUTION_SIZE, project.evolutionSize);
	GetKlinValueU32(klin, KLIN_POKEMON_COUNT, project.pokemonCount);
	GetKlinValueU32(klin, KLIN_TYPE_COUNT, project.typeCount);

	GetKlinValueU32(klin, KLIN_GROUP, project.group);

	GetKlinValueU32(klin, KLIN_SELECTED_PKM_IDX, project.selectedPkmIdx);
	GetKlinValueU32(klin, KLIN_SELECTED_PKM_FORM, project.selectedPkmForm);
	GetKlinValueU32(klin, KLIN_SEARCH_POKESTUDIO, project.showPokeStudio);

	GetKlinValueU32(klin, KLIN_SELECTED_ITEM_IDX, project.selectedItemIdx);

	GetKlinValueU32(klin, KLIN_SELECTED_MOVE_IDX, project.selectedMoveIdx);
	
	GetKlinValueU32(klin, KLIN_MAX_EVENTS, project.maxEvents);

	GetKlinValueU32(klin, KLIN_CUSTOM_FEATURES, project.customFeatures);

	ReleaseKlin(klin);
    return OK;
}

ReturnState SaveProjectSettings(const Project& project)
{
	KlinHandler klin;
	LoadKlin(klin, project.settingsPath);
	if (!klin)
	{
		Log(WARNING, "Could not save project file (%s)", project.settingsPath.c_str());
		return ERROR;
	}

	SetKlinValueString(klin, KLIN_PROJECT_NAME, project.name);
	SetKlinValueU32(klin, KLIN_PROJECT_ORDER, project.order);

	SetKlinValueString(klin, KLIN_CTRMAP_PROJECT_PATH, project.ctrMapProjectPath);
	SetKlinValueString(klin, KLIN_ROM_PATH, project.romPath);

	SetKlinValueU32(klin, KLIN_WINDOW_WIDTH, project.width);
	SetKlinValueU32(klin, KLIN_WINDOW_HEIGHT, project.height);
	SetKlinValueString(klin, KLIN_FONT, project.font);
	SetKlinValueU32(klin, KLIN_FONT_SIZE, project.fontSize);

	SetKlinValueU32(klin, KLIN_LEARNSET_SIZE, project.learnsetSize);
	SetKlinValueU32(klin, KLIN_EVOLUTION_SIZE, project.evolutionSize);
	SetKlinValueU32(klin, KLIN_POKEMON_COUNT, project.pokemonCount);
	SetKlinValueU32(klin, KLIN_TYPE_COUNT, project.typeCount);

	SetKlinValueU32(klin, KLIN_GROUP, project.group);

	SetKlinValueU32(klin, KLIN_SELECTED_PKM_IDX, project.selectedPkmIdx);
	SetKlinValueU32(klin, KLIN_SELECTED_PKM_FORM, project.selectedPkmForm);
	SetKlinValueU32(klin, KLIN_SEARCH_POKESTUDIO, project.showPokeStudio);

	SetKlinValueU32(klin, KLIN_SELECTED_ITEM_IDX, project.selectedItemIdx);

	SetKlinValueU32(klin, KLIN_SELECTED_MOVE_IDX, project.selectedMoveIdx);

	SetKlinValueU32(klin, KLIN_MAX_EVENTS, project.maxEvents);

	SetKlinValueU32(klin, KLIN_CUSTOM_FEATURES, (u32)project.customFeatures);

	SaveKlin(klin, project.settingsPath, true);
	return OK;
}
