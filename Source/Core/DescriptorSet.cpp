
#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/DescriptorSet.h"
#include "../Header/Impl/InstanceImpl.h"

#include <memory>
#include <vector>
#include <bitset>
#include <assert.h>

namespace vk2d {
namespace _internal {

constexpr float			DESCRIPTOR_AUTO_POOL_CATEGORY_MINIMUM_COMPATIBILITY		= 0.75;
constexpr uint32_t		DESCRIPTOR_AUTO_POOL_ALLOCATION_BATCH_SIZE				= 256;

} // _internal
} // vk2d


double vk2d::_internal::DescriptorPoolRequirements::CheckCompatibilityWith(
	const vk2d::_internal::DescriptorPoolRequirements		&	other ) const
{
	double compatibility	= 0.0;

	if( ( typeBits & other.typeBits ) == typeBits ) {
		// we know that at this point the two will be at least somewhat compatible.
		uint32_t bindingAmountsCounted	= 0;

		for( size_t i=0; i < bindingAmounts.size(); ++i ) {
			if( bindingAmounts[ i ] ) {
				double a = double( std::min( bindingAmounts[ i ], other.bindingAmounts[ i ] ) );
				double b = double( std::max( bindingAmounts[ i ], other.bindingAmounts[ i ] ) );
				assert( a );	// Neither a or b should ever be 0.
				assert( b );	// typeBits was set up wrong.
				compatibility += a / b;
				bindingAmountsCounted++;
			}
		}

		auto ac = std::bitset<sizeof( typeBits ) * 8>( typeBits ).count();
		auto bc = std::bitset<sizeof( other.typeBits ) * 8>( other.typeBits ).count();
		if( ac > bc ) std::swap( ac, bc );

		compatibility /= double( bindingAmountsCounted );
		compatibility *= double( ac ) / double( bc );
	}

	assert( compatibility <= 1.0 );
	assert( compatibility >= 0.0 );
	return compatibility;
}

vk2d::_internal::DescriptorSetLayout::DescriptorSetLayout(
	vk2d::_internal::InstanceImpl			*	instance,
	VkDevice									device,
	const VkDescriptorSetLayoutCreateInfo	*	pCreateInfo )
{
	assert( instance );
	assert( device );

	instance_parent		= instance;
	refDevice			= device;
	createInfo			= *pCreateInfo;

	// Create the actual descriptor set layout
	{
		if( vkCreateDescriptorSetLayout(
			refDevice,
			&createInfo,
			nullptr,
			&setLayout
		) != VK_SUCCESS ) {
			instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create descriptor set layout!" );
			return;
		}

		// Set up the descriptor pool requirements struct
		for( size_t i=0; i < createInfo.bindingCount; ++i ) {
			auto descriptorType		= createInfo.pBindings[ i ].descriptorType;

			descriptorPoolRequirements.typeBits		|= uint64_t( 1 ) << uint64_t( descriptorType );
			++descriptorPoolRequirements.bindingAmounts[ descriptorType ];
		}
	}

	is_good		= true;
}

vk2d::_internal::DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(
		refDevice,
		setLayout,
		nullptr
	);
}

VkDescriptorSetLayout vk2d::_internal::DescriptorSetLayout::GetVulkanDescriptorSetLayout() const
{
	return setLayout;
}

const VkDescriptorSetLayoutCreateInfo & vk2d::_internal::DescriptorSetLayout::GetDescriptorSetLayoutCreateInfo() const
{
	return createInfo;
}

const vk2d::_internal::DescriptorPoolRequirements & vk2d::_internal::DescriptorSetLayout::GetDescriptorPoolRequirements() const
{
	return descriptorPoolRequirements;
}

vk2d::_internal::DescriptorSetLayout::operator VkDescriptorSetLayout() const
{
	return setLayout;
}



