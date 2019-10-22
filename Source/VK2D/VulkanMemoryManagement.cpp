
#include "../Header/SourceCommon.h"
#include "../Header/VulkanMemoryManagement.h"


#include <algorithm>
#include <vector>
#include <memory>
#include <assert.h>


namespace vk2d {



VkDeviceSize								CalculateAlignmentForBuffer(
	VkDeviceSize							unaligned_size,
	const VkPhysicalDeviceLimits		&	physical_device_limits
)
{
	VkDeviceSize		buffer_alignment	= std::max( physical_device_limits.minUniformBufferOffsetAlignment, physical_device_limits.minStorageBufferOffsetAlignment );
						buffer_alignment	= std::max( buffer_alignment, VkDeviceSize( physical_device_limits.minMemoryMapAlignment ) );

	return VkDeviceSize( ( ( ( int64_t( unaligned_size ) - 1 ) / buffer_alignment ) + 1 ) * buffer_alignment );
}

namespace _internal {

struct DeviceMemoryPoolDataImpl {
	uint64_t													chunkIDCounter						= {};

	VkPhysicalDevice											refPhysicalDevice					= {};
	VkDevice													refDevice							= {};

	VkDeviceSize												linearChunkSize						= {};
	VkDeviceSize												nonLinearChunkSize					= {};

	VkPhysicalDeviceProperties									physicalDeviceProperties			= {};
	VkPhysicalDeviceMemoryProperties							physicalDeviceMemoryProperties		= {};

