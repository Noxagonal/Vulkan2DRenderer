#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>

namespace vk2d {



template<typename T>
class SynchronizedObject
{
public:
	SynchronizedObject() {

	}
	~SynchronizedObject() {

	}

	void							Set(
		T						&&	object )
	{
		obj = std::move( object );
		Enable();
	}

	void							Set(
		const T					&	object )
	{
		obj = object;
		Enable();
	}

	const T						&	Get()
	{
		WaitTillEnabled();
		return obj;
	}

	const T						&	operator->()
	{
		WaitTillEnabled();
		return obj;
	}

	const T						&	operator*()
	{
		WaitTillEnabled();
		return obj;
	}

private:
	void							Enable()
	{
		is_set						= true;
		condition.notify_all();
	}
	void							WaitTillEnabled()
	{
		while( !is_set.load() ) {
			std::unique_lock<std::mutex> unique_lock( mutex );
			condition.wait_for( unique_lock, std::chrono::microseconds( 100 ) );
		}
	}

	std::mutex						mutex;
	std::condition_variable			condition;
	std::atomic_bool				is_set					= {};
	T								obj;
};



} // vk2d
