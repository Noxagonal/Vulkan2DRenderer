#pragma once

#include "SourceCommon.h"

#include "../../Include/VK2D/RenderPrimitives.h"
#include "../Header/WindowImpl.h"
#include "../Header/RendererImpl.h"
#include "../Header/VulkanMemoryManagement.h"

#include <vector>
#include <stdint.h>



namespace vk2d {



class MeshBuffer {
public:
	MeshBuffer(
		VkDevice							device,
		const VkPhysicalDeviceLimits	&	physicald_device_limits,
		_internal::WindowImpl			*	window_data,
		DeviceMemoryPool				*	device_memory_pool );

	~MeshBuffer();

	void									PushBackVertex(
		Vertex								new_vertex );

	void									PushBackVertices(
		const std::vector<Vertex>		&	new_vertices );

	void									PushBackIndex(
		uint32_t							new_index );

	void									PushBackIndices(
		const std::vector<uint32_t>		&	new_indices );

	bool									CmdUploadToGPU(
		VkCommandBuffer						command_buffer );

private:
	bool									ResizeStagingBuffer(
		VkDeviceSize						new_size );

	bool									ResizeDeviceBuffer(
		VkDeviceSize						new_size );

	std::vector<Vertex>						vertices						= {};
	std::vector<uint32_t>					indices							= {};

	VkBuffer								staging_buffer					= {};
	VkBuffer								device_buffer					= {};

	PoolMemory								staging_buffer_memory			= {};
	PoolMemory								device_buffer_memory			= {};

	VkDeviceSize							current_staging_buffer_size		= {};
	VkDeviceSize							current_device_buffer_size		= {};

	VkDevice								device							= {};
	VkPhysicalDeviceLimits					physicald_device_limits			= {};
	_internal::WindowImpl				*	window_data						= {};
	DeviceMemoryPool					*	device_memory_pool				= {};
};



} // vk2d
