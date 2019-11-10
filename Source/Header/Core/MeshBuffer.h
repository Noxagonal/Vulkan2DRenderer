#pragma once

#include "SourceCommon.h"

#include "../../../Include/Interface/RenderPrimitives.h"
#include "../Header/Impl/WindowImpl.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Core/VulkanMemoryManagement.h"

#include <vector>
#include <stdint.h>
#include <memory>



namespace vk2d {

namespace _internal {



class MeshBuffer {
public:
	struct MeshOffsets {
		uint32_t									first_index;
		uint32_t									vertex_offset;
		VkDeviceSize								vertex_byte_offset;
		VkDeviceSize								index_byte_offset;
	};
	struct PushResult {
		MeshOffsets									offsets;
		bool										success;
		inline explicit operator bool()
		{
			return	success;
		}
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
	PushResult										CmdPushMesh(
		VkCommandBuffer								command_buffer,
		const std::vector<Vertex>				&	new_vertices,
		const std::vector<uint32_t>				&	new_indices );

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
			VkDeviceSize							buffer_vertex_byte_size,
			VkDeviceSize							buffer_index_bute_size );
		~BufferBlock();
		bool										CopyVectorsToStagingBuffers();

		MeshBuffer								*	parent								= {};

		std::vector<Vertex>							vertices							= {};
		std::vector<uint32_t>						indices								= {};

		uint32_t									block_vertex_count					= {};
		uint32_t									block_index_count					= {};

		VkDeviceSize								total_aligned_buffer_byte_size		= {};
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
		BufferBlock								*	buffer_block						= {};	// nullptr if failed.
		bool										buffer_block_need_binding			= {};	// Tells if the buffer block changed since last time.
		MeshOffsets									offsets								= {};
	};

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	BufferBlock									*	AllocateBufferBlockAndStore(
		VkDeviceSize								vertex_portion_byte_size,
		VkDeviceSize								index_portion_byte_size );

	// Removes a buffer block with matching pointer from internal storage.
	void											FreeBufferBlockFromStorage(
		BufferBlock								*	buffer_block );

	ReserveSpaceResult								ReserveSpaceForMesh(
		uint32_t									vertex_count,
		uint32_t									index_count );



	typedef std::vector<std::unique_ptr<BufferBlock>> BufferBlockArray;
	BufferBlockArray								buffer_blocks					= {};
	BufferBlockArray::iterator						current_buffer_block			= {};

	VkDevice										device							= {};
	VkPhysicalDeviceLimits							physicald_device_limits			= {};
	_internal::WindowImpl						*	window_data						= {};
	DeviceMemoryPool							*	device_memory_pool				= {};

	bool											first_draw				= {};
};



} // _internal

} // vk2d