std::unique_ptr<vk2d::_internal::DescriptorSetLayout> vk2d::_internal::CreateDescriptorSetLayout(
	vk2d::_internal::InstanceImpl			*	instance,
	VkDevice									device,
	const VkDescriptorSetLayoutCreateInfo	*	pCreateInfo
)
{
	auto unique_object = std::unique_ptr<vk2d::_internal::DescriptorSetLayout>( new vk2d::_internal::DescriptorSetLayout(
		instance,
		device,
		pCreateInfo ) );
	if( unique_object && unique_object->is_good ) return unique_object;
	return {};
}



vk2d::_internal::PoolDescriptorSet::operator VkResult() const
{
	return result;
};



vk2d::_internal::DescriptorAutoPool::DescriptorAutoPool(
	vk2d::_internal::InstanceImpl	*	instance,
	VkDevice							device
)
{
	assert( instance );
	assert( device );
	instance_parent			= instance;
	refDevice				= device;

	is_good		= true;
}

vk2d::_internal::DescriptorAutoPool::~DescriptorAutoPool()
{
	for( auto & c : poolCategories ) {
		vkDestroyDescriptorPool(
			refDevice,
			c.pool,
			nullptr );
	}
	poolCategories.clear();
}


vk2d::_internal::PoolDescriptorSet vk2d::_internal::DescriptorAutoPool::AllocateDescriptorSet(
	const DescriptorSetLayout		&	rForDescriptorSetLayout )
{
	const auto & setPoolRequirements	= rForDescriptorSetLayout.GetDescriptorPoolRequirements();
	PoolDescriptorSet ret				= {};

	// Organize pool categories from best suitable to worst suitable,
	// and try each one until we find one that will work, if none
	// work, create a new internal Vulkan descriptor pool using the
	// DescriptorPoolRequirements from the descriptor set.
	std::vector<std::pair<double, PoolCategory*>> sortedCategories;
	{
		for( auto & c : poolCategories ) {
			if( !c.isFull ) {
				auto setCompatibilityWithCurrentCategory	= setPoolRequirements.CheckCompatibilityWith( c.originalPoolRequirements );
				if( setCompatibilityWithCurrentCategory > 0.0 &&
					setCompatibilityWithCurrentCategory >= DESCRIPTOR_AUTO_POOL_CATEGORY_MINIMUM_COMPATIBILITY ) {
					// We only consider categories with at least a certain compatibility.
					sortedCategories.push_back( { setCompatibilityWithCurrentCategory, &c } );
				}
			}
		}
		std::sort( sortedCategories.begin(), sortedCategories.end(),
			[]( const std::pair<double, PoolCategory*> & a, const std::pair<double, PoolCategory*> & b )
			{
				return a.first > b.first;
			} );
	}

	// Try allocating from each pool in the sorted categories, until we
	// find one that works or until we run out of sorted categories.
	{
		VkDescriptorSetLayout	setLayout				= rForDescriptorSetLayout;
		VkDescriptorSet			set						= VK_NULL_HANDLE;

		for( auto & sc : sortedCategories ) {
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
			descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext					= nullptr;
			descriptorSetAllocateInfo.descriptorPool		= sc.second->pool;
			descriptorSetAllocateInfo.descriptorSetCount	= 1;
			descriptorSetAllocateInfo.pSetLayouts			= &setLayout;
			auto result										= vkAllocateDescriptorSets( refDevice, &descriptorSetAllocateInfo, &set );
			switch( result ) {
			case VK_SUCCESS:
				++sc.second->counter;
				ret.descriptorSet	= set;
				ret.parentPool		= sc.second->pool;
				ret.result			= result;
				ret.allocated		= true;
				return ret;
			case VK_ERROR_FRAGMENTED_POOL:
			case VK_ERROR_OUT_OF_POOL_MEMORY:		// 1.1 ( After VK_KHR_maintenance1 )
//			case VK_ERROR_OUT_OF_DEVICE_MEMORY:		// 1.0 ( Before VK_KHR_maintenance1 )
				// Mark as full, try next pool
				sc.second->isFull	= true;
				break;
			default:
				instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot allocate Vulkan descriptor sets!" );
				ret.result			= result;
				return ret;
			}
		}
	}

	// Didn't find anything yet, so we'll create
	// a new internal Vulkan descriptor pool.
	{
		PoolCategory newCategory {};

		std::vector<VkDescriptorPoolSize> poolSizes {};
		auto & amounts = setPoolRequirements.GetBindingAmounts();
		for( size_t i = 0; i < amounts.size(); ++i ) {
			auto amount = amounts[ i ];
			if( amount ) {
				VkDescriptorPoolSize poolSize {};
				poolSize.type				= VkDescriptorType( i );
				poolSize.descriptorCount	= amount * DESCRIPTOR_AUTO_POOL_ALLOCATION_BATCH_SIZE;
				poolSizes.push_back( poolSize );
			}
		}

		newCategory.originalPoolRequirements			= setPoolRequirements;

		{
			VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {};
			descriptorPoolCreateInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCreateInfo.pNext				= nullptr;
			descriptorPoolCreateInfo.flags				= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolCreateInfo.maxSets			= DESCRIPTOR_AUTO_POOL_ALLOCATION_BATCH_SIZE;
			descriptorPoolCreateInfo.poolSizeCount		= uint32_t( poolSizes.size() );
			descriptorPoolCreateInfo.pPoolSizes			= poolSizes.data();
			auto result = vkCreateDescriptorPool(
				refDevice,
				&descriptorPoolCreateInfo,
				nullptr,
				&newCategory.pool
			);
			if( result != VK_SUCCESS ) {
				instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan descriptor pool!" );
				ret.result	= result;
				return ret;
			}
		}
		{
			VkDescriptorSetLayout	setLayout				= rForDescriptorSetLayout;
			VkDescriptorSet			set						= VK_NULL_HANDLE;
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
			descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext					= nullptr;
			descriptorSetAllocateInfo.descriptorPool		= newCategory.pool;
			descriptorSetAllocateInfo.descriptorSetCount	= 1;
			descriptorSetAllocateInfo.pSetLayouts			= &setLayout;
			auto result = vkAllocateDescriptorSets(
				refDevice,
				&descriptorSetAllocateInfo,
				&set
			);
			if( result != VK_SUCCESS ) {
				instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot allocate Vulkan descriptor sets!" );
				ret.result	= result;
				return ret;
			}

			++newCategory.counter;

			ret.descriptorSet	= set;
			ret.parentPool		= newCategory.pool;
			ret.result			= result;
			ret.allocated		= true;
		}

		poolCategories.push_back( newCategory );
	}
	return ret;
}

