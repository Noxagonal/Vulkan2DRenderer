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

class RendererImpl;



class MeshBuffer {
public:
	struct MeshOffsets {
		uint32_t											first_index;
		uint32_t											vertex_offset;
		VkDeviceSize										vertex_byte_offset;
		VkDeviceSize										index_byte_offset;
	};
	struct PushResult {
		vk2d::_internal::MeshBuffer::MeshOffsets			offsets;
		bool												success;
		inline explicit operator bool()
		{
			return	success;
		}
	};

	MeshBuffer(
		vk2d::_internal::RendererImpl					*	renderer,
		VkDevice											device,
		const VkPhysicalDeviceLimits					&	physicald_device_limits,
		vk2d::_internal::WindowImpl						*	window_data,
		vk2d::_internal::DeviceMemoryPool				*	device_memory_pool );

	~MeshBuffer();

	// Pushes mesh into render list, dynamically allocates new buffers
	// if needed, binds the new buffers to command buffer if needed
	// and adds vertex and index data to host visible buffer.
	// Returns mesh offsets of whatever buffer object this mesh was
	// put into, needed when recording a Vulkan draw command.
	vk2d::_internal::MeshBuffer::PushResult					CmdPushMesh(
		VkCommandBuffer										command_buffer,
		const std::vector<vk2d::Vertex>					&	new_vertices,
		const std::vector<uint32_t>						&	new_indices );

	bool													CmdUploadMeshDataToGPU(
		VkCommandBuffer										command_buffer );

	// Gets the total amount of individual meshes that have been pushed so far.
	size_t													GetPushedMeshCount();

	// This gets the total amount of vertices already pushed in
	size_t													GetTotalVertexCount();

	// This gets the total amount of indices already pushed in
	size_t													GetTotalIndexCount();

private:
	class BufferBlock {
		friend class vk2d::_internal::MeshBuffer;

	public:
		BufferBlock(
			vk2d::_internal::MeshBuffer					*	mesh_buffer_parent,
			VkDeviceSize									buffer_vertex_byte_size,
			VkDeviceSize									buffer_index_bute_size );
		~BufferBlock();
		bool												CopyVectorsToStagingBuffers();

		vk2d::_internal::MeshBuffer						*	renderer_parent								= {};

		std::vector<vk2d::Vertex>							vertices							= {};
		std::vector<uint32_t>								indices								= {};

		uint32_t											block_vertex_count					= {};
		uint32_t											block_index_count					= {};

		VkDeviceSize										total_aligned_buffer_byte_size		= {};
		VkDeviceSize										total_aligned_vertex_byte_size		= {};
		VkDeviceSize										total_aligned_index_byte_size		= {};

		VkDeviceSize										used_aligned_vertex_byte_size		= {};
		VkDeviceSize										used_aligned_index_byte_size		= {};

		VkDeviceSize										aligned_vertex_buffer_byte_offset	= {};
		VkDeviceSize										aligned_index_buffer_byte_offset	= {};

		vk2d::_internal::CompleteBufferResource				staging_buffer						= {};
		vk2d::_internal::CompleteBufferResource				device_buffer						= {};

		bool												is_good								= {};
	};
	struct ReserveSpaceResult {
		vk2d::_internal::MeshBuffer::BufferBlock		*	buffer_block						= {};	// nullptr if failed.
		bool												buffer_block_need_binding			= {};	// Tells if the buffer block changed since last time.
		vk2d::_internal::MeshBuffer::MeshOffsets			offsets								= {};
	};

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	vk2d::_internal::MeshBuffer::BufferBlock			*	AllocateBufferBlockAndStore(
		VkDeviceSize										vertex_portion_byte_size,
		VkDeviceSize										index_portion_byte_size );

	// Removes a buffer block with matching pointer from internal storage.
	void													FreeBufferBlockFromStorage(
		vk2d::_internal::MeshBuffer::BufferBlock		*	buffer_block );

	vk2d::_internal::MeshBuffer::ReserveSpaceResult			ReserveSpaceForMesh(
		uint32_t											vertex_count,
		uint32_t											index_count );

	vk2d::_internal::RendererImpl						*	renderer_parent					= {};
	VkDevice												device							= {};
	VkPhysicalDeviceLimits									physicald_device_limits			= {};
	vk2d::_internal::WindowImpl							*	window_data						= {};
	vk2d::_internal::DeviceMemoryPool					*	device_memory_pool				= {};

	bool													first_draw						= {};

	using BufferBlockArray									= std::vector<std::unique_ptr<vk2d::_internal::MeshBuffer::BufferBlock>>;
	vk2d::_internal::MeshBuffer::BufferBlockArray			buffer_blocks					= {};
	vk2d::_internal::MeshBuffer::BufferBlockArray::iterator	current_buffer_block			= {};
};



} // _internal

} // vk2d
