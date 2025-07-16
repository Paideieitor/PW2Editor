#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/PokemonForm.h"

#include "Data/ChildData.h"

PokemonForm::PokemonForm(Engine* const engine, u32 group) : Module(engine, group, "")
{
}

PokemonForm::~PokemonForm()
{
}

ReturnState PokemonForm::RenderGUI()
{
	ImGui::Begin(u8"Pokémon");
	if (ImGui::BeginTabBar("Form Tabs"))
	{
		Pokemon* basePkm = &(engine->pokemon[engine->project->selectedPkmIdx]);

		Pokemon* pkm = basePkm;
		string tabLabel = engine->pkmNames[basePkm->idx];
		if (basePkm->forms.size())
			tabLabel = engine->formNames[basePkm->textIdx];
		u32 formIdx = 0;
		for (; formIdx < (u32)basePkm->forms.size() + 1; ++formIdx)
		{
			// Skip for the Base Form
			if (formIdx != 0)
			{
				pkm = &(basePkm->forms[formIdx - 1]);
				tabLabel = engine->formNames[pkm->textIdx];

				// Skip forms without data
				if (!PokemonFormHasData(pkm))
					continue;
			}

			ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
			if (engine->project->selectedPkmForm == formIdx)
				flags |= ImGuiTabItemFlags_SetSelected;

			bool tabOutput = ImGui::BeginTabItem(tabLabel.c_str(), 0, flags);
			if (ImGui::IsItemClicked())
			{
				if (engine->project->selectedPkmForm != formIdx)
				{
					SIMPLE_REVERSE_EVENT(0, engine->project->selectedPkmForm, formIdx);
					engine->SetCurrentPokemon(engine->project->selectedPkmIdx, formIdx);
				}
			}

			if (tabOutput)
				ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
	return OK;
}

void PokemonForm::HandleReverseEvent(const Event* reverseEvent)
{
	switch (reverseEvent->subType)
	{
	case 0: // Change Form tab
	{
		u32* value = (u32*)reverseEvent->value;
		engine->SetCurrentPokemon(engine->project->selectedPkmIdx, *value);
		break;
	}
	}
}
