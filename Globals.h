#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <stdint.h>

using namespace std;

#define WIZARD_NAME "PW2Editor Wizard"
#define ENGINE_NAME "PW2Editor"
#define TARGET_FPS 60

#define ENGINE_GROUP 0u
#define POKEMON_GROUP 1u
#define ITEM_GROUP 2u
#define MOVE_GROUP 3u
#define TRAINER_GROUP 4u
#define ENCOUNTER_GROUP 5u

#define SETTINGS_NAME "settings"
	#define DEFAULT_WINDOW_WIDTH 1440u
	#define DEFAULT_WINDOW_HEIGHT 810u
#ifdef _WIN32
	#define DEFAULT_FONT "Fonts\\default.ttf"
#else
    #define DEFAULT_FONT "Fonts/default.ttf"
#endif
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
	#define DEFAULT_SELECTED_LOCATION_IDX 0u
	#define DEFAULT_SELECTED_ZONE_IDX 0u
	#define DEFAULT_SELECTED_TRAINER_IDX 0u
	#define DEFAULT_MAX_EVENTS 2000u
	#define DEFAULT_CUSTOM_FEATURES falseç

#define STAT_COUNT 6

#define PROJECTS_PATH "projects.txt"
#define CTRMAP_PW2_DIR "pw2"

#define CTRMAP_FILE_EXTENSION "cmproj"
#define CTRMAP_VFSBASE "VFSBase"
#ifdef _WIN32
    #define CTRMAP_FILESYSTEM_DIR "vfs\\data\\a"
    #define ROM_FILESYSTEM_DIR "data\\a"
#else
    #define CTRMAP_FILESYSTEM_DIR "vfs/data/a"
    #define ROM_FILESYSTEM_DIR "data/a"
#endif
#define ROM_HEADER_NAME "header.bin"

#define UNUSED(x) (void)(x)
#define ARRAY_COUNT(arr) sizeof(arr) / sizeof(arr[0])

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

enum ReturnState : u8
{
	OK = 0,
	STOP = 1,
	ERROR = 2,
	EXIT = 3,
};

#endif // _GLOBALS_H
