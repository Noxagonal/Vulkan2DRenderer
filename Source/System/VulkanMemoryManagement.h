#pragma once

#include "Core/SourceCommon.h"



namespace vk2d {

namespace _internal {

class DeviceMemoryPool;
struct DeviceMemoryPoolDataImpl;



// Adjusts the size to fit the alignment requirements for a buffer,
// For example if buffer size is 7 and alignment requirements are
// 4, this function will return 8 as that is the next aligned size
// that will fully contain the buffer data.
VkDeviceSize								CalculateAlignmentForBuffer(
	VkDeviceSize							unaligned_size,
	const VkPhysicalDeviceLimits		&	physical_device_limits );



// Chunk is a single big allocation from the device directly. Aka, single pool of memory.
struct DeviceMemoryPoolChunk {
	// Block is a single virtual allocation from the Chunk. Aka, assignment from a single pool.
	struct Block {
		uint64_t													id									= UINT64_MAX;
		VkDeviceSize												offset								= 0;
		VkDeviceSize												size								= 0;
		VkDeviceSize												alignment							= 1;
	};

	uint64_t														id									= UINT64_MAX;
	VkDeviceMemory													memory								= VK_NULL_HANDLE;
	VkDeviceSize													size								= 0;
	::std::list<vk2d::_internal::DeviceMemoryPoolChunk::Block>		blocks;
	VkResult														result								= VK_RESULT_MAX_ENUM;

	uint64_t														blockIDCounter						= 0;
};

struct DeviceMemoryPoolDataImpl {
	uint64_t														chunkIDCounter						= {};

	VkPhysicalDevice												refPhysicalDevice					= {};
	VkDevice														refDevice							= {};

	VkDeviceSize													linearChunkSize						= {};
	VkDeviceSize													nonLinearChunkSize					= {};

	VkPhysicalDeviceProperties										physicalDeviceProperties			= {};
	VkPhysicalDeviceMemoryProperties								physicalDeviceMemoryProperties		= {};

	std::vector<std::list<vk2d::_internal::DeviceMemoryPoolChunk>>	linearChunks;						// buffers and linear images
	std::vector<std::list<vk2d::_internal::DeviceMemoryPoolChunk>>	nonLinearChunks;					// optimal images
};



class PoolMemory {
	friend class vk2d::_internal::DeviceMemoryPool;

private:
	vk2d::_internal::DeviceMemoryPoolDataImpl	*	allocated_from						= {};

	VkDeviceMemory									memory								= VK_NULL_HANDLE;
	VkDeviceSize									offset								= 0;
	VkDeviceSize									size								= 0;
	VkDeviceSize									alignment							= 0;

	uint64_t										chunkID								= UINT64_MAX;
	uint64_t										blockID								= UINT64_MAX;
	uint32_t										memoryTypeIndex						= UINT32_MAX;
	VkResult										result								= VkResult( INT32_MIN );
	bool											isLinear							= true;
	bool											isAllocated							= false;

public:
	// Can only map this memory if the memory is host visible
	template<typename T>
	inline T *										Map()
	{
		void * mapped_memory {};
		if( vkMapMemory(
			allocated_from->refDevice,
			memory,
			offset,
			size,
			0,
			&mapped_memory
		) != VK_SUCCESS ) {
			return nullptr;
		}
		return reinterpret_cast<T*>( mapped_memory );
	}

	inline void										Unmap()
	{
		vkUnmapMemory(
			allocated_from->refDevice,
			memory
		);
	}

	// Can only copy to this memory if the memory is host visible
	template<typename T>
	VkResult										DataCopy(
		const std::vector<T>					&	data )
	{
		return DataCopy<T>( data.data(), VkDeviceSize( data.size() ) );
	}

	// Can only copy to this memory if the memory is host visible
	template<typename T>
	VkResult										DataCopy(
		const T									*	data,
		VkDeviceSize								count )
	{
		auto mapped_data	= Map<T>();
		if( !mapped_data ) return VK_ERROR_MEMORY_MAP_FAILED;

		VkDeviceSize	byte_size	= sizeof( T ) * count;
		std::memcpy( mapped_data, data, byte_size );
		Unmap();
		return VK_SUCCESS;
	}

