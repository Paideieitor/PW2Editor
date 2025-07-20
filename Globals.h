#ifndef _GLOBALS_H
#define _GLOBALS_H

using namespace std;

#define WIZARD_NAME "PW2Editor Wizard"
#define ENGINE_NAME "PW2Editor"
#define TARGET_FPS 60

#define ENGINE_GROUP 0u
#define POKEMON_GROUP 1u
#define ITEM_GROUP 2u
#define MOVE_GROUP 3u
#define ENCOUNTER_GROUP 4u
#define TRAINER_GROUP 5u


#define SETTINGS_NAME "settings"
	#define DEFAULT_WINDOW_WIDTH 1440u
	#define DEFAULT_WINDOW_HEIGHT 810u
	#define DEFAULT_FONT "Fonts\\default.ttf"
	#define DEFAULT_FONT_SIZE 15u
	#define DEFAULT_LEARNSET_SIZE 26u
	#define DEFAULT_EVOLUTION_SIZE 7u
	#define DEFAULT_POKEMON_COUNT 650u
	#define DEFAULT_TYPE_COUNT 17u
	#define DEFAULT_GROUP POKEMON_GROUP
	#define DEFAULT_SELECTED_PKM_IDX 0u
	#define DEFAULT_SELECTED_PKM_FORM 0u
	#define DEFAULT_SHOW_POKESTUDIO false
	#define DEFAULT_SELECTED_ITEM_IDX 0u
	#define DEFAULT_SELECTED_MOVE_IDX 0u
	#define DEFAULT_MAX_EVENTS 2000u
	#define DEFAULT_CUSTOM_FEATURES false

#define PROJECTS_PATH "Projects"

#define CTRMAP_FILE_EXTENSION "cmproj"
#define CTRMAP_VFSBASE "VFSBase"

#define ARRAY_COUNT(arr) sizeof(arr) / sizeof(arr[0])

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

enum ReturnState : u8
{
	OK = 0,
	STOP = 1,
	ERROR = 2,
	EXIT = 3,
};

#endif // _GLOBALS_H
