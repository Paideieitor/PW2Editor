#include "Utils/FileUtils.h"

#include "Windows/Engine.h"

#include "Modules/Pokemon/Personal.h"

Personal::Personal(Engine* const engine, u32 group) : Module(engine, group, PERSONAL_NARC_PATH)
{
}

Personal::~Personal()
{
}

ReturnState Personal::RenderGUI()
{
	ImGui::Begin("Personal");
	PersonalData& personal = engine->personal[engine->GetCurrentPokemon()->dataIdx];

	ImGui::Text("Base Stats");
	ImGui::BeginGroup();
	InputInt(personal, "HP", BASE_HP, 255);
	InputInt(personal, "Attack", BASE_ATK, 255);
	InputInt(personal, "Defense", BASE_DEF, 255);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	InputInt(personal, "Speed", BASE_SPE, 255);
	InputInt(personal, "Sp.Attack", BASE_SPA, 255);
	InputInt(personal, "Sp.Defense", BASE_SPD, 255);
	ImGui::EndGroup();

	ImGui::Text("Types");
	ComboBox(personal, "##Type 1", engine->types, TYPE_1);
	ImGui::SameLine();
	ComboBox(personal, "##Type 2", engine->types, TYPE_2);

	ImGui::Text("EVs");
	ImGui::BeginGroup();
	InputInt(personal, "HP##EV", EV_HP, 3);
	InputInt(personal, "Attack##EV", EV_ATK, 3);
	InputInt(personal, "Defense##EV", EV_DEF, 3);
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	InputInt(personal, "Speed##EV", EV_SPE, 3);
	InputInt(personal, "Sp.Attack##EV", EV_SPA, 3);
	InputInt(personal, "Sp.Defense##EV", EV_SPD, 3);
	ImGui::EndGroup();

	ImGui::Text("Abilities");
	ComboBox(personal, "##Ability 1", engine->abilities, ABILITY_1);
	ImGui::SameLine();
	ComboBox(personal, "##Ability 2", engine->abilities, ABILITY_2);
	ComboBox(personal, "##Ability Hidden", engine->abilities, ABILITY_HIDDEN);

	ImGui::BeginGroup();
	ImGui::Text("Wild Items");
	ComboBox(personal, "50%", engine->itemNames, WILD_ITEM_50);
	ComboBox(personal, "5%", engine->itemNames, WILD_ITEM_5);
	ComboBox(personal, "1%", engine->itemNames, WILD_ITEM_1);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Egg Groups");
	ComboBox(personal, "##Egg Group 1", engine->eggGroups, EGG_GROUP_1);
	ComboBox(personal, "##Egg Group 2", engine->eggGroups, EGG_GROUP_2);
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Text("Exp & Hapinness");
	InputInt(personal, "Base Exp", BASE_EXP, 255);
	InputInt(personal, "Egg Happiness", EGG_HAPPINESS, 255);
	InputInt(personal, "Base Happiness", BASE_HAPPINESS, 255);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Flags");
	CheckBox(personal, "Grounded Sprite", GROUNDED_SPRITE);
	CheckBox(personal, "Sprite Flip", SPRITE_FLIP);
	ImGui::EndGroup();

	ImGui::Text("Data");
	ImGui::BeginGroup();
	InputInt(personal, "Catch Rate", CATCH_RATE, 255);
	InputInt(personal, "Sex Chance", SEX_CHANCE, 255);
	InputInt(personal, "Growth Rate", GROWTH_RATE, 255);
	InputInt(personal, "Escape Rate", ESCAPE_RATE, 255);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	InputInt(personal, "Evo Stage", EVOLUTION_STAGE, 255);
	InputInt(personal, "Color", COLOR, 63);
	InputInt(personal, "Height", HEIGHT, 255);
	InputInt(personal, "Weight", WEIGHT, 255);
	ImGui::EndGroup();

	if (ImGui::Button("TMs & Tutors##Button"))
		moveMenu = !moveMenu;

	if (moveMenu)
	{
		ImGui::Begin("TMs & Tutors", &moveMenu);

		// TMs
		ImGui::BeginGroup();
		ImGui::Text("TMs");
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::BeginListBox("##TMs"))
		{
			for (u32 tmIdx = 0; tmIdx < (u32)tms.size() && tmIdx < TM_HM_COUNT * 32u; ++tmIdx)
			{
				u32 mask = 1 << (tmIdx % 32u);
				u32 idx = tmIdx / 32u;

				BitCheckBox(personal, tms[tmIdx].c_str(), (PersonalField)(TM_HM_1 + idx), mask);
			}
			ImGui::EndListBox();
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		// Type Tutors
		ImGui::BeginGroup();
		ImGui::Text("Type Tutors");
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::BeginListBox("##Type Tutors"))
		{
			for (u32 tutorIdx = 0; tutorIdx < (u32)typeTutors.size() && tutorIdx < 32u; ++tutorIdx)
			{
				u32 mask = 1 << (tutorIdx % 32u);
				BitCheckBox(personal, typeTutors[tutorIdx].c_str(), TYPE_TUTORS, mask);
			}
			ImGui::EndListBox();
		}
		ImGui::EndGroup();

		// Tutors
		for (u32 tutorIdx = 0; tutorIdx < (u32)tutors.size(); ++tutorIdx)
		{
			ImGui::BeginGroup();
			ImGui::Text(tutors[tutorIdx].first);
			ImGui::SetNextItemWidth(150.0f);
			if (ImGui::BeginListBox((string("##") + tutors[tutorIdx].first).c_str()))
			{
				for (u32 moveIdx = 0; moveIdx < (u32)tutors[tutorIdx].second.size() && moveIdx < 32u; ++moveIdx)
				{
					u32 mask = 1 << (moveIdx % 32u);
					BitCheckBox(personal, tutors[tutorIdx].second[moveIdx].c_str(),
						(PersonalField)(SPECIAL_TUTORS_1 + tutorIdx), mask);
				}
				ImGui::EndListBox();
			}
			ImGui::EndGroup();

			if (tutorIdx != 1)
				ImGui::SameLine();
		}

		ImGui::End();
	}

	ImGui::End();
	return OK;
}

