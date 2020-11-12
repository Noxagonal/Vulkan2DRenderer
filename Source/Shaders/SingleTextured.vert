#version 450
#extension GL_KHR_vulkan_glsl : enable



////////////////////////////////////////////////////////////////
// Shader program interface.
////////////////////////////////////////////////////////////////

// Set 0: Window frame data.
layout(std140, set=0, binding=0) uniform			WindowFrameData {
	vec2		multiplier;
	vec2		offset;
} window_frame_data;

// Vertex.
struct Vertex {
	vec2		coords;
	vec2		UVs;
	vec4		color;
	float		point_size;
	uint		single_texture_channel;
};
layout(std430, set=2, binding=0) readonly buffer	VertexBuffer {
	Vertex		ssbo[];
} vertex_buffer;

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	uint		index_offset;				// Offset into the index buffer.
	uint		index_count;				// Amount of indices this shader should handle.
	uint		vertex_offset;				// Offset to first vertex in vertex buffer.
	uint		texture_channel_offset;		// Location of the texture channels in the texture channel weights ssbo.
	uint		texture_channel_count;		// Just the amount of texture channels.
} push_constants;

// Output to fragment shader
layout(location=0) out		vec2	fragment_output_UV;
layout(location=1) out		vec4	fragment_output_color;
layout(location=2) out flat	uint	fragment_output_texture_channel;



////////////////////////////////////////////////////////////////
// Entrypoints.
////////////////////////////////////////////////////////////////

void SingleTexturedVertex()
{
	fragment_output_UV				= vertex_buffer.ssbo[ gl_VertexIndex ].UVs;
	fragment_output_color			= vertex_buffer.ssbo[ gl_VertexIndex ].color;
	vec2 vertex_coords				= vertex_buffer.ssbo[ gl_VertexIndex ].coords;
	vec2 transformed_vertex_coords	= vertex_coords * window_frame_data.multiplier + window_frame_data.offset;
	fragment_output_texture_channel	= vertex_buffer.ssbo[ gl_VertexIndex ].single_texture_channel;
	gl_Position						= vec4( transformed_vertex_coords, 1.0f, 1.0f );
	gl_PointSize					= vertex_buffer.ssbo[ gl_VertexIndex ].point_size;
}
