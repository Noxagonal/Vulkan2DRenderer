#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {

namespace vk2d_internal {

// Descriptor set allocations.
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA					= 0;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TRANSFORMATION					= 1;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER	= 2;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER	= 3;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_SAMPLER_AND_SAMPLER_DATA			= 4;
constexpr uint32_t GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE							= 5;



struct WindowCoordinateScaling {
	alignas( 8 )	glm::vec2					multiplier				= {};
	alignas( 8 )	glm::vec2					offset					= {};
};

struct FrameData {
	alignas( 8 )	WindowCoordinateScaling		coordinate_scaling		= {};
};

struct GraphicsPrimaryRenderPushConstants {
	alignas( 4 )	uint32_t					transformation_offset			= {};	// Offset into transformations buffer.
	alignas( 4 )	uint32_t					index_offset					= {};	// Offset into the index buffer.
	alignas( 4 )	uint32_t					index_count						= {};	// Amount of indices this shader should handle.
	alignas( 4 )	uint32_t					vertex_offset					= {};	// Offset to first vertex in vertex buffer.
};

struct GraphicsBlurPushConstants
{
	alignas( 4 )	std::array<float, 4>		blur_info				= {};	// [ 0 ] = sigma, [ 1 ] = precomputed normalizer, [ 2 ] = initial coefficient, [ 3 ] = initial natural exponentation, 
	alignas( 4 )	std::array<float, 2>		pixel_size				= {};	// Pixel size on a canvas considered ranging from 0 to 1.
};



enum class GraphicsShaderListID {
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



class GraphicsShaderList {
public:
	GraphicsShaderList()												= default;

	GraphicsShaderList(
		const GraphicsShaderList		&	other )						= default;

	GraphicsShaderList(
		GraphicsShaderList			&&	other )							= default;

	template<typename T>
	GraphicsShaderList(
		const std::initializer_list<T>	&	init_list )					= delete;

	inline GraphicsShaderList(
		VkShaderModule						vertex,
		VkShaderModule						fragment
	) :
		vertex( vertex ),
		fragment( fragment )
	{}

	GraphicsShaderList				&	operator=(
		const GraphicsShaderList		&	other )						= default;

	GraphicsShaderList				&	operator=(
		GraphicsShaderList			&&	other )							= default;

	bool									operator<(
		const GraphicsShaderList		&	other ) const;
	bool									operator>(
		const GraphicsShaderList		&	other ) const;
	bool									operator<=(
		const GraphicsShaderList		&	other ) const;
	bool									operator>=(
		const GraphicsShaderList		&	other ) const;
	bool									operator==(
		const GraphicsShaderList		&	other ) const;
	bool									operator!=(
		const GraphicsShaderList		&	other ) const;

	VkShaderModule							vertex						= {};
	VkShaderModule							fragment					= {};
};



class GraphicsPipelineSettings {
public:
	GraphicsPipelineSettings()											= default;
	GraphicsPipelineSettings(
		const GraphicsPipelineSettings	&	other )						= default;
	GraphicsPipelineSettings(
		GraphicsPipelineSettings		&&	other )						= default;
	template<typename T>
	GraphicsPipelineSettings(
		const std::initializer_list<T>	&	init_list )					= delete;

	GraphicsPipelineSettings			&	operator=(
		const GraphicsPipelineSettings	&	other )						= default;
	GraphicsPipelineSettings			&	operator=(
		GraphicsPipelineSettings		&&	other )						= default;

	bool									operator<(
		const GraphicsPipelineSettings	&	other ) const;
	bool									operator>(
		const GraphicsPipelineSettings	&	other ) const;
	bool									operator<=(
		const GraphicsPipelineSettings	&	other ) const;
	bool									operator>=(
		const GraphicsPipelineSettings	&	other ) const;
	bool									operator==(
		const GraphicsPipelineSettings	&	other ) const;
	bool									operator!=(
		const GraphicsPipelineSettings	&	other ) const;

	VkPipelineLayout						vk_pipeline_layout			= {};
	VkRenderPass							vk_render_pass				= {};
	VkPrimitiveTopology						primitive_topology			= {};
	VkPolygonMode							polygon_mode				= {};
	GraphicsShaderList						shader_programs				= {};
	VkSampleCountFlags						samples						= {};
	VkBool32								enable_blending				= {};
};



class ComputePipelineSettings
{
public:
	ComputePipelineSettings()											= default;
	ComputePipelineSettings(
		const ComputePipelineSettings	&	other )						= default;
	ComputePipelineSettings(
		ComputePipelineSettings			&&	other )						= default;
	template<typename T>
	ComputePipelineSettings(
		const std::initializer_list<T>	&	init_list )					= delete;

	ComputePipelineSettings				&	operator=(
		const ComputePipelineSettings	&	other )						= default;
	ComputePipelineSettings				&	operator=(
		ComputePipelineSettings			&&	other )						= default;

	bool									operator<(
		const ComputePipelineSettings	&	other ) const;
	bool									operator>(
		const ComputePipelineSettings	&	other ) const;
	bool									operator<=(
		const ComputePipelineSettings	&	other ) const;
	bool									operator>=(
		const ComputePipelineSettings	&	other ) const;
	bool									operator==(
		const ComputePipelineSettings	&	other ) const;
	bool									operator!=(
		const ComputePipelineSettings	&	other ) const;

	VkPipelineLayout						vk_pipeline_layout			= {};
	VkShaderModule							vk_shader_program			= {};
};



} // vk2d_internal

} // vk2d
