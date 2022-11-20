#pragma once

#include <core/SourceCommon.h>

#include <system/MeshBuffer.h>
#include <mesh/vertex/RawVertexData.hpp>



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct AlignedMeshBufferOffsets
{
	AlignedMeshBufferOffsets() = default;
	AlignedMeshBufferOffsets(
		const vk2d::vk2d_internal::MeshBuffer::PushResult		&	push_result,
		const vk2d::vk2d_internal::RawVertexData				&	raw_vertex_data
	)
	{
		// Because mesh may be a custom size it introduces a problem in how memory is being accessed inside the shader.
		// GraphicsPrimaryRenderPushConstants holds offsets to different buffers. In order to access different buffers properly inside
		// the shader, we'll recalculate the offsets. Different sized vertices are already properly aligned by the MeshBuffer.

		auto & transformation_block = push_result.location_info.transformation_block;
		auto & index_block = push_result.location_info.index_block;
		auto & vertex_block = push_result.location_info.vertex_block;
		auto & texture_channel_weight_block = push_result.location_info.texture_channel_weight_block;

		transformation_offset = uint32_t( transformation_block.byte_offset / sizeof( std::remove_reference_t<decltype( transformation_block )>::Type ) );
		assert( transformation_block.byte_offset % sizeof( std::remove_reference_t<decltype( transformation_block )>::Type ) == 0 );

		index_offset = uint32_t( index_block.byte_offset / sizeof( std::remove_reference_t<decltype( index_block )>::Type ) );
		assert( index_block.byte_offset % sizeof( std::remove_reference_t<decltype( index_block )>::Type ) == 0 );

		vertex_offset = uint32_t( vertex_block.byte_offset / raw_vertex_data.vertex_descriptor.size );
		assert( vertex_block.byte_offset % raw_vertex_data.vertex_descriptor.size == 0 );

		texture_channel_weight_offset = uint32_t( texture_channel_weight_block.byte_offset / sizeof( std::remove_reference_t<decltype( texture_channel_weight_block )>::Type ) );
		assert( texture_channel_weight_block.byte_offset % sizeof( std::remove_reference_t<decltype( texture_channel_weight_block )>::Type ) == 0 );
	}

	uint32_t	transformation_offset = {};
	uint32_t	index_offset = {};
	uint32_t	vertex_offset = {};
	uint32_t	texture_channel_weight_offset = {};
};



} // vk2d_internal
} // vk2d
