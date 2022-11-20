
#include <core/SourceCommon.hpp>

#include "VulkanMemoryManagement.hpp"



namespace vk2d {

namespace vk2d_internal {



VkDeviceSize								CalculateAlignmentForBuffer(
	VkDeviceSize							unaligned_size,
	const VkPhysicalDeviceLimits		&	physical_device_limits
)
{
	VkDeviceSize		buffer_alignment	= std::max(
		physical_device_limits.minUniformBufferOffsetAlignment,
		physical_device_limits.minStorageBufferOffsetAlignment
	);
	buffer_alignment	= std::max(
		buffer_alignment,
		VkDeviceSize( physical_device_limits.minMemoryMapAlignment )
	);

	return VkDeviceSize( ( ( ( int64_t( unaligned_size ) - 1 ) / buffer_alignment ) + 1 ) * buffer_alignment );
}

uint32_t FindMemoryTypeIndex(
	const VkPhysicalDeviceMemoryProperties		&	memoryProperties,
	const VkMemoryRequirements					&	memoryRequirements,
	VkMemoryPropertyFlags							propertyFlags )
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

VkMemoryRequirements GetBufferMemoryRequirements(
	VkDevice			device,
	VkBuffer			buffer )
{
	VkMemoryRequirements ret {};
	vkGetBufferMemoryRequirements( device, buffer, &ret );
	return ret;
}

VkMemoryRequirements GetImageMemoryRequirements(
	VkDevice			device,
	VkImage				image )
{
	VkMemoryRequirements ret {};
	vkGetImageMemoryRequirements( device, image, &ret );
	return ret;
}

void FreeChunkMemory(
	VkDevice					device,
	VkDeviceMemory				memory,
	VkAllocationCallbacks	*	pAllocationCallbacks )
{
	assert( device );
	assert( memory );
	vkFreeMemory( device, memory, pAllocationCallbacks );
}



} // vk2d_internal

} // vk2d



