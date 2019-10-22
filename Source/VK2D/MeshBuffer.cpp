
#include "../Header/SourceCommon.h"
#include "../Header/MeshBuffer.h"

#include "../../Include/VK2D/RenderPrimitives.h"
#include "../Header/VulkanMemoryManagement.h"
#include "../Header/WindowImpl.h"



namespace vk2d {



MeshBuffer::MeshBuffer(
	VkDevice							device,
	const VkPhysicalDeviceLimits	&	physicald_device_limits,
	_internal::WindowDataImpl		*	window_data,
	DeviceMemoryPool				*	device_memory_pool )
{
	this->device						= device;
	this->physicald_device_limits		= physicald_device_limits;
	this->window_data					= window_data;
	this->device_memory_pool			= device_memory_pool;
}









MeshBuffer::~MeshBuffer()
{
	vkDestroyBuffer(
		device,
		staging_buffer,
		nullptr
	);
	vkDestroyBuffer(
		device,
		device_buffer,
		nullptr
	);
	device_memory_pool->FreeMemory( staging_buffer_memory );
	device_memory_pool->FreeMemory( device_buffer_memory );
}









void MeshBuffer::PushBackVertex(
	Vertex								new_vertex
)
{
	vertices.push_back( new_vertex );
}

void MeshBuffer::PushBackVertices(
	const std::vector<Vertex>		&	new_vertices
)
{
	vertices.reserve( vertices.size() + new_vertices.size() );
	vertices.insert( vertices.end(), new_vertices.begin(), new_vertices.end() );
}

void MeshBuffer::PushBackIndex(
	uint32_t							new_index
)
{
	indices.push_back( new_index );
}

void MeshBuffer::PushBackIndices(
	const std::vector<uint32_t>		&	new_indices
)
{
	indices.reserve( indices.size() + new_indices.size() );
	indices.insert( indices.end(), new_indices.begin(), new_indices.end() );
}










bool									MeshBuffer::CmdUploadToGPU(
	VkCommandBuffer						command_buffer
)
{
	// Calculate buffer offsets and size
	VkDeviceSize			vertex_buffer_byte_size				= VkDeviceSize( vertices.size() * sizeof( *vertices.data() ) );
	VkDeviceSize			index_buffer_byte_size				= VkDeviceSize( indices.size() * sizeof( *indices.data() ) );
	VkDeviceSize			vertex_buffer_aligned_byte_size		= CalculateAlignmentForBuffer( vertex_buffer_byte_size , physicald_device_limits );
	VkDeviceSize			index_buffer_aligned_byte_size		= CalculateAlignmentForBuffer( index_buffer_byte_size, physicald_device_limits );
	VkDeviceSize			vertex_buffer_aligned_byte_offset	= 0;
	VkDeviceSize			index_buffer_aligned_byte_offset	= vertex_buffer_aligned_byte_size;
	VkDeviceSize			total_aligned_buffer_byte_size		= vertex_buffer_aligned_byte_size + index_buffer_aligned_byte_size;

	// Check the size of the current staging buffer, resize if needed
	if( current_staging_buffer_size < total_aligned_buffer_byte_size ) {
		if( !ResizeStagingBuffer(
			total_aligned_buffer_byte_size
		) ) return false;
	}

	// Copy data from dynamic vectors to staging buffer
	// so we can further copy it to the GPU.
	{
		void * original_mapped_memory = device_memory_pool->MapMemory(
			staging_buffer_memory
		);
		if( !original_mapped_memory ) return false;

		void * vertex_mapped_memory	= (uint8_t*)original_mapped_memory + vertex_buffer_aligned_byte_offset;
		void * index_mapped_memory	= (uint8_t*)original_mapped_memory + index_buffer_aligned_byte_offset;
		{
			// Copy over the vertex data to staging buffer
			std::memcpy( vertex_mapped_memory, vertices.data(), vertex_buffer_byte_size );

			// Clear inbetween bits, just in case
			VkDeviceSize vertex_index_buffer_gap	= vertex_buffer_aligned_byte_size - vertex_buffer_byte_size;
			if( vertex_index_buffer_gap ) {
				std::memset( (uint8_t*)vertex_mapped_memory + vertex_buffer_byte_size, 0, vertex_index_buffer_gap );
			}
		}
		{
			// Copy over the index data to staging buffer
			std::memcpy( index_mapped_memory, indices.data(), index_buffer_byte_size );

			// Clear tail of the buffer, just in case
			VkDeviceSize buffer_tail_size	= index_buffer_aligned_byte_size - index_buffer_byte_size;
			if( buffer_tail_size ) {
				std::memset( (uint8_t*)index_mapped_memory + index_buffer_byte_size, 0, buffer_tail_size );
			}
		}
		device_memory_pool->UnmapMemory( staging_buffer_memory );
	}

	// Check the size of the current device buffer, resize if needed
	if( current_device_buffer_size < total_aligned_buffer_byte_size ) {
		if( !ResizeDeviceBuffer(
			total_aligned_buffer_byte_size
		) ) return false;
	}

	VkBufferCopy copy_region {};
	copy_region.srcOffset	= 0;
	copy_region.dstOffset	= 0;
	copy_region.size		= std::min( current_staging_buffer_size, current_device_buffer_size );

	vkCmdCopyBuffer(
		command_buffer,
		staging_buffer,
		device_buffer,
		1,
		&copy_region
	);

	return true;
}









bool MeshBuffer::ResizeStagingBuffer(
	VkDeviceSize		new_size
)
{
	if( new_size == current_staging_buffer_size ) return true;

	// Destroy old buffer and free it's memory
	vkDestroyBuffer(
		device,
		staging_buffer,
		nullptr
	);
	device_memory_pool->FreeMemory(
		staging_buffer_memory
	);

	current_staging_buffer_size					= 0;


	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext					= nullptr;
	buffer_create_info.flags					= 0;
	buffer_create_info.size						= new_size;
	buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount	= 0;
	buffer_create_info.pQueueFamilyIndices		= nullptr;

	if( vkCreateBuffer(
		device,
		&buffer_create_info,
		nullptr,
		&staging_buffer
	) != VK_SUCCESS ) {
		return false;
	}
	staging_buffer_memory = device_memory_pool->AllocateAndBindBufferMemory(
		staging_buffer,
		&buffer_create_info,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);
	if( staging_buffer_memory != VK_SUCCESS ) return false;

	current_staging_buffer_size		= staging_buffer_memory.size;

	return true;
}









bool MeshBuffer::ResizeDeviceBuffer(
	VkDeviceSize		new_size
)
{
	if( new_size == current_device_buffer_size ) return true;

	// Resizing device buffers means that we need to synchronize
	// frame as this buffer might still be in use.
	SynchronizeFrame( window_data, device );

	// Destroy old buffer and free it's memory
	vkDestroyBuffer(
		device,
		device_buffer,
		nullptr
	);
	device_memory_pool->FreeMemory(
		device_buffer_memory
	);

	current_device_buffer_size						= 0;


	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext					= nullptr;
	buffer_create_info.flags					= 0;
	buffer_create_info.size						= new_size;
	buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount	= 0;
	buffer_create_info.pQueueFamilyIndices		= nullptr;

	if( vkCreateBuffer(
		device,
		&buffer_create_info,
		nullptr,
		&device_buffer
	) != VK_SUCCESS ) {
		return false;
	}
	device_buffer_memory = device_memory_pool->AllocateAndBindBufferMemory(
		device_buffer,
		&buffer_create_info,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	if( device_buffer_memory != VK_SUCCESS ) return false;

	current_device_buffer_size		= device_buffer_memory.size;

	return true;
}








} // vk2d
