
#include <core/SourceCommon.h>
#include "Synchronization.hpp"



void vk2d::vk2d_internal::Fence::Set()
{
	std::lock_guard<std::mutex> lock_guard( condition_variable_mutex );
	is_set = true;
	condition_variable.notify_all();
}

bool vk2d::vk2d_internal::Fence::IsSet()
{
	return is_set.load();
}

bool vk2d::vk2d_internal::Fence::Wait(
	std::chrono::nanoseconds timeout
)
{
	if( timeout == std::chrono::nanoseconds::max() ) {
		return Wait( std::chrono::steady_clock::time_point::max() );
	}
	return Wait( std::chrono::steady_clock::now() + timeout );
}

bool vk2d::vk2d_internal::Fence::Wait(
	std::chrono::steady_clock::time_point timeout
)
{
	// In theory, using atomic variables is faster than using mutex, so we'll do some
	// maneuvering here to only use atomic variables whenever "is_set" is true.

	while( !is_set.load() ) {
		std::unique_lock<std::mutex> unique_lock( condition_variable_mutex );
		if( is_set.load() ) return true;
		condition_variable.wait_until( unique_lock, timeout );
		if( std::chrono::steady_clock::now() >= timeout ) {
			return is_set.load();
		}
	}
	return true;
}
