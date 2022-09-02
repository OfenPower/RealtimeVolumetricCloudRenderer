#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

struct Application;
struct InputHandler;
struct VolumetricCloudAtmoshpereRenderer;

struct ImGUIRenderer
{
	ImGUIRenderer();
	~ImGUIRenderer();
	void Initialize(Application* app);
	void InstallCallbacks();
	void BeginNewFrame();
	void SetupSettingsWindow();
	void SetupDemoWindow();
	void Draw();


public:
	ImGuiIO* io;
	Application* application;
	InputHandler* inputHandler;
	VolumetricCloudAtmoshpereRenderer* cloudRenderer;
};

