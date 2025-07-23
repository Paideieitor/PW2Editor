#ifndef _TRAINER_TEAM_H
#define _TRAINER_TEAM_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/TrainerData.h"

class TrainerTeam : public Module
{
public:
	TrainerTeam() = delete;
	TrainerTeam(Engine* const engine, u32 group);
	~TrainerTeam();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	void ComboBox(TrainerTeamData& team, u32 slot, const char* label, const std::vector<std::string>& items, TrainerPokemonField field);
	void InputInt(TrainerTeamData& team, u32 slot, const char* label, TrainerPokemonField field, int maxValue);
	void CheckBox(TrainerTeamData& team, u32 slot, const char* label, TrainerPokemonField field);
};

#endif // _TRAINER_TEAM_H

