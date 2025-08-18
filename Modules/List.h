#ifndef _POKEMON_LIST_H
#define _POKEMON_LIST_H

#include <vector>

#include "Modules/Module.h"

class List : public Module
{
public:
	List() = delete;
	List(Engine* const engine, u32 group, u32* const selectedIdx, const vector<string>* const list);
	~List() {}

	virtual ReturnState RenderGUI() override;

private:

	void Search();
	bool SearchCheck(u32 idx);

	u32* const selectedIdx;
	const vector<string>* const list;

	vector<u32> selectable;
	string searchName;
};

#endif // _POKEMON_LIST_H
