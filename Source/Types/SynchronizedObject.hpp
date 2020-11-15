#pragma once

#include "../Core/SourceCommon.h"

namespace vk2d {
namespace _internal {



/// @brief		This is used to synchronize work.
class Fence
{
	Fence()												= default;
	Fence( const vk2d::_internal::Fence & other )		= delete;
	Fence( vk2d::_internal::Fence && other )			= default;

	/// @brief	Unblocks waiting on this fence, any thread calling
	/// vk2d::_interna::Wait() is allowed to continue after calling this.
	inline void						Set()
	{
		is_set						= true;
		condition.notify_all();
	}

	/// @brief		Blocks calling thread until this fence is set by another thread.
	/// @param[in]	timeout
	///				Maximum time to wait before returning false.
	/// @return		true if successfully waited, false if timeout.
	inline bool						Wait(
		std::chrono::nanoseconds	timeout		= std::chrono::nanoseconds::max() )
	{
		if( !is_set.load() ) {
			auto wait_until_time = std::chrono::steady_clock::now() + timeout;
			while( true ) {
				std::unique_lock<std::mutex> unique_lock( mutex );
				condition.wait_until( unique_lock, wait_until_time );
				if( std::chrono::steady_clock::now() >= wait_until_time ) {
					if( is_set.load() ) {
						return true;
					} else {
						return false;
					}
				}
			}
		}
		return true;
	}

private:
	std::mutex						mutex;
	std::condition_variable			condition;
	std::atomic_bool				is_set					= {};
};



/// @brief		This is used to synchronize an object. Similar in function to std::future.
/// @tparam		T
///				Type of object or value contained in this class and we want to have fenced access to.
template<typename T>
class FencedObject
{
public:

	/// @brief		Sets the contained value and allows getting the value.
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// @return		Contained value reference.
	void							Set(
		const T					&	value )
	{
		this->value = value;
		fence.Set();
	}

	/// @brief		Sets the contained value and allows getting the value.
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// @return		Contained value reference.
	void							Set(
		T						&&	value )
	{
		this->value = std::move( value );
		fence.Set();
	}

	/// @brief		Gets access to contained value, waits for signal first.
	/// @return		Contained value reference.
	T							&	Get()
	{
		fence.Wait();
		return this->value;
	}

	/// @brief		Sets the contained value and allows getting the value.
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// @return		Contained value reference.
	T							&	operator=( const T & value )
	{
		Set( value );
	}

	/// @brief		Sets the contained value and allows getting the value.
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// @return		Contained value reference.
	T							&	operator=( T && value )
	{
		Set( std::move( value ) );
	}

	/// @brief		Gets access to contained value, waits for signal first.
	/// @return		Contained value reference.
	T							&	operator->()
	{
		return Get();
	}

	/// @brief		Gets access to contained value, waits for signal first.
	/// @return		Contained value reference.
	T							&	operator*()
	{
		return Get();
	}

private:
	Fence							fence					= {};
	T								value;
};



} // _internal
} // vk2d
