
#include "../Header/Core/SourceCommon.h"
#include "../Header/Core/MeshBuffer.h"

#include "../../Include/Interface/RenderPrimitives.h"
#include "../Header/Core/VulkanMemoryManagement.h"
#include "../Header/Impl/WindowImpl.h"

#include "../Header/Core/VulkanMemoryManagement.h"

#include <algorithm>



vk2d::_internal::MeshBuffer::MeshBuffer(
	VkDevice							device,
	const VkPhysicalDeviceLimits	&	physicald_device_limits,
	vk2d::_internal::WindowImpl		*	window_data,
	DeviceMemoryPool				*	device_memory_pool )
{
	this->device						= device;
	this->physicald_device_limits		= physicald_device_limits;
	this->window_data					= window_data;
	this->device_memory_pool			= device_memory_pool;

	first_draw					= true;
}

vk2d::_internal::MeshBuffer::~MeshBuffer()
{}

vk2d::_internal::MeshBuffer::PushResult vk2d::_internal::MeshBuffer::CmdPushMesh(
	VkCommandBuffer						command_buffer,
	const std::vector<Vertex>		&	new_vertices,
	const std::vector<uint32_t>		&	new_indices )
{
	auto reserve_result = ReserveSpaceForMesh(
		uint32_t( new_vertices.size() ),
		uint32_t( new_indices.size() )
	);

	if( !reserve_result.buffer_block ) return {};

	if( reserve_result.buffer_block_need_binding ) {
		VkDeviceSize buffer_offset = reserve_result.buffer_block->aligned_vertex_buffer_byte_offset;
		vkCmdBindVertexBuffers(
			command_buffer,
			0,
			1, &reserve_result.buffer_block->device_buffer,
			&buffer_offset
		);
		vkCmdBindIndexBuffer(
			command_buffer,
			reserve_result.buffer_block->device_buffer,
			reserve_result.buffer_block->aligned_index_buffer_byte_offset,
			VK_INDEX_TYPE_UINT32
		);
	}

	{
		auto & bv = reserve_result.buffer_block->vertices;
		auto & bi = reserve_result.buffer_block->indices;

		bv.insert( bv.end(), new_vertices.begin(), new_vertices.end() );
		bi.insert( bi.end(), new_indices.begin(), new_indices.end() );
	}

	first_draw	= false;

	vk2d::_internal::MeshBuffer::PushResult ret {};
	ret.offsets = reserve_result.offsets;
	ret.success = true;
	return ret;
}

bool vk2d::_internal::MeshBuffer::CmdUploadMeshDataToGPU(
	VkCommandBuffer			command_buffer
)
{
	for( auto & b : buffer_blocks ) {
		auto bp = b.get();

		std::array<VkBufferCopy, 2> regions {};
		regions[ 0 ].srcOffset = bp->aligned_vertex_buffer_byte_offset;
		regions[ 0 ].dstOffset = bp->aligned_vertex_buffer_byte_offset;
		regions[ 0 ].size = bp->used_aligned_vertex_byte_size;

		regions[ 1 ].srcOffset = bp->aligned_index_buffer_byte_offset;
		regions[ 1 ].dstOffset = bp->aligned_index_buffer_byte_offset;
		regions[ 1 ].size = bp->used_aligned_index_byte_size;

		bp->CopyVectorsToStagingBuffers();

		vkCmdCopyBuffer(
			command_buffer,
			bp->staging_buffer,
			bp->device_buffer,
			uint32_t( regions.size() ),
			regions.data()
		);
	}

	current_buffer_block	= buffer_blocks.begin();
	first_draw				= true;

	return true;
}

vk2d::_internal::MeshBuffer::BufferBlock * vk2d::_internal::MeshBuffer::AllocateBufferBlockAndStore(
	VkDeviceSize								vertex_portion_byte_size,
	VkDeviceSize								index_portion_byte_size
)
{
	auto block = std::make_unique<vk2d::_internal::MeshBuffer::BufferBlock>(
		this,
		vertex_portion_byte_size,
		index_portion_byte_size );
	if( block && block->is_good ) {
		auto ret = block.get();
		buffer_blocks.push_back( std::move( block ) );
		return ret;
	}

	return {};
}

