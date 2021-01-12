#pragma once

#include "Core/SourceCommon.h"



namespace vk2d {

namespace _internal {

// Descriptor set allocations.
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA					= 0;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TRANSFORMATION					= 1;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER	= 2;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER	= 3;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_SAMPLER_AND_SAMPLER_DATA			= 4;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE							= 5;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_texture_channel_weights			= 6;



struct WindowCoordinateScaling {
	alignas( 8 )	glm::vec2					multiplier				= {};
	alignas( 8 )	glm::vec2					offset					= {};
};

struct FrameData {
	alignas( 8 )	WindowCoordinateScaling		coordinate_scaling		= {};
};

struct GraphicsPrimaryRenderPushConstants {
	alignas( 4 )	uint32_t					transformation_offset	= {};	// Offset into transformations buffer.
	alignas( 4 )	uint32_t					index_offset			= {};	// Offset into the index buffer.
	alignas( 4 )	uint32_t					index_count				= {};	// Amount of indices this shader should handle.
	alignas( 4 )	uint32_t					vertex_offset			= {};	// Offset to first vertex in vertex buffer.
	alignas( 4 )	uint32_t					texture_channel_weight_offset	= {};	// Location of the texture channels in the texture channel weights ssbo.
	alignas( 4 )	uint32_t					texture_channel_weight_count	= {};	// Just the amount of texture channels.
};

struct GraphicsBlurPushConstants
{
	alignas( 4 )	std::array<float, 4>		blur_info				= {};	// [ 0 ] = sigma, [ 1 ] = precomputed normalizer, [ 2 ] = initial coefficient, [ 3 ] = initial natural exponentation, 
	alignas( 4 )	std::array<float, 2>		pixel_size				= {};	// Pixel size on a canvas considered ranging from 0 to 1.
};



enum class GraphicsShaderProgramID {
	SINGLE_TEXTURED,
	SINGLE_TEXTURED_UV_BORDER_COLOR,

	MULTITEXTURED_TRIANGLE,
	MULTITEXTURED_LINE,
	MULTITEXTURED_POINT,
	MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR,
	MULTITEXTURED_LINE_UV_BORDER_COLOR,
	MULTITEXTURED_POINT_UV_BORDER_COLOR,

	RENDER_TARGET_BOX_BLUR_HORISONTAL,
	RENDER_TARGET_BOX_BLUR_VERTICAL,
	RENDER_TARGET_GAUSSIAN_BLUR_HORISONTAL,
	RENDER_TARGET_GAUSSIAN_BLUR_VERTICAL,

	SHADER_STAGE_ID_COUNT
};

enum class ComputeShaderProgramID
{
	SHADER_STAGE_ID_COUNT
};



class GraphicsShaderProgram {
public:
	GraphicsShaderProgram()															= default;
	GraphicsShaderProgram( const vk2d::_internal::GraphicsShaderProgram & other )	= default;
	GraphicsShaderProgram( vk2d::_internal::GraphicsShaderProgram && other )		= default;
	template<typename T>
	GraphicsShaderProgram( std::initializer_list<T> )								= delete;
	inline GraphicsShaderProgram(
		VkShaderModule										vertex,
		VkShaderModule										fragment
	) :
		vertex( vertex ),
		fragment( fragment )
	{}

	vk2d::_internal::GraphicsShaderProgram & operator=( const vk2d::_internal::GraphicsShaderProgram & other )		= default;
	vk2d::_internal::GraphicsShaderProgram & operator=( vk2d::_internal::GraphicsShaderProgram && other )			= default;

	bool operator<( const vk2d::_internal::GraphicsShaderProgram & other ) const;
	bool operator>( const vk2d::_internal::GraphicsShaderProgram & other ) const;
	bool operator<=( const vk2d::_internal::GraphicsShaderProgram & other ) const;
	bool operator>=( const vk2d::_internal::GraphicsShaderProgram & other ) const;
	bool operator==( const vk2d::_internal::GraphicsShaderProgram & other ) const;
	bool operator!=( const vk2d::_internal::GraphicsShaderProgram & other ) const;

	VkShaderModule						vertex				= {};
	VkShaderModule						fragment			= {};
};



class GraphicsPipelineSettings {
public:
	GraphicsPipelineSettings()																= default;
	GraphicsPipelineSettings( const vk2d::_internal::GraphicsPipelineSettings & other )		= default;
	GraphicsPipelineSettings( vk2d::_internal::GraphicsPipelineSettings && other )			= default;
	template<typename T>
	GraphicsPipelineSettings( std::initializer_list<T> )									= delete;

	vk2d::_internal::GraphicsPipelineSettings & operator=( const vk2d::_internal::GraphicsPipelineSettings & other )	= default;
	vk2d::_internal::GraphicsPipelineSettings & operator=( vk2d::_internal::GraphicsPipelineSettings && other )			= default;

	bool operator<( const vk2d::_internal::GraphicsPipelineSettings & other ) const;
	bool operator>( const vk2d::_internal::GraphicsPipelineSettings & other ) const;
	bool operator<=( const vk2d::_internal::GraphicsPipelineSettings & other ) const;
	bool operator>=( const vk2d::_internal::GraphicsPipelineSettings & other ) const;
	bool operator==( const vk2d::_internal::GraphicsPipelineSettings & other ) const;
	bool operator!=( const vk2d::_internal::GraphicsPipelineSettings & other ) const;

	VkPipelineLayout						vk_pipeline_layout			= {};
	VkRenderPass							vk_render_pass				= {};
	VkPrimitiveTopology						primitive_topology			= {};
	VkPolygonMode							polygon_mode				= {};
	vk2d::_internal::GraphicsShaderProgram	shader_programs				= {};
	VkSampleCountFlags						samples						= {};
	VkBool32								enable_blending				= {};
};



class ComputePipelineSettings
{
public:
	ComputePipelineSettings()																= default;
	ComputePipelineSettings( const vk2d::_internal::ComputePipelineSettings & other )		= default;
	ComputePipelineSettings( vk2d::_internal::ComputePipelineSettings && other )			= default;
	template<typename T>
	ComputePipelineSettings( std::initializer_list<T> )										= delete;

	vk2d::_internal::ComputePipelineSettings & operator=( const vk2d::_internal::ComputePipelineSettings & other )	= default;
	vk2d::_internal::ComputePipelineSettings & operator=( vk2d::_internal::ComputePipelineSettings && other )		= default;

	bool operator<( const vk2d::_internal::ComputePipelineSettings & other ) const;
	bool operator>( const vk2d::_internal::ComputePipelineSettings & other ) const;
	bool operator<=( const vk2d::_internal::ComputePipelineSettings & other ) const;
	bool operator>=( const vk2d::_internal::ComputePipelineSettings & other ) const;
	bool operator==( const vk2d::_internal::ComputePipelineSettings & other ) const;
	bool operator!=( const vk2d::_internal::ComputePipelineSettings & other ) const;

	VkPipelineLayout						vk_pipeline_layout			= {};
	VkShaderModule							vk_shader_program			= {};
};



} // _internal

} // vk2d
