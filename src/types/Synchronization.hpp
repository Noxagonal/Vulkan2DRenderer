#pragma once

#include <core/SourceCommon.h>



namespace vk2d {
namespace vk2d_internal {



/// @brief		This is used to synchronize work.
class Fence
{
public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Fence() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Fence(
		const Fence									&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Fence(
		Fence										&&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Unblocks waiting on this fence
	///
	///				Any thread calling Wait() is allowed to continue after calling this.
	void											Set();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tests without blocking if this fence is set.
	/// 
	/// @return		true if set, false if not set.
	bool											IsSet();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Blocks calling thread until this fence is set by another thread.
	/// 
	/// @param[in]	timeout
	///				Maximum time to wait before returning false.
	/// 
	/// @return		true if successfully waited, false if timeout.
	bool											Wait(
		std::chrono::nanoseconds					timeout				= std::chrono::nanoseconds::max() );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Blocks calling thread until this fence is set by another thread.
	/// 
	/// @param[in]	timeout
	///				Time point to wait for before returning false.
	/// 
	/// @return		true if successfully waited, false if timeout.
	bool											Wait(
		std::chrono::steady_clock::time_point		timeout );

private:
	std::mutex										condition_variable_mutex;
	std::condition_variable							condition_variable;
	std::atomic_bool								is_set					= {};
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is used to synchronize an object. Similar in function to std::future.
/// 
/// @tparam		T
///				Type of object or value contained in this class and we want to have fenced access to.
template<typename T>
class FencedObject
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the contained value and allows getting the value.
	/// 
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// 
	/// @return		Contained value reference.
	void							Set(
		const T					&	value )
	{
		this->value = value;
		fence.Set();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the contained value and allows getting the value.
	/// 
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// 
	/// @return		Contained value reference.
	void							Set(
		T						&&	value )
	{
		this->value = std::move( value );
		fence.Set();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets access to contained value, waits for signal first.
	/// 
	/// @return		Contained value reference.
	T							&	Get()
	{
		fence.Wait();
		return this->value;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the contained value and allows getting the value.
	/// 
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// 
	/// @return		Contained value reference.
	T							&	operator=( const T & value )
	{
		Set( value );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the contained value and allows getting the value.
	/// 
	/// @param		value
	///				Value that we wish to copy from to internal memory.
	/// 
	/// @return		Contained value reference.
	T							&	operator=( T && value )
	{
		Set( std::move( value ) );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets access to contained value, waits for signal first.
	/// 
	/// @return		Contained value reference.
	T							&	operator->()
	{
		return Get();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets access to contained value, waits for signal first.
	/// 
	/// @return		Contained value reference.
	T							&	operator*()
	{
		return Get();
	}

private:
	Fence							fence					= {};
	T								value;
};



} // vk2d_internal
} // vk2d
