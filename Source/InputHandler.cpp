#include "InputHandler.h"

#include "Application.h"
#include "VolumetricCloudAtmosphereRenderer.h"

#include "imgui/imgui.h"

InputHandler::InputHandler()
    : application{nullptr}, camera {nullptr}
{
}

void InputHandler::Initialize(Application* app)
{
    application = app;
    camera = &application->volumetricCloudAtmoshpereRenderer.camera;
}

void InputHandler::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camera->Position = glm::vec3(0, 0, 0);
        camera->Front = glm::vec3(0, 0, -1);
    }
}

void InputHandler::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse || !isMouseCapturedInWindow)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos);  // Reversed since y-coordinates go from bottom to left

    lastX = (float)xpos;
    lastY = (float)ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void InputHandler::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    bool wantCaptureMouse = ImGui::GetIO().WantCaptureMouse;

    // release mouse by left clicking if cursor was captured
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && isMouseCapturedInWindow) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isMouseCapturedInWindow = false;
    }
    // capture mouse by left clicking if cursor was released
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !isMouseCapturedInWindow && !wantCaptureMouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isMouseCapturedInWindow = true;
    }
}

void InputHandler::ProcessRealtimeInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
}
