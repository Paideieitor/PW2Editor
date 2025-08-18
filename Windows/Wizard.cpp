#include <algorithm>

#include "Externals/nativefiledialog/nfd.h"

#include "Utils/FileUtils.h"
#include "Utils/StringUtils.h"
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
		ImGui::Text(projectList.at(selectedIdx).name);
	else
		ImGui::Text("There are no projects");

	ImGui::Separator();

	if (ImGui::Button("Load Project"))
	{
		Log(INFO, "Wizard Loading Project");
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

	ImGui::BeginDisabled(projectList.empty());
	if (ImGui::Button("Open Project"))
	{
		Log(INFO, "Wizard Opening Project!");
		UpdateProjectListOrderOpen(selectedIdx);
		ImGui::EndDisabled();
		ImGui::End();
		return STOP;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Delete Project"))
		showDeleteMessage = true;

	if (showDeleteMessage)
	{
		ImGui::Begin("WARNING:");

		ImGui::Text("Do you want to delete this projects data? This can't be undone!");

		bool removeButton = ImGui::Button("No, only remove");
		ImGui::SameLine();
		bool deleteButton = ImGui::Button("Yes, delete data");
		if (removeButton || deleteButton)
		{
			Log(INFO, "Wizard Removing Project");

			if (deleteButton)
			{
				Log(INFO, "Wizard Deleteing Project!");
				string projectPath = projectList.at(selectedIdx).path;
				if (RemoveFolder(projectPath) != -1)
					Log(INFO, "Wizard Deleted Project at %s!", projectPath.c_str());
				else
					Log(WARNING, "Unable to delete project %s", projectPath.c_str());
			}

			projectList.erase(projectList.begin() + selectedIdx);
			if (!projectList.size())
				selectedIdx = 0;
			else if (selectedIdx >= projectList.size())
				selectedIdx = (int)projectList.size() - 1;

			UpdateProjectListOrderDelete(selectedIdx);
			showDeleteMessage = false;

			SetProjectPaths(PROJECTS_PATH);
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
			for (int projectIdx = 0; projectIdx < (int)projectList.size(); ++projectIdx)
			{
				Project& project = projectList.at(projectIdx);
				if (ImGui::Selectable(LABEL(project.name, projectIdx), selectedIdx == projectIdx))
					selectedIdx = projectIdx;
			}
			ImGui::EndListBox();
		}
		ImGui::EndDisabled();
	}

	ImGui::End();

	return OK;
}

vector<string> Wizard::GetProjectPaths(const string& path)
{
	vector<string> projectPaths;

	FILE* file;
	fopen_s(&file, path.c_str(), "r");
	if (file)
	{
		char line[1024];
		fgets(line, 1024, file);
		while (!feof(file))
		{
			string lineStr = line;
			CleanLine(lineStr);
			projectPaths.emplace_back(lineStr);

			fgets(line, 1024, file);
		}
		fclose(file);
	}

	return projectPaths;
}

void Wizard::SetProjectPaths(const string& path)
{
	FILE* file;
	fopen_s(&file, path.c_str(), "w");
	if (file)
	{
		for (u32 idx = 0; idx < (u32)projectList.size(); ++idx)
		{
			fwrite(projectList.at(idx).path.c_str(), sizeof(char), projectList.at(idx).path.size(), file);
			fputc('\n', file);
		}
		fclose(file);
	}
}

void Wizard::LoadProjectList()
{
	projectList.reserve(16);

	vector<string> folderList;
	if (PathExists(PROJECTS_PATH))
		folderList = GetProjectPaths(PROJECTS_PATH);
	
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

		project.ctrMapProjectDir = path;
		SeparatePathAndFile(project.ctrMapProjectDir, project.name);
		project.name = PathRemoveExtension(project.name);

		for (u32 projectIdx = 0; projectIdx < (u32)projectList.size(); ++projectIdx)
		{
			if (project.name == projectList.at(projectIdx).name)
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
				project.romDir = lineStr.substr(lineStr.find_first_of('\"') + 1);
				project.romDir = project.romDir.substr(0, project.romDir.length() - 2);
				NormalizePathSeparator(project.romDir);

				if (!PathExists(project.romDir))
				{
					Log(WARNING, "ROM path (%s) does not exist", project.romDir.c_str());
					fclose(ctrProjectFile);
					return STOP;
				}

				break;
			}
		}
		fclose(ctrProjectFile);

		FILE* headerFile = nullptr;
		string headerPath = PathConcat(project.romDir, ROM_HEADER_NAME);
		fopen_s(&headerFile, headerPath.c_str(), "rb");
		if (!headerFile)
		{
			Log(WARNING, "Could not open ROM header file (%s)", headerPath.c_str());
			return STOP;
		}

		char headerData[16];
		if (fread_s(headerData, 16, sizeof(char), 16, headerFile) != 16)
		{
			Log(WARNING, "Could not read ROM header data (%s)", headerPath.c_str());
			fclose(headerFile);
			return STOP;
		}

		u32 headerIdx = 0;
		for (; headerIdx < 12; ++headerIdx)
			project.game.push_back(headerData[headerIdx]);
		for (; headerIdx < 16; ++headerIdx)
			project.gameCode.push_back(headerData[headerIdx]);

		CleanLine(project.game);
		fclose(headerFile);

		project.path = PathConcat(project.ctrMapProjectDir, CTRMAP_PW2_DIR);
		if (PathExists(project.path))
		{
			if (LoadProjectSettings(project, project.path) != OK)
			{
				Log(WARNING, "Could not load existing project (%s)", project.path.c_str());
				return STOP;
			}

			Log(INFO, "Loaded Project %s", project.name.c_str());
		}
		else
		{
			if (!CreateFolder(project.path.c_str()))
			{
				Log(WARNING, "Unable to create %s project folder", project.name.c_str());
				return STOP;
			}


			project.settingsPath = PathConcat(project.path, string(SETTINGS_NAME) + KLIN_TERMINATION);
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

			Log(INFO, "Created Project %s", project.name.c_str());
		}
		
		projectList.insert(projectList.begin(), project);

		UpdateProjectListOrderAdd();

		SetProjectPaths(PROJECTS_PATH);

		return OK;
	}
	case NFD_CANCEL:
		return STOP;
	}

	Log(INFO, "Error Creating Project!");
	return ERROR;
}