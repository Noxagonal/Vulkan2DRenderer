#pragma once

#include <core/Common.hpp>

#include <type_traits>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Provides continuous hashing.
///
///				Uses std::hash internally, all basic types are supported.
class Hasher
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Calculate hash or continue calculating it.
	///
	///				To calculate combined hash from multiple values, you can just keep calling this function. After you've finished
	///				with all values, call GetHash() to get the combined hash result.
	///
	///				Hash is calculated with std::hash so all basic types are supported, to add support for non-standard types, you
	///				may add a specialization to std::hash and it will automatically be supported by this function as well.
	///
	/// @note		Multithreading: Not thread safe.
	///
	/// @tparam		T
	///				Type of the data we want to hash.
	///
	/// @param		data
	///				Data to hash.
	template<typename T>
	constexpr inline void		Hash(
		T value
	)
	{
		auto new_hash = std::hash<T> {}( value );
		hash ^= ( new_hash << 1 );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get combined hashed value from all previously hashed values.
	///
	/// @note		Multithreading: Not thread safe.
	///
	/// @return		Hash result.
	constexpr inline size_t		GetHash()
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	size_t hash;
};



} // vk2d
