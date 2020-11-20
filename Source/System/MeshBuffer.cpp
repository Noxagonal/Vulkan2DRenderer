
#include "../Core/SourceCommon.h"

#include "../../Include/Types/MeshPrimitives.hpp"

#include "VulkanMemoryManagement.h"
#include "VulkanMemoryManagement.h"
#include "MeshBuffer.h"

#include "../Interface/WindowImpl.h"
#include "../Interface/InstanceImpl.h"



vk2d::_internal::MeshBuffer::MeshBuffer(
	vk2d::_internal::InstanceImpl	*	instance,
	VkDevice							device,
	const VkPhysicalDeviceLimits	&	physicald_device_limits,
	DeviceMemoryPool				*	device_memory_pool )
{
	assert( instance );
	assert( device );
	assert( device_memory_pool );

	this->instance				= instance;
	this->device						= device;
	this->physicald_device_limits		= physicald_device_limits;
	this->device_memory_pool			= device_memory_pool;

	first_draw							= true;
}

vk2d::_internal::MeshBuffer::~MeshBuffer()
{}

vk2d::_internal::MeshBuffer::PushResult vk2d::_internal::MeshBuffer::CmdPushMesh(
	VkCommandBuffer						command_buffer,
	const std::vector<uint32_t>		&	new_indices,
	const std::vector<vk2d::Vertex>	&	new_vertices,
	const std::vector<float>			new_texture_channels )
{
	auto reserve_result			= ReserveSpaceForMesh(
		uint32_t( new_indices.size() ),
		uint32_t( new_vertices.size() ),
		uint32_t( new_texture_channels.size() )
	);

	if( !reserve_result.success ) return {};

	if( bound_index_buffer_block != reserve_result.index_block ) {
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::BIND_INDEX_BUFFER
		);
		vkCmdBindIndexBuffer(
			command_buffer,
			reserve_result.index_block->device_buffer.buffer,
			0,
			VK_INDEX_TYPE_UINT32
		);
		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			instance->GetGraphicsPrimaryRenderPipelineLayout(),
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER,
			1, &reserve_result.index_block->descriptor_set.descriptorSet,
			0, nullptr
		);
		bound_index_buffer_block	= reserve_result.index_block;
	}
	if( bound_vertex_buffer_block != reserve_result.vertex_block ) {
		VkDeviceSize offset = 0;
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::BIND_VERTEX_BUFFER
		);
		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			instance->GetGraphicsPrimaryRenderPipelineLayout(),
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER,
			1, &reserve_result.vertex_block->descriptor_set.descriptorSet,
			0, nullptr
		);
		bound_vertex_buffer_block	= reserve_result.vertex_block;
	}
	if( bound_texture_channel_buffer_block != reserve_result.texture_channel_block ) {

		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::BIND_DESCRIPTOR_SET
		);
		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			instance->GetGraphicsPrimaryRenderPipelineLayout(),
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE_CHANNEL_WEIGHTS,
			1, &reserve_result.texture_channel_block->descriptor_set.descriptorSet,
			0, nullptr
		);
		bound_texture_channel_buffer_block	= reserve_result.texture_channel_block;
	}

	{
		auto & ib = reserve_result.index_block->host_data;
		auto & vb = reserve_result.vertex_block->host_data;
		auto & tb = reserve_result.texture_channel_block->host_data;

		if( new_indices.size() ) {
			ib.insert( ib.end(), new_indices.begin(), new_indices.end() );
		}
		if( new_vertices.size() ) {
			vb.insert( vb.end(), new_vertices.begin(), new_vertices.end() );
		}
		if( new_texture_channels.size() ) {
			tb.insert( tb.end(), new_texture_channels.begin(), new_texture_channels.end() );
		}
	}

	first_draw						= false;

	vk2d::_internal::MeshBuffer::PushResult ret {};
	ret.location_info				= reserve_result;
	ret.success						= true;

	pushed_mesh_count				+= 1;
	pushed_index_count				+= uint32_t( new_indices.size() );
	pushed_vertex_count				+= uint32_t( new_vertices.size() );
	pushed_texture_channel_count	+= uint32_t( new_texture_channels.size() );

	return ret;
}

