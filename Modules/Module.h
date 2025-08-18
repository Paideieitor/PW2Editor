#ifndef _MODULE_H
#define _MODULE_H

#include <string>

#include "Globals.h"

class Engine;

class Module
{
public:
	Module() = delete;
	Module(Engine* const engine, u32 group);
	~Module() {}

	const u32 group;

	virtual ReturnState RenderGUI() = 0;

protected:

	Engine* const engine;
};

#endif // _MODULE_H