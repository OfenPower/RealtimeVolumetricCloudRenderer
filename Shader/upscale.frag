#version 430 core

uniform sampler2D buff;		// subbufertex - the texture which contains the rendered clouds

uniform vec2 resolution;		// (WIDTH, HEIGHT)
uniform float downscaleFactor;	// factor 4

out vec4 color;

void main()
{
	// get the "downscale-th" fragment screen space coordinate, for example: 
	// If downscale = 4, then uv is (0,0) for the first 4 fragments on the screen x-axis (beginning at lower left origin) AND for the screen y-axis.
	// Then, uv is (1,0) for the following 4 fragments on the x-axis and (2,0) for the next 4 fragments, and so on. 
	// This means, that for every fragment in this shader, uv contains the same screen space position for a block of 4x4 consecutive fragments, so
	// that those 4x4 fragment block can be assigned with the same color from a texture, when uv is used as texture coordinates.
	vec2 uv = floor(gl_FragCoord.xy / 4);	

	// convert screen space uv to texture coordinates, so that the uv can be used to sample the quarterResolutionColorbuffer texture. 
	// Since the resolution for the quarterResolutionColorbuffer was downscaled, the current fullscreen resolution needs to be downscaled too 
	// in order for the texture coordinates to be valid
	uv = uv / (resolution / 4);
	
	// Sample a color from the quarterResolutionColorbuffer texture. Since uv is the same for downscaled-many (=4x4) consecutive fragments,
	// the sampled color will be the same for those fragments (pixelated look if downscaled is too high, e.g. downscaled=16) 
	vec4 sampledColor = texture(buff, uv.xy);

	// output fragment color
	color.xyz = sampledColor.xyz;
	color.a = 1.0;

	// ideally one would use temporal reprojection instead of simply upscaling. A good resource on that can be found here
	// http://john-chapman-graphics.blogspot.ca/2013/01/what-is-motion-blur-motion-pictures-are.html. But simply upscaling
	// works surprisingly well with cloud shapes
}