void vk2d::_internal::MeshBuffer::FreeBufferBlockFromStorage(
	vk2d::_internal::MeshBuffer::BufferBlock	*	buffer_block
)
{
	auto it = buffer_blocks.begin();
	while( it != buffer_blocks.end() ) {
		if( it->get() == buffer_block ) {
			it = buffer_blocks.erase( it );
			return;
		}
		++it;
	}
}

vk2d::_internal::MeshBuffer::ReserveSpaceResult vk2d::_internal::MeshBuffer::ReserveSpaceForMesh(
	uint32_t				vertex_count,
	uint32_t				index_count
)
{
	VkDeviceSize vertex_byte_size			= VkDeviceSize( vertex_count ) * sizeof( Vertex );
	VkDeviceSize index_byte_size			= VkDeviceSize( index_count ) * sizeof( uint32_t );

	bool buffer_blocks_list_empty_initially = buffer_blocks.empty();
	if( !buffer_blocks_list_empty_initially ) {
		// Find from existing space, start from last location, advance to the next buffer block if needed
		while( current_buffer_block != buffer_blocks.end() ) {
			auto bp = current_buffer_block->get();
			VkDeviceSize buffer_vertex_size_left	= bp->total_aligned_vertex_byte_size - bp->used_aligned_vertex_byte_size;
			VkDeviceSize buffer_index_size_left		= bp->total_aligned_index_byte_size - bp->used_aligned_index_byte_size;
			if( vertex_byte_size <= buffer_vertex_size_left &&
				index_byte_size <= buffer_index_size_left ) {
				vk2d::_internal::MeshBuffer::ReserveSpaceResult		ret {};
				ret.buffer_block					= bp;
				ret.buffer_block_need_binding		= first_draw;
				ret.offsets.first_index				= bp->block_index_count;
				ret.offsets.vertex_offset			= bp->block_vertex_count;
				ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
				ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
				bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
				bp->used_aligned_index_byte_size	+= index_byte_size;
				bp->block_vertex_count				+= vertex_count;
				bp->block_index_count				+= index_count;
				return ret;
			} else {
				if( bp->used_aligned_vertex_byte_size == 0 ) {
					this->window_data->SynchronizeFrame();
					current_buffer_block = buffer_blocks.erase( current_buffer_block );
				} else {
					++current_buffer_block;
				}
			}
		}
	}

	// Not enough space, allocate more
	{
		auto bp		= AllocateBufferBlockAndStore(
			std::max( vertex_byte_size, VkDeviceSize( VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE ) ),
			std::max( index_byte_size, VkDeviceSize( VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE ) )
		);
		if( bp ) {
			vk2d::_internal::MeshBuffer::ReserveSpaceResult		ret {};
			ret.buffer_block					= bp;
			ret.buffer_block_need_binding		= true;
			ret.offsets.first_index				= bp->block_index_count;
			ret.offsets.vertex_offset			= bp->block_vertex_count;
			ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
			ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
			bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
			bp->used_aligned_index_byte_size	+= index_byte_size;
			bp->block_vertex_count				+= vertex_count;
			bp->block_index_count				+= index_count;
			if( buffer_blocks_list_empty_initially ) {
				current_buffer_block = buffer_blocks.begin();
			} else {
				current_buffer_block = buffer_blocks.end() - 1;
			}
			return ret;
		}
	}

	return {};
}









