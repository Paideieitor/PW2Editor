#ifndef _MENU_BAR_H
#define _MENU_BAR_H

#include "Globals.h"

#include "Modules/Module.h"

#include "Utils/KlangFormat.h"

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

	bool patcherOptions = false;
	Klang patchSettings = Klang();

	void ChangeGroup(u32 newGroup);
	
	void PatcherOptions();
};

#endif // _MENU_BAR_H