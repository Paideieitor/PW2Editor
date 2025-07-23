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

	string ctrMapProjectPath = "";
	string romPath = "";

	string settingsPath = "";

	u32 width = DEFAULT_WINDOW_WIDTH;
	u32 height = DEFAULT_WINDOW_HEIGHT;
	string font = DEFAULT_FONT;
	u32 fontSize = DEFAULT_FONT_SIZE;

	u32 learnsetSize = DEFAULT_LEARNSET_SIZE;
	u32 evolutionSize = DEFAULT_EVOLUTION_SIZE;
	u32 pokemonCount = DEFAULT_POKEMON_COUNT;
	u32 typeCount = DEFAULT_TYPE_COUNT;

	u32 group = DEFAULT_GROUP;

	u32 selectedPkmIdx = DEFAULT_SELECTED_PKM_IDX;
	u32 selectedPkmForm = DEFAULT_SELECTED_PKM_FORM;
	u32 showPokeStudio = DEFAULT_SHOW_POKESTUDIO;

	u32 selectedItemIdx = DEFAULT_SELECTED_ITEM_IDX;

	u32 selectedMoveIdx = DEFAULT_SELECTED_MOVE_IDX;

	u32 selectedLocationIdx = DEFAULT_SELECTED_LOCATION_IDX;
	u32 selectedZoneIdx = DEFAULT_SELECTED_ZONE_IDX;

	u32 maxEvents = DEFAULT_MAX_EVENTS;
};

ReturnState LoadProjectSettings(Project& project, const string& name);
ReturnState SaveProjectSettings(const Project& project);
#endif // _PROJECT_H