#ifndef _MODULE_H
#define _MODULE_H

#include <string>

#include "Globals.h"

class Engine;
struct Event;

class Module
{
public:
	Module() = delete;
	Module(Engine* const engine, u32 group, const string& fsPath);
	~Module() {}

	const u32 group;

	virtual ReturnState RenderGUI() = 0;

	virtual void HandleReverseEvent(const Event* reverseEvent) = 0;
	virtual void HandleSaveEvent(Event* saveEvent) = 0;
	virtual void HandleGroupEvent(Event* groupEvent) {}

protected:

	Engine* const engine;

	const u32 moduleIdx;
	const string savePath;
};

#endif // _MODULE_H