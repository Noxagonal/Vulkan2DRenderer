
#include "GLSLGenerators.hpp"

#include <format>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vulkan::glsl::GenerateInterfaceVersion(
	size_t version
)
{
	return std::format( "#version {}\n", version );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vulkan::glsl::GenerateInterfaceVertex()
{
	return
R"glsl(
struct Vertex {
	vec2	coords;
	vec2	UVs;
	vec4	color;
	float	point_size;
	uint	single_texture_channel;
};
)glsl";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vulkan::glsl::GenerateInterfaceWindowFrameData()
{
	return
R"glsl(
layout(std140, set=0, binding=0) uniform WindowFrameData {
	vec2	multiplier;
	vec2	offset;
} window_frame_data;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceTransformationBuffer()
{
	return
R"glsl(
layout( std430, set=1, binding=0 ) readonly buffer TransformationBuffer {
	mat4	data[];
} transformation_buffer;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceIndexBuffer()
{
	return
R"glsl(
layout( std430, set=2, binding=0 ) readonly buffer IndexBuffer {
	uint	data[];
} index_buffer;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceVertexBuffer()
{
	return
R"glsl(
layout( std430, set=3, binding=0 ) readonly buffer VertexBuffer {
	Vertex	data[];
} vertex_buffer;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceSampler()
{
	return
R"glsl(
layout(set=4, binding=0) uniform sampler image_sampler;
layout(std140, set=4, binding=1) uniform image_sampler_data {
	vec4	border_color;
	uvec2	border_color_enable;
} sampler_data;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceSampledImage()
{
	return
R"glsl(
layout(set=5, binding=0) uniform texture2DArray sampled_image;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfacePushConstants()
{
	return
R"glsl(
layout(std140, push_constant) uniform PushConstants {
	uint	transformation_offset;	// Offset into the transformation buffer.
	uint	index_offset;			// Offset into the index buffer.
	uint	index_count;			// Amount of indices this shader should handle.
	uint	vertex_offset;			// Offset to first vertex in vertex buffer.
} push_constants;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceVertexOutput()
{
	return
R"glsl(
layout(location=0) out		vec2	vertex_output_UV;
layout(location=1) out		vec4	vertex_output_color;
layout(location=2) out		vec2	vertex_output_original_coords;
layout(location=3) out flat	uint	vertex_output_vertex_index;
layout(location=4) out flat	uint	vertex_output_texture_channel;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceFragmentInput()
{
	return
R"glsl(
layout(location=0) in		vec2	fragment_input_UV;
layout(location=1) in		vec4	fragment_input_color;
layout(location=2) in		vec2	fragment_input_original_coords;
layout(location=3) in flat	uint	fragment_input_vertex_index;
layout(location=4) in flat	uint	fragment_input_texture_channel;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateInterfaceFragmentOutput()
{
	return
R"glsl(
layout(location=0) out		vec4	fragment_output_color;
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateVertexDefaultMain()
{
	return
R"glsl(
void main()
{
	// These are always done.
	vec4 raw_vertex_coords			= vec4( vertex_buffer.data[ gl_VertexIndex ].coords, 0.0, 1.0 );
	vertex_output_UV				= vertex_buffer.data[ gl_VertexIndex ].UVs;
	vertex_output_color				= vertex_buffer.data[ gl_VertexIndex ].color;
	vertex_output_original_coords	= raw_vertex_coords.xy;
	vertex_output_vertex_index		= gl_VertexIndex;
	vertex_output_texture_channel	= vertex_buffer.data[ gl_VertexIndex ].single_texture_channel;

	mat4 transformation_matrix		= transformation_buffer.data[ gl_InstanceIndex + push_constants.transformation_offset ];

	// User main call goes here.
	// These may be user provided. Current code is the fallback if user does not provide vertex shader. 
	// Does the user write to gl_Position or return the position from main?
	// - User may want to modify multiple values... So it's probably a good idea to let the user write to gl_Position and others directly.
	vec2 transformed_vertex_coords	= ( transformation_matrix * raw_vertex_coords ).xy;
	vec2 viewport_vertex_coords		= transformed_vertex_coords * window_frame_data.multiplier + window_frame_data.offset;

	gl_Position						= vec4( viewport_vertex_coords, 0.5, 1.0 );
	gl_PointSize					= vertex_buffer.data[ gl_VertexIndex ].point_size;
}
)glsl";
}

std::string vk2d::vulkan::glsl::GenerateFragmentDefaultMain()
{
	return
R"glsl(
void main()
{
	if( sampler_data.border_color_enable.x > 0 ) {
		if( fragment_input_UV.x < 0.0 || fragment_input_UV.x > 1.0 ) {
			fragment_output_color	= sampler_data.border_color;
			return;
		}
	}
	if( sampler_data.border_color_enable.y > 0 ) {
		if( fragment_input_UV.y < 0.0 || fragment_input_UV.y > 1.0 ) {
			fragment_output_color	= sampler_data.border_color;
			return;
		}
	}

	vec4	texture_color		= texture( sampler2DArray( sampled_image, image_sampler ), vec3( fragment_input_UV, float( fragment_input_texture_channel ) ) );
	fragment_output_color		= texture_color * fragment_input_color;
}
)glsl";
}
