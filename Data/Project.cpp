#include "System.h"

#include "Utils/FileUtils.h"
#include "Utils/KlinFormat.h"

#include "Data/Project.h"

#define FULL_MASK 0xFFFFFFFF

#define POKEMON_MASK 0x0000FFFF
#define FORM_MASK 0xFFFF0000

#define LOCATION_MASK 0x00003FFF
#define SEASON_MASK 0x0000C000
#define ZONE_MASK 0xFFFF0000

u32 GetProjectValue(u32 var, u32 mask, u8 move)
{
	return (var & mask) >> move;
}

void SetProjectValue(u32& var, u32 mask, u8 move, u32 value)
{
	var = ((value << move) & mask) + (var & (FULL_MASK - mask));
}

u32 GetProjectPokemon(const Project& project)
{
	return GetProjectValue(project.pokemon, POKEMON_MASK, 0);
}

void SetProjectPokemon(Project& project, u32 idx)
{
	SetProjectValue(project.pokemon, POKEMON_MASK, 0, idx);
}

u32 GetProjectForm(const Project& project)
{
	return GetProjectValue(project.pokemon, FORM_MASK, 16);
}

void SetProjectForm(Project& project, u32 idx)
{
	SetProjectValue(project.pokemon, FORM_MASK, 16, idx);
}

u32 GetProjectLocation(const Project& project)
{
	return GetProjectValue(project.location, LOCATION_MASK, 0);
}

void SetProjectLocation(Project& project, u32 idx)
{
	SetProjectValue(project.location, LOCATION_MASK, 0, idx);
}

u32 GetProjectZone(const Project& project)
{
	return GetProjectValue(project.location, ZONE_MASK, 16);
}

void SetProjectZone(Project& project, u32 idx)
{
	SetProjectValue(project.location, ZONE_MASK, 16, idx);
}

u32 GetProjectSeason(const Project& project)
{
	return GetProjectValue(project.location, SEASON_MASK, 14);
}

void SetProjectSeason(Project& project, u32 idx)
{
	SetProjectValue(project.location, SEASON_MASK, 14, idx);
}

ReturnState LoadProjectSettings(Project& project, const string& path)
{
	project.path = path;

	project.name = PathGetLastName(PathEraseLastName(path));

	if (IsFilePath(project.path))
	{
		Log(WARNING, "Project %s is not a folder", project.name.c_str());
		return ERROR;
	}

	project.settingsPath = PathConcat(project.path, string(SETTINGS_NAME) + KLIN_TERMINATION);
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

	if (!GetKlinValueString(klin, KLIN_CTRMAP_PROJECT_PATH, project.ctrMapProjectDir))
	{
		Log(WARNING, "Could not find project ctrmap path in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	if (!GetKlinValueString(klin, KLIN_ROM_PATH, project.romDir))
	{
		Log(WARNING, "Could not find project rom path in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	if (!GetKlinValueString(klin, KLIN_GAME, project.game))
	{
		Log(WARNING, "Could not find project game in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	if (!GetKlinValueString(klin, KLIN_GAME_CODE, project.gameCode))
	{
		Log(WARNING, "Could not find project game code in project file (%s)", project.settingsPath.c_str());
		ReleaseKlin(klin);
		return STOP;
	}

	GetKlinValueU32(klin, KLIN_WINDOW_WIDTH, project.width);
	GetKlinValueU32(klin, KLIN_WINDOW_HEIGHT, project.height);
	GetKlinValueString(klin, KLIN_FONT, project.font);
	GetKlinValueU32(klin, KLIN_FONT_SIZE, project.fontSize);

	GetKlinValueU32(klin, KLIN_GROUP, project.group);

	GetKlinValueU32(klin, KLIN_SELECTED_PKM_IDX, project.pokemon);

	GetKlinValueU32(klin, KLIN_SELECTED_ITEM_IDX, project.item);

	GetKlinValueU32(klin, KLIN_SELECTED_MOVE_IDX, project.move);

	GetKlinValueU32(klin, KLIN_SELECTED_LOCATION_IDX, project.location);
	
	GetKlinValueU32(klin, KLIN_SELECTED_TRAINER_IDX, project.trainer);

	GetKlinValueU32(klin, KLIN_MAX_EVENTS, project.maxEvents);

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

	SetKlinValueString(klin, KLIN_CTRMAP_PROJECT_PATH, project.ctrMapProjectDir);
	SetKlinValueString(klin, KLIN_ROM_PATH, project.romDir);

	SetKlinValueString(klin, KLIN_GAME, project.game);
	SetKlinValueString(klin, KLIN_GAME_CODE, project.gameCode);

	SetKlinValueU32(klin, KLIN_WINDOW_WIDTH, project.width);
	SetKlinValueU32(klin, KLIN_WINDOW_HEIGHT, project.height);
	SetKlinValueString(klin, KLIN_FONT, project.font);
	SetKlinValueU32(klin, KLIN_FONT_SIZE, project.fontSize);

	SetKlinValueU32(klin, KLIN_GROUP, project.group);

	SetKlinValueU32(klin, KLIN_SELECTED_PKM_IDX, project.pokemon);

	SetKlinValueU32(klin, KLIN_SELECTED_ITEM_IDX, project.item);

	SetKlinValueU32(klin, KLIN_SELECTED_MOVE_IDX, project.move);

	SetKlinValueU32(klin, KLIN_SELECTED_LOCATION_IDX, project.location);

	SetKlinValueU32(klin, KLIN_SELECTED_TRAINER_IDX, project.trainer);

	SetKlinValueU32(klin, KLIN_MAX_EVENTS, project.maxEvents);

	SaveKlin(klin, project.settingsPath, true);
	return OK;
}
