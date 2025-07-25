#ifndef _MOVE_SEARCH_H
#define _MOVE_SEARCH_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/MoveData.h"

class MoveSearch : public Module
{
public:
	MoveSearch() = delete;
	MoveSearch(Engine* const engine, u32 group);
	~MoveSearch();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;
	virtual void HandleGroupEvent(Event* groupEvent) override;

private:

	void ClearSearch();

	void Search();
	bool SearchCheck(u32 itemIdx);
	bool SearchCheckMove(u32 itemIdx);

	bool EnableButton(int* params, u32 field, int nullValue, u32 eventID);
	bool ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID);
	bool InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID, bool allowNegative);
	bool CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID);
	void AdvancedSearchMenu();

	int selectedIdx = -1;
	vector<u32> selectable = vector<u32>();

	string searchName = string();
	MoveData moveParams;

	bool advancedSearchMenu = false;
};

#endif // _MOVE_SEARCH_H
