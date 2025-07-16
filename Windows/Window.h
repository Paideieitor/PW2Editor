#ifndef _WINDOW_H
#define _WINDOW_H

#define NULL_MODULE 0
#define WIZARD_MODULE 1
#define ENGINE_MODULE 2

#include <vector>
#include <string>

#include "Externals/glad/glad.h"
#include "Externals/glfw3/glfw3.h"
#include "Externals/imgui/imgui.h"
#include "Externals/imgui/imgui_impl_glfw.h"
#include "Externals/imgui/imgui_impl_opengl3.h"
#include "Externals/imgui/imgui_stdlib.h"

#include "Globals.h"
#include "Log.h"

class Window
{
public:
	Window();
	~Window();

	ReturnState Init(int width, int height, const string& name, const string& font, float fontSize);
	ReturnState Update();

protected:

	virtual ReturnState RenderGUI() = 0;

	int width; 
	int height; 
	string name;

	GLFWwindow* window;

	string font;
	float fontSize;

private:

	void Release();
};

#endif // _WINDOW_H