	inline VkDeviceMemory							GetVulkanMemory() const
	{
		return memory;
	}
	inline VkDeviceSize								GetOffset() const
	{
		return offset;
	}
	inline VkDeviceSize								GetSize() const
	{
		return size;
	}
	inline VkDeviceSize								GetAlignment() const
	{
		return alignment;
	}
	inline											operator VkResult() const
	{
		return result;
	};
};

class CompleteBufferResource {
public:
	VkResult										result								= {};
	VkBuffer										buffer								= {};
	VkBufferView									view								= {};
	vk2d::_internal::PoolMemory						memory								= {};

	inline											operator VkResult()
	{
		return result;
	}
};

class CompleteImageResource {
public:
	VkResult										result								= {};
	VkImage											image								= {};
	VkImageView										view								= {};
	vk2d::_internal::PoolMemory						memory								= {};

	inline											operator VkResult()
	{
		return result;
	}
};

class DeviceMemoryPool {
	friend class vk2d::_internal::PoolMemory;
	friend struct vk2d::_internal::DeviceMemoryPoolDataImpl;
	friend std::unique_ptr<vk2d::_internal::DeviceMemoryPool>								MakeDeviceMemoryPool(
		VkPhysicalDevice								physicalDevice,
		VkDevice										device,
		VkDeviceSize									linearAllocationChunkSize,
		VkDeviceSize									nonLinearAllocationChunkSize
	);

private:
	// Only accessible through MakeDeviceMemoryPool
																							DeviceMemoryPool(
		VkPhysicalDevice								physicalDevice,
		VkDevice										device,
		VkDeviceSize									linearAllocationChunkSize			= uint64_t( 1024 ) * 1024 * 64,
		VkDeviceSize									nonLinearAllocationChunkSize		= uint64_t( 1024 ) * 1024 * 256 );

public:
																							~DeviceMemoryPool();

