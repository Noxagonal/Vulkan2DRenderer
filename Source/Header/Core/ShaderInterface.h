#pragma once

#include "SourceCommon.h"

#include "../../../Include/Interface/RenderPrimitives.h"


namespace vk2d {

namespace _internal {

// Descriptor set allocations.
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA					= 0;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER	= 1;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER	= 2;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_SAMPLER_AND_SAMPLER_DATA			= 3;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE							= 4;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE_CHANNEL_WEIGHTS			= 5;



struct WindowCoordinateScaling {
	alignas( 8 )	vk2d::Vector2f				multiplier				= {};
	alignas( 8 )	vk2d::Vector2f				offset					= {};
};

struct FrameData {
	alignas( 8 )	WindowCoordinateScaling		coordinate_scaling		= {};
};

struct GraphicsPushConstants {
	alignas( 4 )	uint32_t					index_offset			= {};	// Offset into the index buffer.
	alignas( 4 )	uint32_t					index_count				= {};	// Amount of indices this shader should handle.
	alignas( 4 )	uint32_t					vertex_offset			= {};	// Offset to first vertex in vertex buffer.
	alignas( 4 )	uint32_t					texture_channel_offset	= {};	// Location of the texture channels in the texture channel weights ssbo.
	alignas( 4 )	uint32_t					texture_channel_count	= {};	// Just the amount of texture channels.
};

struct ComputeBlurPushConstants
{
	alignas( 4 )	uint32_t					kernel_count			= {};	// How many kernels we're using to determine blur.
	alignas( 4 )	uint32_t					image_size_x			= {};	// Size of the image we're processing.
	alignas( 4 )	uint32_t					image_size_y			= {};	// Size of the image we're processing.
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

	SHADER_STAGE_ID_COUNT
};

enum class ComputeShaderProgramID
{
	RENDER_TARGET_BLUR_PASS_1,
	RENDER_TARGET_BLUR_PASS_2,

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

	VkRenderPass							vk_render_pass				= {};
	VkPrimitiveTopology						primitive_topology			= {};
	VkPolygonMode							polygon_mode				= {};
	vk2d::_internal::GraphicsShaderProgram	shader_programs				= {};
	VkSampleCountFlags						samples						= {};
};



class ComputeShaderProgram
{
public:
	ComputeShaderProgram()														= default;
	ComputeShaderProgram( const vk2d::_internal::ComputeShaderProgram & other )	= default;
	ComputeShaderProgram( vk2d::_internal::ComputeShaderProgram && other )		= default;
	template<typename T>
	ComputeShaderProgram( std::initializer_list<T> )							= delete;
	inline ComputeShaderProgram(
		VkShaderModule										compute
	) :
		compute( compute )
	{}

	vk2d::_internal::ComputeShaderProgram & operator=( const vk2d::_internal::ComputeShaderProgram & other )	= default;
	vk2d::_internal::ComputeShaderProgram & operator=( vk2d::_internal::ComputeShaderProgram && other )			= default;

	bool operator<( const vk2d::_internal::ComputeShaderProgram & other ) const;
	bool operator>( const vk2d::_internal::ComputeShaderProgram & other ) const;
	bool operator<=( const vk2d::_internal::ComputeShaderProgram & other ) const;
	bool operator>=( const vk2d::_internal::ComputeShaderProgram & other ) const;
	bool operator==( const vk2d::_internal::ComputeShaderProgram & other ) const;
	bool operator!=( const vk2d::_internal::ComputeShaderProgram & other ) const;

	VkShaderModule						compute				= {};
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

	vk2d::_internal::ComputeShaderProgram	shader_programs				= {};
	VkPipelineLayout						pipeline_layout				= {};
};



} // _internal

} // vk2d