void vk2d::_internal::DescriptorAutoPool::FreeDescriptorSet(
	vk2d::_internal::PoolDescriptorSet		&	rDescriptorSet
)
{
	if( rDescriptorSet.allocated ) {
		assert( rDescriptorSet.parentPool );
		auto it = poolCategories.begin();
		while( it != poolCategories.end() ) {
			if( it->pool == rDescriptorSet.parentPool ) {
				// found the right pool.
				vkFreeDescriptorSets(
					refDevice,
					rDescriptorSet.parentPool,
					1, &rDescriptorSet.descriptorSet
				);
				--it->counter;
				it->isFull		= false;
				if( it->counter == 0 ) {
					vkDestroyDescriptorPool(
						refDevice,
						it->pool,
						nullptr
					);
					it = poolCategories.erase( it );
				} else {
					++it;
				}
				break;
			} else {
				++it;
			}
		}
		rDescriptorSet			= {};
	}
}


std::unique_ptr<vk2d::_internal::DescriptorAutoPool> vk2d::_internal::CreateDescriptorAutoPool(
	vk2d::_internal::InstanceImpl	*	instance,
	VkDevice							device
)
{
	auto unique_object = std::unique_ptr<vk2d::_internal::DescriptorAutoPool>( new vk2d::_internal::DescriptorAutoPool(
		instance,
		device ) );
	if( unique_object && unique_object->is_good ) {
		return unique_object;
	} else {
		instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create descriptor auto pool!" );
	}
	return {};
}
