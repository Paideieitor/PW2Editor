#ifndef _CHILD_H
#define _CHILD_H

#include "Globals.h"

#include "Modules/Module.h"

class Child : public Module
{
public:
	Child() = delete;
	Child(Engine* const engine, u32 group);
	~Child();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override;
};

#endif // _CHILD_H