vk2d::_internal::MeshBuffer::BufferBlock::BufferBlock(
	vk2d::_internal::MeshBuffer	*	mesh_buffer_parent,
	VkDeviceSize					buffer_vertex_byte_size,
	VkDeviceSize					buffer_index_byte_size
)
{
	parent			= mesh_buffer_parent;

	total_aligned_vertex_byte_size				= CalculateAlignmentForBuffer( buffer_vertex_byte_size, parent->physicald_device_limits );
	total_aligned_index_byte_size				= CalculateAlignmentForBuffer( buffer_index_byte_size, parent->physicald_device_limits );
	total_aligned_buffer_byte_size				= total_aligned_vertex_byte_size + total_aligned_index_byte_size;

	vertices.reserve( total_aligned_vertex_byte_size / sizeof( Vertex ) );
	indices.reserve( total_aligned_index_byte_size / sizeof( uint32_t ) );

	aligned_vertex_buffer_byte_offset			= 0;
	aligned_index_buffer_byte_offset			= total_aligned_vertex_byte_size;

	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext					= nullptr;
	buffer_create_info.flags					= 0;
	buffer_create_info.size						= total_aligned_buffer_byte_size;
	buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount	= 0;
	buffer_create_info.pQueueFamilyIndices		= nullptr;

	// Staging buffer.
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if( vkCreateBuffer(
		parent->device,
		&buffer_create_info,
		nullptr,
		&staging_buffer
	) != VK_SUCCESS ) return;

	staging_buffer_memory = parent->device_memory_pool->AllocateAndBindBufferMemory(
		staging_buffer,
		&buffer_create_info,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);

	// Device buffer.
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if( vkCreateBuffer(
		parent->device,
		&buffer_create_info,
		nullptr,
		&device_buffer
	) != VK_SUCCESS ) return;

	device_buffer_memory = parent->device_memory_pool->AllocateAndBindBufferMemory(
		device_buffer,
		&buffer_create_info,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	is_good			= true;
}

vk2d::_internal::MeshBuffer::BufferBlock::~BufferBlock()
{
	vkDestroyBuffer( parent->device, staging_buffer, nullptr );
	vkDestroyBuffer( parent->device, device_buffer, nullptr );

	parent->device_memory_pool->FreeMemory( staging_buffer_memory );
	parent->device_memory_pool->FreeMemory( device_buffer_memory );
}

bool vk2d::_internal::MeshBuffer::BufferBlock::CopyVectorsToStagingBuffers()
{
	void * original_mapped_memory = staging_buffer_memory.Map<void*>();
	if( !original_mapped_memory ) return false;

	void * vertex_mapped_memory	= (uint8_t*)original_mapped_memory + aligned_vertex_buffer_byte_offset;
	void * index_mapped_memory	= (uint8_t*)original_mapped_memory + aligned_index_buffer_byte_offset;
	{
		// Copy over the vertex data to staging buffer
		std::memcpy( vertex_mapped_memory, vertices.data(), vertices.size() * sizeof( Vertex ) );

		// Clear inbetween bits, just in case
//		VkDeviceSize vertex_index_buffer_gap	=  total_aligned_vertex_byte_size - VkDeviceSize( vertices.size() );
//		if( vertex_index_buffer_gap ) {
//			std::memset( (uint8_t*)vertex_mapped_memory + VkDeviceSize( vertices.size() ), 0, vertex_index_buffer_gap );
//		}
	}
	{
		// Copy over the index data to staging buffer
		std::memcpy( index_mapped_memory, indices.data(), indices.size() * sizeof( uint32_t ) );

		// Clear tail of the buffer, just in case
//		VkDeviceSize buffer_tail_size	= total_aligned_index_byte_size - VkDeviceSize( indices.size() );
//		if( buffer_tail_size ) {
//			std::memset( (uint8_t*)index_mapped_memory + VkDeviceSize( indices.size() ), 0, buffer_tail_size );
//		}
	}

	staging_buffer_memory.Unmap();

	vertices.clear();
	indices.clear();

	block_vertex_count				= 0;
	block_index_count				= 0;

	used_aligned_vertex_byte_size	= 0;
	used_aligned_index_byte_size	= 0;

	return true;
}
