#pragma once

#include "SourceCommon.h"

#include "../../Include/VK2D/RenderPrimitives.h"
#include "../Header/WindowImpl.h"
#include "../Header/RendererImpl.h"
#include "../Header/VulkanMemoryManagement.h"

#include <vector>
#include <stdint.h>
#include <memory>



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
	class BufferBlock {
		friend class MeshBuffer;

	public:
		BufferBlock(
			MeshBuffer							*	mesh_buffer_parent,
			VkDeviceSize							buffer_vertex_size,
			VkDeviceSize							buffer_index_size );
		~BufferBlock();
		bool										CopyVectorsToStagingBuffers();

		MeshBuffer								*	parent								= {};

		std::vector<Vertex>							vertices							= {};
		std::vector<uint32_t>						indices								= {};

		VkDeviceSize								total_aligned_vertex_byte_size		= {};
		VkDeviceSize								total_aligned_index_byte_size		= {};

		VkDeviceSize								used_aligned_vertex_byte_size		= {};
		VkDeviceSize								used_aligned_index_byte_size		= {};

		VkDeviceSize								aligned_vertex_buffer_byte_offset	= {};
		VkDeviceSize								aligned_index_buffer_byte_offset	= {};

		VkBuffer									staging_buffer						= {};
		VkBuffer									device_buffer						= {};

		PoolMemory									staging_buffer_memory				= {};
		PoolMemory									device_buffer_memory				= {};

		bool										is_good								= {};
	};
	struct ReserveSpaceResult {
		BufferBlock								*	buffer_block						= {};	// nullptr if failed
		MeshOffsets									offsets								= {};
	};

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	BufferBlock									*	AllocateBufferBlockAndStore();

	// Removes a buffer block with matching pointer from internal storage.
	void											FreeBufferBlockFromStorage(
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
