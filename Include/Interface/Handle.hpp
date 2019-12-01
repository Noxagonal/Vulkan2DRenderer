#pragma once

#include "../Core/Common.h"

#include <memory>
#include <mutex>

namespace vk2d {

namespace _internal {

struct HandleInfo {
	std::mutex		mutex;
	bool			exists		= {};
	uint32_t		users		= {};
};

} // _internal



template<typename T>
class Handle {
public:
	Handle( T * object )
	{
		info	= object->handle_info;
		++info->users;
	}
	Handle( const Handle<T> & other )					= default;
	Handle( Handle<T> && other )						= default;

	~Handle()											= default;

	Handle<T> & operator=( const Handle<T> & other )	= default;
	Handle<T> & operator=( Handle<T> && other )			= default;

	bool												Exists()
	{
		if( info ) {
			std::lock_guard<std::mutex> lock_guard( info->mutex );
			return info->exists;
		}
	}

private:
	T												*	object				= nullptr;
	std::shared_ptr<vk2d::_internal::HandleInfo>		info				= nullptr;
};



} // vk2d
