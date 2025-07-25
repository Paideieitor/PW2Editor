#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Trainer/TrainerTeam.h"

TrainerTeam::TrainerTeam(Engine* const engine, u32 group) : Module(engine, group, TRAINER_POKEMON_NARC_PATH)
{
}

TrainerTeam::~TrainerTeam()
{
}

ReturnState TrainerTeam::RenderGUI()
{
	return ReturnState();
}

void TrainerTeam::HandleReverseEvent(const Event* reverseEvent)
{
}

void TrainerTeam::HandleSaveEvent(Event* saveEvent)
{
}

void TrainerTeam::ComboBox(TrainerData& trainerData, const char* label, const std::vector<std::string>& items, TrainerField field)
{
}

void TrainerTeam::InputInt(TrainerData& trainerData, const char* label, TrainerField field, int maxValue)
{
}

void TrainerTeam::CheckBox(TrainerData& trainerData, const char* label, TrainerField field)
{
}
