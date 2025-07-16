#include "Windows/Wizard.h"
#include "Windows/Engine.h"

int main(int argc, char* argv[])
{
	Project project;
	ReturnState state = OK;
	{
		Wizard* wizard = new Wizard();
		state = wizard->Init(WIZARD_WIDTH, WIZARD_HEIGHT, WIZARD_NAME, DEFAULT_FONT, (float)DEFAULT_FONT_SIZE);
		if (state != OK)
		{
			Log(CRITICAL, "Failed loading Wizard!");
			return 0;
		}
		while (state == OK)
			state = wizard->Update();
		if (state == ERROR)
		{
			Log(CRITICAL, "Error while running Wizard!");
			return 0;
		}
		project = wizard->GetProject();
		delete wizard;
	}
	if (state == EXIT)
		return 0;

	Engine* engine = new Engine(&project);
	state = engine->Init((int)project.width, (int)project.height, ENGINE_NAME, project.font, (float)project.fontSize);
	if (state != OK)
	{
		Log(CRITICAL, "Failed loading Engine!");
		return 0;
	}
	while (state == OK)
	{
		state = engine->Update();
		if (state == EXIT)
		{
			
		}
	}
	delete engine;

	return 0;
}