#include "Utils/FileUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/PokemonText.h"

PokemonText::PokemonText(Engine* const engine, u32 group) : Module(engine, group, TEXT_NARC_PATH)
{
}

PokemonText::~PokemonText()
{
}

ReturnState PokemonText::RenderGUI()
{
	ImGui::Begin("Text Data");
	Pokemon* pkm = engine->GetCurrentPokemon();

	TextInput("Name", &(engine->pkmNames[pkm->idx]), PKM_NAME_FILE_ID, engine->pkmNames);
	if (pkm->pokeStudio)
	{
		ImGui::End();
		return OK;
	}
	if (pkm->forms.size())
		TextInput("FormName", &(engine->formNames[pkm->textIdx]), PKM_FORM_NAME_FILE_ID, engine->formNames);

	TextInput("Title", &(engine->pkmTitles[pkm->idx]), PKM_TITLE_FILE_ID, engine->pkmTitles);

	InputTextMultiline("Description", &(engine->pkmDescriptions[pkm->textIdx]),
		PKM_DESCRIPTION_FILE_ID, engine->pkmDescriptions);

	ImGui::End();
	return OK;
}

void PokemonText::HandleReverseEvent(const Event* reverseEvent)
{
	if (reverseEvent->subType == 0)
	{
		u32* value = (u32*)reverseEvent->value;
		engine->project->selectedPkmForm = *value;
		return;
	}

	Pokemon* pkm = engine->GetCurrentPokemon();
	string* value = (string*)reverseEvent->value;
	switch (reverseEvent->subType)
	{
	case PKM_NAME_FILE_ID:
	{
		engine->pkmNames[pkm->idx] = *value;
		break;
	}
	case PKM_FORM_NAME_FILE_ID:
	{
		engine->formNames[pkm->textIdx] = *value;
		break;
	}
	case PKM_DESCRIPTION_FILE_ID:
	{
		engine->pkmDescriptions[pkm->textIdx] = *value;
		break;
	}
	case PKM_TITLE_FILE_ID:
	{
		engine->pkmTitles[pkm->idx] = *value;
		break;
	}
	}
}

void PokemonText::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	vector<string>* originalData = (vector<string>*)saveEvent->value;
	vector<string>* data = nullptr;
	switch (saveEvent->subType)
	{
	case PKM_NAME_FILE_ID:
		data = &engine->pkmNames;
		break;
	case PKM_FORM_NAME_FILE_ID:
		data = &engine->formNames;
		break;
	case PKM_DESCRIPTION_FILE_ID:
		data = &engine->pkmDescriptions;
		break;
	case PKM_TITLE_FILE_ID:
		data = &engine->pkmTitles;
		break;
	}
	if (!data || *originalData == *data)
		return;
	// Update the latest saved instance of the data for other changes
	saveEvent->value = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	SaveAlle5File(filePath, *data);
}

void PokemonText::TextInput(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputText(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}

void PokemonText::InputTextMultiline(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputTextMultiline(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}
