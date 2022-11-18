
#include <core/SourceCommon.h>

#include <mesh/MeshPrimitives.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>
#include <system/MeshBuffer.h>

#include <interface/WindowImpl.h>
#include <interface/InstanceImpl.h>



vk2d::vk2d_internal::MeshBuffer::MeshBuffer(
	InstanceImpl					&	instance
) :
	instance( instance ),
	index_buffer_blocks( *this ),
	vertex_buffer_blocks( *this ),
	texture_channel_weight_buffer_blocks( *this ),
	transformation_buffer_blocks( *this )
{
	this->first_draw = true;
}

vk2d::vk2d_internal::MeshBuffer::PushResult vk2d::vk2d_internal::MeshBuffer::CmdPushMesh(
	VkCommandBuffer							command_buffer,
	const std::vector<uint32_t>			&	new_indices,
	const RawVertexData					&	new_vertices,
	const std::vector<float>			&	new_texture_channel_weights,
	const std::vector<glm::mat4>		&	new_transformations
)
{
	// TODO: Could save some memory when calling CmdPushMesh with empty new_transformations, could just point to an identity matrix stored on the first index.
	// Whenever new_transformations is empty, just submit the render once and have the transformation point to the first index.

	std::vector<glm::mat4>					default_transformation		= { glm::mat4( 1.0f ) };
	const std::vector<glm::mat4>		*	new_transformations_actual	= &default_transformation;
	if( new_transformations.size() )		new_transformations_actual	= &new_transformations;

	auto reserve_result = ReserveSpaceForMesh(
		uint32_t( new_indices.size() ),
		new_vertices,
		uint32_t( new_texture_channel_weights.size() ),
		uint32_t( new_transformations_actual->size() )
	);

	if( !reserve_result.success ) return {};

	auto BindBuffer =[ this, command_buffer ](
		auto									&	location_info,
		MeshBufferBlock<typename std::remove_reference_t<decltype( location_info )>::Type>
												*	currently_bound_block,
		uint32_t									bind_set,
		CommandBufferCheckpointType					checkpoint_type,
		bool										is_index_buffer				= false
	) -> decltype( currently_bound_block )
	{
		if( currently_bound_block != location_info.block )
		{
			CmdInsertCommandBufferCheckpoint(
				command_buffer,
				"MeshBuffer",
				checkpoint_type
			);
			if( is_index_buffer )
			{
				vkCmdBindIndexBuffer(
					command_buffer,
					location_info.block->device_buffer.buffer,
					0,
					VK_INDEX_TYPE_UINT32
				);
			}
			vkCmdBindDescriptorSets(
				command_buffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				instance.GetGraphicsPrimaryRenderPipelineLayout(),
				bind_set,
				1, &location_info.block->descriptor_set.descriptorSet,
				0, nullptr
			);
			currently_bound_block = location_info.block;
		}
		return currently_bound_block;
	};

	bound_index_buffer_block = BindBuffer(
		reserve_result.index_block,
		bound_index_buffer_block,
		GRAPHICS_DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER,
		CommandBufferCheckpointType::BIND_INDEX_BUFFER,
		true
	);
	
	bound_vertex_buffer_block = BindBuffer(
		reserve_result.vertex_block,
		bound_vertex_buffer_block,
		GRAPHICS_DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER,
		CommandBufferCheckpointType::BIND_VERTEX_BUFFER,
		true
	);
	
	bound_texture_channel_weight_buffer_block = BindBuffer(
		reserve_result.texture_channel_weight_block,
		bound_texture_channel_weight_buffer_block,
		GRAPHICS_DESCRIPTOR_SET_ALLOCATION_texture_channel_weights,
		CommandBufferCheckpointType::BIND_DESCRIPTOR_SET,
		true
	);
	
	bound_transformation_buffer_block = BindBuffer(
		reserve_result.transformation_block,
		bound_transformation_buffer_block,
		GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TRANSFORMATION,
		CommandBufferCheckpointType::BIND_DESCRIPTOR_SET,
		true
	);

	{
		auto & index_block_data						= reserve_result.index_block.block->host_data;
		auto & vertex_block_data					= reserve_result.vertex_block.block->host_data;
		auto & texture_channel_weight_block_data	= reserve_result.texture_channel_weight_block.block->host_data;
		auto & transformation_block_data			= reserve_result.transformation_block.block->host_data;

		if( new_indices.size() ) {
			index_block_data.insert( index_block_data.end(), new_indices.begin(), new_indices.end() );
		}
		if( new_vertices.vertex_data.size() ) {
			vertex_block_data.insert( vertex_block_data.end(), new_vertices.vertex_data.begin(), new_vertices.vertex_data.end() );
		}
		if( new_texture_channel_weights.size() ) {
			texture_channel_weight_block_data.insert( texture_channel_weight_block_data.end(), new_texture_channel_weights.begin(), new_texture_channel_weights.end() );
		}
		if( new_transformations_actual->size() ) {
			transformation_block_data.insert( transformation_block_data.end(), new_transformations_actual->begin(), new_transformations_actual->end() );
		}
	}

	first_draw							= false;

	MeshBuffer::PushResult ret {};
	ret.location_info					= reserve_result;
	ret.success							= true;

	pushed_mesh_count					+= 1;
	pushed_index_count					+= uint32_t( new_indices.size() );
	pushed_vertex_count					+= uint32_t( new_vertices.vertex_count );
	pushed_texture_channel_weight_count	+= uint32_t( new_texture_channel_weights.size() );
	pushed_transformation_count			+= uint32_t( new_transformations_actual->size() );

	return ret;
}

