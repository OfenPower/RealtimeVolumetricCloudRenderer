#pragma once

#include "Camera.h"
#include "Quad.h"

#include "Shader.h"

#include "glm/glm.hpp"

struct Application;

struct VolumetricCloudAtmoshpereRenderer
{
	// -----------------------------------------------------------------------------------------------------
	enum ShaderType {
		VOLUMETRIC_CLOUD_ATMOSPHERE,
		UPSCALE,
		FULLSCREEN_QUAD,
		NUM_SHADER
	};

	enum FramebufferType {
		QUARTER_RESOLUTION,
		MAIN,
		LOW_FREQUENCY_NOISE,
		HIGH_FREQUENCY_NOISE,
		NUM_FRAMEBUFFER
	};

	enum Texture2DType {
		QUARTER_RESOLUTION_COLORBUFFER,
		MAIN_COLORBUFFER,
		NUM_TEXTURE_2D
	};

	enum Texture3DType {
		PERLIN_WORLEY,
		WORLEY,
		NUM_TEXTURE_3D
	};
	// -----------------------------------------------------------------------------------------------------

	VolumetricCloudAtmoshpereRenderer();
	void Initialize(Application* app);
	void InitializeShader();
	void InitializeFramebuffer();
	void InitializeNoiseFramebuffer();
	void InitializeQuarterResolutionFramebuffer();
	void InitializeMainFramebuffer();

	void ComputeNoiseTextures();
	void ComputeLowFrequencyPerlinWorleyNoiseTexture();
	void ComputeHighFrequencyWorleyTexture();
	
	void Draw();


public:
	Application* application;

	// quarter resolution
	int quarterWindowWidth;
	int quarterWindowHeight;

	// fullscreen quad
	Quad fullscreenQuad;
	QuadVertexData quadVertexData;

	Camera camera;
	Shader shader[NUM_SHADER];
	unsigned int framebuffer[NUM_FRAMEBUFFER];
	unsigned int texture2D[NUM_TEXTURE_2D];
	unsigned int texture3D[NUM_TEXTURE_3D];

	// cloud model
	float coverageScale = 0.45f;
	float ambientColorScale = 0.7f;
	float cloudType = 0.8f;
	float lowFrequencyNoiseScale = 0.3f;
	bool ignoreDetailNoise = false;
	float highFrequencyNoiseScale = 0.3f;
	float highFrequencyNoiseErodeMuliplier = 0.19f;
	float highFrequencyHeightTransitionMultiplier = 10.0f;
	float anvilBias = 0.1f;
	// cloud lighting
	glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float rainCloudAbsorptionGain = 2.3f;
	float cloudAttenuationScale = 2.5f;
	float phaseEccentricity = 0.5f;
	float phaseSilverLiningIntensity = 0.15f;
	float phaseSilverLiningSpread = 0.5f;
	float coneSpreadMultplier = 0.2f;
	float shadowSampleConeSpreadMultiplier = 0.6f;
	float powderedSugarEffectMultiplier = 10.0f;
	float toneMapperEyeExposure = 0.8f;
	// raymarch
	float maxRenderDistance = 150000.0f;
	float maxHorizontalSampleCount = 192.0f;
	float maxVerticalSampleCount = 128.0f;
	bool useEarlyExitAtFullOpacity = true;
	bool useBayerFilter = true;
	float earthRadius = 600000.0f;
	float volumetricCloudsStartRadius = 607000.0f;
	float volumetricCloudsEndRadius = 633000.0f;
	bool renderDirectlyToFullscreen = false;
	bool renderActualQuarterResolutionBuffer = false;
	// windsettings
	glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
	float windUpwardBias = 0.15f;
	float cloudSpeed = 300.0f;
	float cloudTopOffset = 500.0f;
	// sun settings
	float sunIntensity = 0.4f;
	bool moveSunManually = true;
	float sunPitch = 90.0f;
	// debug 
	bool debugBool = false;
	float debugFloat = 0.4f;
	float debugFloat2 = 0.7f;
	float debugFloat3 = 1.0f;
};

