#ifndef _ITEM_MODULE_H
#define _ITEM_MODULE_H

#include "Modules/Module.h"

class Item : public Module
{
public:
	Item() = delete;
	Item(Engine* const engine, u32 group);
	~Item() {}

	virtual ReturnState RenderGUI() override;

private:

	const vector<string>* const types;

	const vector<string> pockets = {
		"Items",
		"Medicine",
		"TMs & HMs",
		"Berries",
		"Key Items",
	};
};

#endif // _ITEM_MODULE_H