bool vk2d::vk2d_internal::MeshBuffer::CmdUploadMeshDataToGPU(
	VkCommandBuffer command_buffer
)
{
	auto CmdUploadBuffer = [command_buffer](
		auto & from_list
	)
	{
		for( auto & b : from_list.blocks )
		{
			auto bb = b.get();
			if( bb->used_byte_size )
			{
				bb->CopyVectorsToStagingBuffers();

				std::array<VkBufferCopy, 1> copy_regions {};
				copy_regions[ 0 ].srcOffset		= 0;
				copy_regions[ 0 ].dstOffset		= 0;
				copy_regions[ 0 ].size			= bb->used_byte_size;
				vkCmdCopyBuffer(
					command_buffer,
					bb->staging_buffer.buffer,
					bb->device_buffer.buffer,
					uint32_t( copy_regions.size() ),
					copy_regions.data()
				);
				bb->used_byte_size				= 0;
			}
		}
	};

	CmdUploadBuffer( index_buffer_blocks );
	CmdUploadBuffer( vertex_buffer_blocks );
	CmdUploadBuffer( texture_channel_weight_buffer_blocks );
	CmdUploadBuffer( transformation_buffer_blocks );

	pushed_mesh_count							= 0;
	pushed_index_count							= 0;
	pushed_vertex_count							= 0;
	pushed_texture_channel_weight_count			= 0;
	pushed_transformation_count					= 0;
	bound_index_buffer_block					= nullptr;
	bound_vertex_buffer_block					= nullptr;
	bound_texture_channel_weight_buffer_block	= nullptr;
	bound_transformation_buffer_block			= nullptr;
	first_draw									= true;

	return true;
}

uint32_t vk2d::vk2d_internal::MeshBuffer::GetPushedMeshCount()
{
	return pushed_mesh_count;
}

uint32_t vk2d::vk2d_internal::MeshBuffer::GetTotalVertexCount()
{
	return pushed_vertex_count;
}

uint32_t vk2d::vk2d_internal::MeshBuffer::GetTotalIndexCount()
{
	return pushed_index_count;
}

uint32_t vk2d::vk2d_internal::MeshBuffer::GetTotalTextureChannelCount()
{
	return pushed_texture_channel_weight_count;
}

uint32_t vk2d::vk2d_internal::MeshBuffer::GetTotalTransformationCount()
{
	return pushed_transformation_count;
}

vk2d::vk2d_internal::MeshBlockLocationInfo vk2d::vk2d_internal::MeshBuffer::ReserveSpaceForMesh(
	uint32_t				index_count,
	const RawVertexData	&	vertices,
	uint32_t				texture_channel_weight_count,
	uint32_t				transformation_count
)
{
	auto ReserveSpaceFromBufferBlock =[ this ](
		auto						&	from_list,
		VkDeviceSize					byte_size,
		VkDeviceSize					allocation_step_size,
		VkBufferUsageFlags				usage_flags					= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		MeshBufferDescriptorSetType		descriptor_set_type			= MeshBufferDescriptorSetType::STORAGE
	)->MeshBufferBlockLocationInfo<typename std::remove_reference_t<decltype( from_list )>::Type>
	{
		auto buffer_block = from_list.FindMeshBufferWithEnoughSpace(
			byte_size,
			allocation_step_size,
			usage_flags,
			descriptor_set_type
		);
		if( !buffer_block )
		{
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot reserve space for mesh in MeshBuffer, cannot find or create index MeshBufferBlock with enough free space!" );
			return {};
		}

		auto byte_offset = buffer_block->ReserveSpace( byte_size );

		auto ret = MeshBufferBlockLocationInfo<typename std::remove_reference_t<decltype( from_list )>::Type>();
		ret.block		= buffer_block;
		ret.byte_size	= byte_size;
		ret.byte_offset	= byte_offset;
		return ret;
	};

	MeshBlockLocationInfo location_info {};

	location_info.index_block = ReserveSpaceFromBufferBlock(
		index_buffer_blocks,
		index_count * sizeof( decltype( index_buffer_blocks )::Type ),
		VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		MeshBufferDescriptorSetType::STORAGE
	);
	
	location_info.vertex_block = ReserveSpaceFromBufferBlock(
		vertex_buffer_blocks,
		vertices.vertex_data.size(),
		VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE,
		/* VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | ...VERTEX BUFFER NOT USED DIRECTLY... */ VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		MeshBufferDescriptorSetType::STORAGE
	);
	
	location_info.texture_channel_weight_block = ReserveSpaceFromBufferBlock(
		texture_channel_weight_buffer_blocks,
		texture_channel_weight_count * sizeof( decltype( texture_channel_weight_buffer_blocks )::Type ),
		VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_texture_channel_weight_SIZE,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		MeshBufferDescriptorSetType::STORAGE
	);
	
	location_info.transformation_block = ReserveSpaceFromBufferBlock(
		transformation_buffer_blocks,
		transformation_count * sizeof( decltype(transformation_buffer_blocks)::Type ),
		VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_TRANSFORMATION_SIZE,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		MeshBufferDescriptorSetType::STORAGE
	);

	location_info.success = true;
	return location_info;
}
