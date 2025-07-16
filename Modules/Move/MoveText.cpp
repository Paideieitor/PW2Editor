#include "Utils/FileUtils.h"
#include "Utils/Alle5Format.h"
#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/Move/MoveText.h"

MoveText::MoveText(Engine* const engine, u32 group) : Module(engine, group, TEXT_NARC_PATH)
{
}

MoveText::~MoveText()
{
}

ReturnState MoveText::RenderGUI()
{
	ImGui::Begin("Move Text Data");
	u32 moveIdx = engine->project->selectedMoveIdx;

	if (TextInput("Name", &(engine->moveNames[moveIdx]), MOVE_NAME_FILE_ID, engine->moveNames))
	{
		string& name = engine->moveNames[moveIdx];

		SAVE_CHECK_REVERSE_EVENT(MOVE_NAME_MAYUS_FILE_ID, engine->moveNamesMayus[moveIdx], UpperCase(name), MOVE_NAME_MAYUS_FILE_ID, engine->moveNamesMayus);
		engine->moveNamesMayus[moveIdx] = UpperCase(name);

		string battlePrefix[] = {"", "The wild ", "The foe's "};
		string battleMessage;
		battleMessage.reserve(64);
		for (u32 idx = 0; idx < 3; ++idx)
		{
			battleMessage.clear();
			battleMessage = battlePrefix[idx] + u8"[Pokémon Nick(0)] used\n" + name + "!";
			SAVE_CHECK_REVERSE_EVENT(MOVE_USE_FILE_ID, engine->moveUses[(moveIdx * 3) + idx], battleMessage, MOVE_USE_FILE_ID, engine->moveUses);
			engine->moveUses[(moveIdx * 3) + idx] = battleMessage;
		}
	}

	ImGui::Text(engine->moveNamesMayus[moveIdx]);
	ImGui::Separator();
	for (u32 idx = 0; idx < 3; ++idx)
		ImGui::Text(engine->moveUses[(moveIdx * 3) + idx]);
	ImGui::Separator();
	InputTextMultiline("Description", &(engine->moveDescriptions[moveIdx]),
		MOVE_DESCRIPTION_FILE_ID, engine->moveDescriptions);

	ImGui::Text(to_string(moveIdx));

	ImGui::End();
	return OK;
}

void MoveText::HandleReverseEvent(const Event* reverseEvent)
{
	string* value = (string*)reverseEvent->value;
	u32 moveIdx = engine->project->selectedMoveIdx;

	switch (reverseEvent->subType)
	{
	case MOVE_NAME_FILE_ID:
	{
		engine->moveNames[moveIdx] = *value;
		break;
	}
	case MOVE_NAME_MAYUS_FILE_ID:
	{
		engine->moveNamesMayus[moveIdx] = *value;
		break;
	}
	case MOVE_USE_FILE_ID:
	{
		engine->moveUses[moveIdx] = *value;
		break;
	}
	case MOVE_DESCRIPTION_FILE_ID:
	{
		engine->moveDescriptions[moveIdx] = *value;
		break;
	}
	}
}

void MoveText::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	vector<string>* originalData = (vector<string>*)saveEvent->value;
	vector<string>* data = nullptr;
	switch (saveEvent->subType)
	{
	case MOVE_NAME_FILE_ID:
		data = &engine->moveNames;
		break;
	case MOVE_NAME_MAYUS_FILE_ID:
		data = &engine->moveNamesMayus;
		break;
	case MOVE_USE_FILE_ID:
		data = &engine->moveUses;
		break;
	case MOVE_DESCRIPTION_FILE_ID:
		data = &engine->moveDescriptions;
		break;
	}
	if (!data || (originalData!= nullptr && *originalData == *data))
		return;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	SaveAlle5File(filePath, *data);
}

void MoveText::HandleGroupEvent(Event* groupEvent)
{
	engine->AddSaveEvent(moduleIdx, MOVE_NAME_FILE_ID, (u32)nullptr);
	engine->AddSaveEvent(moduleIdx, MOVE_NAME_MAYUS_FILE_ID, (u32)nullptr);
	engine->AddSaveEvent(moduleIdx, MOVE_USE_FILE_ID, (u32)nullptr);
	engine->AddSaveEvent(moduleIdx, MOVE_DESCRIPTION_FILE_ID, (u32)nullptr);
}

bool MoveText::TextInput(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputText(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
		return true;
	}
	return false;
}

void MoveText::InputTextMultiline(const char* label, string* text, u32 fileID, const vector<string>& list)
{
	string previous = *text;
	if (ImGui::InputTextMultiline(label, &previous))
	{
		SAVE_CHECK_REVERSE_EVENT(fileID, *text, previous, fileID, list);
		*text = previous;
	}
}
