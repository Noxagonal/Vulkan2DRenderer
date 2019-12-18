#version 450
#extension GL_KHR_vulkan_glsl : enable

// Uniforms
layout(set=0, binding=0) uniform sampler		imageSampler;
layout(set=0, binding=1) uniform				imageSamplerData {
	vec4	borderColor;
	vec4	borderColorEnable;
} samplerData;

layout(set=1, binding=0) uniform texture2D		image;

// From vertex shader
layout(location=0) in vec2 fragment_UV;
layout(location=1) in vec4 fragment_Color;

layout(location=0) out vec4 finalFragmentColor;

void main()
{
	finalFragmentColor			= texture( sampler2D( image, imageSampler ), fragment_UV ) * fragment_Color;

	if( samplerData.borderColorEnable.x > 0.5 ) {
		if( fragment_UV.x < 0.0 || fragment_UV.x > 1.0 ) {
			finalFragmentColor	= samplerData.borderColor;
		}
	}
	if( samplerData.borderColorEnable.y > 0.5 ) {
		if( fragment_UV.y < 0.0 || fragment_UV.y > 1.0 ) {
			finalFragmentColor	= samplerData.borderColor;
		}
	}

//	DEBUG:
//	finalFragmentColor			= fragment_Color;
}
