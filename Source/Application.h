#pragma once

#include "InputHandler.h"
#include "VolumetricCloudAtmosphereRenderer.h"

struct Application
{
	Application();
	void Initialize(const int screenWidth, const int screenHeight, const char* title);
	void Run();

	// glfw callbacks
	void FramebufferSizeCallback(GLFWwindow* window, int width, int height);


public:
	GLFWwindow* mainWindow;
	int windowWidth;
	int windowHeight;

	InputHandler inputHandler;
	VolumetricCloudAtmoshpereRenderer volumetricCloudAtmoshpereRenderer;
	
	
};

