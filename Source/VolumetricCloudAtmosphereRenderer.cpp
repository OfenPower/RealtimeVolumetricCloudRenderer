#include "VolumetricCloudAtmosphereRenderer.h"

#include "Application.h"
#include "ComputeShader.h"

#include <iostream>


VolumetricCloudAtmoshpereRenderer::VolumetricCloudAtmoshpereRenderer()
{

}

void VolumetricCloudAtmoshpereRenderer::Initialize(Application* app)
{
    application = app;

    // set quarter resolution
    quarterWindowWidth = app->windowWidth / 4;
    quarterWindowHeight = app->windowHeight / 4;
    
	InitializeShader();
	InitializeFramebuffer();
    InitializeShaderUniformLocations();
    ComputeNoiseTextures();

	// initialize fullscreenquad with quad vertex data
	fullscreenQuad.Initialize(&quadVertexData);

    
}

void VolumetricCloudAtmoshpereRenderer::InitializeShader()
{
	Shader volumetricCloudAtmosphereShader("../Shader/volumetricCloudAtmosphere.vert", "../Shader/volumetricCloudAtmosphere.frag");
	Shader upscaleShader("../Shader/upscale.vert", "../Shader/upscale.frag");
	Shader fullscreenQuadShader("../Shader/fullscreenQuad.vert", "../Shader/fullscreenQuad.frag");

	// assign shader to array
	shader[VOLUMETRIC_CLOUD_ATMOSPHERE] = volumetricCloudAtmosphereShader;
	shader[UPSCALE] = upscaleShader;
	shader[FULLSCREEN_QUAD] = fullscreenQuadShader;
}

void VolumetricCloudAtmoshpereRenderer::InitializeFramebuffer()
{
	InitializeNoiseFramebuffer();
    InitializeQuarterResolutionFramebuffer();
    InitializeMainFramebuffer();
}

void VolumetricCloudAtmoshpereRenderer::InitializeNoiseFramebuffer()
{
    // Low Frequency Shape Noise Framebuffer
    unsigned int shapeNoiseFramebuffer;
    unsigned int perlworltex;
    glGenFramebuffers(1, &shapeNoiseFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shapeNoiseFramebuffer);
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
        std::cout << "ERROR::FRAMEBUFFER lowFrequencyShapeFramebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    // assign framebuffer to array
    framebuffer[LOW_FREQUENCY_NOISE] = shapeNoiseFramebuffer;
    texture3D[PERLIN_WORLEY] = perlworltex;

    // High Frequency Detail Noise Framebuffer
    unsigned int detailNoiseFramebuffer;
    unsigned int worltex;
    glGenFramebuffers(1, &detailNoiseFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, detailNoiseFramebuffer);
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
        std::cout << "ERROR::FRAMEBUFFER highFrequencyDetailFramebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    // assign framebuffer to array
    framebuffer[HIGH_FREQUENCY_NOISE] = detailNoiseFramebuffer;
    texture3D[WORLEY] = worltex;
}

