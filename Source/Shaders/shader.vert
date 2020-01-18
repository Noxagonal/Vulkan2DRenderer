#version 450
#extension GL_KHR_vulkan_glsl : enable

// const uint VERTEX_SIZE_IN_FLOATS = 12;

// Vertex.
layout(location=0) in vec2 vertex_input_coords;
layout(location=1) in vec2 vertex_input_UVs;
layout(location=2) in vec4 vertex_input_color;
layout(location=3) in float vertex_input_point_size;
// layout(location=4) in vec3 ___padding___;

// Set 0: Window frame data.
layout(set=0, binding=0) uniform WindowFrameData {
	vec2 multiplier;
	vec2 offset;
} window_frame_data;

// Push constants.
layout(push_constant) uniform PushConstants {
	uint index_offset;				// Offset into the index buffer.
	uint index_count;				// Amount of indices this shader should handle.
	uint texture_channel_offset;	// Location of the texture channels in the texture channel weights ssbo.
	uint texture_channel_count;		// Just the amount of texture channels.
} push_constants;

// Output to fragment shader
layout(location=0) out vec2 fragment_output_UV;
layout(location=1) out vec4 fragment_output_color;

void main()
{
	fragment_output_UV				= vertex_input_UVs;
	fragment_output_color			= vertex_input_color;
	vec2 transformed_vertex_coords	= vertex_input_coords * window_frame_data.multiplier + window_frame_data.offset;
	gl_Position						= vec4( transformed_vertex_coords, 1.0f, 1.0f );
	gl_PointSize					= vertex_input_point_size;

//	DEBUGGING.
//	gl_Position						= vec4( vertex_input_coords / vec2( 800.0, 600.0 ), 1.0f, 1.0f );
//	gl_PointSize					= 1.0;
}
