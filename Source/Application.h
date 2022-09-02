#pragma once

#include "InputHandler.h"
#include "VolumetricCloudAtmosphereRenderer.h"
#include "ImGUIRenderer.h"

#include "glfw/glfw3.h"

struct Application
{
	Application();
	~Application();
	void Initialize(const int screenWidth, const int screenHeight, const char* title);
	void Run();
	void DisplayCloudFramerateAndCameraPositionInWindowTitle(float deltaTime);

	// glfw callback
	void FramebufferSizeCallback(GLFWwindow* window, int width, int height);


public:
	GLFWwindow* mainWindow;
	int windowWidth;
	int windowHeight;

	ImGUIRenderer uiRenderer;
	InputHandler inputHandler;
	VolumetricCloudAtmoshpereRenderer volumetricCloudAtmoshpereRenderer;
	
	
};