void VolumetricCloudAtmoshpereRenderer::InitializeQuarterResolutionFramebuffer()
{
    // create the quarter resolution downscaled framebuffer in which the clouds will be rendered
    unsigned int quarterResolutionFramebuffer;
    unsigned int quarterResolutionColorbuffer;
    glGenFramebuffers(1, &quarterResolutionFramebuffer);
    glGenTextures(1, &quarterResolutionColorbuffer);
    glBindTexture(GL_TEXTURE_2D, quarterResolutionColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, quarterWindowWidth, quarterWindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, quarterResolutionFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, quarterResolutionColorbuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: quarterResolutionFramebuffer is not complete!" << std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // assign to array
    framebuffer[QUARTER_RESOLUTION] = quarterResolutionFramebuffer;
    texture2D[QUARTER_RESOLUTION_COLORBUFFER] = quarterResolutionColorbuffer;
}

void VolumetricCloudAtmoshpereRenderer::InitializeMainFramebuffer()
{
    // create the main full sized framebuffer, which will produce the final image
    unsigned int mainFramebuffer;
    unsigned int mainColorbuffer;
    glGenFramebuffers(1, &mainFramebuffer);
    glGenTextures(1, &mainColorbuffer);
    glBindTexture(GL_TEXTURE_2D, mainColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, application->windowWidth, application->windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainColorbuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: mainFramebuffer is not complete!" << std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // assign to array
    framebuffer[MAIN] = mainFramebuffer;
    texture2D[MAIN_COLORBUFFER] = mainColorbuffer;
}

void VolumetricCloudAtmoshpereRenderer::InitializeShaderUniformLocations()
{
    InitializeVolumetricCloudAtmosphereShaderUniformLocations();
    InitializeUpscaleShaderUniformLocations();
}

void VolumetricCloudAtmoshpereRenderer::InitializeVolumetricCloudAtmosphereShaderUniformLocations()
{
    Shader& volumetricCloudAtmosphereShader = shader[VOLUMETRIC_CLOUD_ATMOSPHERE];
    volumetricCloudAtmosphereShader.use();
    unsigned int volumetricCloudAtmosphereShaderId = volumetricCloudAtmosphereShader.ID;
    uniformLocation[INV_VIEW] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "invView");
    uniformLocation[INV_PROJ] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "invProj");
    uniformLocation[CAM_POS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "camPos");
    uniformLocation[TIME] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "time");
    uniformLocation[RESOLUTION_1] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "resolution");
    uniformLocation[PERLIN_WORLEY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "perlworl");
    uniformLocation[WORLEY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "worl");
    uniformLocation[SUN_INTENSITY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "sunIntensity");
    uniformLocation[SUN_POSITION] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "sunPosition");
    uniformLocation[COVERAGE_SCALE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "coverageScale");
    uniformLocation[CLOUD_COLOR] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudColor");
    uniformLocation[RAIN_CLOUD_ABSORPTION_GAIN] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "rainCloudAbsorptionGain");
    uniformLocation[CLOUD_ATTENUATION_SCALE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudAttenuationScale");
    uniformLocation[AMBIENT_COLOR_SCALE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "ambientColorScale");
    uniformLocation[CLOUD_TYPE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudType");
    uniformLocation[TONEMAPPER_EYE_EXPOSURE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "toneMapperEyeExposure");
    uniformLocation[IGNORE_DETAIL_NOISE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "ignoreDetailNoise");
    uniformLocation[ANVIL_BIAS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "anvilBias");
    uniformLocation[LOW_FREQUENCY_NOISE_SCALE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "lowFrequencyNoiseScale");
    uniformLocation[HIGH_FREQUENCY_NOISE_SCALE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyNoiseScale");
    uniformLocation[HIGH_FREQUENCY_NOISE_ERODE_MULTIPLIER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyNoiseErodeMuliplier");
    uniformLocation[HIGH_FREQUENCY_HEIGHT_TRANSITION_MULTIPLIER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "highFrequencyHeightTransitionMultiplier");
    uniformLocation[PHASE_ECCENTRICITY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseEccentricity");
    uniformLocation[PHASE_SILVER_LINING_INTENSITY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseSilverLiningIntensity");
    uniformLocation[PHASE_SILVER_LINING_SPREAD] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "phaseSilverLiningSpread");
    uniformLocation[CONE_SPREAD_MULTIPLIER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "coneSpreadMultplier");
    uniformLocation[SHADOW_SAMPLE_CONE_SPREAD_MULTIPLIER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "shadowSampleConeSpreadMultiplier");
    uniformLocation[POWDERED_SUGAR_EFFECT_MULTIPLIER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "powderedSugarEffectMultiplier");
    uniformLocation[MAX_RENDER_DISTANCE] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxRenderDistance");
    uniformLocation[MAX_HORIZONTAL_SAMPLE_COUNT] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxHorizontalSampleCount");
    uniformLocation[MAX_VERTICAL_SAMPLE_COUNT] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "maxVerticalSampleCount");
    uniformLocation[USE_EARLY_EXIT_AT_FULL_OPACITY] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "useEarlyExitAtFullOpacity");
    uniformLocation[USE_BAYER_FILTER] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "useBayerFilter");
    uniformLocation[EARTH_RADIUS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "earthRadius");
    uniformLocation[VOLUMETRIC_CLOUDS_START_RADIUS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "volumetricCloudsStartRadius");
    uniformLocation[VOLUMETRIC_CLOUDS_END_RADIUS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "volumetricCloudsEndRadius");
    uniformLocation[WIND_DIRECTION] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "windDirection");
    uniformLocation[WIND_UPWARD_BIAS] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "windUpwardBias");
    uniformLocation[CLOUD_SPEED] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudSpeed");
    uniformLocation[CLOUD_TOP_OFFSET] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "cloudTopOffset");
    uniformLocation[DEBUG_BOOL] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugBool");
    uniformLocation[DEBUG_FLOAT1] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat");
    uniformLocation[DEBUG_FLOAT2] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat2");
    uniformLocation[DEBUG_FLOAT3] = glGetUniformLocation(volumetricCloudAtmosphereShaderId, "debugFloat3");

    // assign texture units
    glUniform1i(uniformLocation[PERLIN_WORLEY], 0);
    glUniform1i(uniformLocation[WORLEY], 1);
}

