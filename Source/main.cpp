
#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "Application.h"
#include "InputHandler.h"

#include "Shader.h"

// window settings
const unsigned int SCREEN_WIDTH = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

// main application class
Application* application = nullptr;

// glfw callback definition the easy but dirty way.
// Better way for callback implementation here: https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
//
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) { application->FramebufferSizeCallback(window, width, height); }
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) { application->inputHandler.CursorPosCallback(window, xpos, ypos); }
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { application->inputHandler.KeyCallback(window, key, scancode, action, mods); }
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) { application->inputHandler.MouseButtonCallback(window, button, action, mods); }


int main()
{
	// initialize application
	application = new Application();
	application->Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Realtime Volumetric Clouds");

	// bind glfw callbacks
	GLFWwindow* window = application->mainWindow;
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	// cleanup
	glfwTerminate();
	delete application;

	return 0;
}