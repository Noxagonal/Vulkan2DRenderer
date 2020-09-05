#pragma once

#include "SourceCommon.h"

#include "../../../Include/Interface/RenderPrimitives.h"
#include "../Impl/WindowImpl.h"
#include "../Impl/InstanceImpl.h"
#include "../Core/VulkanMemoryManagement.h"
#include "DescriptorSet.h"

#include <cstring>
#include <vector>
#include <stdint.h>
#include <memory>



namespace vk2d {

namespace _internal {

class MeshBuffer;

template<typename T>
class MeshBufferBlock;

using IndexBufferBlocks									= std::vector<std::unique_ptr<vk2d::_internal::MeshBufferBlock<uint32_t>>>;
using VertexBufferBlocks								= std::vector<std::unique_ptr<vk2d::_internal::MeshBufferBlock<vk2d::Vertex>>>;
using TextureChannelBufferBlocks						= std::vector<std::unique_ptr<vk2d::_internal::MeshBufferBlock<float>>>;

enum class MeshBufferDescriptorSetType : uint32_t {
	NONE,
	UNIFORM,
	STORAGE,
};



class MeshBuffer {
	template<typename T>
	friend class vk2d::_internal::MeshBufferBlock;
public:
	struct MeshBlockLocationInfo {
		bool												success						= {};

		vk2d::_internal::MeshBufferBlock<uint32_t>		*	index_block					= {};
		vk2d::_internal::MeshBufferBlock<vk2d::Vertex>	*	vertex_block				= {};
		vk2d::_internal::MeshBufferBlock<float>			*	texture_channel_block		= {};

		uint32_t											index_size					= {};	// size of data.
		VkDeviceSize										index_byte_size				= {};	// size of data in bytes.
		uint32_t											index_offset				= {};	// offset into buffer.
		VkDeviceSize										index_byte_offset			= {};	// offset into buffer in bytes.

		uint32_t											vertex_size					= {};	// size of data.
		VkDeviceSize										vertex_byte_size			= {};	// size of data in bytes.
		uint32_t											vertex_offset				= {};	// offset into buffer.
		VkDeviceSize										vertex_byte_offset			= {};	// offset into buffer in bytes.

		uint32_t											texture_channel_size		= {};	// size of data.
		VkDeviceSize										texture_channel_byte_size	= {};	// size of data in bytes.
		uint32_t											texture_channel_offset		= {};	// offset into buffer.
		VkDeviceSize										texture_channel_byte_offset	= {};	// offset into buffer in bytes.
	};

	struct PushResult {
		vk2d::_internal::MeshBuffer::MeshBlockLocationInfo	location_info;
		bool												success;
		inline explicit operator bool()
		{
			return	success;
		}
	};

	MeshBuffer(
		vk2d::_internal::InstanceImpl					*	instance,
		VkDevice											device,
		const VkPhysicalDeviceLimits					&	physicald_device_limits,
		vk2d::_internal::DeviceMemoryPool				*	device_memory_pool );

	~MeshBuffer();

	// Pushes mesh into render list, dynamically allocates new buffers
	// if needed, binds the new buffers to command buffer if needed
	// and adds vertex and index data to host visible buffer.
	// Returns mesh offsets of whatever buffer object this mesh was
	// put into, needed when recording a Vulkan draw command.
	vk2d::_internal::MeshBuffer::PushResult					CmdPushMesh(
		VkCommandBuffer										command_buffer,
		const std::vector<uint32_t>						&	new_indices,
		const std::vector<vk2d::Vertex>					&	new_vertices,
		const std::vector<float>							new_texture_channels );

	bool													CmdUploadMeshDataToGPU(
		VkCommandBuffer										command_buffer );

	// Gets the total amount of individual meshes that have been pushed so far.
	uint32_t												GetPushedMeshCount();

	// This gets the total amount of vertices already pushed in
	uint32_t												GetTotalVertexCount();

	// This gets the total amount of indices already pushed in
	uint32_t												GetTotalIndexCount();

