#pragma once

#include "../Header/SourceCommon.h"

#include <list>
#include <memory>
#include <utility>

namespace vk2d {



// Adjusts the size to fit the alignment requirements for a buffer,
// For example if buffer size is 7 and alignment requirements are
// 4, this function will return 8 as that is the next aligned size
// that will fully contain the buffer data.
VkDeviceSize								CalculateAlignmentForBuffer(
	VkDeviceSize							unaligned_size,
	const VkPhysicalDeviceLimits		&	physical_device_limits );



class DeviceMemoryPool;

struct PoolMemory {
	friend class DeviceMemoryPool;
	friend class UniquePoolMemory;

public:
	VkDeviceMemory									memory								= VK_NULL_HANDLE;
	VkDeviceSize									offset								= 0;
	VkDeviceSize									size								= 0;
	VkDeviceSize									alignment							= 0;

	inline											VK2D_APIENTRY						operator VkResult()
	{
		return result;
	};

private:
	uint64_t										chunkID								= UINT64_MAX;
	uint64_t										blockID								= UINT64_MAX;
	uint32_t										memoryTypeIndex						= UINT32_MAX;
	VkResult										result								= VkResult( INT32_MIN );
	bool											isLinear							= true;
	bool											isAllocated							= false;
};

class UniquePoolMemory {
private:
	DeviceMemoryPool							*	refDeviceMemoryPool					= nullptr;
	PoolMemory										memory;

public:
	inline																				UniquePoolMemory()
	{};
	inline																				UniquePoolMemory( nullptr_t )
	{};

	inline																				UniquePoolMemory(
		DeviceMemoryPool						*	deviceMemoryPool,
		PoolMemory									memory
	) :
		refDeviceMemoryPool( deviceMemoryPool ),
		memory( memory )
	{};


	VK2D_API																			UniquePoolMemory(
		UniquePoolMemory						&	other )								= delete;

	VK2D_API																			UniquePoolMemory(
		UniquePoolMemory						&&	other );

	VK2D_API																			~UniquePoolMemory();

	VK2D_API const UniquePoolMemory				&	VK2D_APIENTRY						operator=(
		nullptr_t );

	VK2D_API const UniquePoolMemory				&	VK2D_APIENTRY						operator=(
		UniquePoolMemory						&	other )								= delete;

	VK2D_API const UniquePoolMemory				&	VK2D_APIENTRY						operator=(
		UniquePoolMemory						&&	other );

	VK2D_API	PoolMemory							*	VK2D_APIENTRY					operator->();

	inline											VK2D_APIENTRY						operator VkResult()
	{
		return memory.result;
	}

private:
	VK2D_API void									VK2D_APIENTRY						Swap(
		UniquePoolMemory						&&	other );
};

namespace _internal {
struct DeviceMemoryPoolDataImpl;
}; // vkr_internal

class DeviceMemoryPool {
	friend struct _internal::DeviceMemoryPoolDataImpl;
	friend std::unique_ptr<DeviceMemoryPool>				MakeDeviceMemoryPool(
		VkPhysicalDevice									physicalDevice,
		VkDevice											device,
		VkDeviceSize										linearAllocationChunkSize,
		VkDeviceSize										nonLinearAllocationChunkSize
	);

private:
	// Chunk is a single big allocation from the device directly. Aka, single pool of memory.
	struct Chunk {
		// Block is a single virtual allocation from the Chunk. Aka, assignment from a single pool.
		struct Block {
			uint64_t									id									= UINT64_MAX;
			VkDeviceSize								offset								= 0;
			VkDeviceSize								size								= 0;
			VkDeviceSize								alignment							= 1;
		};

		uint64_t										id									= UINT64_MAX;
		VkDeviceMemory									memory								= VK_NULL_HANDLE;
		VkDeviceSize									size								= 0;
		::std::list<Block>								blocks;
		VkResult										result								= VK_RESULT_MAX_ENUM;

		uint64_t										blockIDCounter						= 0;
	};

private:
	// Only accessible through MakeDeviceMemoryPool
	VK2D_API																				DeviceMemoryPool(
		VkPhysicalDevice								physicalDevice,
		VkDevice										device,
		VkDeviceSize									linearAllocationChunkSize			= 1024Ui64 * 1024 * 64,
		VkDeviceSize									nonLinearAllocationChunkSize		= 1024Ui64 * 1024 * 256 );

public:
	VK2D_API																				~DeviceMemoryPool();

