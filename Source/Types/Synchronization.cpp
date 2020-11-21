
#include "../Core/SourceCommon.h"
#include "Synchronization.hpp"

void vk2d::_internal::Fence::Set()
{
	is_set = true;
	condition.notify_all();
}

bool vk2d::_internal::Fence::IsSet()
{
	return is_set.load();
}

bool vk2d::_internal::Fence::Wait(
	std::chrono::nanoseconds timeout
)
{
	if( timeout == std::chrono::nanoseconds::max() ) {
		return Wait( std::chrono::steady_clock::time_point::max() );
	}
	return Wait( std::chrono::steady_clock::now() + timeout );
}

bool vk2d::_internal::Fence::Wait(
	std::chrono::steady_clock::time_point timeout
)
{
	while( !is_set.load() ) {
		constexpr auto spinlock_period	= std::chrono::microseconds( 100 );
		auto time_left					= timeout - std::chrono::steady_clock::now();
		auto lock_wait_period			= ( time_left > spinlock_period ) ? spinlock_period : time_left;
		std::unique_lock<std::mutex> unique_lock( mutex );
		condition.wait_for( unique_lock, lock_wait_period );
		if( std::chrono::steady_clock::now() >= timeout ) {
			if( is_set.load() ) {
				return true;
			} else {
				return false;
			}
		}
	}
	return true;
}
