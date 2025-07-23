#ifndef _KLIN_FORMAT_H
#define _KLIN_FORMAT_H

#include <vector>
#include <string>

#include "Globals.h"

#define KLIN_TERMINATION ".KLIN"

typedef void* KlinHandler;

enum KlinType
{
	KLIN_NONE,
	KLIN_STRING,
	KLIN_U32,
	KLIN_STRING_LIST,
	KLIN_U32_LIST,
};
struct KlinParam
{
	const char* name;
	KlinType type;
};

enum KlinValue : u32
{
	KLIN_NULL,
	KLIN_PROJECT_NAME,
	KLIN_PROJECT_ORDER,
	KLIN_CTRMAP_PROJECT_PATH,
	KLIN_ROM_PATH,
	KLIN_LEARNSET_SIZE,
	KLIN_EVOLUTION_SIZE,
	KLIN_POKEMON_COUNT,
	KLIN_TYPE_COUNT,
	KLIN_WINDOW_WIDTH,
	KLIN_WINDOW_HEIGHT,
	KLIN_SELECTED_PKM_IDX,
	KLIN_SELECTED_PKM_FORM,
	KLIN_SEARCH_POKESTUDIO,
	KLIN_FONT,
	KLIN_FONT_SIZE,
	KLIN_MAX_EVENTS,
	KLIN_GROUP,
	KLIN_SELECTED_ITEM_IDX,
	KLIN_SELECTED_MOVE_IDX,
	KLIN_SELECTED_LOCATION_IDX,
	KLIN_SELECTED_ZONE_IDX,
	KLIN_MAX_VALUE,
};
const KlinParam KlinParams[] = {
	{"Null", KLIN_NONE},
	{"Project Name", KLIN_STRING},
	{"Project Order", KLIN_U32},
	{"CTRMap Project Path", KLIN_STRING},
	{"ROM Path", KLIN_STRING},
	{"Learnset Size", KLIN_U32},
	{"Evolution Size", KLIN_U32},
	{"Pokemon Count", KLIN_U32},
	{"Type Count", KLIN_U32},
	{"Window Width", KLIN_U32},
	{"Window Height", KLIN_U32},
	{"Selected Pokemon Idx", KLIN_U32},
	{"Selected Pokemon Form", KLIN_U32},
	{"Search PokeStudio", KLIN_U32},
	{"Font", KLIN_STRING},
	{"Font Size", KLIN_U32},
	{"Max Events", KLIN_U32},
	{"Group", KLIN_U32},
	{"Selected Item Idx", KLIN_U32},
	{"Selected Move Idx", KLIN_U32},
	{"Selected Location Idx", KLIN_U32},
	{"Selected Zone Idx", KLIN_U32},
};

void LoadKlin(KlinHandler& handler, const string& path);
void ReleaseKlin(KlinHandler& handler);
void SaveKlin(KlinHandler& handler, const string& path, bool release = false);

bool GetKlinValueString(const KlinHandler& handler, const KlinValue value, string& output);
bool GetKlinValueU32(const KlinHandler& handler, const KlinValue value, u32& output);
bool GetKlinListString(const KlinHandler& handler, const KlinValue value, vector<string>& output);
bool GetKlinListU32(const KlinHandler& handler, const KlinValue value, vector<u32>& output);

bool SetKlinValueString(const KlinHandler& handler, const KlinValue value, const string& param);
bool SetKlinValueU32(const KlinHandler& handler, const KlinValue value, const u32 param);
bool SetKlinListString(const KlinHandler& handler, const KlinValue value, const vector<string>& param);
bool SetKlinListU32(const KlinHandler& handler, const KlinValue value, const vector<u32>& param);

void DeleteKlinValue(const KlinHandler& handler, const KlinValue value);

#endif // _KLIN_FORMAT_H