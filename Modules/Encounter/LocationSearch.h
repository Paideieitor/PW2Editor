#ifndef _LOCATION_SEARCH_H
#define _LOCATION_SEARCH_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/ZoneData.h"

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

	void Search();
	bool SearchCheck(u32 locationIdx);

	int selectedIdx = -1;
	vector<u32> selectable = vector<u32>();

	string searchName = string();
};

#endif // _LOCATION_SEARCH_H
