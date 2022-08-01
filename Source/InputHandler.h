#pragma once

#include "Camera.h"

#include "glfw/glfw3.h"

struct Application;

struct InputHandler
{
	InputHandler();
	void Initialize(Application* app);

	// glfw callbacks
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
	void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ProcessRealtimeInput(GLFWwindow* window, float deltaTime);

public:
	Application* application;
	Camera* camera;

	// mouse attributes
	float lastX = 400;
	float lastY = 300;
	bool firstMouse = true;
	bool isMouseCapturedInWindow = false;
};

