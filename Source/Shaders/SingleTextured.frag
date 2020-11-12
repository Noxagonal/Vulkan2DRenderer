#version 450
#extension GL_KHR_vulkan_glsl : enable



////////////////////////////////////////////////////////////////
// Shader program interface.
////////////////////////////////////////////////////////////////

// Set 3: Sampler
layout(set=3, binding=0) uniform sampler			image_sampler;
layout(std140, set=3, binding=1) uniform			image_sampler_data {
	vec4		border_color;
	uvec2		border_color_enable;
} sampler_data;

// Set 4: Texture
layout(set=4, binding=0) uniform texture2DArray		sampled_image;

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	uint		index_offset;				// Offset into the index buffer.
	uint		index_count;				// Amount of indices this shader should handle.
	uint		vertex_offset;				// Offset to first vertex in vertex buffer.
	uint		texture_channel_offset;		// Location of the texture channels in the texture channel weights ssbo.
	uint		texture_channel_count;		// Just the amount of texture channels.
} push_constants;

// From vertex shader.
layout(location=0) in		vec2	fragment_input_UV;
layout(location=1) in		vec4	fragment_input_color;
layout(location=2) in flat	uint	fragment_input_texture_channel;

// Color output.
layout(location=0) out vec4 final_fragment_color;



////////////////////////////////////////////////////////////////
// Entrypoints.
////////////////////////////////////////////////////////////////

// Single textured triangle / line / point that can have custom uv border color.
// Only a single texture channel can be used per vertex.
void SingleTexturedFragmentWithUVBorderColor()
{
	if( sampler_data.border_color_enable.x > 0 ) {
		if( fragment_input_UV.x < 0.0 || fragment_input_UV.x > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
			return;
		}
	}
	if( sampler_data.border_color_enable.y > 0 ) {
		if( fragment_input_UV.y < 0.0 || fragment_input_UV.y > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
			return;
		}
	}

	vec4	texture_color		= texture( sampler2DArray( sampled_image, image_sampler ), vec3( fragment_input_UV, float( fragment_input_texture_channel ) ) );
	final_fragment_color		= texture_color * fragment_input_color;
}



// Single textured triangle / line / point that can NOT have custom uv border color.
// Only a single texture channel can be used per vertex.
void SingleTexturedFragment()
{
	vec4	texture_color		= texture( sampler2DArray( sampled_image, image_sampler ), vec3( fragment_input_UV, float( fragment_input_texture_channel ) ) );
	final_fragment_color		= texture_color * fragment_input_color;
}
