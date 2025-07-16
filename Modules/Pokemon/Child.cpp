#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/Child.h"

Child::Child(Engine* const engine, u32 group) : Module(engine, group, CHILD_NARC_PATH)
{
}

Child::~Child()
{
}

ReturnState Child::RenderGUI()
{
	ImGui::Begin("Child");
	Pokemon* pkm = &(engine->pokemon[engine->project->selectedPkmIdx]);
	if (pkm->idx >= engine->child.size())
	{
		ImGui::End();
		return OK;
	}
	
	ChildData previous = engine->child[pkm->idx];
	if (ImGui::TextInputComboBox("##Child", engine->pkmNames, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(0, engine->child[pkm->idx], previous, pkm->idx, engine->child[pkm->idx]);
		engine->child[pkm->idx] = previous;
	}

	ImGui::End();
	return OK;
}

void Child::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case 0:
	{
		ChildData* value = (ChildData*)reverseEvent->value;
		engine->child[engine->GetCurrentPokemon()->idx] = *value;
		break;
	}
	}
}

void Child::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	ChildData* originalData = (ChildData*)saveEvent->value;
	const ChildData& data = engine->child[saveEvent->subType];
	if (*originalData == data)
		return;
	// Update the latest saved instance of the data for other changes
	*((ChildData*)saveEvent->value) = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SaveChild(data, filePath);
}
