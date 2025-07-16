#ifndef _LEARNSET_H
#define _LEARNSET_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/LearnsetData.h"

class Learnset : public Module
{
public:
	Learnset() = delete;
	Learnset(Engine* const engine, u32 group);
	~Learnset();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override;

	static vector<pair<u32, int>> GetOrderedLearnset(const LearnsetData& learnset);

private:

	void ComboBox(LearnsetData& personal, const char* label, const std::vector<std::string>& items, u32 idx);
	void InputInt(LearnsetData& personal, const char* label, u32 idx, int maxValue);
};

#endif // _LEARNSET_H