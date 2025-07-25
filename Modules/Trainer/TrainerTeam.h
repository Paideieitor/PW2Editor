#ifndef _TRAINER_TEAM_H
#define _TRAINER_TEAM_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/TrainerData.h"

class TrainerTeama : public Module
{
public:
	TrainerTeama() = delete;
	TrainerTeama(Engine* const engine, u32 group);
	~TrainerTeama();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	void ComboBox(TrainerData& trainerData, const char* label, const std::vector<std::string>& items, TrainerField field);
	void InputInt(TrainerData& trainerData, const char* label, TrainerField field, int maxValue);
	void CheckBox(TrainerData& trainerData, const char* label, TrainerField field);
};

#endif // _TRAINER_TEAM_H

