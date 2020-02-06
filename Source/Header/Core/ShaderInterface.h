#pragma once

#include "SourceCommon.h"

#include "../../../Include/Interface/RenderPrimitives.h"


namespace vk2d {

namespace _internal {

// Descriptor set allocations.
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA					= 0;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER		= 1;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER	= 2;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_TEXTURE_AND_SAMPLER				= 3;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_TEXTURE_CHANNEL_WEIGHTS			= 4;



struct WindowCoordinateScaling {
	alignas( 8 )	vk2d::Vector2f				multiplier				= {};
	alignas( 8 )	vk2d::Vector2f				offset					= {};
};

struct WindowFrameData {
	alignas( 8 )	WindowCoordinateScaling		coordinate_scaling		= {};
};

struct PushConstants {
	alignas( 4 )	uint32_t					index_offset			= {};	// Offset into the index buffer.
	alignas( 4 )	uint32_t					index_count				= {};	// Amount of indices this shader should handle.
	alignas( 4 )	uint32_t					vertex_offset			= {};	// Offset to first vertex in vertex buffer.
	alignas( 4 )	uint32_t					texture_channel_offset	= {};	// Location of the texture channels in the texture channel weights ssbo.
	alignas( 4 )	uint32_t					texture_channel_count	= {};	// Just the amount of texture channels.
};



enum class ShaderProgramID {
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



class ShaderProgram {
public:
	ShaderProgram()														= default;
	ShaderProgram( const vk2d::_internal::ShaderProgram & other )		= default;
	ShaderProgram( vk2d::_internal::ShaderProgram && other )			= default;
	template<typename T>
	ShaderProgram( std::initializer_list<T> )							= delete;
	inline ShaderProgram(
		VkShaderModule										vertex,
		VkShaderModule										fragment
	) :
		vertex( vertex ),
		fragment( fragment )
	{}

	vk2d::_internal::ShaderProgram & operator=( const vk2d::_internal::ShaderProgram & other )		= default;
	vk2d::_internal::ShaderProgram & operator=( vk2d::_internal::ShaderProgram && other )			= default;

	bool operator<( const vk2d::_internal::ShaderProgram & other ) const;
	bool operator>( const vk2d::_internal::ShaderProgram & other ) const;
	bool operator<=( const vk2d::_internal::ShaderProgram & other ) const;
	bool operator>=( const vk2d::_internal::ShaderProgram & other ) const;
	bool operator==( const vk2d::_internal::ShaderProgram & other ) const;
	bool operator!=( const vk2d::_internal::ShaderProgram & other ) const;

	VkShaderModule						vertex				= {};
	VkShaderModule						fragment			= {};
};



class PipelineSettings {
public:
	PipelineSettings()																= default;
	PipelineSettings( const vk2d::_internal::PipelineSettings & other )				= default;
	PipelineSettings( vk2d::_internal::PipelineSettings && other )					= default;
	template<typename T>
	PipelineSettings( std::initializer_list<T> )									= delete;

	vk2d::_internal::PipelineSettings & operator=( const vk2d::_internal::PipelineSettings & other )	= default;
	vk2d::_internal::PipelineSettings & operator=( vk2d::_internal::PipelineSettings && other )			= default;

	bool operator<( const vk2d::_internal::PipelineSettings & other ) const;
	bool operator>( const vk2d::_internal::PipelineSettings & other ) const;
	bool operator<=( const vk2d::_internal::PipelineSettings & other ) const;
	bool operator>=( const vk2d::_internal::PipelineSettings & other ) const;
	bool operator==( const vk2d::_internal::PipelineSettings & other ) const;
	bool operator!=( const vk2d::_internal::PipelineSettings & other ) const;

	VkRenderPass						render_pass					= {};
	VkPrimitiveTopology					primitive_topology			= {};
	VkPolygonMode						polygon_mode				= {};
	vk2d::_internal::ShaderProgram		shader_programs				= {};
	VkSampleCountFlags					samples						= {};
};



} // _internal

} // vk2d
