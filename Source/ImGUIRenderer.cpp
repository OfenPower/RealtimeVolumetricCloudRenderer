#include "ImGUIRenderer.h"

#include "Application.h"
#include "InputHandler.h"
#include "VolumetricCloudAtmosphereRenderer.h"

#include "glfw/glfw3.h"

#include <iostream>

ImGUIRenderer::ImGUIRenderer()
{
    
}

void ImGUIRenderer::Initialize(Application* app)
{
    application = app;
    inputHandler = &application->inputHandler;
    cloudRenderer = &application->volumetricCloudAtmoshpereRenderer;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(application->mainWindow, false);   // callbacks wont be installed here, they will be installed after custom glfw callbacks in main()
    ImGui_ImplOpenGL3_Init("#version 430");
}

void ImGUIRenderer::InstallCallbacks()
{
    ImGui_ImplGlfw_InstallCallbacks(application->mainWindow);
}

void ImGUIRenderer::BeginNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // show mouse and un-capture mouse, when moving over the UI
    if (io.WantCaptureMouse) {
        inputHandler->isMouseCapturedInWindow = false;
        GLFWwindow* window = application->mainWindow;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(window, io.MousePos.x, io.MousePos.y);
    }
}

void ImGUIRenderer::SetupSettingsWindow()
{
    // render ImGui UI
    ImGui::Begin("Volumetric Cloud Renderer");
    ImGui::Text("Hold STRG and left-click into a slider to set its value directly!");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);       // slider will be 40% of the window width
    if (ImGui::CollapsingHeader("Cloud Model"))
    {
        ImGui::SliderFloat("Cloud Coverage Scale", &cloudRenderer->coverageScale, 0, 1);
        ImGui::SliderFloat("Cloud Type", &cloudRenderer->cloudType, 0, 1);
        ImGui::SliderFloat("Low Frequency Noise Scale", &cloudRenderer->lowFrequencyNoiseScale, 0, 5);
        ImGui::SliderFloat("Anvil Bias", &cloudRenderer->anvilBias, 0.0, 1.0);
        ImGui::Checkbox("Ignore High Frequency Noise", &cloudRenderer->ignoreDetailNoise);
        ImGui::SliderFloat("High Frequency Noise Scale", &cloudRenderer->highFrequencyNoiseScale, 0, 2);
        ImGui::SliderFloat("High Frequency Noise Erode Muliplier", &cloudRenderer->highFrequencyNoiseErodeMuliplier, 0, 1);
        ImGui::SliderFloat("High Frequency Height Transition Multiplier", &cloudRenderer->highFrequencyHeightTransitionMultiplier, 0, 10);
    }
    if (ImGui::CollapsingHeader("Cloud Lighting Settings"))
    {
        ImGui::ColorEdit3("Cloud Base Color", &cloudRenderer->cloudColor[0]);
        ImGui::SliderFloat("Sun Color Intensity", &cloudRenderer->sunIntensity, 0, 2);
        ImGui::SliderFloat("Ambient Light Color Scale", &cloudRenderer->ambientColorScale, 0, 4);
        ImGui::SliderFloat("Rain Cloud Absorption Gain", &cloudRenderer->rainCloudAbsorptionGain, 0, 20);
        ImGui::SliderFloat("Cloud Attenuation Scale", &cloudRenderer->cloudAttenuationScale, 0, 20);
        ImGui::SliderFloat("Phase Eccentricity", &cloudRenderer->phaseEccentricity, -1, 1);
        ImGui::SliderFloat("Phase Silver Lining Intensity", &cloudRenderer->phaseSilverLiningIntensity, 0, 4);
        ImGui::SliderFloat("Phase Silver Lining Spread", &cloudRenderer->phaseSilverLiningSpread, 0, 2);
        ImGui::SliderFloat("Lighting Sample Cone Spread Multplier", &cloudRenderer->coneSpreadMultplier, 0, 2);
        ImGui::SliderFloat("Shadow Sample Cone Spread Multiplier", &cloudRenderer->shadowSampleConeSpreadMultiplier, 0, 3);
        ImGui::SliderFloat("Powdered Sugar Effect Multiplier", &cloudRenderer->powderedSugarEffectMultiplier, 0, 10);
        ImGui::SliderFloat("Tone Mapping Eye Exposure", &cloudRenderer->toneMapperEyeExposure, 0, 3);
    }
    if (ImGui::CollapsingHeader("Raymarch Settings"))
    {
        ImGui::SliderFloat("Max Render Distance", &cloudRenderer->maxRenderDistance, 0, 1000000);
        ImGui::SliderFloat("Max Horizontal Sample Count", &cloudRenderer->maxHorizontalSampleCount, 1, 512);
        ImGui::SliderFloat("Max Vertical Sample Count", &cloudRenderer->maxVerticalSampleCount, 1, 512);
        ImGui::Checkbox("Use Early Exit At Full Opacity", &cloudRenderer->useEarlyExitAtFullOpacity);
        ImGui::Checkbox("Use Bayer Filter", &cloudRenderer->useBayerFilter);
        ImGui::SliderFloat("Earth Radius in m", &cloudRenderer->earthRadius, 100000.0, 1000000);
        ImGui::SliderFloat("Cloud Volume Start Radius", &cloudRenderer->volumetricCloudsStartRadius, 100000.0, 1000000);
        ImGui::SliderFloat("Cloud Volume End Radius", &cloudRenderer->volumetricCloudsEndRadius, 100000.0, 1000000);
        ImGui::Checkbox("Render directly to fullscreen", &cloudRenderer->renderDirectlyToFullscreen);
        ImGui::Checkbox("Render Actual Quarter Resolution Buffer", &cloudRenderer->renderActualQuarterResolutionBuffer);
    }
    if (ImGui::CollapsingHeader("Wind Settings"))
    {
        ImGui::SliderFloat3("Wind Direction", &cloudRenderer->windDirection[0], -1, 1);
        ImGui::SliderFloat("Wind Upward Bias", &cloudRenderer->windUpwardBias, 0, 1);
        ImGui::SliderFloat("Cloud Speed", &cloudRenderer->cloudSpeed, 0, 20000);
        ImGui::SliderFloat("Cloud Top Offset", &cloudRenderer->cloudTopOffset, 0, 10000);
    }
    if (ImGui::CollapsingHeader("Sun Settings"))
    {
        ImGui::Checkbox("Set Custom Sun Pitch Angle", &cloudRenderer->moveSunManually);
        ImGui::SliderFloat("Sun Pitch Angle in Degree", &cloudRenderer->sunPitch, 0, 180);
    }
    /*
    if (ImGui::CollapsingHeader("Debug Settings"))
    {
        ImGui::Checkbox("Debug bool", &debugBool);
        ImGui::SliderFloat("Debug Float", &debugFloat, 0, 3);
        ImGui::SliderFloat("Debug Float 2", &debugFloat2, 0, 5);
        ImGui::SliderFloat("Debug Float 3", &debugFloat3, 0, 1);
    }
    */
    ImGui::End();
}

void ImGUIRenderer::Draw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGUIRenderer::SetupDemoWindow()
{
    ImGui::ShowDemoWindow();
}