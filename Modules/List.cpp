#include "Utils/StringUtils.h"

#include "Windows/Engine.h"

#include "Modules/List.h"

List::List(Engine* const engine, u32 group, u32* const selectedIdx, const vector<string>* const list) :
	Module(engine, group), selectedIdx(selectedIdx), list(list), searchName(string())
{
	Search();
}

ReturnState List::RenderGUI()
{
	ImGui::Begin("List");

	if (ImGui::InputText("Search", &searchName))
		Search();

	if (ImGui::BeginListBox(" ", ImVec2(ImGui::GetWindowWidth() - 15.0f, ImGui::GetWindowHeight() * 0.90f)))
	{
		for (u32 i = 0; i < (u32)selectable.size(); ++i)
		{
			u32 idx = selectable.at(i);
			if (ImGui::Selectable(LABEL(list->at(idx), idx), *selectedIdx == idx))
			{
				*selectedIdx = idx;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::End();
	return OK;
}

void List::Search()
{
	selectable.clear();

	for (u32 idx = 0; idx < (u32)list->size(); ++idx)
	{
		if (SearchCheck(idx))
		{
			selectable.emplace_back(idx);
		}
	}
}

bool List::SearchCheck(u32 itemIdx)
{
	const string& name = list->at(itemIdx);
	if (searchName.size())
	{
		string lowerName = LowerCase(name);
		string lowerSearchName = LowerCase(searchName);
		if (lowerName.find(lowerSearchName) == string::npos)
			if (!IsNumber(searchName[0]) || itemIdx != std::stoi(searchName))
				return false;
	}

	return true;
}