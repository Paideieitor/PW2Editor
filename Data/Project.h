#ifndef _PROJECT_H
#define _PROJECT_H

#include <vector>
#include <string>

#include "Globals.h"

struct Project
{
	string name = "";
	string path = "";
	u32 order = 0;

	string ctrMapProjectDir = "";
	string romDir = "";

	string game = "";
	string gameCode = "";

	string settingsPath = "";

	u32 width = DEFAULT_WINDOW_WIDTH;
	u32 height = DEFAULT_WINDOW_HEIGHT;
	string font = DEFAULT_FONT;
	u32 fontSize = DEFAULT_FONT_SIZE;

	u32 group = DEFAULT_GROUP;

	u32 pokemon = DEFAULT_SELECTED_PKM_IDX;

	u32 item = DEFAULT_SELECTED_ITEM_IDX;

	u32 move = DEFAULT_SELECTED_MOVE_IDX;

	u32 location = DEFAULT_SELECTED_LOCATION_IDX;

	u32 trainer = DEFAULT_SELECTED_TRAINER_IDX;

	u32 maxEvents = DEFAULT_MAX_EVENTS;
};

u32 GetProjectPokemon(const Project& project);
void SetProjectPokemon(Project& project, u32 idx);
u32 GetProjectForm(const Project& project);
void SetProjectForm(Project& project, u32 idx);

u32 GetProjectLocation(const Project& project);
void SetProjectLocation(Project& project, u32 idx);
u32 GetProjectZone(const Project& project);
void SetProjectZone(Project& project, u32 idx);
u32 GetProjectSeason(const Project& project);
void SetProjectSeason(Project& project, u32 idx);

ReturnState LoadProjectSettings(Project& project, const string& path);
ReturnState SaveProjectSettings(const Project& project);
#endif // _PROJECT_H