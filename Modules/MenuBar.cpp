#include "Windows/Engine.h"

#include "Modules/MenuBar.h"

MenuBar::MenuBar(Engine* const engine, u32 group) : Module(engine, group, "")
{
}

MenuBar::~MenuBar()
{
}

ReturnState MenuBar::RenderGUI()
{
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		engine->commandInput = true;
	if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
		engine->commandInput = false;

	if (engine->commandInput)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_S))
		{
			engine->Save();
			ImGui::Text("Saved");
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Z))
			engine->HandleReverseEvent();

		for (u32 group = POKEMON_GROUP; group <= TRAINER_GROUP; ++group)
			if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_0 + group)))
				ChangeGroup(group);
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				engine->Save();

			if (ImGui::MenuItem("Undo", "Ctrl+Z"))
				engine->HandleReverseEvent();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Editors"))
		{
			if (ImGui::MenuItem(u8"Pokémon", "Ctrl+1"))
				ChangeGroup(POKEMON_GROUP);
			if (ImGui::MenuItem("Items", "Ctrl+2"))
				ChangeGroup(ITEM_GROUP);
			if (ImGui::MenuItem("Moves", "Ctrl+3"))
				ChangeGroup(MOVE_GROUP);
			if (ImGui::MenuItem("Encounters", "Ctrl+4"))
				ChangeGroup(ENCOUNTER_GROUP);
			if (ImGui::MenuItem("Trainers", "Ctrl+5"))
				ChangeGroup(TRAINER_GROUP);

			ImGui::EndMenu();
		}

		ImGui::BeginDisabled(!engine->PMCCheck());
		if (ImGui::BeginMenu("Patcher"))
		{
			if (ImGui::MenuItem("Options"))
			{
				patcherOptions = !patcherOptions;
				if (patcherOptions)
				{
					string patchSettingsPath = DEPLOY_PATCH_DIR; 
					if (!PathExists(patchSettingsPath))
						patchSettingsPath = DEV_PATCH_DIR;

					patchSettingsPath = ConcatPath(patchSettingsPath, "settings.h");
					LoadKlang(patchSettings, patchSettingsPath);
				}
			}

			bool patchInstalled = engine->PatchIsInstalled();

			string buildText = "Build";
			if (patchInstalled)
				buildText = "Rebuild";
			if (ImGui::MenuItem(buildText.c_str()))
			{
				engine->Save();
				engine->BuildPatch();
				engine->Quit(PATCH_QUIT_MESSAGE);
			}

			ImGui::BeginDisabled(!patchInstalled);
			if (ImGui::MenuItem("Uninstall"))
			{
				engine->Save();
				engine->UninstallPatch();
				engine->Quit(PATCH_QUIT_MESSAGE);
			}
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		ImGui::EndDisabled();

		ImGui::EndMainMenuBar();
	}

	if (patcherOptions)
	{
		PatcherOptions();
	}
			
	return OK;
}

void MenuBar::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case 0:
		u32* value = (u32*)reverseEvent->value;
		engine->project->group = *value;
		break;
	}
}

void MenuBar::ChangeGroup(u32 newGroup)
{
	SIMPLE_REVERSE_EVENT(0, engine->project->group, newGroup);
	engine->project->group = newGroup;
}

void PatchSetting(KlangVar& var, bool isChild)
{
	string text;
	if (isChild)
		text += "    ";
	text += var.name;

	ImGui::Text(text);
	ImGui::SameLine();

	bool value = (bool)var.Value();
	if (ImGui::Checkbox((string("##") + var.name).c_str(), &value))
		var.SetValue((int)value);
}

void MenuBar::PatcherOptions()
{
	ImGui::Begin("Patcher Options", &patcherOptions);

	for (u32 setting = 0; setting < (u32)patchSettings.vars.size(); ++setting)
	{
		KlangVar& parent = patchSettings.vars[setting];
		PatchSetting(parent, false);

		ImGui::BeginDisabled(parent.Value() == 0);
		for (u32 child = 0; child < (u32)parent.dependentVars.size(); ++child)
		{
			KlangVar& dependent = parent.dependentVars[child];
			PatchSetting(dependent, true);
		}
		ImGui::EndDisabled();

		ImGui::Separator();
	}

	ImGui::End();
}