bool vk2d::_internal::MeshBuffer::CmdUploadMeshDataToGPU(
	VkCommandBuffer			command_buffer
)
{
	// Index buffer
	for( auto & b : index_buffer_blocks ) {
		auto bb = b.get();
		if( bb->used_byte_size ) {
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

	// Vertex buffer
	for( auto & b : vertex_buffer_blocks ) {
		auto bb = b.get();
		if( bb->used_byte_size ) {
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

	// Texture channel buffer
	for( auto & b : texture_channel_buffer_blocks ) {
		auto bb = b.get();
		if( bb->used_byte_size ) {
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

	pushed_mesh_count					= 0;
	pushed_index_count					= 0;
	pushed_vertex_count					= 0;
	pushed_texture_channel_count		= 0;
	bound_index_buffer_block			= nullptr;
	bound_vertex_buffer_block			= nullptr;
	bound_texture_channel_buffer_block	= nullptr;
	first_draw							= true;

	return true;
}

uint32_t vk2d::_internal::MeshBuffer::GetPushedMeshCount()
{
	return pushed_mesh_count;
}

uint32_t vk2d::_internal::MeshBuffer::GetTotalVertexCount()
{
	return pushed_vertex_count;
}

uint32_t vk2d::_internal::MeshBuffer::GetTotalIndexCount()
{
	return pushed_index_count;
}

uint32_t vk2d::_internal::MeshBuffer::GetTotalTextureChannelCount()
{
	return pushed_texture_channel_count;
}

vk2d::_internal::MeshBuffer::MeshBlockLocationInfo vk2d::_internal::MeshBuffer::ReserveSpaceForMesh(
	uint32_t		index_count,
	uint32_t		vertex_count,
	uint32_t		texture_channel_count
)
{
	vk2d::_internal::MeshBufferBlock<uint32_t>		*	index_buffer_block				= nullptr;
	vk2d::_internal::MeshBufferBlock<vk2d::Vertex>	*	vertex_buffer_block				= nullptr;
	vk2d::_internal::MeshBufferBlock<float>			*	texture_channel_buffer_block	= nullptr;

	VkDeviceSize										index_buffer_position			= 0;
	VkDeviceSize										vertex_buffer_position			= 0;
	VkDeviceSize										texture_channel_buffer_position	= 0;

	{
		// Index buffer block
		index_buffer_block					= FindIndexBufferWithEnoughSpace( index_count );
		if( !index_buffer_block ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot reserve space for mesh in MeshBuffer, cannot find or create index MeshBufferBlock with enough free space!" );
			return {};
		}
		index_buffer_position				= index_buffer_block->ReserveSpace( index_count );

		// Vertex buffer block
		vertex_buffer_block					= FindVertexBufferWithEnoughSpace( vertex_count );
		if( !vertex_buffer_block ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot reserve space for mesh in MeshBuffer, cannot find or create vertex MeshBufferBlock with enough free space!" );
			return {};
		}
		vertex_buffer_position				= vertex_buffer_block->ReserveSpace( vertex_count );

		// Texture channel buffer block
		texture_channel_buffer_block		= FindTextureChannelBufferWithEnoughSpace( texture_channel_count );
		if( !texture_channel_buffer_block ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot reserve space for mesh in MeshBuffer, cannot find or create texture channel MeshBufferBlock with enough free space!" );
			return {};
		}
		texture_channel_buffer_position		= texture_channel_buffer_block->ReserveSpace( texture_channel_count );
	}

	vk2d::_internal::MeshBuffer::MeshBlockLocationInfo location_info {};
	location_info.index_block					= index_buffer_block;
	location_info.vertex_block					= vertex_buffer_block;
	location_info.texture_channel_block			= texture_channel_buffer_block;

	location_info.index_size					= index_count;
	location_info.index_byte_size				= index_count * sizeof( index_buffer_block->host_data.front() );
	location_info.index_offset					= uint32_t( index_buffer_position / sizeof( index_buffer_block->host_data.front() ) );
	location_info.index_byte_offset				= index_buffer_position;

	location_info.vertex_size					= vertex_count;
	location_info.vertex_byte_size				= vertex_count * sizeof( vertex_buffer_block->host_data.front() );
	location_info.vertex_offset					= uint32_t( vertex_buffer_position / sizeof( vertex_buffer_block->host_data.front() ) );
	location_info.vertex_byte_offset			= vertex_buffer_position;

	location_info.texture_channel_size			= texture_channel_count;
	location_info.texture_channel_byte_size		= texture_channel_count * sizeof( texture_channel_buffer_block->host_data.front() );
	location_info.texture_channel_offset		= uint32_t( texture_channel_buffer_position / sizeof( texture_channel_buffer_block->host_data.front() ) );
	location_info.texture_channel_byte_offset	= texture_channel_buffer_position;

	location_info.success						= true;

	return location_info;
}

vk2d::_internal::MeshBufferBlock<uint32_t>* vk2d::_internal::MeshBuffer::FindIndexBufferWithEnoughSpace(
	uint32_t count
)
{
	for( auto & i : index_buffer_blocks ) {
		if( i->CheckDataFits( count ) ) {
			return i.get();
		}
	}
	// Not found in existing blocks, create new
	{
		auto new_block = AllocateIndexBufferBlockAndStore(
			std::max(
				VkDeviceSize( count ) * sizeof( index_buffer_blocks.front()->host_data.front() ),
				VkDeviceSize( VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE )
			)
		);

		if( new_block && new_block->IsGood() ) {
			assert( new_block->CheckDataFits( count ) );
			return new_block;
		} else {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create new index MeshBufferBlock!" );
			return nullptr;
		}
	}
	return nullptr;
}

vk2d::_internal::MeshBufferBlock<vk2d::Vertex>* vk2d::_internal::MeshBuffer::FindVertexBufferWithEnoughSpace(
	uint32_t count
)
{
	for( auto & i : vertex_buffer_blocks ) {
		if( i->CheckDataFits( count ) ) {
			return i.get();
		}
	}
	// Not found in existing blocks, create new
	{
		auto new_block = AllocateVertexBufferBlockAndStore(
			std::max(
				VkDeviceSize( count ) * sizeof( vertex_buffer_blocks.front()->host_data.front() ),
				VkDeviceSize( VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE )
			)
		);

		if( new_block && new_block->IsGood() ) {
			assert( new_block->CheckDataFits( count ) );
			return new_block;
		} else {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create new vertex MeshBufferBlock!" );
			return nullptr;
		}
	}
	return nullptr;
}

vk2d::_internal::MeshBufferBlock<float>* vk2d::_internal::MeshBuffer::FindTextureChannelBufferWithEnoughSpace(
	uint32_t count
)
{
	for( auto & i : texture_channel_buffer_blocks ) {
		if( i->CheckDataFits( count ) ) {
			return i.get();
		}
	}
	// Not found in existing blocks, create new
	{
		auto new_block = AllocateTextureChannelBufferBlockAndStore(
			std::max(
				VkDeviceSize( count ) * sizeof( texture_channel_buffer_blocks.front()->host_data.front() ),
				VkDeviceSize( VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_TEXTURE_CHANNEL_SIZE )
			)
		);

		if( new_block && new_block->IsGood() ) {
			assert( new_block->CheckDataFits( count ) );
			return new_block;
		} else {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create new texture channel MeshBufferBlock!" );
			return nullptr;
		}
	}
	return nullptr;
}

vk2d::_internal::MeshBufferBlock<uint32_t>* vk2d::_internal::MeshBuffer::AllocateIndexBufferBlockAndStore(
	VkDeviceSize byte_size
)
{
	auto buffer_block	= std::make_unique<vk2d::_internal::MeshBufferBlock<uint32_t>>(
		this,
		byte_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		vk2d::_internal::MeshBufferDescriptorSetType::STORAGE
		);
	if( buffer_block && buffer_block->IsGood() ) {
		auto ret		= buffer_block.get();
		index_buffer_blocks.push_back( std::move( buffer_block ) );
		return ret;
	} else {
		return nullptr;
	}
}

vk2d::_internal::MeshBufferBlock<vk2d::Vertex>* vk2d::_internal::MeshBuffer::AllocateVertexBufferBlockAndStore(
	VkDeviceSize byte_size
)
{
	auto buffer_block	= std::make_unique<vk2d::_internal::MeshBufferBlock<vk2d::Vertex>>(
		this,
		byte_size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		vk2d::_internal::MeshBufferDescriptorSetType::STORAGE
		);
	if( buffer_block && buffer_block->IsGood() ) {
		auto ret		= buffer_block.get();
		vertex_buffer_blocks.push_back( std::move( buffer_block ) );
		return ret;
	} else {
		return nullptr;
	}
}

vk2d::_internal::MeshBufferBlock<float>* vk2d::_internal::MeshBuffer::AllocateTextureChannelBufferBlockAndStore(
	VkDeviceSize byte_size
)
{
	auto buffer_block	= std::make_unique<vk2d::_internal::MeshBufferBlock<float>>(
		this,
		byte_size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		vk2d::_internal::MeshBufferDescriptorSetType::STORAGE
		);
	if( buffer_block && buffer_block->IsGood() ) {
		auto ret		= buffer_block.get();
		texture_channel_buffer_blocks.push_back( std::move( buffer_block ) );
		return ret;
	} else {
		return nullptr;
	}
}

void vk2d::_internal::MeshBuffer::FreeBufferBlockFromStorage(
	vk2d::_internal::MeshBufferBlock<uint32_t>		*	buffer_block
)
{
	if( index_buffer_blocks.size() ) {
		auto it = index_buffer_blocks.begin();
		while( it != index_buffer_blocks.end() ) {
			if( it->get() == buffer_block ) {
				index_buffer_blocks.erase( it );
				return;
			}
			++it;
		}
	}
}

void vk2d::_internal::MeshBuffer::FreeBufferBlockFromStorage(
	vk2d::_internal::MeshBufferBlock<vk2d::Vertex>	*	buffer_block
)
{
	if( vertex_buffer_blocks.size() ) {
		auto it = vertex_buffer_blocks.begin();
		while( it != vertex_buffer_blocks.end() ) {
			if( it->get() == buffer_block ) {
				vertex_buffer_blocks.erase( it );
				return;
			}
			++it;
		}
	}
}

void vk2d::_internal::MeshBuffer::FreeBufferBlockFromStorage(
	vk2d::_internal::MeshBufferBlock<float>			*	buffer_block 
)
{
	if( texture_channel_buffer_blocks.size() ) {
		auto it = texture_channel_buffer_blocks.begin();
		while( it != texture_channel_buffer_blocks.end() ) {
			if( it->get() == buffer_block ) {
				texture_channel_buffer_blocks.erase( it );
				return;
			}
			++it;
		}
	}
}
