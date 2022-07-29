#include "VolumetricCloudAtmosphereRenderer.h"

#include "Application.h"
#include "ComputeShader.h"

#include <iostream>


VolumetricCloudAtmoshpereRenderer::VolumetricCloudAtmoshpereRenderer()
{

}

void VolumetricCloudAtmoshpereRenderer::Initialize(Application* app)
{
	InitializeShader();
	InitializeFramebuffer();

    application = app;

	// initialize fullscreenquad with quad vertex data
	fullscreenQuad.Initialize(&quadVertexData);

    // set quarter resolution
    quarterWindowWidth = app->windowWidth / 4;
    quarterWindowHeight = app->windowHeight / 4;
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

void VolumetricCloudAtmoshpereRenderer::ComputeNoiseTextures()
{

}

void VolumetricCloudAtmoshpereRenderer::ComputeLowFrequencyPerlinWorleyNoiseTexture()
{

}

void VolumetricCloudAtmoshpereRenderer::ComputeHighFrequencyWorleyTexture()
{

}

void VolumetricCloudAtmoshpereRenderer::Draw()
{
}
