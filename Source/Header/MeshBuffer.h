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
	struct MeshOffsets {
		VkDeviceSize								vertex_byte_offset;
		VkDeviceSize								index_byte_offset;
	};

	MeshBuffer(
		VkDevice									device,
		const VkPhysicalDeviceLimits			&	physicald_device_limits,
		_internal::WindowImpl					*	window_data,
		DeviceMemoryPool						*	device_memory_pool );

	~MeshBuffer();

	// Pushes mesh into render list, dynamically allocates new buffers
	// if needed, binds the new buffers to command buffer if needed
	// and adds vertex and index data to host visible buffer.
	// Returns mesh offsets of whatever buffer object this mesh was
	// put into, needed when recording a Vulkan draw command.
	MeshOffsets										CmdPushMesh(
		VkCommandBuffer								command_buffer,
		const std::vector<Vertex>				&	new_vertices,
		const std::vector<uint32_t>				&	new_indices );

	// Pushes mesh into render list, dynamically allocates new buffers
	// if needed, binds the new buffers to command buffer if needed
	// and adds vertex and index data to host visible buffer.
	// Returns mesh offsets of whatever buffer object this mesh was
	// put into, needed when recording a Vulkan draw command.
	MeshOffsets										CmdPushMesh(
		VkCommandBuffer								command_buffer,
		const std::vector<Vertex>				&	new_vertices,
		const std::vector<VertexIndex_2>		&	new_indices );

	// Pushes mesh into render list, dynamically allocates new buffers
	// if needed, binds the new buffers to command buffer if needed
	// and adds vertex and index data to host visible buffer.
	// Returns mesh offsets of whatever buffer object this mesh was
	// put into, needed when recording a Vulkan draw command.
	MeshOffsets										CmdPushMesh(
		VkCommandBuffer								command_buffer,
		const std::vector<Vertex>				&	new_vertices,
		const std::vector<VertexIndex_3>		&	new_indices );

	bool											CmdUploadMeshDataToGPU(
		VkCommandBuffer								command_buffer );

	// Gets the total amount of individual meshes that have been pushed so far.
	size_t											GetPushedMeshCount();

	// This gets the total amount of vertices already pushed in
	size_t											GetTotalVertexCount();

	// This gets the total amount of indices already pushed in
	size_t											GetTotalIndexCount();

private:
	// Keeps track of allocation blocks, we'll need a method of dynamically
	// allocating more memory as the rendering gets more complicated.
	// This struct is POD and completely managed by the MeshBuffer object.
	// OPTIMIZE MEMORY: Might be a good idea to create VertexBufferBlock
	//   and IndexBufferBlock and individually manage both instead of
	//   storing both vertices and indices within a single buffer object
	//   with pre-set size limit on both halfs of the buffer.
	struct BufferBlock {
		std::vector<Vertex>							vertices						= {};
		std::vector<uint32_t>						indices							= {};

		VkBuffer									staging_buffer					= {};
		VkBuffer									device_buffer					= {};

		PoolMemory									staging_buffer_memory			= {};
		PoolMemory									device_buffer_memory			= {};

		VkDeviceSize								buffer_index_size_left			= {};
		VkDeviceSize								buffer_vertex_size_left			= {};
	};
	struct ReserveSpaceResult {
		BufferBlock								*	buffer_block					= {};	// nullptr if failed
		MeshOffsets									offsets							= {};
	};

	// Returns only reference, buffer block is stored inside this object
	BufferBlock									*	AllocateBufferBlock();

	// Takes only reference, buffer block is stored inside this object
	void											FreeBufferBlock(
		BufferBlock								*	buffer_block );

	ReserveSpaceResult								ReserveSpaceForMesh(
		VkDeviceSize								vertex_count,
		VkDeviceSize								index_count );



	std::vector<std::unique_ptr<BufferBlock>>		buffer_blocks					= {};

	VkDevice										device							= {};
	VkPhysicalDeviceLimits							physicald_device_limits			= {};
	_internal::WindowImpl						*	window_data						= {};
	DeviceMemoryPool							*	device_memory_pool				= {};
};



} // vk2d
