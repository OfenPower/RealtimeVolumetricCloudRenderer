#pragma once

#include "glfw/glfw3.h"

struct InputHandler
{
	InputHandler();

	// glfw callbacks
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
	void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ProcessRealtimeInput(GLFWwindow* window, float deltaTime);

public:

};