	// Allocates memory for a buffer
	vk2d::_internal::PoolMemory																AllocateBufferMemory(
		VkBuffer										buffer,
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for an image
	vk2d::_internal::PoolMemory																AllocateImageMemory(
		VkImage											image,
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for a buffer and binds the buffer to the memory location, if memory
	// allocation or binding fails, the whole operation fails and no memory is allocated.
	vk2d::_internal::PoolMemory																AllocateAndBindBufferMemory(
		VkBuffer										buffer,
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for an image and binds the image to the memory location, if memory
	// allocation or binding fails, the whole operation fails and no memory is allocated.
	vk2d::_internal::PoolMemory																AllocateAndBindImageMemory(
		VkImage											image,
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Creates a buffer object backed with unique non-aliased memory allocated for it.
	// Creates a buffer, allocates memory for it and binds the buffer to a memory location,
	// if buffer creation, memory allocation or binding fails, the whole operation fails and
	// no resources are created. Optionally creates buffer view if it's create info is provided.
	vk2d::_internal::CompleteBufferResource													CreateCompleteBufferResource(
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags,
		const VkBufferViewCreateInfo				*	pBufferViewCreateInfo				= nullptr );

	// Creates an image object backed with unique non-aliased memory allocated for it.
	// Creates an image, allocates memory for it and binds the image to a memory location,
	// if image creation, memory allocation or binding fails, the whole operation fails and
	// no resources are created. Optionally creates image view if it's create info is provided.
	vk2d::_internal::CompleteImageResource													CreateCompleteImageResource(
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags,
		const VkImageViewCreateInfo					*	pImageViewCreateInfo				= nullptr );

	// Creates a buffer object backed with unique non-aliased memory allocated for it.
	// Creates a buffer, allocates memory for it and binds the buffer to a memory location,
	// Copies data into appropriate memory location to be used with buffer.
	// If any of the operations fail, the whole operation fails and no resources
	// are created. Optionally creates buffer view if it's create info is provided.
	template<typename T>
	vk2d::_internal::CompleteBufferResource													CreateCompleteHostBufferResourceWithData(
		const std::vector<T>						&	data,
		VkBufferUsageFlags								buffer_usage,
		const std::vector<uint32_t>					&	concurrent_family_indices			= {},
		const void									*	pNextToBufferCreateInfo				= nullptr,
		const VkBufferViewCreateInfo				*	pBufferViewCreateInfo				= nullptr )
	{
		return CreateCompleteHostBufferResourceWithData<T>(
			data.data(),
			VkDeviceSize( data.size() ),
			buffer_usage,
			concurrent_family_indices,
			pNextToBufferCreateInfo,
			pBufferViewCreateInfo
		);
	}

	// Creates a buffer object backed with unique non-aliased memory allocated for it.
	// Creates a buffer, allocates memory for it and binds the buffer to a memory location,
	// Copies data into appropriate memory location to be used with buffer.
	// If any of the operations fail, the whole operation fails and no resources
	// are created. Optionally creates buffer view if it's create info is provided.
	template<typename T>
	vk2d::_internal::CompleteBufferResource													CreateCompleteHostBufferResourceWithData(
		const T										*	data,
		VkDeviceSize									count,
		VkBufferUsageFlags								buffer_usage,
		const std::vector<uint32_t>					&	concurrent_family_indices			= {},
		const void									*	pNextToBufferCreateInfo				= nullptr,
		const VkBufferViewCreateInfo				*	pBufferViewCreateInfo				= nullptr )
	{
		VkDeviceSize byte_size	= sizeof( T ) * count;

		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.pNext					= pNextToBufferCreateInfo;
		buffer_create_info.flags					= 0;
		buffer_create_info.size						= byte_size;
		buffer_create_info.usage					= buffer_usage;
		buffer_create_info.sharingMode				= ( concurrent_family_indices.size() > 0 ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		buffer_create_info.queueFamilyIndexCount	= uint32_t( concurrent_family_indices.size() );
		buffer_create_info.pQueueFamilyIndices		= concurrent_family_indices.data();

		auto resource = CreateCompleteBufferResource(
			&buffer_create_info,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			pBufferViewCreateInfo
		);
		if( resource != VK_SUCCESS ) {
			return { resource.result };
		}

		auto result = resource.memory.DataCopy( data, count );
		if( result != VK_SUCCESS ) {
			FreeCompleteResource( resource );
			return { result };
		}
		return resource;
	}

	// Frees whatever memory has been allocated previously, either buffer or image memory.
	void																	FreeMemory(
		vk2d::_internal::PoolMemory										&	memory );

	// Deletes the buffer and frees the memory.
	void																	FreeCompleteResource(
		vk2d::_internal::CompleteBufferResource							&	resource );

	// Deletes the image and frees the memory.
	void																	FreeCompleteResource(
		vk2d::_internal::CompleteImageResource							&	resource );

	const VkPhysicalDeviceProperties									&	GetPhysicalDeviceProperties();
	const VkPhysicalDeviceMemoryProperties								&	GetPhysicalDeviceMemoryProperties();

private:
	std::pair<VkResult, vk2d::_internal::DeviceMemoryPoolChunk*>			AllocateChunk(
		std::list<vk2d::_internal::DeviceMemoryPoolChunk>				*	chunkGroup,
		VkDeviceSize														size,
		uint32_t															memoryTypeIndex );

	vk2d::_internal::DeviceMemoryPoolChunk::Block						*	AllocateBlockInChunk(
		vk2d::_internal::DeviceMemoryPoolChunk							*	chunk,
		VkMemoryRequirements											&	rMemoryRequirements );

	vk2d::_internal::PoolMemory												AllocateMemory(
		bool																isLinear,
		VkMemoryRequirements												memoryRequirements,
		uint32_t															memoryTypeIndex );

	void																	FreeChunk(
		std::list<vk2d::_internal::DeviceMemoryPoolChunk>				*	chunkGroup,
		vk2d::_internal::DeviceMemoryPoolChunk							*	chunk );

	void																	FreeBlock(
		uint32_t															memoryTypeIndex,
		bool																isLinear,
		uint64_t															chunkID,
		uint64_t															blockID );

	std::unique_ptr<vk2d::_internal::DeviceMemoryPoolDataImpl>				data						= {};

	bool																	is_good								= {};
};



std::unique_ptr<vk2d::_internal::DeviceMemoryPool>		MakeDeviceMemoryPool(
	VkPhysicalDevice									physicalDevice,
	VkDevice											device,
	VkDeviceSize										linearAllocationChunkSize			= VkDeviceSize( 1024 ) * 1024 * 64,
	VkDeviceSize										nonLinearAllocationChunkSize		= VkDeviceSize( 1024 ) * 1024 * 256
);



} // _internal

} // vk2d
