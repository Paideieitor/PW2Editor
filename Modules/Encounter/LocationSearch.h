#ifndef _LOCATION_SEARCH_H
#define _LOCATION_SEARCH_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/ZoneData.h"
#include "Data/EncounterData.h"

class LocationSearch : public Module
{
public:
	LocationSearch() = delete;
	LocationSearch(Engine* const engine, u32 group);
	~LocationSearch();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	void TextInput(const char* label, string* text, u32 fileID, const vector<string>& list);

	bool EnableButton(int* params, u32 field, int nullValue, u32 eventID);
	bool ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID);
	bool InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID);

	void Search();
	bool SearchCheck(u32 locationIdx);
	bool SearchCheckEncounter(u32 encounterIdx);

	void ClearSearch();

	void AdvancedSearchMenu();

	int selectedIdx = -1;
	vector<u32> selectable = vector<u32>();

	string searchName = string();

	EncounterSlot encounterParams;

	bool advancedSearchMenu = false;
};

#endif // _LOCATION_SEARCH_H
