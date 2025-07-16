#include "Utils/FileUtils.h"
#include "Utils/Alle5Format.h"

#include "Windows/Engine.h"

#include "Modules/Item/ItemText.h"

ItemText::ItemText(Engine* const engine, u32 group) : Module(engine, group, TEXT_NARC_PATH)
{
}

ItemText::~ItemText()
{
}

ReturnState ItemText::RenderGUI()
{
	ImGui::Begin("Item Text Data");

	TextInput("Name", &(engine->itemNames[engine->project->selectedItemIdx]), ITEM_NAME_FILE_ID, engine->itemNames);

	TextInput("Color", &(engine->itemColors[engine->project->selectedItemIdx]), ITEM_NAME_COLOR_FILE_ID, engine->itemColors);
	TextInput("Plural", &(engine->itemPlurals[engine->project->selectedItemIdx]), ITEM_NAME_PLURAL_FILE_ID, engine->itemPlurals);
	
	InputTextMultiline("Description", &(engine->itemDescriptions[engine->project->selectedItemIdx]),
		ITEM_DESCRIPTION_FILE_ID, engine->itemDescriptions);

	ImGui::Text(to_string(engine->project->selectedItemIdx));

	ImGui::End();
	return OK;
}

void ItemText::HandleReverseEvent(const Event* reverseEvent)
{
	string* value = (string*)reverseEvent->value;
	switch (reverseEvent->subType)
	{
	case ITEM_NAME_FILE_ID:
	{
		engine->itemNames[engine->project->selectedItemIdx] = *value;
		break;
	}
	case ITEM_DESCRIPTION_FILE_ID:
	{
		engine->itemDescriptions[engine->project->selectedItemIdx] = *value;
		break;
	}
	case ITEM_NAME_COLOR_FILE_ID:
	{
		engine->itemColors[engine->project->selectedItemIdx] = *value;
		break;
	}
	case ITEM_NAME_PLURAL_FILE_ID:
	{
		engine->itemPlurals[engine->project->selectedItemIdx] = *value;
		break;
	}
	}
}

void ItemText::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	vector<string>* originalData = (vector<string>*)saveEvent->value;
	vector<string>* data = nullptr;
	switch (saveEvent->subType)
	{
	case ITEM_NAME_FILE_ID:
		data = &engine->itemNames;
		break;
	case ITEM_DESCRIPTION_FILE_ID:
		data = &engine->itemDescriptions;
		break;
	case ITEM_NAME_COLOR_FILE_ID:
		data = &engine->itemColors;
		break;
	case ITEM_NAME_PLURAL_FILE_ID:
		data = &engine->itemPlurals;
		break;
	}
	if (!data || *originalData == *data)
		return;
	// Update the latest saved instance of the data for other changes
	saveEvent->value = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	SaveAlle5File(filePath, *data);
}

void ItemText::TextInput(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputText(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}

void ItemText::InputTextMultiline(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputTextMultiline(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}