	std::vector<std::list<DeviceMemoryPool::Chunk>>				linearChunks;						// buffers and linear images
	std::vector<std::list<DeviceMemoryPool::Chunk>>				nonLinearChunks;					// optimal images
};

} // _internal

VK2D_API uint32_t VK2D_APIENTRY FindMemoryTypeIndex( const VkPhysicalDeviceMemoryProperties & memoryProperties, const VkMemoryRequirements & memoryRequirements, VkMemoryPropertyFlags propertyFlags )
{
	for( uint32_t i=0; i < memoryProperties.memoryTypeCount; ++i ) {
		if( memoryRequirements.memoryTypeBits & ( 1 << i ) ) {
			if( ( memoryProperties.memoryTypes[ i ].propertyFlags & propertyFlags ) == propertyFlags ) {
				return i;
			}
		}
	}
	return UINT32_MAX;
}

VK2D_API VkMemoryRequirements VK2D_APIENTRY GetBufferMemoryRequirements(
	VkDevice			device,
	VkBuffer			buffer )
{
	VkMemoryRequirements ret {};
	vkGetBufferMemoryRequirements( device, buffer, &ret );
	return ret;
}

VK2D_API VkMemoryRequirements VK2D_APIENTRY GetImageMemoryRequirements(
	VkDevice			device,
	VkImage				image )
{
	VkMemoryRequirements ret {};
	vkGetImageMemoryRequirements( device, image, &ret );
	return ret;
}

VK2D_API UniquePoolMemory::UniquePoolMemory( UniquePoolMemory && other )
{
	Swap( std::forward<UniquePoolMemory>( other ) );
}

VK2D_API UniquePoolMemory::~UniquePoolMemory()
{
	if( refDeviceMemoryPool ) {
		refDeviceMemoryPool->FreeMemory( memory );
	}
}

VK2D_API const UniquePoolMemory & VK2D_APIENTRY UniquePoolMemory::operator=(
	nullptr_t )
{
	this->~UniquePoolMemory();
	return *this;
}

VK2D_API const UniquePoolMemory & VK2D_APIENTRY UniquePoolMemory::operator=(
	UniquePoolMemory						&&	other )
{
	Swap( std::forward<UniquePoolMemory>( other ) );
	return *this;
}

VK2D_API PoolMemory * VK2D_APIENTRY UniquePoolMemory::operator->()
{
	return &memory;
}

VK2D_API void VK2D_APIENTRY UniquePoolMemory::Swap(
	UniquePoolMemory			&&	other )
{
	std::swap( refDeviceMemoryPool, other.refDeviceMemoryPool );
	std::swap( memory, other.memory );
}

void VK2D_APIENTRY FreeChunkMemory(
	VkDevice					device,
	VkDeviceMemory				memory,
	VkAllocationCallbacks	*	pAllocationCallbacks )
{
	assert( device );
	assert( memory );
	vkFreeMemory( device, memory, pAllocationCallbacks );
}

VK2D_API DeviceMemoryPool::DeviceMemoryPool(
	VkPhysicalDevice					physicalDevice,
	VkDevice							device,
	VkDeviceSize						linearAllocationChunkSize,
	VkDeviceSize						nonLinearAllocationChunkSize )
{
	data								= std::make_unique<_internal::DeviceMemoryPoolDataImpl>();
	if( !data ) {
		return;
	}

	data->refPhysicalDevice					= physicalDevice;
	data->refDevice							= device;

	vkGetPhysicalDeviceProperties(
		physicalDevice,
		&data->physicalDeviceProperties
	);
	vkGetPhysicalDeviceMemoryProperties(
		physicalDevice,
		&data->physicalDeviceMemoryProperties
	);

	data->linearChunkSize					= linearAllocationChunkSize;
	data->nonLinearChunkSize				= nonLinearAllocationChunkSize;

	data->linearChunks.resize( data->physicalDeviceMemoryProperties.memoryTypeCount );
	data->nonLinearChunks.resize( data->physicalDeviceMemoryProperties.memoryTypeCount );

	is_good			= true;
}

VK2D_API DeviceMemoryPool::~DeviceMemoryPool()
{
	if( data ) {
		for( auto & c : data->linearChunks ) {
			for( auto & t : c ) {
				FreeChunkMemory( data->refDevice, t.memory, nullptr );
			}
		}
		for( auto & c : data->nonLinearChunks ) {
			for( auto & t : c ) {
				FreeChunkMemory( data->refDevice, t.memory, nullptr );
			}
		}
		data->linearChunks.clear();
		data->nonLinearChunks.clear();
	}
}

VK2D_API PoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateBufferMemory(
	VkBuffer						buffer,
	const VkBufferCreateInfo	*	pBufferCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	if( data ) {
		auto memoryRequirements			= GetBufferMemoryRequirements( data->refDevice, buffer );
		auto memoryTypeIndex			= FindMemoryTypeIndex( data->physicalDeviceMemoryProperties, memoryRequirements, propertyFlags );

		if( memoryTypeIndex == UINT32_MAX ) return PoolMemory();
		return AllocateMemory( true, memoryRequirements, memoryTypeIndex );
	}
	return {};
}

VK2D_API PoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateImageMemory(
	VkImage							image,
	const VkImageCreateInfo		*	pImageCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	if( data ) {
		auto memoryRequirements			= GetImageMemoryRequirements( data->refDevice, image );
		auto memoryTypeIndex			= FindMemoryTypeIndex( data->physicalDeviceMemoryProperties, memoryRequirements, propertyFlags );

		if( memoryTypeIndex == UINT32_MAX ) return PoolMemory();
		if( pImageCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL ) {
			return AllocateMemory( false, memoryRequirements, memoryTypeIndex );
		} else {
			return AllocateMemory( true, memoryRequirements, memoryTypeIndex );
		}
	}
	return {};
}

VK2D_API PoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateAndBindBufferMemory(
	VkBuffer						buffer,
	const VkBufferCreateInfo	*	pBufferCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	PoolMemory memory = AllocateBufferMemory(
		buffer,
		pBufferCreateInfo,
		propertyFlags );

	if( memory == VK_SUCCESS ) {
		auto bind_result = vkBindBufferMemory(
			data->refDevice,
			buffer,
			memory.memory,
			memory.offset
		);
		if( bind_result != VK_SUCCESS ) {
			FreeMemory( memory );
			memory = {};
			memory.result = bind_result;
		}
	}

	return memory;
}

VK2D_API PoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateAndBindImageMemory(
	VkImage							image,
	const VkImageCreateInfo		*	pImageCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	PoolMemory memory = AllocateImageMemory(
		image,
		pImageCreateInfo,
		propertyFlags );

	if( memory == VK_SUCCESS ) {
		auto bind_result = vkBindImageMemory(
			data->refDevice,
			image,
			memory.memory,
			memory.offset
		);
		if( bind_result != VK_SUCCESS ) {
			FreeMemory( memory );
			memory = {};
			memory.result = bind_result;
		}
	}

	return memory;
}

VK2D_API UniquePoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateUniqueBufferMemory(
	VkBuffer						buffer,
	const VkBufferCreateInfo	*	pBufferCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	if( data ) {
		auto memoryRequirements			= GetBufferMemoryRequirements( data->refDevice, buffer );
		auto memoryTypeIndex			= FindMemoryTypeIndex( data->physicalDeviceMemoryProperties, memoryRequirements, propertyFlags );

		if( memoryTypeIndex == UINT32_MAX ) return UniquePoolMemory( this, {} );

		auto poolMemory					= AllocateMemory( true, memoryRequirements, memoryTypeIndex );
		UniquePoolMemory object( this, poolMemory );
		return object;
	}
	return {};
}

VK2D_API UniquePoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateUniqueImageMemory(
	VkImage							image,
	const VkImageCreateInfo		*	pImageCreateInfo,
	VkMemoryPropertyFlags			propertyFlags )
{
	if( data ) {
		auto memoryRequirements			= GetImageMemoryRequirements( data->refDevice, image );
		auto memoryTypeIndex			= FindMemoryTypeIndex( data->physicalDeviceMemoryProperties, memoryRequirements, propertyFlags );

		if( memoryTypeIndex == UINT32_MAX ) return UniquePoolMemory( this, {} );

		PoolMemory	poolMemory;
		if( pImageCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL ) {
			poolMemory	= AllocateMemory( false, memoryRequirements, memoryTypeIndex );
		} else {
			poolMemory	= AllocateMemory( true, memoryRequirements, memoryTypeIndex );
		}

		UniquePoolMemory object( this, poolMemory );
		return object;
	}
	return {};
}

VK2D_API void VK2D_APIENTRY DeviceMemoryPool::FreeMemory(
	PoolMemory			&	memory )
{
	if( memory.isAllocated ) {
		FreeBlock( memory.memoryTypeIndex, memory.isLinear, memory.chunkID, memory.blockID );
	}
	memory.isAllocated		= false;
}

VkPhysicalDeviceProperties emptyVkPhysicalDeviceProperties {};
VK2D_API const VkPhysicalDeviceProperties & VK2D_APIENTRY DeviceMemoryPool::GetPhysicalDeviceProperties()
{
	if( data ) {
		return data->physicalDeviceProperties;
	}
	return emptyVkPhysicalDeviceProperties;
}

VkPhysicalDeviceMemoryProperties emptyVkPhysicalDeviceMemoryProperties {};
VK2D_API const VkPhysicalDeviceMemoryProperties & VK2D_APIENTRY DeviceMemoryPool::GetPhysicalDeviceMemoryProperties()
{
	if( data ) {
		return data->physicalDeviceMemoryProperties;
	}
	return emptyVkPhysicalDeviceMemoryProperties;
}

VK2D_API void * VK2D_APIENTRY DeviceMemoryPool::MapMemory(
	const PoolMemory		&	memory )
{
	void * mapped_data = nullptr;
	if( vkMapMemory(
		data->refDevice,
		memory.memory,
		memory.offset,
		memory.size,
		0,
		&mapped_data
	) != VK_SUCCESS ) {
		return {};
	}
	return mapped_data;
}

VK2D_API void VK2D_APIENTRY DeviceMemoryPool::UnmapMemory(
	const PoolMemory	&	memory )
{
	vkUnmapMemory( data->refDevice, memory.memory );
}

VK2D_API std::pair<VkResult, DeviceMemoryPool::Chunk*> VK2D_APIENTRY DeviceMemoryPool::AllocateChunk(
	std::list<DeviceMemoryPool::Chunk>	*	chunkGroup,
	VkDeviceSize							size,
	uint32_t								memoryTypeIndex )
{
	assert( chunkGroup );

	VkMemoryAllocateInfo ai {};
	ai.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.pNext				= nullptr;
	ai.allocationSize		= size;
	ai.memoryTypeIndex		= memoryTypeIndex;
	VkDeviceMemory memory	= VK_NULL_HANDLE;
	VkResult result			= vkAllocateMemory( data->refDevice, &ai, nullptr, &memory );
	if( result != VK_SUCCESS ) {
		return { result, nullptr };
	}

	chunkGroup->push_back( DeviceMemoryPool::Chunk() );
	auto new_chunk		= &chunkGroup->back();
	new_chunk->id		= data->chunkIDCounter;
	new_chunk->memory	= memory;
	new_chunk->size		= size;
	new_chunk->result	= result;
	// To make make the code more compact we're going to push some dummy limits in front and back.
	// This will make sure we don't have to check if we're at the front or back of the list when searching for a free spot
	new_chunk->blocks.push_front( { UINT64_MAX, 0, 0, 1 } );
	new_chunk->blocks.push_back( { UINT64_MAX - 1, size, 0, 1 } );

	++data->chunkIDCounter;
	return { result, new_chunk };
}

VK2D_API DeviceMemoryPool::Chunk::Block * VK2D_APIENTRY DeviceMemoryPool::AllocateBlockInChunk(
	DeviceMemoryPool::Chunk		*	chunk,
	VkMemoryRequirements		&	rMemoryRequirements )
{
	assert( chunk );

	auto alignment				= rMemoryRequirements.alignment;
	auto size					= rMemoryRequirements.size;

	for( auto b = chunk->blocks.begin(); b != chunk->blocks.end(); ++b ) {
		if( b->id != UINT64_MAX ) {
			auto prev = b;
			--prev;

			auto range_begin	= prev->offset + prev->size;
			auto range_end		= b->offset;
			range_begin			= ( ( range_begin + alignment - 1 ) / alignment ) * alignment;
			auto range_size		= range_end - range_begin;

			if( range_size >= size ) {
				auto block		= chunk->blocks.insert( b, { chunk->blockIDCounter, range_begin, size, alignment } );
				++chunk->blockIDCounter;
				return &( *block );
			}
		}
	}
	return nullptr;
}

VK2D_API PoolMemory VK2D_APIENTRY DeviceMemoryPool::AllocateMemory( bool isLinear, VkMemoryRequirements memoryRequirements, uint32_t memoryTypeIndex )
{
	// TODO: add tests and error reports

	assert( memoryTypeIndex != UINT32_MAX );

	std::list<DeviceMemoryPool::Chunk>	*	chunkGroup		= nullptr;
	if( isLinear ) {
		chunkGroup		= &data->linearChunks[ memoryTypeIndex ];
	} else {
		chunkGroup		= &data->nonLinearChunks[ memoryTypeIndex ];
	}

	DeviceMemoryPool::Chunk			*	selectedChunk	= nullptr;
	DeviceMemoryPool::Chunk::Block	*	selectedBlock	= nullptr;
	for( auto & c : *chunkGroup ) {
		selectedChunk	= &c;
		selectedBlock					= AllocateBlockInChunk( selectedChunk, memoryRequirements );
		if( selectedBlock ) break;
	}

	// no chunks with free space, allocate a new chunk from the device
	if( !selectedBlock ) {
		auto				bufferImageGranularity	= data->physicalDeviceProperties.limits.bufferImageGranularity;
		VkDeviceSize		chunkSize				= 0;
		if( isLinear ) {
			chunkSize		= ( ( std::max( memoryRequirements.size, data->linearChunkSize ) + bufferImageGranularity - 1 ) / bufferImageGranularity ) * bufferImageGranularity;
		} else {
			chunkSize		= ( ( std::max( memoryRequirements.size, data->nonLinearChunkSize ) + bufferImageGranularity - 1 ) / bufferImageGranularity ) * bufferImageGranularity;
		}
		auto allocatedChunkInfo	= AllocateChunk( chunkGroup, chunkSize, memoryTypeIndex );
		selectedChunk		= allocatedChunkInfo.second;

		// ran out of memory
		if( !selectedChunk ) {
			PoolMemory pm {};
			pm.result		= allocatedChunkInfo.first;
			return pm;
		}

		selectedBlock		= AllocateBlockInChunk( selectedChunk, memoryRequirements );

		// should never happen, error
		assert( selectedBlock );
		if( !selectedBlock ) {
			PoolMemory pm {};
			pm.result		= allocatedChunkInfo.first;
			return pm;
		}
	}

	// construct PoolMemory
	PoolMemory ret {};
	ret.memory				= selectedChunk->memory;
	ret.offset				= selectedBlock->offset;
	ret.size				= selectedBlock->size;
	ret.alignment			= selectedBlock->alignment;
	ret.chunkID				= selectedChunk->id;
	ret.blockID				= selectedBlock->id;
	ret.memoryTypeIndex		= memoryTypeIndex;
	ret.result				= selectedChunk->result;
	ret.isLinear			= isLinear;
	ret.isAllocated			= true;
	return ret;
}

VK2D_API void VK2D_APIENTRY DeviceMemoryPool::FreeChunk( std::list<DeviceMemoryPool::Chunk>* chunkGroup, DeviceMemoryPool::Chunk * chunk )
{
	assert( chunkGroup );
	assert( chunk );
	FreeChunkMemory( data->refDevice, chunk->memory, nullptr );
	for( auto c = chunkGroup->begin(); c != chunkGroup->end(); ++c ) {
		if( c->id == chunk->id ) {
			chunkGroup->erase( c );
			return;
		}
	}
}

VK2D_API void VK2D_APIENTRY DeviceMemoryPool::FreeBlock(
	uint32_t		memoryTypeIndex,
	bool			isLinear,
	uint64_t		chunkID,
	uint64_t		blockID )
{
	assert( memoryTypeIndex != UINT32_MAX );
	assert( chunkID != UINT64_MAX );
	assert( blockID != UINT64_MAX );

	std::list<DeviceMemoryPool::Chunk>	*	chunkGroup		= nullptr;
	if( isLinear ) {
		chunkGroup						= &data->linearChunks[ memoryTypeIndex ];
	} else {
		chunkGroup						= &data->nonLinearChunks[ memoryTypeIndex ];
	}

	DeviceMemoryPool::Chunk				*	selectedChunk	= nullptr;
	for( auto & c : *chunkGroup ) {
		if( c.id == chunkID ) {
			selectedChunk				= &c;
			break;
		}
	}
	if( !selectedChunk ) {
		return;
	}

	for( auto b = selectedChunk->blocks.begin(); b != selectedChunk->blocks.end(); ++b ) {
		if( b->id == blockID ) {
			selectedChunk->blocks.erase( b );
			break;
		}
	}
	if( selectedChunk->blocks.size() <= 2 ) {
		FreeChunk( chunkGroup, selectedChunk );
	}
}



VK2D_API std::unique_ptr<DeviceMemoryPool> VK2D_APIENTRY MakeDeviceMemoryPool(
	VkPhysicalDevice		physicalDevice,
	VkDevice				device,
	VkDeviceSize			linearAllocationChunkSize,
	VkDeviceSize			nonLinearAllocationChunkSize )
{
	auto device_memory_pool = std::unique_ptr<DeviceMemoryPool>(
		new DeviceMemoryPool(
			physicalDevice,
			device,
			linearAllocationChunkSize,
			nonLinearAllocationChunkSize
		) );
	if( device_memory_pool->is_good ) {
		return device_memory_pool;
	}
	return {};
}



} // vk2d
