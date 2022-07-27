#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Camera.h"
#include "ComputeShader.h"

#include <iostream>

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void processRealtimeInput(GLFWwindow* window, float deltaTime);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

// Mouse
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
bool isMouseCapturedInWindow = false;
bool UIwantsToCaptureMouseInput = true;

// Window dimensions
const GLuint WIDTH = 1920, HEIGHT = 1080;
int downscale = 4;
GLuint downscalesq = downscale * downscale;
GLfloat ASPECT = float(WIDTH) / float(HEIGHT);

// cloud model
float coverageScale = 0.45f;
float ambientColorScale = 0.7f; 
float cloudType = 0.8;
float lowFrequencyNoiseScale = 0.3;
bool ignoreDetailNoise = false;
float highFrequencyNoiseScale = 0.3f;
float highFrequencyNoiseErodeMuliplier = 0.19;
float highFrequencyHeightTransitionMultiplier = 10;
float anvilBias = 0.1;
// cloud lighting
glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);
float rainCloudAbsorptionGain = 2.3f;
float cloudAttenuationScale = 2.5f;
float phaseEccentricity = 0.5;
float phaseSilverLiningIntensity = 0.15;
float phaseSilverLiningSpread = 0.5;
float coneSpreadMultplier = 0.2;
float shadowSampleConeSpreadMultiplier = 0.6;
float powderedSugarEffectMultiplier = 10.0;
float toneMapperEyeExposure = 0.8;
// raymarch
float maxRenderDistance = 150000;
float maxHorizontalSampleCount = 192;
float maxVerticalSampleCount = 128;
bool useEarlyExitAtFullOpacity = true;
bool useBayerFilter = true;
float earthRadius = 600000.0; 			        
float volumetricCloudsStartRadius = 607000.0;
float volumetricCloudsEndRadius = 633000.0;
bool renderDirectlyToFullscreen = false;
bool renderActualQuarterResolutionBuffer = false;
// windsettings
glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
float windUpwardBias = 0.15;	
float cloudSpeed = 300.0;
float cloudTopOffset = 500;		
// sun settings
float sunIntensity = 0.4;
bool moveSunManually = true;
float sunPitch = 90;
// debug 
bool debugBool = false;
float debugFloat = 0.4f;
float debugFloat2 = 0.7f;
float debugFloat3 = 1.0;

