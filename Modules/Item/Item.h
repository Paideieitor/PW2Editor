#ifndef _ITEM_H
#define _ITEM_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/ItemData.h"

class Item : public Module
{
public:
	Item() = delete;
	Item(Engine* const engine, u32 group);
	~Item();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	void ComboBox(ItemData& itemData, const char* label, const std::vector<std::string>& items, ItemField field);
	void InputInt(ItemData& itemData, const char* label, ItemField field, int maxValue);
	void CheckBox(ItemData& itemData, const char* label, ItemField field);

	ItemData itemCopy;
};

#endif // _ITEM_H