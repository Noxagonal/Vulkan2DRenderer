
#include <core/SourceCommon.hpp>

#include "QueueResolver.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



vk2d::vk2d_internal::DeviceQueueResolver::DeviceQueueResolver(
	VkInstance												instance,
	VkPhysicalDevice										physicalDevice,
	std::vector<std::pair<VkQueueFlags, float>>				queueTypes )
{
	assert( instance );
	assert( physicalDevice );

	refInstance			= instance;
	refPhysicalDevice	= physicalDevice;

	{
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &count, nullptr );
		family_properties.resize( count );
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &count, family_properties.data() );
	}

	// available queues for the family, we can substract from this to test if queues are available or not
	std::vector<uint32_t> available_queues( family_properties.size() );
	// family allocations, outside vector represents the queue family, inside vector represents indices to the queueTypes
	std::vector<std::vector<uint32_t>> family_allocations( family_properties.size() );
	for( size_t i=0; i < family_properties.size(); ++i ) {
		available_queues[ i ]	= family_properties[ i ].queueCount;
	}
	// queueGetInfo tells how to get the queues from the device after device creation
	queueGetInfo.resize( queueTypes.size() );
	if( queueGetInfo.size() != queueTypes.size() ) {
		assert( 0 );
		return;
	}
	for( auto & i : queueGetInfo ) {
		i.queueIndex			= UINT32_MAX;
		i.queueFamilyIndex		= UINT32_MAX;
		i.based_on				= UINT32_MAX;
	}

	// find queue families for queue types and also check for available queue count in that family
	for( uint32_t q=0; q < queueTypes.size(); ++q ) {
		uint32_t family_candidate = UINT32_MAX;
		for( uint32_t f=0; f < family_properties.size(); ++f ) {
			if( family_properties[ f ].queueFlags & queueTypes[ q ].first ) {
				if( available_queues[ f ] ) {
					if( family_candidate == UINT32_MAX ) {
						// found initial candidate
						family_candidate = f;
					} else if( available_queues[ family_candidate ] &&
						family_properties[ f ].queueCount < family_properties[ family_candidate ].queueCount ) {
						// found a better candidate
						family_candidate = f;
					}
				}
			}
		}
		// "allocate" queue from candidate if found
		if( family_candidate != UINT32_MAX ) {
			--available_queues[ family_candidate ];
			queueGetInfo[ q ].queueFamilyIndex	= family_candidate;
			queueGetInfo[ q ].queueIndex		= uint32_t( family_allocations[ family_candidate ].size() );
			family_allocations[ family_candidate ].push_back( q );
		}
	}

	// Check that everyone got a queue family
	std::vector<bool> queue_allocated_test( queueTypes.size() );
	if( queue_allocated_test.size() != queueTypes.size() ) {
		assert( 0 );
		return;
	}
	for( size_t a=0; a < family_allocations.size(); ++a ) {
		for( size_t b=0; b < family_allocations[ a ].size(); ++b ) {
			queue_allocated_test[ family_allocations[ a ][ b ] ] = true;
		}
	}

	// Those that didn't get an allocation will share the allocation with someone else with similar properties and less existing dependants
	std::vector<uint32_t> queue_dependants( queueTypes.size() );
	if( queue_dependants.size() != queueTypes.size() ) {
		assert( 0 );
		return;
	}
	for( uint32_t a=0; a < queue_allocated_test.size(); ++a ) {
		if( !queue_allocated_test[ a ] ) {
			uint32_t	candidate			= UINT32_MAX;
			float		candidate_priority	= 0.0f;
			for( uint32_t t=0; t < queueTypes.size(); ++t ) {
				if( !( a == t ) && queue_allocated_test[ t ] ) {
					if( family_properties[ queueGetInfo[ t ].queueFamilyIndex ].queueFlags & queueTypes[ a ].first ) {
						if( candidate == UINT32_MAX ) {
							// initial candidate
							candidate			= t;
							candidate_priority	= queueTypes[ t ].second;
						} else if(
							( std::abs( queueTypes[ t ].second - queueTypes[ a ].second ) + ( queue_dependants[ t ] / 15.0 ) ) <
							( std::abs( queueTypes[ t ].second - candidate_priority ) + ( queue_dependants[ candidate ] / 15.0 ) ) ) {
							// found better candidate
							candidate			= t;
							candidate_priority	= queueTypes[ t ].second;
						}
					}
				}
			}
			if( candidate != UINT32_MAX ) {
				queueGetInfo[ a ].based_on = candidate;
				++queue_dependants[ candidate ];
			}
		}
	}

	// build queue create infos
	queuePriorities.resize( family_properties.size() );
	queueCreateInfos.reserve( family_properties.size() );
	for( size_t i=0; i < family_properties.size(); ++i ) {
		queuePriorities[ i ].resize( family_allocations[ i ].size() );
		for( size_t p=0; p < family_allocations[ i ].size(); ++p ) {
			queuePriorities[ i ][ p ] = queueTypes[ family_allocations[ i ][ p ] ].second;
		}
		if( family_allocations[ i ].size() ) {
			VkDeviceQueueCreateInfo ci {};
			ci.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			ci.pNext				= nullptr;
			ci.flags				= 0;
			ci.queueFamilyIndex		= uint32_t( i );
			ci.queueCount			= uint32_t( family_allocations[ i ].size() );
			ci.pQueuePriorities		= queuePriorities[ i ].data();
			queueCreateInfos.push_back( ci );
		}
	}

	is_good		= true;
}



