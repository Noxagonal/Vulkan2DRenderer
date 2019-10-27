
#include "../Header/SourceCommon.h"
#include "../Header/MeshBuffer.h"

#include "../../Include/VK2D/RenderPrimitives.h"
#include "../Header/VulkanMemoryManagement.h"
#include "../Header/WindowImpl.h"

#include "../Header/VulkanMemoryManagement.h"

#include <algorithm>



namespace vk2d {



MeshBuffer::MeshBuffer(
	VkDevice							device,
	const VkPhysicalDeviceLimits	&	physicald_device_limits,
	_internal::WindowImpl			*	window_data,
	DeviceMemoryPool				*	device_memory_pool )
{
	this->device						= device;
	this->physicald_device_limits		= physicald_device_limits;
	this->window_data					= window_data;
	this->device_memory_pool			= device_memory_pool;

	first_draw					= true;
}

MeshBuffer::~MeshBuffer()
{
}

MeshBuffer::PushResult MeshBuffer::CmdPushMesh(
	VkCommandBuffer						command_buffer,
	const std::vector<Vertex>		&	new_vertices,
	const std::vector<uint32_t>		&	new_indices )
{
	auto reserve_result = ReserveSpaceForMesh(
		new_vertices.size() * sizeof( Vertex ),
		new_indices.size() * sizeof( uint32_t )
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

	MeshBuffer::PushResult ret {};
	ret.offsets = reserve_result.offsets;
	ret.success = true;
	return ret;
}

bool MeshBuffer::CmdUploadMeshDataToGPU(
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

		vkCmdCopyBuffer(
			command_buffer,
			bp->staging_buffer,
			bp->device_buffer,
			uint32_t( regions.size() ),
			regions.data()
			);

		bp->CopyVectorsToStagingBuffers();
	}

	current_buffer_block	= buffer_blocks.begin();
	first_draw				= true;

	return true;
}

MeshBuffer::BufferBlock * MeshBuffer::AllocateBufferBlockAndStore(
	VkDeviceSize								vertex_portion_byte_size,
	VkDeviceSize								index_portion_byte_size
)
{
	// TODO: make sure buffers are reserved with more memory if mesh exceeds the size of the buffer,
	// TODO: This may introduce gaps in the buffer blocks as the last mesh should be last on the list,
	// consider a trimming function to remove the possible empty buffer blocks in the middle.
	auto block = std::make_unique<MeshBuffer::BufferBlock>(
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

void MeshBuffer::FreeBufferBlockFromStorage(
	BufferBlock			*	buffer_block
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

MeshBuffer::ReserveSpaceResult MeshBuffer::ReserveSpaceForMesh(
	VkDeviceSize			vertex_byte_size,
	VkDeviceSize			index_byte_size )
{
	bool buffer_blocks_list_empty_initially = buffer_blocks.empty();
	if( !buffer_blocks_list_empty_initially ) {
		// Find from existing space, start from last location, advance to the next buffer block if needed
		while( current_buffer_block != buffer_blocks.end() ) {
			auto bp = current_buffer_block->get();
			VkDeviceSize buffer_vertex_size_left	= bp->total_aligned_vertex_byte_size - bp->used_aligned_vertex_byte_size;
			VkDeviceSize buffer_index_size_left		= bp->total_aligned_index_byte_size - bp->used_aligned_index_byte_size;
			if( vertex_byte_size <= buffer_vertex_size_left &&
				index_byte_size <= buffer_index_size_left ) {
				MeshBuffer::ReserveSpaceResult		ret {};
				ret.buffer_block					= bp;
				ret.buffer_block_need_binding		= first_draw;
				ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
				ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
				bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
				bp->used_aligned_index_byte_size	+= index_byte_size;
				return ret;
			} else {
				if( bp->used_aligned_vertex_byte_size == 0 ) {
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
			MeshBuffer::ReserveSpaceResult		ret {};
			ret.buffer_block					= bp;
			ret.buffer_block_need_binding		= true;
			ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
			ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
			bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
			bp->used_aligned_index_byte_size	+= index_byte_size;
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









MeshBuffer::BufferBlock::BufferBlock(
	MeshBuffer				*	mesh_buffer_parent,
	VkDeviceSize				buffer_vertex_byte_size,
	VkDeviceSize				buffer_index_byte_size
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
	buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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
	buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
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

MeshBuffer::BufferBlock::~BufferBlock()
{
	vkDestroyBuffer( parent->device, staging_buffer, nullptr );
	vkDestroyBuffer( parent->device, device_buffer, nullptr );

	parent->device_memory_pool->FreeMemory( staging_buffer_memory );
	parent->device_memory_pool->FreeMemory( device_buffer_memory );
}

bool MeshBuffer::BufferBlock::CopyVectorsToStagingBuffers()
{
	void * original_mapped_memory = parent->device_memory_pool->MapMemory(
		staging_buffer_memory
	);
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

	parent->device_memory_pool->UnmapMemory( staging_buffer_memory );

	vertices.clear();
	indices.clear();

	used_aligned_vertex_byte_size	= 0;
	used_aligned_index_byte_size	= 0;

	return true;
}



} // vk2d
