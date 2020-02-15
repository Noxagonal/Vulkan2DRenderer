#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>

namespace vk2d {



class Fence
{
	Fence()									= default;
	Fence( const vk2d::Fence & other )		= delete;
	Fence( vk2d::Fence && other )			= default;
	~Fence()								= default;

	inline void						Set()
	{
		is_set						= true;
		condition.notify_all();
	}
	inline void						Wait()
	{
		while( !is_set.load() ) {
			std::unique_lock<std::mutex> unique_lock( mutex );
			condition.wait_for( unique_lock, std::chrono::microseconds( 100 ) );
		}
	}

private:
	std::mutex						mutex;
	std::condition_variable			condition;
	std::atomic_bool				is_set					= {};
};



template<typename T>
class FencedObject
{
public:
	FencedObject() {

	}
	~FencedObject() {

	}

	void							Set(
		T						&&	object )
	{
		obj = std::move( object );
		fence.Set();
	}

	void							Set(
		const T					&	object )
	{
		obj = object;
		fence.Set();
	}

	const T						&	Get()
	{
		fence.Wait();
		return obj;
	}

	const T						&	operator->()
	{
		fence.Wait();
		return obj;
	}

	const T						&	operator*()
	{
		fence.Wait();
		return obj;
	}

private:
	Fence							fence					= {};
	T								obj;
};



} // vk2d