	// This gets the total amount of texture channels already pushed in
	uint32_t												GetTotalTextureChannelCount();

private:
	vk2d::_internal::MeshBuffer::MeshBlockLocationInfo		ReserveSpaceForMesh(
		uint32_t											index_count,
		uint32_t											vertex_count,
		uint32_t											texture_channel_count );

	// Find an index buffer with enough space to hold the data, if none found
	// this function will allocate a new buffer that will have enough space.
	// Returns nullptr on failure.
	vk2d::_internal::MeshBufferBlock<uint32_t>			*	FindIndexBufferWithEnoughSpace(
		uint32_t											count );

	// Find an index buffer with enough space to hold the data, if none found
	// this function will allocate a new buffer that will have enough space.
	// Returns nullptr on failure.
	vk2d::_internal::MeshBufferBlock<vk2d::Vertex>		*	FindVertexBufferWithEnoughSpace(
		uint32_t											count );

	// Find an index buffer with enough space to hold the data, if none found
	// this function will allocate a new buffer that will have enough space.
	// Returns nullptr on failure.
	vk2d::_internal::MeshBufferBlock<float>				*	FindTextureChannelBufferWithEnoughSpace(
		uint32_t											count );

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	vk2d::_internal::MeshBufferBlock<uint32_t>			*	AllocateIndexBufferBlockAndStore(
		VkDeviceSize										byte_size );

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	vk2d::_internal::MeshBufferBlock<vk2d::Vertex>		*	AllocateVertexBufferBlockAndStore(
		VkDeviceSize										byte_size );

	// Creates a new buffer block and stores it internally,
	// returns a pointer to it if successful or nullptr on failure.
	vk2d::_internal::MeshBufferBlock<float>				*	AllocateTextureChannelBufferBlockAndStore(
		VkDeviceSize										byte_size );

	// Removes a buffer block with matching pointer from internal storage.
	void													FreeBufferBlockFromStorage(
		vk2d::_internal::MeshBufferBlock<uint32_t>		*	buffer_block );

	// Removes a buffer block with matching pointer from internal storage.
	void													FreeBufferBlockFromStorage(
		vk2d::_internal::MeshBufferBlock<vk2d::Vertex>	*	buffer_block );

	// Removes a buffer block with matching pointer from internal storage.
	void													FreeBufferBlockFromStorage(
		vk2d::_internal::MeshBufferBlock<float>			*	buffer_block );

	vk2d::_internal::InstanceImpl						*	instance_parent							= {};
	VkDevice												device									= {};
	VkPhysicalDeviceLimits									physicald_device_limits					= {};
	vk2d::_internal::DeviceMemoryPool					*	device_memory_pool						= {};

	bool													first_draw								= {};

	uint32_t												pushed_mesh_count						= {};
	uint32_t												pushed_index_count						= {};
	uint32_t												pushed_vertex_count						= {};
	uint32_t												pushed_texture_channel_count			= {};

	vk2d::_internal::MeshBufferBlock<uint32_t>			*	bound_index_buffer_block				= {};
	vk2d::_internal::MeshBufferBlock<vk2d::Vertex>		*	bound_vertex_buffer_block				= {};
	vk2d::_internal::MeshBufferBlock<float>				*	bound_texture_channel_buffer_block		= {};

