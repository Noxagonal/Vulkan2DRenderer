
#include "../Header/SourceCommon.h"
#include "../Header/MeshBuffer.h"

#include "../../Include/VK2D/RenderPrimitives.h"
#include "../Header/VulkanMemoryManagement.h"
#include "../Header/WindowImpl.h"

#include "../Header/VulkanMemoryManagement.h"



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
}

MeshBuffer::~MeshBuffer()
{
}

MeshBuffer::BufferBlock * MeshBuffer::AllocateBufferBlockAndStore()
{
	auto block = std::make_unique<MeshBuffer::BufferBlock>( this, sizeof( vk2d::Vertex ) * 5000, sizeof( uint32_t ) * 5000 );
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
	// Find from existing space
	// TODO: TESTING ONLY: instead of parsing through all buffer blocks from the beginning we should
	// keep track of where the last mesh was recorded to and only check from there worwards.
	for( auto & b : buffer_blocks ) {
		auto bp = b.get();
		VkDeviceSize buffer_vertex_size_left	= bp->total_aligned_vertex_byte_size - bp->used_aligned_vertex_byte_size;
		VkDeviceSize buffer_index_size_left		= bp->total_aligned_index_byte_size - bp->used_aligned_index_byte_size;
		if( buffer_vertex_size_left <= vertex_byte_size &&
			buffer_index_size_left <= index_byte_size ) {
			MeshBuffer::ReserveSpaceResult		ret;
			ret.buffer_block					= bp;
			ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
			ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
			bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
			bp->used_aligned_index_byte_size	+= index_byte_size;
			return ret;
		}
	}

	// Not enough space, allocate more
	{
		auto bp		= AllocateBufferBlockAndStore();
		if( bp ) {
			MeshBuffer::ReserveSpaceResult		ret;
			ret.buffer_block					= bp;
			ret.offsets.vertex_byte_offset		= bp->used_aligned_vertex_byte_size;
			ret.offsets.index_byte_offset		= bp->used_aligned_index_byte_size;
			bp->used_aligned_vertex_byte_size	+= vertex_byte_size;
			bp->used_aligned_index_byte_size	+= index_byte_size;
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
	auto total_aligned_buffer_byte_size			= total_aligned_vertex_byte_size + total_aligned_index_byte_size;

	vertices.reserve( total_aligned_vertex_byte_size / sizeof( Vertex ) );
	indices.reserve( total_aligned_index_byte_size / sizeof( uint32_t ) );

	aligned_vertex_buffer_byte_offset			= 0;
	aligned_index_buffer_byte_offset			= total_aligned_buffer_byte_size;

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
		&staging_buffer
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
		std::memcpy( vertex_mapped_memory, vertices.data(), used_aligned_vertex_byte_size );

		// Clear inbetween bits, just in case
		VkDeviceSize vertex_index_buffer_gap	=  total_aligned_vertex_byte_size - VkDeviceSize( vertices.size() );
		if( vertex_index_buffer_gap ) {
			std::memset( (uint8_t*)vertex_mapped_memory + VkDeviceSize( vertices.size() ), 0, vertex_index_buffer_gap );
		}
	}
	{
		// Copy over the index data to staging buffer
		std::memcpy( index_mapped_memory, indices.data(), used_aligned_index_byte_size );

		// Clear tail of the buffer, just in case
		VkDeviceSize buffer_tail_size	= total_aligned_index_byte_size - VkDeviceSize( indices.size() );
		if( buffer_tail_size ) {
			std::memset( (uint8_t*)index_mapped_memory + VkDeviceSize( indices.size() ), 0, buffer_tail_size );
		}
	}

	parent->device_memory_pool->UnmapMemory( staging_buffer_memory );
}



} // vk2d
