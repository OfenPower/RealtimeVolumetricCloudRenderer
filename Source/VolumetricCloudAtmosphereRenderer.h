#pragma once

#include "Camera.h"
#include "Quad.h"
#include "QuadVertexData.h"
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
		T_PERLIN_WORLEY,
		T_WORLEY,
		NUM_TEXTURE_3D
	};

	enum UniformLocation {
		// volumetric cloud shader
		INV_VIEW,
		INV_PROJ,
		CAM_POS,
		TIME,
		RESOLUTION_1,
		UL_PERLIN_WORLEY,
		UL_WORLEY,
		SUN_INTENSITY,
		SUN_POSITION,
		COVERAGE_SCALE,
		CLOUD_COLOR,
		RAIN_CLOUD_ABSORPTION_GAIN,
		CLOUD_ATTENUATION_SCALE,
		AMBIENT_COLOR_SCALE,
		CLOUD_TYPE,
		TONEMAPPER_EYE_EXPOSURE,
		IGNORE_DETAIL_NOISE,
		ANVIL_BIAS,
		LOW_FREQUENCY_NOISE_SCALE,
		HIGH_FREQUENCY_NOISE_SCALE,
		HIGH_FREQUENCY_NOISE_ERODE_MULTIPLIER,
		HIGH_FREQUENCY_HEIGHT_TRANSITION_MULTIPLIER,
		PHASE_ECCENTRICITY,
		PHASE_SILVER_LINING_INTENSITY,
		PHASE_SILVER_LINING_SPREAD,
		CONE_SPREAD_MULTIPLIER,
		SHADOW_SAMPLE_CONE_SPREAD_MULTIPLIER,
		POWDERED_SUGAR_EFFECT_MULTIPLIER,
		MAX_RENDER_DISTANCE,
		MAX_HORIZONTAL_SAMPLE_COUNT,
		MAX_VERTICAL_SAMPLE_COUNT,
		USE_EARLY_EXIT_AT_FULL_OPACITY,
		USE_BAYER_FILTER,
		EARTH_RADIUS,
		VOLUMETRIC_CLOUDS_START_RADIUS,
		VOLUMETRIC_CLOUDS_END_RADIUS,
		WIND_DIRECTION,
		WIND_UPWARD_BIAS,
		CLOUD_SPEED,
		CLOUD_TOP_OFFSET,
		DEBUG_BOOL,
		DEBUG_FLOAT1,
		DEBUG_FLOAT2,
		DEBUG_FLOAT3,
		// upscale shader
		RESOLUTION_2,
		BUFF,
		// enum count
		NUM_UNIFORM_LOCATIONS
	};
	// -----------------------------------------------------------------------------------------------------

	VolumetricCloudAtmoshpereRenderer();
	void Initialize(Application* app);
	void InitializeShader();
	void InitializeFramebuffer();
	void InitializeNoiseFramebuffer();
	void InitializeQuarterResolutionFramebuffer();
	void InitializeMainFramebuffer();
	void InitializeShaderUniformLocations();
	void InitializeVolumetricCloudAtmosphereShaderUniformLocations();
	void InitializeUpscaleShaderUniformLocations();

	void ComputeNoiseTextures();
	void ComputeLowFrequencyPerlinWorleyNoiseTexture();
	void ComputeHighFrequencyWorleyTexture();

	void Update(float deltaTime);
	void Draw();


public:
	Application* application;

	// quarter resolution
	int quarterWindowWidth;
	int quarterWindowHeight;

	// 2D quad to which will be rendered
	Quad quad;
	QuadVertexData quadVertexData;

	// camera, view and projection matrix
	Camera camera;
	glm::mat4 view;
	glm::mat4 projection;
	
	// arrays für shader, framebuffer, etc.
	Shader shader[NUM_SHADER];
	unsigned int framebuffer[NUM_FRAMEBUFFER];
	unsigned int texture2D[NUM_TEXTURE_2D];
	unsigned int texture3D[NUM_TEXTURE_3D];
	unsigned int uniformLocation[NUM_UNIFORM_LOCATIONS];

	// volumetric cloud variables
	//

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
	float maxHorizontalSampleCount = 128.0f;
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

