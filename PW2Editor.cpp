#include <stdio.h>
#include <iostream>

#include "Windows/Wizard.h"
#include "Windows/Engine.h"

int WinMain(int argc, char* argv[])
{
	CreateFolder("Logs");

	Project project;
	ReturnState state = OK;
	{
		Log(INFO, "Starting Wizard");
		Wizard* wizard = new Wizard();
		state = wizard->Init(WIZARD_WIDTH, WIZARD_HEIGHT, WIZARD_NAME, DEFAULT_FONT, (float)DEFAULT_FONT_SIZE);
		if (state != OK)
		{
			Log(CRITICAL, "Failed loading Wizard!");
			return 0;
		}
		Log(INFO, "Wizard Loop");
		while (state == OK)
			state = wizard->Update();
		if (state == ERROR)
		{
			Log(CRITICAL, "Error while running Wizard!");
			return 0;
		}
		Log(INFO, "Wizard selected a project");
		project = wizard->GetProject();
		Log(INFO, "    Selected project: %s", project.name.c_str());
		delete wizard;
	}
	if (state == EXIT)
	{
		Log(INFO, "Wizard closed");
		return 0;
	}
	
	Log(INFO, "Starting Engine");
	Engine* engine = new Engine(&project);
	state = engine->Init((int)project.width, (int)project.height, ENGINE_NAME, project.font, (float)project.fontSize);
	if (state != OK)
	{
		Log(CRITICAL, "Failed loading Engine!");
		return 0;
	}
	Log(INFO, "Engine Loop");
	while (state == OK)
	{
		state = engine->Update();
		if (state == EXIT)
		{
	
		}
	}
	delete engine;
	
	Log(INFO, "Engine closed");
	return 0;
}