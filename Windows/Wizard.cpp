#include <algorithm>

#include "Externals/nativefiledialog/nfd.h"

#include "Utils/FileUtils.h"
#include "Utils/KlinFormat.h"

#include "Windows/Wizard.h"

Wizard::Wizard()
	: selectedIdx(0), projectList(vector<Project>()), showDeleteMessage(false)
{
	LoadProjectList();
}

Wizard::~Wizard()
{
}

Project Wizard::GetProject() const
{
	if (selectedIdx < 0 || selectedIdx >= projectList.size())
		return Project();
	return projectList[selectedIdx];
}

ReturnState Wizard::RenderGUI()
{
	ImGuiWindowFlags flags = 0;
	if (showDeleteMessage)
		flags |= ImGuiWindowFlags_NoInputs;
	ImGui::Begin("Project Selection", nullptr, flags);

	ImGui::Text("Selected Project:");
	if (projectList.size())
		ImGui::Text(projectList[selectedIdx].name);
	else
		ImGui::Text("There are no projects");

	ImGui::Separator();

	if (ImGui::Button("Create Project"))
	{
		switch (CreateProject())
		{
		case OK:
			selectedIdx = 0;
			break;
		case STOP:
			break;
		case ERROR:
			return ERROR;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Open Project"))
	{
		UpdateProjectListOrderOpen(selectedIdx);
		ImGui::End();
		return STOP;
	}

	ImGui::SameLine();

	if (ImGui::Button("Delete Project"))
		showDeleteMessage = true;

	if (showDeleteMessage)
	{
		ImGui::Begin("WARNING:");

		ImGui::Text("Are you sure you want to delete this project? This can't be undone!");
		if (ImGui::Button("Delete"))
		{
			string projectPath = string(PROJECTS_PATH) + PATH_SEPARATOR + projectList[selectedIdx].name;
			if (RemoveFolder(projectPath) != -1)
			{
				projectList.erase(projectList.begin() + selectedIdx);
				if (!projectList.size())
					selectedIdx = 0;
				else if (selectedIdx >= projectList.size())
					selectedIdx = (int)projectList.size() - 1;

				UpdateProjectListOrderDelete(selectedIdx);
				showDeleteMessage = false;
			}
			else
			{
				Log(WARNING, "Unable to delete project %s", projectPath.c_str());
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
			showDeleteMessage = false;

		ImGui::End();
	}

	ImGui::Separator();

	ImGui::Text("List of Projects:");

	if (projectList.size())
	{
		ImGui::BeginDisabled(showDeleteMessage);
		if (ImGui::BeginListBox(" "))
		{
			for (int projectIdx = 0; projectIdx < projectList.size(); ++projectIdx)
			{
				if (ImGui::Selectable(LABEL(projectList[projectIdx].name, projectIdx), selectedIdx == projectIdx))
					selectedIdx = projectIdx;
			}
			ImGui::EndListBox();
		}
		ImGui::EndDisabled();
	}

	ImGui::End();

	return OK;
}

void Wizard::LoadProjectList()
{
	projectList.reserve(16);

	if (!PathExists(PROJECTS_PATH))
	{
		Log(WARNING, "Projects folder does not exist, creating folder...");
		if (IsFilePath(PROJECTS_PATH))
		{
			Log(CRITICAL, "Unable to create projects folder");
			return;
		}
	}

	vector<string> folderList = GetFolderElementList(PROJECTS_PATH);
	for (u32 idx = 0; idx < folderList.size(); ++idx)
	{
		Project project;
		ReturnState state = LoadProjectSettings(project, folderList[idx]);
		switch (state)
		{
		case OK:
			projectList.push_back(project);
			break;
		case ERROR:
			folderList.erase(folderList.begin() + idx);
			--idx;
			break;
		}
	}

	SortProjectListOrder();

	VerifyProjectListOrder();
}

bool ProjectListOrderCmp(const Project& a, const Project& b)
{
	if (a.order < b.order)
		return true;
	return false;
}
void Wizard::SortProjectListOrder()
{
	sort(projectList.begin(), projectList.end(), ProjectListOrderCmp);
}

void Wizard::VerifyProjectListOrder()
{
	for (u32 idx = 0; idx < projectList.size(); ++idx)
		UpdateProjectOrder(projectList[idx], idx);
}

void Wizard::UpdateProjectOrder(Project& project, const u32 order)
{
	project.order = order;

	KlinHandler klin;
	LoadKlin(klin, project.settingsPath);
	if (!klin)
	{
		Log(WARNING, "Could not load project file (%s)", project.settingsPath.c_str());
		return;
	}

	SetKlinValueU32(klin, KLIN_PROJECT_ORDER, project.order);

	SaveKlin(klin, project.settingsPath, 1);
}

void Wizard::UpdateProjectListOrderOpen(u32 newFirstIdx)
{
	UpdateProjectOrder(projectList[newFirstIdx], 0);
	for (u32 idx = 0; idx < newFirstIdx; ++idx)
		UpdateProjectOrder(projectList[idx], projectList[idx].order + 1);

	SortProjectListOrder();
}

void Wizard::UpdateProjectListOrderAdd()
{
	for (u32 idx = 1; idx < projectList.size(); ++idx)
		UpdateProjectOrder(projectList[idx], projectList[idx].order + 1);

	SortProjectListOrder();
}

void Wizard::UpdateProjectListOrderDelete(u32 deletedIdx)
{
	for (u32 idx = deletedIdx; idx < projectList.size(); ++idx)
		UpdateProjectOrder(projectList[idx], projectList[idx].order - 1);

	SortProjectListOrder();
}

ReturnState Wizard::CreateProject()
{
	char* outPath = NULL;
	nfdresult_t result = NFD_OpenDialog(CTRMAP_FILE_EXTENSION, NULL, &outPath);

	switch (result)
	{
	case NFD_ERROR:
		Log(CRITICAL, "%s", NFD_GetError());
		return ERROR;
	case NFD_OKAY:
	{
		string path = outPath;
		free(outPath);
		NormalizePathSeparator(path);

		Project project;
		project.order = 0;

		project.ctrMapProjectPath = path;
		SeparatePathAndFile(project.ctrMapProjectPath, project.name);
		project.name = project.name.substr(0, project.name.length() - sizeof(CTRMAP_FILE_EXTENSION));

		for (u32 projectIdx = 0; projectIdx < projectList.size(); ++projectIdx)
		{
			if (project.name == projectList[projectIdx].name)
			{
				Log(WARNING, "Could not create project %s, there is already a project with the same name", project.name.c_str());
				return STOP;
			}
		}

		FILE* ctrProjectFile = nullptr;
		fopen_s(&ctrProjectFile, path.c_str(), "r");

		if (!ctrProjectFile)
		{
			Log(WARNING, "Could not open CTRMap project file (%s)", path.c_str());
			return STOP;
		}

		while (!feof(ctrProjectFile))
		{
			char line[1024];
			fgets(line, 1024, ctrProjectFile);
			string lineStr = line;
			bool isBaseLine = (bool)lineStr.find(CTRMAP_VFSBASE);
			if (isBaseLine == 0)
			{
				project.romPath = lineStr.substr(lineStr.find_first_of('\"') + 1);
				project.romPath = project.romPath.substr(0, project.romPath.length() - 2);
				NormalizePathSeparator(project.romPath);

				if (!PathExists(project.romPath))
				{
					Log(WARNING, "ROM path (%s) does not exist", project.romPath.c_str());
					fclose(ctrProjectFile);
					return STOP;
				}

				break;
			}
		}
		fclose(ctrProjectFile);

		project.path = string(PROJECTS_PATH) + PATH_SEPARATOR + project.name;
		if (!CreateFolder(project.path.c_str()))
		{
			Log(WARNING, "Unable to create %s project folder", project.name.c_str());
			return STOP;
		}

		project.settingsPath = project.path + PATH_SEPARATOR + SETTINGS_NAME + KLIN_TERMINATION;
		if (!CreateFile(project.settingsPath))
		{
			Log(WARNING, "Could not create project file (%s)", project.settingsPath.c_str());
			return STOP;
		}

		if (SaveProjectSettings(project) != OK)
		{
			Log(WARNING, "Could not save project in file (%s)", project.settingsPath.c_str());
			return STOP;
		}

		projectList.insert(projectList.begin(), project);

		UpdateProjectListOrderAdd();

		return OK;
	}
	case NFD_CANCEL:
		return STOP;
	}

	return ERROR;
}