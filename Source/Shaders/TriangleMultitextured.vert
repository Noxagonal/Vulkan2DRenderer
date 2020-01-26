#version 450
#extension GL_KHR_vulkan_glsl : enable

// const uint VERTEX_SIZE_IN_FLOATS = 12;

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
layout(location=0) out vec2 fragment_output_UV;
layout(location=1) out vec4 fragment_output_color;
layout(location=2) out vec2 fragment_output_original_coords;



// Entrypoint
void MultitexturedTriangle()
{
	fragment_output_UV				= vertex_buffer.ssbo[ gl_VertexIndex ].UVs;
	fragment_output_color			= vertex_buffer.ssbo[ gl_VertexIndex ].color;
	vec2 vertex_coords				= vertex_buffer.ssbo[ gl_VertexIndex ].coords;
	vec2 transformed_vertex_coords	= vertex_coords * window_frame_data.multiplier + window_frame_data.offset;
	fragment_output_original_coords	= vertex_coords;
	gl_Position						= vec4( transformed_vertex_coords, 1.0f, 1.0f );
	gl_PointSize					= vertex_buffer.ssbo[ gl_VertexIndex ].point_size;

//	DEBUGGING.
//	gl_Position						= vec4( vertex_input_coords / vec2( 800.0, 600.0 ), 1.0f, 1.0f );
//	gl_PointSize					= 1.0;
}