void Personal::HandleReverseEvent(const Event* reverseEvent)
{
	int* value = (int*)reverseEvent->value;
	engine->personal[engine->GetCurrentPokemon()->dataIdx][reverseEvent->subType] = *value;
}

void Personal::HandleSaveEvent(Event* saveEvent)
{
	// Check that the data has changed in order to save it
	PersonalData* originalData = (PersonalData*)saveEvent->value;
	const PersonalData& data = engine->personal[saveEvent->subType];
	if (*originalData == data)
		return;
	// Update the latest saved instance of the data for other changes
	*((PersonalData*)saveEvent->value) = data;

	string filePath = MAKE_FILE_PATH(savePath, saveEvent->subType);
	engine->SavePersonal(data, filePath);
}

void Personal::ComboBox(PersonalData& personal, const char* label, const std::vector<std::string>& items, PersonalField field)
{
	int selected = personal[field];
	if (selected >= items.size())
	{
		InputInt(personal, label, field, 0xFFFF);
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(label, items, &selected))
	{
		SAVE_CHECK_REVERSE_EVENT(field, personal[field], selected, 
			engine->GetCurrentPokemon()->dataIdx, personal);
		personal[field] = selected;
	}
}

void Personal::InputInt(PersonalData& personal, const char* label, PersonalField field, int maxValue)
{
	int value = personal[field];
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::InputInt(LABEL(label, (int)field), &value))
	{
		if (value > maxValue)
			value = maxValue;
		if (value < 0)
			value = 0;

		SAVE_CHECK_REVERSE_EVENT(field, personal[field], value, 
			engine->GetCurrentPokemon()->dataIdx, personal);
		personal[field] = value;
	}
}

void Personal::CheckBox(PersonalData& personal, const char* label, PersonalField field)
{
	bool value = (bool)personal[field];
	if (ImGui::Checkbox(label, &value))
	{
		SAVE_CHECK_REVERSE_EVENT(field, personal[field], (int)value, 
			engine->GetCurrentPokemon()->dataIdx, personal);
		personal[field] = (int)value;
	}
}

void Personal::BitCheckBox(PersonalData& personal, const char* label, PersonalField field, u32 mask)
{
	int data = personal[field];
	bool value = (data & mask) != 0;
	if (ImGui::Checkbox(label, &value))
	{
		value ? (data | mask) : (data & ~mask);

		SAVE_CHECK_REVERSE_EVENT(field, personal[field], data,
			engine->GetCurrentPokemon()->dataIdx, personal);
		personal[field] = data;
	}
}
