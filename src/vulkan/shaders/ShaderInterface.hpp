#pragma once

#include <core/SourceCommon.hpp>

#include <utility/Hasher.hpp>



namespace vk2d {
namespace vulkan {



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



enum class GraphicsShaderListID_DEPRICATED {
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

enum class ComputeShaderProgramID_DEPRICATED
{
	SHADER_STAGE_ID_COUNT
};



class GraphicsShaderList {
public:
	inline									GraphicsShaderList() = default;

	inline									GraphicsShaderList(
		const GraphicsShaderList		&	other
	) = default;

	template<typename T>
	inline									GraphicsShaderList(
		const std::initializer_list<T>	&	init_list
	) = delete;

	inline									GraphicsShaderList(
		VkShaderModule						vertex,
		VkShaderModule						fragment
	) :
		vertex( vertex ),
		fragment( fragment )
	{
		hash = CalculateHash();
	}

	inline GraphicsShaderList			&	operator=(
		const GraphicsShaderList		&	other
	) = default;

	inline bool								operator<(
		const GraphicsShaderList		&	other
	) const
	{
		return hash < other.GetHash();
	}

	inline VkShaderModule					GetVertexShader() const
	{
		return vertex;
	}

	inline VkShaderModule					GetFragmentShader() const
	{
		return fragment;
	}

	inline size_t							GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline constexpr size_t					CalculateHash() const
	{
		Hasher hasher;
		hasher.Hash( reinterpret_cast<size_t>( vertex ) );
		hasher.Hash( reinterpret_cast<size_t>( fragment ) );
		return hasher.GetHash();
	}

	VkShaderModule							vertex						= {};
	VkShaderModule							fragment					= {};

	size_t									hash						= {};
};



} // vulkan
} // vk2d
