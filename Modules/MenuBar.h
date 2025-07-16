#ifndef _MENU_BAR_H
#define _MENU_BAR_H

#include "Globals.h"

#include "Modules/Module.h"

struct Patch;

class MenuBar : public Module
{
public:
	MenuBar() = delete;
	MenuBar(Engine* const engine, u32 group);
	~MenuBar();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override {}

private:

	void ChangeGroup(u32 newGroup);
};

#endif // _MENU_BAR_H