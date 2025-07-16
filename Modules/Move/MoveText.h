#ifndef _MOVE_TEXT_H
#define _MOVE_TEXT_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

class MoveText : public Module
{
public:
	MoveText() = delete;
	MoveText(Engine* const engine, u32 group);
	~MoveText();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override;
	virtual void HandleGroupEvent(Event* groupEvent) override;

private:

	bool TextInput(const char* label, string* text, u32 fileID, const vector<string>& list);
	void InputTextMultiline(const char* label, string* text, u32 fileID, const vector<string>& list);
};

#endif // _MOVE_TEXT_H