	// Allocates memory for a buffer
	VK2D_API PoolMemory									VK2D_APIENTRY						AllocateBufferMemory(
		VkBuffer										buffer,
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for an image
	VK2D_API PoolMemory									VK2D_APIENTRY						AllocateImageMemory(
		VkImage											image,
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for a buffer and binds the buffer to the memory location, if memory
	// allocation or binding fails, the whole operation fails and no memory is allocated.
	VK2D_API PoolMemory									VK2D_APIENTRY						AllocateAndBindBufferMemory(
		VkBuffer										buffer,
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Allocates memory for an image and binds the image to the memory location, if memory
	// allocation or binding fails, the whole operation fails and no memory is allocated.
	VK2D_API PoolMemory									VK2D_APIENTRY						AllocateAndBindImageMemory(
		VkImage											image,
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Same as AllocateBufferMemory() but returns an UniquePoolMemory instead that works
	// similarly to std::unique_ptr, it gets freed as soon as it goes out of scope.
	VK2D_API UniquePoolMemory							VK2D_APIENTRY						AllocateUniqueBufferMemory(
		VkBuffer										buffer,
		const VkBufferCreateInfo					*	pBufferCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Same as AllocateImageMemory() but returns an UniquePoolMemory instead that works
	// similarly to std::unique_ptr, it gets freed as soon as it goes out of scope.
	VK2D_API UniquePoolMemory							VK2D_APIENTRY						AllocateUniqueImageMemory(
		VkImage											image,
		const VkImageCreateInfo						*	pImageCreateInfo,
		VkMemoryPropertyFlags							propertyFlags );

	// Frees whatever memory has been allocated previously, either buffer or image memory.
	VK2D_API void										VK2D_APIENTRY						FreeMemory(
		PoolMemory									&	memory );

	VK2D_API const VkPhysicalDeviceProperties		&	VK2D_APIENTRY						GetPhysicalDeviceProperties();
	VK2D_API const VkPhysicalDeviceMemoryProperties	&	VK2D_APIENTRY						GetPhysicalDeviceMemoryProperties();

	// Makes sure that PoolMemory gets mapped correctly, as this is a pool allocation you
	// will have to take into consideration the offset and the size of the data.
	VK2D_API void									*	VK2D_APIENTRY						MapMemory(
		const PoolMemory							&	memory );

	// Makes sure that PoolMemory gets unmapped correctly.
	VK2D_API void										VK2D_APIENTRY						UnmapMemory(
		const PoolMemory							&	memory );

private:
	VK2D_API std::pair<VkResult, DeviceMemoryPool::Chunk*> VK2D_APIENTRY					AllocateChunk(
		std::list<DeviceMemoryPool::Chunk>												*	chunkGroup,
		VkDeviceSize																		size,
		uint32_t																			memoryTypeIndex );

	VK2D_API DeviceMemoryPool::Chunk::Block			*	VK2D_APIENTRY						AllocateBlockInChunk(
		DeviceMemoryPool::Chunk															*	chunk,
		VkMemoryRequirements															&	rMemoryRequirements );

	VK2D_API PoolMemory									VK2D_APIENTRY						AllocateMemory(
		bool																				isLinear,
		VkMemoryRequirements																memoryRequirements,
		uint32_t																			memoryTypeIndex );

	VK2D_API void										VK2D_APIENTRY						FreeChunk(
		std::list<DeviceMemoryPool::Chunk>												*	chunkGroup,
		DeviceMemoryPool::Chunk															*	chunk );

	VK2D_API void										VK2D_APIENTRY						FreeBlock(
		uint32_t																			memoryTypeIndex,
		bool																				isLinear,
		uint64_t																			chunkID,
		uint64_t																			blockID );

	::std::unique_ptr<_internal::DeviceMemoryPoolDataImpl>		data						= {};

	bool												is_good								= {};
};



VK2D_API std::unique_ptr<DeviceMemoryPool>				VK2D_APIENTRY						MakeDeviceMemoryPool(
	VkPhysicalDevice									physicalDevice,
	VkDevice											device,
	VkDeviceSize										linearAllocationChunkSize			= 1024Ui64 * 1024 * 64,
	VkDeviceSize										nonLinearAllocationChunkSize		= 1024Ui64 * 1024 * 256
);



} // vk2d