void VolumetricCloudAtmoshpereRenderer::InitializeUpscaleShaderUniformLocations()
{
    // Setup upscale shader
    //
    Shader& upscaleShader = shader[UPSCALE];
    upscaleShader.use();
    unsigned int upscaleShaderId = upscaleShader.ID;
    uniformLocation[RESOLUTION_2] = glGetUniformLocation(upscaleShaderId, "resolution");
    uniformLocation[BUFF] = glGetUniformLocation(upscaleShaderId, "buff");
    
    // assign texture units
    glUniform1i(uniformLocation[BUFF], 0);
}

void VolumetricCloudAtmoshpereRenderer::ComputeNoiseTextures()
{
    ComputeLowFrequencyPerlinWorleyNoiseTexture();
    ComputeHighFrequencyWorleyTexture();
}

void VolumetricCloudAtmoshpereRenderer::ComputeLowFrequencyPerlinWorleyNoiseTexture()
{
    ComputeShader perlinWorleyShader("../Shader/perlinWorley.comp");
    perlinWorleyShader.use();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[LOW_FREQUENCY_NOISE]);
    glBindImageTexture(0, texture3D[PERLIN_WORLEY], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);	// layout (rgba8, binding = 0) uniform image3D outVolTex;
    glActiveTexture(GL_TEXTURE0);                               // TODO: entfernen, macht keinen Sinn
    glBindTexture(GL_TEXTURE_3D, texture3D[PERLIN_WORLEY]);     // TODO: entfernen, macht keinen Sinn
    glDispatchCompute(128, 128, 128);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_3D, 0);                            // TODO: entfernen, macht keinen Sinn
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VolumetricCloudAtmoshpereRenderer::ComputeHighFrequencyWorleyTexture()
{
    // worley compute shader
    ComputeShader worleyShader("../Shader/worley.comp");
    worleyShader.use();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[HIGH_FREQUENCY_NOISE]);
    glBindImageTexture(0, texture3D[WORLEY], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glActiveTexture(GL_TEXTURE0);           // TODO: entfernen, macht keinen Sinn
    glBindTexture(GL_TEXTURE_3D, texture3D[WORLEY]);  // TODO: entfernen, macht keinen Sinn
    glDispatchCompute(32, 32, 32);          
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_3D, 0);        // TODO: entfernen, macht keinen Sinn
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VolumetricCloudAtmoshpereRenderer::Draw()
{
    // TODO!
}