vk2d::vk2d_internal::DeviceMemoryPool::DeviceMemoryPool(
	VkPhysicalDevice					physicalDevice,
	VkDevice							device,
	VkDeviceSize						linearAllocationChunkSize,
	VkDeviceSize						nonLinearAllocationChunkSize )
{
	data								= std::make_unique<vk2d_internal::DeviceMemoryPoolDataImpl>();
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

vk2d::vk2d_internal::DeviceMemoryPool::~DeviceMemoryPool()
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

vk2d::vk2d_internal::PoolMemory vk2d::vk2d_internal::DeviceMemoryPool::AllocateBufferMemory(
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

vk2d::vk2d_internal::PoolMemory vk2d::vk2d_internal::DeviceMemoryPool::AllocateImageMemory(
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

vk2d::vk2d_internal::PoolMemory vk2d::vk2d_internal::DeviceMemoryPool::AllocateAndBindBufferMemory(
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

vk2d::vk2d_internal::PoolMemory vk2d::vk2d_internal::DeviceMemoryPool::AllocateAndBindImageMemory(
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

vk2d::vk2d_internal::CompleteBufferResource vk2d::vk2d_internal::DeviceMemoryPool::CreateCompleteBufferResource(
	const VkBufferCreateInfo		*	pBufferCreateInfo,
	VkMemoryPropertyFlags				propertyFlags,
	const VkBufferViewCreateInfo	*	pBufferViewCreateInfo )
{
	VkBuffer object {};
	auto result = vkCreateBuffer(
		data->refDevice,
		pBufferCreateInfo,
		nullptr,
		&object
	);
	if( result != VK_SUCCESS ) {
		return { result };
	}

	auto pool_memory = AllocateAndBindBufferMemory(
		object,
		pBufferCreateInfo,
		propertyFlags
	);
	if( pool_memory != VK_SUCCESS ) {
		vkDestroyBuffer(
			data->refDevice,
			object,
			nullptr
		);
		return { pool_memory.result };
	}

	VkBufferView view {};
	if( pBufferViewCreateInfo ) {
		VkBufferViewCreateInfo view_create_info	= *pBufferViewCreateInfo;
		view_create_info.buffer					= object;
		result = vkCreateBufferView(
			data->refDevice,
			&view_create_info,
			nullptr,
			&view
		);
		if( result != VK_SUCCESS ) {
			vkDestroyBuffer(
				data->refDevice,
				object,
				nullptr
			);
			FreeMemory( pool_memory );
			return { result };
		}
	}

	CompleteBufferResource resource {};
	resource.result			= VK_SUCCESS;
	resource.buffer			= object;
	resource.view			= view;
	resource.memory			= pool_memory;
	return resource;
}

vk2d::vk2d_internal::CompleteImageResource vk2d::vk2d_internal::DeviceMemoryPool::CreateCompleteImageResource(
	const VkImageCreateInfo			*	pImageCreateInfo,
	VkMemoryPropertyFlags				propertyFlags,
	const VkImageViewCreateInfo		*	pImageViewCreateInfo )	// Optional
{
	VkImage object {};
	auto result = vkCreateImage(
		data->refDevice,
		pImageCreateInfo,
		nullptr,
		&object
	);
	if( result != VK_SUCCESS ) {
		return { result };
	}

	auto pool_memory = AllocateAndBindImageMemory(
		object,
		pImageCreateInfo,
		propertyFlags
	);
	if( pool_memory != VK_SUCCESS ) {
		vkDestroyImage(
			data->refDevice,
			object,
			nullptr
		);
		return { pool_memory.result };
	}

	VkImageView view {};
	if( pImageViewCreateInfo ) {
		VkImageViewCreateInfo view_create_info	= *pImageViewCreateInfo;
		view_create_info.image					= object;
		result = vkCreateImageView(
			data->refDevice,
			&view_create_info,
			nullptr,
			&view
		);
		if( result != VK_SUCCESS ) {
			vkDestroyImage(
				data->refDevice,
				object,
				nullptr
			);
			FreeMemory( pool_memory );
			return { result };
		}
	}

	CompleteImageResource resource {};
	resource.result			= VK_SUCCESS;
	resource.image			= object;
	resource.view			= view;
	resource.memory			= pool_memory;
	return resource;
}

void vk2d::vk2d_internal::DeviceMemoryPool::FreeMemory(
	PoolMemory			&	memory )
{
	if( memory.isAllocated ) {
		FreeBlock( memory.memoryTypeIndex, memory.isLinear, memory.chunkID, memory.blockID );
	}
	memory.isAllocated		= false;
}

void vk2d::vk2d_internal::DeviceMemoryPool::FreeCompleteResource(
	CompleteBufferResource		&	resource )
{
	vkDestroyBuffer(
		data->refDevice,
		resource.buffer,
		nullptr
	);
	vkDestroyBufferView(
		data->refDevice,
		resource.view,
		nullptr
	);
	FreeMemory( resource.memory );
	resource = {};
}

void vk2d::vk2d_internal::DeviceMemoryPool::FreeCompleteResource(
	CompleteImageResource		&	resource )
{
	vkDestroyImage(
		data->refDevice,
		resource.image,
		nullptr
	);
	vkDestroyImageView(
		data->refDevice,
		resource.view,
		nullptr
	);
	FreeMemory( resource.memory );
	resource = {};
}

VkPhysicalDeviceProperties emptyVkPhysicalDeviceProperties {};
const VkPhysicalDeviceProperties & vk2d::vk2d_internal::DeviceMemoryPool::GetPhysicalDeviceProperties()
{
	if( data ) {
		return data->physicalDeviceProperties;
	}
	return emptyVkPhysicalDeviceProperties;
}

VkPhysicalDeviceMemoryProperties emptyVkPhysicalDeviceMemoryProperties {};
const VkPhysicalDeviceMemoryProperties & vk2d::vk2d_internal::DeviceMemoryPool::GetPhysicalDeviceMemoryProperties()
{
	if( data ) {
		return data->physicalDeviceMemoryProperties;
	}
	return emptyVkPhysicalDeviceMemoryProperties;
}

std::pair<VkResult, vk2d::vk2d_internal::DeviceMemoryPoolChunk*> vk2d::vk2d_internal::DeviceMemoryPool::AllocateChunk(
	std::list<DeviceMemoryPoolChunk>	*	chunkGroup,
	VkDeviceSize											size,
	uint32_t												memoryTypeIndex )
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

	chunkGroup->push_back( DeviceMemoryPoolChunk() );
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

vk2d::vk2d_internal::DeviceMemoryPoolChunk::Block * vk2d::vk2d_internal::DeviceMemoryPool::AllocateBlockInChunk(
	DeviceMemoryPoolChunk		*	chunk,
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

vk2d::vk2d_internal::PoolMemory vk2d::vk2d_internal::DeviceMemoryPool::AllocateMemory(
	bool					isLinear,
	VkMemoryRequirements	memoryRequirements,
	uint32_t				memoryTypeIndex )
{
	// TODO: add tests and error reports

	assert( memoryTypeIndex != UINT32_MAX );

	std::list<DeviceMemoryPoolChunk>	*	chunkGroup		= nullptr;
	if( isLinear ) {
		chunkGroup		= &data->linearChunks[ memoryTypeIndex ];
	} else {
		chunkGroup		= &data->nonLinearChunks[ memoryTypeIndex ];
	}

	DeviceMemoryPoolChunk			*	selectedChunk	= nullptr;
	DeviceMemoryPoolChunk::Block	*	selectedBlock	= nullptr;
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
	ret.allocated_from		= data.get();
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

void vk2d::vk2d_internal::DeviceMemoryPool::FreeChunk(
	std::list<DeviceMemoryPoolChunk>	*	chunkGroup,
	DeviceMemoryPoolChunk				*	chunk
)
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

void vk2d::vk2d_internal::DeviceMemoryPool::FreeBlock(
	uint32_t		memoryTypeIndex,
	bool			isLinear,
	uint64_t		chunkID,
	uint64_t		blockID )
{
	assert( memoryTypeIndex != UINT32_MAX );
	assert( chunkID != UINT64_MAX );
	assert( blockID != UINT64_MAX );

	std::list<DeviceMemoryPoolChunk>	*	chunkGroup		= nullptr;
	if( isLinear ) {
		chunkGroup						= &data->linearChunks[ memoryTypeIndex ];
	} else {
		chunkGroup						= &data->nonLinearChunks[ memoryTypeIndex ];
	}

	DeviceMemoryPoolChunk				*	selectedChunk	= nullptr;
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



std::unique_ptr<vk2d::vk2d_internal::DeviceMemoryPool> vk2d::vk2d_internal::MakeDeviceMemoryPool(
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
