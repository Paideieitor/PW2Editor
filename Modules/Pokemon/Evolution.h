#ifndef _EVOLUTION_H
#define _EVOLUTION_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/EvolutionData.h"

class Evolution : public Module
{
public:
	Evolution() = delete;
	Evolution(Engine* const engine, u32 group);
	~Evolution();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override;

private:

	void ComboBox(EvolutionData& evolution, const char* label, const std::vector<std::string>& items, u32 idx, EvolutionField field);
	void InputInt(EvolutionData& evolution, const char* label, u32 idx, int maxValue);
};

#endif // _EVOLUTION_H