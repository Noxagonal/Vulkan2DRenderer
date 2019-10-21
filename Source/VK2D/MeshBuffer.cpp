
#include "../Header/SourceCommon.h"

#include "../Header/MeshBuffer.h"

MeshBuffer::MeshBuffer()
{
}

MeshBuffer::~MeshBuffer()
{

}

void MeshBuffer::ResizeDeviceBuffer(
	VkDeviceSize		new_vertex_size,
	VkDeviceSize		new_index_size )
{


	// TODO: resizing device buffers means that
	// we need to halt all gpu rendering until
	// we have created a new device vertex buffers.

	// TODO: Implement a function in window class to wait for render operations before continuing.

	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext  = nullptr;
	buffer_create_info.flags  = 0;
	buffer_create_info.size  = todo;
	buffer_create_info.usage  = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_create_info.sharingMode  = ( render_family == transfer_family ) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	buffer_create_info.queueFamilyIndexCount  = todo;
	buffer_create_info.pQueueFamilyIndices  = todo;

	vkCreateBuffer(
		device,
		&buffer_create_info,
		nullptr,
		device_buffer
	);
}