// ===================================================================================================================================================================

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
void renderQuad()
{
    // renders a 1x1 XY quad in NDC
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// ===================================================================================================================================================================

int main()
{
    // Setup glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Realtime Volumetric Clouds", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    //glfwSetWindowPos(window, 700, 50);
    glfwSwapInterval(0);    //turn off vsync

    // setup glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // initialize viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    //Shader class built on the one in learnopengl.com
    Shader volumetricCloudAtmosphereShader("../Shader/volumetricCloudAtmosphere.vert", "../Shader/volumetricCloudAtmosphere.frag");
    Shader fullscreenQuadShader("../Shader/fullscreenQuad.vert", "../Shader/fullscreenQuad.frag");
    Shader upscaleShader("../Shader/upscale.vert", "../Shader/upscale.frag");

    // Setup Framebuffers
    //
    
    // create the main full sized framebuffer
    GLuint mainFramebuffer, mainColorbuffer;

    glGenFramebuffers(1, &mainFramebuffer);
    glGenTextures(1, &mainColorbuffer);
    glBindTexture(GL_TEXTURE_2D, mainColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainColorbuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: mainFramebuffer is not complete!" << std::endl;


    // create the quarter resolution downscaled framebuffer in which the clouds will be rendered
    GLuint quarterResolutionFramebuffer, quarterResolutionColorbuffer;

    glGenFramebuffers(1, &quarterResolutionFramebuffer);
    glGenTextures(1, &quarterResolutionColorbuffer);
    glBindTexture(GL_TEXTURE_2D, quarterResolutionColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH/downscale, HEIGHT/downscale, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, quarterResolutionFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, quarterResolutionColorbuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: quarterResolutionFramebuffer is not complete!" << std::endl;


    // bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


// ==============================================================================================================================================

    // ------------------------------------------------------
    // Low Frequency Shape Noise
    // ------------------------------------------------------

    // Framebuffer
    unsigned int shapeNoiseFramebuffer;
    glGenFramebuffers(1, &shapeNoiseFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shapeNoiseFramebuffer);

    // r = Perlin-Worley FBM octave texture, res 128 * 128 * 128
    // g = Worley X + 1 octave
    // b = Worley X + 2 octave
    // c = Worley X + 3 octave
    unsigned int perlworltex;
    glGenTextures(1, &perlworltex);
    glBindTexture(GL_TEXTURE_3D, perlworltex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 128, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // framebuffer attachment
    glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, perlworltex, 0, 0);

    // framebuffer ok?
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    // perlin-worley compute shader
    ComputeShader perlinWorleyShader("../Shader/perlinWorley.comp");
    perlinWorleyShader.use();
    glBindImageTexture(0, perlworltex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);	// layout (rgba8, binding = 0) uniform image3D outVolTex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, perlworltex);
    glDispatchCompute(128, 128, 128);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "shape noise done" << std::endl;


    // ------------------------------------------------------
    // High Frequency Detail Noise
    // ------------------------------------------------------

    // Framebuffer
    unsigned int detailNoiseFramebuffer;
    glGenFramebuffers(1, &detailNoiseFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, detailNoiseFramebuffer);

    unsigned int worltex;
    glGenTextures(1, &worltex);
    glBindTexture(GL_TEXTURE_3D, worltex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // framebuffer attachment
    glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, worltex, 0, 0);

    // framebuffer ok?
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    // worley compute shader
    ComputeShader worleyShader("../Shader/worley.comp");
    worleyShader.use();
    glBindImageTexture(0, worltex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);	
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, worltex);
    glDispatchCompute(32, 32, 32);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "detail noise done" << std::endl;

    // ==============================================================================================================================================

    // Quad Vertex Data 
    float quadVertices[] = {
        // positions            // texture Coords
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
    };
    
    // setup quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);   // position attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // texture coordinates


    // ==============================================================================================================================================

    // Setup volumetric cloud shader
    //
    volumetricCloudAtmosphereShader.use();
    unsigned int volumetricCloudAtmosphereShaderId = volumetricCloudAtmosphereShader.ID;
    unsigned int uInvView = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "invView");
    unsigned int uInvProj = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "invProj");
    unsigned int uCamPos = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "camPos");
    unsigned int uTime = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "time");
    unsigned int uResolution = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "resolution");
    unsigned int uPerlinWorleySampler = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "perlworl");
    unsigned int uWorleySampler = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "worl");
    unsigned int uDepthBufferSampler = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "depthBuffer");
    unsigned int uSunIntensity = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "sunIntensity");
    unsigned int uSunPosition = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "sunPosition");  
    unsigned int uConverageScale = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "coverageScale");
    unsigned int uCloudColor = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudColor");
    unsigned int uRainCloudAbsorptionGain = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "rainCloudAbsorptionGain");
    unsigned int uCloudAttenuationScale = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudAttenuationScale");
    unsigned int uAmbientColorScale = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "ambientColorScale");   
    unsigned int uCloudType = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudType");
    unsigned int uToneMapperExposure = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "toneMapperEyeExposure");
    unsigned int uIgnoreDetailNoise = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "ignoreDetailNoise");
    unsigned int uAnvilBias = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "anvilBias");
    unsigned int uLowFrequencyNoiseScale = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "lowFrequencyNoiseScale");
    unsigned int uHighFrequencyNoiseScale = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyNoiseScale");
    unsigned int uHighFrequencyNoiseErodeMultiplier = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyNoiseErodeMuliplier");
    unsigned int uHighFrequencyHeightTransitionMulitplier = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyHeightTransitionMultiplier");
    unsigned int uPhaseEccentricity = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseEccentricity");
    unsigned int uPhaseSilverLiningIntensity = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseSilverLiningIntensity");
    unsigned int uPhaseSilverLiningSpread = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseSilverLiningSpread");
    unsigned int uConeSpreadMultiplier = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "coneSpreadMultplier");
    unsigned int uShadowSampleConeSpreadMultiplier = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "shadowSampleConeSpreadMultiplier");
    unsigned int uPowderedSugarEffectMultiplier = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "powderedSugarEffectMultiplier");
    unsigned int uMaxRenderDistance = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxRenderDistance");
    unsigned int uMaxHorizontalSampleCount = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxHorizontalSampleCount");
    unsigned int uMaxVerticalSampleCount = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxVerticalSampleCount");
    unsigned int uUseEarlyExitAtFullOpacity = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "useEarlyExitAtFullOpacity");
    unsigned int uUseBayerFilter = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "useBayerFilter");
    unsigned int uEarthRadius = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "earthRadius");
    unsigned int uVolumetricCloudsStartRadius = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "volumetricCloudsStartRadius");
    unsigned int uVolumetricCloudsEndRadius = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "volumetricCloudsEndRadius");
    unsigned int uWindDirection = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "windDirection");
    unsigned int uWindUpwardBias = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "windUpwardBias");
    unsigned int uCloudSpeed = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudSpeed");
    unsigned int uCloudTopOffset = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudTopOffset");
    unsigned int uDebugBool = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugBool");
    unsigned int uDebugFloat = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat");
    unsigned int uDebugFloat2 = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat2");
    unsigned int uDebugFloat3 = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat3");
    // assign texture units
    glUniform1i(uPerlinWorleySampler, 0);
    glUniform1i(uWorleySampler, 1);
    glUniform1i(uDepthBufferSampler, 2);

    // Setup upscale shader
    //
    upscaleShader.use();
    unsigned int u2Resolution = glGetUniformLocation(upscaleShader.ID, "resolution");
    unsigned int uDownscaleFactor = glGetUniformLocation(upscaleShader.ID, "downscaleFactor");
    unsigned int uBuffSampler = glGetUniformLocation(upscaleShader.ID, "buff");
    // assign texture units
    glUniform1i(uBuffSampler, 0);
    
    // view and projection matrix
    glm::mat4 view;
    glm::mat4 projection;

    // render loop
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        // Frame Time
        GLfloat currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // ImGUI Stuff 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // show mouse and un-capture mouse, when moving over the UI
        UIwantsToCaptureMouseInput = io.WantCaptureMouse;       
        if (UIwantsToCaptureMouseInput) {
            isMouseCapturedInWindow = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPos(window, io.MousePos.x, io.MousePos.y);
        }
        //ImGui::ShowDemoWindow();

        // print elapsed time in ms and camera position to window title
        char buffer[300];
        sprintf_s(buffer, "Elapsed Time: %f ms - Camera Position: (x=%f, y=%f, z=%f)", deltaTime * 1000.0, camera.Position.x, camera.Position.y + earthRadius, camera.Position.z);
        glfwSetWindowTitle(window, buffer);
      
        // update camera view and projection matrix
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.01f, 1000000.0f);

        // handle Realtime Input
        processRealtimeInput(window, deltaTime);

        // =========================================================================================================================

        // Write to quarter resolution downsized buffer (default case)
        if (!renderDirectlyToFullscreen) {
            glBindFramebuffer(GL_FRAMEBUFFER, quarterResolutionFramebuffer);
            glViewport(0, 0, WIDTH/downscale, HEIGHT/downscale);
        }
        else {
            // render to the fullscreen buffer in order to see the performance impact (if desired)
            glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
            glViewport(0, 0, WIDTH, HEIGHT);
        }
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // volumetric cloud shader settings
        //
        volumetricCloudAtmosphereShader.use();
        
        glUniform3fv(uCamPos, 1, &camera.Position[0]);
        glUniformMatrix4fv(uInvView, 1, GL_FALSE, &glm::inverse(view)[0][0]);
        glUniformMatrix4fv(uInvProj, 1, GL_FALSE, &glm::inverse(projection)[0][0]);
        glUniform1f(uTime, currentFrameTime);
        // set resolution based on scale settings
        if (!renderDirectlyToFullscreen) {
            glUniform2f(uResolution, float(WIDTH)/downscale, float(HEIGHT)/downscale);
        }
        else {
            glUniform2f(uResolution, float(WIDTH), float(HEIGHT));
        }
        // cloud density model
        glUniform1f(uConverageScale, coverageScale);
        glUniform1f(uCloudType, cloudType);
        glUniform1f(uAnvilBias, anvilBias);
        glUniform1f(uLowFrequencyNoiseScale, lowFrequencyNoiseScale);
        glUniform1i(uIgnoreDetailNoise, ignoreDetailNoise);
        glUniform1f(uHighFrequencyNoiseScale, highFrequencyNoiseScale);
        glUniform1f(uHighFrequencyNoiseErodeMultiplier, highFrequencyNoiseErodeMuliplier);
        glUniform1f(uHighFrequencyHeightTransitionMulitplier, highFrequencyHeightTransitionMultiplier);
        // cloud lighting
        glUniform3fv(uCloudColor, 1, &cloudColor[0]);
        glUniform1f(uRainCloudAbsorptionGain, rainCloudAbsorptionGain);
        glUniform1f(uCloudAttenuationScale, cloudAttenuationScale);
        glUniform1f(uPhaseEccentricity, phaseEccentricity);
        glUniform1f(uPhaseSilverLiningIntensity, phaseSilverLiningIntensity);
        glUniform1f(uPhaseSilverLiningSpread, phaseSilverLiningSpread);
        glUniform1f(uConeSpreadMultiplier, coneSpreadMultplier);
        glUniform1f(uShadowSampleConeSpreadMultiplier, shadowSampleConeSpreadMultiplier);
        glUniform1f(uPowderedSugarEffectMultiplier, powderedSugarEffectMultiplier);
        glUniform1f(uToneMapperExposure, toneMapperEyeExposure);
        glUniform1f(uAmbientColorScale, ambientColorScale); 
        // wind settings
        // if windDirection is a zero vector, the resulting normalized windDirection is NaN in all vector components. 
        // If that is the case, just forward a zero vector to the wind direction uniform 
        glUniform3fv(uWindDirection, 1, glm::all(glm::isnan(glm::normalize(windDirection))) ? &glm::vec3(0, 0, 0)[0] : &windDirection[0]);
        glUniform1f(uWindUpwardBias, windUpwardBias);
        glUniform1f(uCloudSpeed, cloudSpeed);
        glUniform1f(uCloudTopOffset, cloudTopOffset);
        // raymarch
        glUniform1f(uMaxRenderDistance, maxRenderDistance);
        glUniform1f(uMaxHorizontalSampleCount, maxHorizontalSampleCount);
        glUniform1f(uMaxVerticalSampleCount, maxVerticalSampleCount);
        glUniform1i(uUseEarlyExitAtFullOpacity, useEarlyExitAtFullOpacity);
        glUniform1i(uUseBayerFilter, useBayerFilter);
        glUniform1f(uEarthRadius, earthRadius);
        glUniform1f(uVolumetricCloudsStartRadius, volumetricCloudsStartRadius);
        glUniform1f(uVolumetricCloudsEndRadius, volumetricCloudsEndRadius);
        // sun settings
        glUniform1f(uSunIntensity, sunIntensity);
        if (moveSunManually) {
            float sunPitchRadians = glm::radians(sunPitch);
            constexpr float yaw = glm::radians(90.0);
            float sunposx = cos(yaw) * cos(sunPitchRadians);
            float sunposy = sin(sunPitchRadians);
            float sunposz = sin(yaw) * cos(sunPitchRadians);
            glUniform3f(uSunPosition, float(sunposx), float(sunposy), float(sunposz));
        }
        else {
            float pitch = glm::radians(180.0 * ((sin(currentFrameTime * 0.05) + 1.0) * 0.5));
            constexpr float yaw = glm::radians(90.0);
            float sunposx = cos(yaw) * cos(pitch);
            float sunposy = sin(pitch);
            float sunposz = sin(yaw) * cos(pitch);
            glUniform3f(uSunPosition, float(sunposx), float(sunposy), float(sunposz));
        }
        // debug
        glUniform1i(uDebugBool, debugBool);
        glUniform1f(uDebugFloat, debugFloat);
        glUniform1f(uDebugFloat2, debugFloat2);
        glUniform1f(uDebugFloat3, debugFloat3);
        
        // bind noise textures and weathermap
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, perlworltex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, worltex);
        
        // draw
        renderQuad();

        // =========================================================================================================================

        // render to the fullscreen framebuffer (where upscaling is applied to the quarterResolutionColorbuffer) 
        if (!renderDirectlyToFullscreen && !renderActualQuarterResolutionBuffer) 
        {    
            glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
            glViewport(0, 0, WIDTH, HEIGHT);

            upscaleShader.use();
            glUniform2f(u2Resolution, float(WIDTH), float(HEIGHT));
            glUniform1f(uDownscaleFactor, float(downscale));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, quarterResolutionColorbuffer);

            // draw
            renderQuad();
        }
           
        // =========================================================================================================================
        
        // render to default (screen) buffer 
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        fullscreenQuadShader.use();
        glActiveTexture(GL_TEXTURE0);
        if (!renderActualQuarterResolutionBuffer) {
            glBindTexture(GL_TEXTURE_2D, mainColorbuffer);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, quarterResolutionColorbuffer);
        }
        
        // draw
        renderQuad();

        // =========================================================================================================================

        // render ImGui UI
        ImGui::Begin("Volumetric Cloud Renderer");
        ImGui::Text("Hold STRG and left-click into a slider to set its value directly!");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);       // slider will be 40% of the window width
        if (ImGui::CollapsingHeader("Cloud Model"))
        {
            ImGui::SliderFloat("Cloud Coverage Scale", &coverageScale, 0, 1);
            ImGui::SliderFloat("Cloud Type", &cloudType, 0, 1);
            ImGui::SliderFloat("Low Frequency Noise Scale", &lowFrequencyNoiseScale, 0, 5);
            ImGui::SliderFloat("Anvil Bias", &anvilBias, 0.0, 1.0);
            ImGui::Checkbox("Ignore High Frequency Noise", &ignoreDetailNoise);
            ImGui::SliderFloat("High Frequency Noise Scale", &highFrequencyNoiseScale, 0, 2);
            ImGui::SliderFloat("High Frequency Noise Erode Muliplier", &highFrequencyNoiseErodeMuliplier, 0, 1);
            ImGui::SliderFloat("High Frequency Height Transition Multiplier", &highFrequencyHeightTransitionMultiplier, 0, 10);
        }
        if (ImGui::CollapsingHeader("Cloud Lighting Settings"))
        {
            ImGui::ColorEdit3("Cloud Base Color", &cloudColor[0]);
            ImGui::SliderFloat("Sun Color Intensity", &sunIntensity, 0, 2);
            ImGui::SliderFloat("Ambient Light Color Scale", &ambientColorScale, 0, 4);
            ImGui::SliderFloat("Rain Cloud Absorption Gain", &rainCloudAbsorptionGain, 0, 20);
            ImGui::SliderFloat("Cloud Attenuation Scale", &cloudAttenuationScale, 0, 20);
            ImGui::SliderFloat("Phase Eccentricity", &phaseEccentricity, -1, 1);
            ImGui::SliderFloat("Phase Silver Lining Intensity", &phaseSilverLiningIntensity, 0, 4);
            ImGui::SliderFloat("Phase Silver Lining Spread", &phaseSilverLiningSpread, 0, 2);
            ImGui::SliderFloat("Lighting Sample Cone Spread Multplier", &coneSpreadMultplier, 0, 2);
            ImGui::SliderFloat("Shadow Sample Cone Spread Multiplier", &shadowSampleConeSpreadMultiplier, 0, 3);
            ImGui::SliderFloat("Powdered Sugar Effect Multiplier", &powderedSugarEffectMultiplier, 0, 10);
            ImGui::SliderFloat("Tone Mapping Eye Exposure", &toneMapperEyeExposure, 0, 3);
        }
        if (ImGui::CollapsingHeader("Raymarch Settings"))
        {
            ImGui::SliderFloat("Max Render Distance", &maxRenderDistance, 0, 1000000);
            ImGui::SliderFloat("Max Horizontal Sample Count", &maxHorizontalSampleCount, 1, 512);
            ImGui::SliderFloat("Max Vertical Sample Count", &maxVerticalSampleCount, 1, 512);
            ImGui::Checkbox("Use Early Exit At Full Opacity", &useEarlyExitAtFullOpacity);
            ImGui::Checkbox("Use Bayer Filter", &useBayerFilter);
            ImGui::SliderFloat("Earth Radius in m", &earthRadius, 100000.0, 1000000);
            ImGui::SliderFloat("Cloud Volume Start Radius", &volumetricCloudsStartRadius, 100000.0, 1000000);
            ImGui::SliderFloat("Cloud Volume End Radius", &volumetricCloudsEndRadius, 100000.0, 1000000);
            ImGui::Checkbox("Render directly to fullscreen", &renderDirectlyToFullscreen);
            ImGui::Checkbox("Render Actual Quarter Resolution Buffer", &renderActualQuarterResolutionBuffer);
        }
        if (ImGui::CollapsingHeader("Wind Settings"))
        {
            ImGui::SliderFloat3("Wind Direction", &windDirection[0], -1, 1);
            ImGui::SliderFloat("Wind Upward Bias", &windUpwardBias, 0, 1);
            ImGui::SliderFloat("Cloud Speed", &cloudSpeed, 0, 20000);
            ImGui::SliderFloat("Cloud Top Offset", &cloudTopOffset, 0, 10000);
        }
        if (ImGui::CollapsingHeader("Sun Settings"))
        {
            ImGui::Checkbox("Set Custom Sun Pitch Angle", &moveSunManually);
            ImGui::SliderFloat("Sun Pitch Angle in Degree", &sunPitch, 0, 180);
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
        
        // finally render imgui
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // =========================================================================================================================
        
        // Swap the screen buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // shutdown ImGui stuff
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &mainFramebuffer);
    glDeleteBuffers(1, &quarterResolutionFramebuffer);
    glDeleteTextures(1, &mainColorbuffer);
    glDeleteTextures(1, &quarterResolutionColorbuffer);
    glDeleteTextures(1, &perlworltex);
    glDeleteTextures(1, &worltex);
    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
        
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse || !isMouseCapturedInWindow)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

void processRealtimeInput(GLFWwindow* window, float deltaTime)
{   
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }   
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
}