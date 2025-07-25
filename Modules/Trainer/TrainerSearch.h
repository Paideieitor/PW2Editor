#ifndef _TRAINER_SEARCH_H
#define _TRAINER_SEARCH_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/TrainerData.h"

class TrainerSearch : public Module
{
public:
	TrainerSearch() = delete;
	TrainerSearch(Engine* const engine, u32 group);
	~TrainerSearch();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override {}

private:

	void ClearSearch();

	void Search();
	bool SearchCheck(u32 trIdx);
	bool SearchCheckTrainer(u32 trIdx);

	bool EnableButton(int* params, u32 field, int nullValue, u32 eventID);
	bool ComboBox(const char* label, const std::vector<std::string>& items, int* params, u32 field, int nullValue, u32 eventID);
	bool InputInt(const char* label, int maxValue, int* params, u32 field, int nullValue, u32 eventID);
	bool CheckBox(const char* label, int* params, u32 field, int nullValue, u32 eventID);
	void AdvancedSearchMenu();

	int selectedIdx = -1;
	vector<u32> selectable = vector<u32>();

	string searchName = string();
	TrainerData trainerParams;

	bool advancedSearchMenu = false;
};

#endif // _ITEM_SEARCH_H

