#ifndef _WIZARD_H
#define _WIZARD_H

#include "Windows/Window.h"

#include "Data/Project.h"

#define WIZARD_WIDTH 390
#define WIZARD_HEIGHT 450

class Wizard : public Window
{
public:
	Wizard();
	~Wizard();

	Project GetProject() const;

protected:

	virtual ReturnState RenderGUI() override;

private:

	void LoadProjectList();

	void SortProjectListOrder();
	void VerifyProjectListOrder();
	
	void UpdateProjectOrder(Project& project, u32 order);
	void UpdateProjectListOrderOpen(u32 newFirstIdx);
	void UpdateProjectListOrderAdd();
	void UpdateProjectListOrderDelete(u32 deletedIdx);

	ReturnState CreateProject();

	int selectedIdx;
	vector<Project> projectList;

	bool showDeleteMessage;
};

#endif // _WIZARD_H

