#ifndef _MOVE_H
#define _MOVE_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/MoveData.h"

class Move : public Module
{
public:
	Move() = delete;
	Move(Engine* const engine, u32 group);
	~Move();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;
	virtual void HandleGroupEvent(Event* groupEvent) override;

private:

	void ComboBox(MoveData& itemData, const char* label, const std::vector<std::string>& items, MoveField field);
	void InputInt(MoveData& itemData, const char* label, MoveField field, int maxValue, bool allowNegative = false);
	void CheckBox(MoveData& itemData, const char* label, MoveField field);
};

#endif // _MOVE_H