	vk2d::_internal::IndexBufferBlocks						index_buffer_blocks						= {};
	vk2d::_internal::VertexBufferBlocks						vertex_buffer_blocks					= {};
	vk2d::_internal::TextureChannelBufferBlocks				texture_channel_buffer_blocks			= {};
	vk2d::_internal::IndexBufferBlocks::iterator			current_index_buffer_block				= {};
	vk2d::_internal::VertexBufferBlocks::iterator			current_vertex_buffer_block				= {};
	vk2d::_internal::TextureChannelBufferBlocks::iterator	current_texture_channel_buffer_block	= {};
};







template<typename T>
class MeshBufferBlock {
	friend class vk2d::_internal::MeshBuffer;

public:
	MeshBufferBlock(
		vk2d::_internal::MeshBuffer						*	mesh_buffer,
		VkDeviceSize										buffer_byte_size,
		VkBufferUsageFlags									buffer_usage_flags,
		vk2d::_internal::MeshBufferDescriptorSetType		descriptor_set_type
	)
	{
		assert( mesh_buffer );
		assert( buffer_byte_size );
		assert( buffer_usage_flags );

		mesh_buffer_parent			= mesh_buffer;
		auto instance				= mesh_buffer_parent->instance_parent;
		auto memory_pool			= instance->GetDeviceMemoryPool();

		total_byte_size				= vk2d::_internal::CalculateAlignmentForBuffer(
			buffer_byte_size,
			instance->GetVulkanPhysicalDeviceProperties().limits
		);

		host_data.reserve( total_byte_size / sizeof( T ) + 1 );

		// Create staging buffer
		{
			VkBufferCreateInfo buffer_create_info {};
			buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.pNext					= nullptr;
			buffer_create_info.flags					= 0;
			buffer_create_info.size						= total_byte_size;
			buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
			buffer_create_info.queueFamilyIndexCount	= 0;
			buffer_create_info.pQueueFamilyIndices		= nullptr;
			staging_buffer			= memory_pool->CreateCompleteBufferResource(
				&buffer_create_info,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			if( staging_buffer != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create MeshBufferBlock, cannot create staging buffer!" );
				return;
			}
		}

		// Create device buffer
		{
			VkBufferCreateInfo buffer_create_info {};
			buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.pNext					= nullptr;
			buffer_create_info.flags					= 0;
			buffer_create_info.size						= total_byte_size;
			buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage_flags;
			buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
			buffer_create_info.queueFamilyIndexCount	= 0;
			buffer_create_info.pQueueFamilyIndices		= nullptr;
			device_buffer			= memory_pool->CreateCompleteBufferResource(
				&buffer_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
			if( device_buffer != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create MeshBufferBlock, cannot create device buffer!" );
				return;
			}
		}

		// Create descriptor set
		{
			auto descriptor_pool	= instance->GetDescriptorPool();

			switch( descriptor_set_type ) {
			case vk2d::_internal::MeshBufferDescriptorSetType::NONE:
				break;
			case vk2d::_internal::MeshBufferDescriptorSetType::UNIFORM:
			{
				descriptor_set			= descriptor_pool->AllocateDescriptorSet( instance->GetUniformBufferDescriptorSetLayout() );

				VkDescriptorBufferInfo descriptor_write_buffer_info {};
				descriptor_write_buffer_info.buffer		= device_buffer.buffer;
				descriptor_write_buffer_info.offset		= 0;
				descriptor_write_buffer_info.range		= device_buffer.memory.GetSize();
				std::array<VkWriteDescriptorSet, 1> descriptor_write {};
				descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_write[ 0 ].pNext				= nullptr;
				descriptor_write[ 0 ].dstSet			= descriptor_set.descriptorSet;
				descriptor_write[ 0 ].dstBinding		= 0;
				descriptor_write[ 0 ].dstArrayElement	= 0;
				descriptor_write[ 0 ].descriptorCount	= 1;
				descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_write[ 0 ].pImageInfo		= nullptr;
				descriptor_write[ 0 ].pBufferInfo		= &descriptor_write_buffer_info;
				descriptor_write[ 0 ].pTexelBufferView	= nullptr;
				vkUpdateDescriptorSets(
					mesh_buffer_parent->device,
					uint32_t( descriptor_write.size() ), descriptor_write.data(),
					0, nullptr
				);
			}
				break;
			case vk2d::_internal::MeshBufferDescriptorSetType::STORAGE:
			{
				descriptor_set			= descriptor_pool->AllocateDescriptorSet( instance->GetStorageBufferDescriptorSetLayout() );

				VkDescriptorBufferInfo descriptor_write_buffer_info {};
				descriptor_write_buffer_info.buffer		= device_buffer.buffer;
				descriptor_write_buffer_info.offset		= 0;
				descriptor_write_buffer_info.range		= device_buffer.memory.GetSize();
				std::array<VkWriteDescriptorSet, 1> descriptor_write {};
				descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_write[ 0 ].pNext				= nullptr;
				descriptor_write[ 0 ].dstSet			= descriptor_set.descriptorSet;
				descriptor_write[ 0 ].dstBinding		= 0;
				descriptor_write[ 0 ].dstArrayElement	= 0;
				descriptor_write[ 0 ].descriptorCount	= 1;
				descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptor_write[ 0 ].pImageInfo		= nullptr;
				descriptor_write[ 0 ].pBufferInfo		= &descriptor_write_buffer_info;
				descriptor_write[ 0 ].pTexelBufferView	= nullptr;
				vkUpdateDescriptorSets(
					mesh_buffer_parent->device,
					uint32_t( descriptor_write.size() ), descriptor_write.data(),
					0, nullptr
				);
			}
				break;
			default:
				assert( 0 && "Should never reach this! Check how 'descriptor_set_type' was set." );
				break;
			}
		}
		is_good			= true;
	}

	~MeshBufferBlock()
	{
		auto descriptor_pool	= mesh_buffer_parent->instance_parent->GetDescriptorPool();
		auto memory_pool		= mesh_buffer_parent->instance_parent->GetDeviceMemoryPool();

		descriptor_pool->FreeDescriptorSet( descriptor_set );
		memory_pool->FreeCompleteResource( device_buffer );
		memory_pool->FreeCompleteResource( staging_buffer );
	}

	bool													CopyVectorsToStagingBuffers()
	{
		auto mapped_memory	= staging_buffer.memory.Map<T>();
		if( !mapped_memory ) {
			mesh_buffer_parent->instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot copy mesh buffer block to  map staging buffer memory" );
			return false;
		} else {
			std::memcpy( mapped_memory, host_data.data(), used_byte_size );
			staging_buffer.memory.Unmap();

			host_data.clear();

			return true;
		}
	}

	// Checks if something fits into this MeshBufferBlock.
	// Parameter count is not in byte size, if this MeshBufferBlock is type
	// float and parameter count is 1 then space for 4 bytes is checked for.
	bool													CheckDataFits(
		uint32_t											count
	)
	{
		VkDeviceSize	reserve_size		= count * sizeof( T );
		if( used_byte_size + reserve_size <= total_byte_size ) {
			return true;
		} else {
			return false;
		}
	}

	// If data fits, this will reserve space for it in the buffer.
	// Returns the location in bytes to the beginning of the reserved space.
	// Parameter count is not in byte size, if this MeshBufferBlock is type
	// float and parameter count is 1 then space for 4 bytes is reserved.
	VkDeviceSize											ReserveSpace(
		uint32_t											count
	)
	{
		VkDeviceSize reserve_size	= count * sizeof( T );
		assert( used_byte_size + reserve_size <= total_byte_size );
		auto ret					= used_byte_size;
		used_byte_size				+= reserve_size;
		return ret;
	}

	bool													IsGood()
	{
		return is_good;
	}

private:
	vk2d::_internal::MeshBuffer							*	mesh_buffer_parent			= {};

	// OPTIMIZATION:
	// Get rid of host_data vector and implement double buffering in the future,
	// instead of copying the data in CPU, we should just swap buffers.
	std::vector<T>											host_data					= {};

	VkDeviceSize											total_byte_size				= {};	// Total size of buffer in bytes.
	VkDeviceSize											used_byte_size				= {};	// Used size of uint data in bytes.

	vk2d::_internal::CompleteBufferResource					staging_buffer				= {};
	vk2d::_internal::CompleteBufferResource					device_buffer				= {};
	vk2d::_internal::PoolDescriptorSet						descriptor_set				= {};

	bool													is_good						= {};
};



} // _internal

} // vk2d
