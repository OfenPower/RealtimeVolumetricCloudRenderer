#include "Application.h"

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>

Application::Application()
{
}

void Application::Initialize(const int screenWidth, const int screenHeight, const char* title)
{
    windowWidth = screenWidth;
    windowHeight = screenHeight;

    // setup glfw with opengl version 4.30 core
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create main window
    mainWindow = glfwCreateWindow(screenWidth, screenHeight, title, NULL, NULL);
    if (mainWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(mainWindow);
    glfwSwapInterval(0);    //turn off vsync

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    // initialize viewport
    glViewport(0, 0, windowWidth, windowHeight);

    // initialize renderer and input handler
    volumetricCloudAtmoshpereRenderer.Initialize(this);
}

void Application::Run()
{
    // main loop
    // -----------
    float deltaTime = (float)glfwGetTime();      // glfwGetTime() measures time elapsed since GLFW was initialized
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(mainWindow))
    {
        // Input Processing
        inputHandler.ProcessRealtimeInput(mainWindow, deltaTime);

        // Update 
        // [...]

        // Render
        //volumetricCloudAtmoshpereRenderer.render();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(mainWindow);
        glfwPollEvents();

        // per-frame time logic
        float currentFrameTime = (float)glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // print elapsed time in ms and camera position to window title
        char buffer[300];
        //sprintf_s(buffer, "Elapsed Time: %f ms - Camera Position: (x=%f, y=%f, z=%f)", deltaTime * 1000.0, camera.Position.x, camera.Position.y + earthRadius, camera.Position.z);
        glfwSetWindowTitle(mainWindow, buffer);
    }
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
}