vk2d::vk2d_internal::DeviceQueueResolver::~DeviceQueueResolver()
{}



const std::vector<VkDeviceQueueCreateInfo> & vk2d::vk2d_internal::DeviceQueueResolver::GetDeviceQueueCreateInfos()
{
	return queueCreateInfos;
}



std::vector<vk2d::vk2d_internal::ResolvedQueue> vk2d::vk2d_internal::DeviceQueueResolver::GetQueues(
	VkDevice device
) const
{
	std::vector<ResolvedQueue> ret( queueGetInfo.size() );
	for( uint32_t i=0; i < ret.size(); ++i ) {
		ret[ i ].queue						= VK_NULL_HANDLE;
		ret[ i ].queue_family_index			= UINT32_MAX;
		ret[ i ].supports_presentation		= VK_FALSE;
		ret[ i ].queue_family_properties	= {};
		ret[ i ].queue_mutex				= nullptr;
		ret[ i ].based_on					= UINT32_MAX;
	}

	// get the original queues first
	for( size_t i=0; i < queueGetInfo.size(); ++i ) {
		if( queueGetInfo[ i ].based_on == UINT32_MAX ) {
			if( queueGetInfo[ i ].queueFamilyIndex != UINT32_MAX ) {
				vkGetDeviceQueue( device, queueGetInfo[ i ].queueFamilyIndex, queueGetInfo[ i ].queueIndex, &ret[ i ].queue );
				ret[ i ].queue_family_index			= queueGetInfo[ i ].queueFamilyIndex;
				ret[ i ].supports_presentation		= glfwGetPhysicalDevicePresentationSupport( refInstance, refPhysicalDevice, ret[ i ].queue_family_index );
				ret[ i ].queue_family_properties	= family_properties[ queueGetInfo[ i ].queueFamilyIndex ];
				ret[ i ].queue_mutex				= std::make_shared<std::mutex>();
			}
		}
	}

	// get the information for the shared queue after we know the originals
	for( size_t i=0; i < queueGetInfo.size(); ++i ) {
		if( queueGetInfo[ i ].based_on != UINT32_MAX ) {
			auto based_on						= queueGetInfo[ i ].based_on;
			ret[ i ].queue						= ret[ based_on ].queue;
			ret[ i ].queue_family_index			= ret[ based_on ].queue_family_index;
			ret[ i ].supports_presentation		= ret[ based_on ].supports_presentation;
			ret[ i ].queue_family_properties	= ret[ based_on ].queue_family_properties;
			ret[ i ].queue_mutex				= ret[ based_on ].queue_mutex;
			ret[ i ].based_on					= based_on;
		}
	}

	return ret;
}



bool vk2d::vk2d_internal::DeviceQueueResolver::IsGood()
{
	return is_good;
}
