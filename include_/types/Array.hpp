#pragma once

#include "core/Common.h"

#include <initializer_list>
#include <ostream>
#include <vector>
#include <new>
#include <assert.h>



namespace vk2d {



// This works as a replacement of sorts to std::vector but with a few more features.
// TODO: vk2d::Array is experimental at this point.
// TODO: vk2d::Array key feature should be that the memory is allocated from a memory pool, this is not implemented yet.
template<typename T>
class Array
{
public:
	Array()										= default;
	Array( const vk2d::Array<T> & other )
	{
		CopyOther( other );
	}
	Array( vk2d::Array<T> && other )
	{
		SwapOther( std::move( other ) );
	}
	Array( size_t initial_size )
	{
		Resize( initial_size );
	}
	Array( std::initializer_list<T> initializer )
	{
		size_t size = initializer.size();
		if( size ) {
			ResizeNoConstruct( size );
			size_t counter = 0;
			auto it = initializer.begin();
			while( it != initializer.end() ) {
				if constexpr( std::is_move_constructible_v<T> ) {
					new( &data_ptr[ counter++ ] ) T( std::move( *it++ ) );
				} else {
					new( &data_ptr[ counter++ ] ) T( *it++ );
				}
			}
		}
	}
	Array( const std::vector<T> & vec )
	{
		if( std::size( vec ) ) {
			ResizeNoConstruct( std::size( vec ) );
			for( size_t i = 0; i < data_size; ++i ) {
				if constexpr( std::is_move_constructible_v<T> ) {
					new( &data_ptr[ i ] ) T( std::move( vec[ i ] ) );
				} else {
					new( &data_ptr[ i ] ) T( vec[ i ] );
				}
			}
		}
	}
	~Array()
	{
		DestructRange( 0, data_size );
		DoDeallocate( data_ptr );
	}
	operator std::vector<T>()
	{
		std::vector<T> ret;
		if( data_size ) {
			ret.reserve( data_size );
			for( size_t i = 0; i < data_size; ++i ) {
				ret.push_back( data_ptr[ i ] );
			}
		}
		return ret;
	};

	vk2d::Array<T> & operator=( const vk2d::Array<T> & other )
	{
		CopyOther( other );
		return *this;
	}
	vk2d::Array<T> & operator=( vk2d::Array<T> && other )
	{
		SwapOther( std::move( other ) );
		return *this;
	}

	T & operator[]( size_t index )
	{
		assert( index < data_size && "Index out of range." );
		if( !data_size ) throw std::out_of_range( "Index out of range." );
		return data_ptr[ index ];
	}
	vk2d::Array<T> & operator+=( const vk2d::Array<T> & other )
	{
		if( other.data_size ) {
			auto old_size = data_size;
			auto new_size = data_size + other.data_size;
			ResizeNoConstruct( data_size + other.data_size );
			for( size_t i = 0; i < other.data_size; ++i ) {
				new( &data_ptr[ i + old_size ] ) T( other.data_ptr[ i ] );
			}
		}
		return *this;
	}

	void PushFront( const T & value )
	{
		ShiftRight();
		new( &data_ptr[ 0 ] ) T( value );
	}
	void PushBack( const T & value )
	{
		ResizeNoConstruct( data_size + 1, DEFAULT_HEADROOM );
		new( &data_ptr[ data_size - 1 ] ) T( value );
	}
	void PushFront( T && value )
	{
		ShiftRight();
		if constexpr( std::is_move_constructible_v<T> ) {
			new( &data_ptr[ 0 ] ) T( std::move( value ) );
		} else {
			new( &data_ptr[ 0 ] ) T( value );
		}
	}
	void PushBack( T && value )
	{
		ResizeNoConstruct( data_size + 1, DEFAULT_HEADROOM );
		if constexpr( std::is_move_constructible_v<T> ) {
			new( &data_ptr[ data_size - 1 ] ) T( std::move( value ) );
		} else {
			new( &data_ptr[ data_size - 1 ] ) T( value );
		}
	}
	template<typename... Args>
	void EmplaceFront( Args && ...args )
	{
		ShiftRight();
		new( &data_ptr[ 0 ] ) T( std::forward<Args>( args )... );
	}
	template<typename... Args>
	void EmplaceBack( Args && ...args )
	{
		ResizeNoConstruct( data_size + 1, DEFAULT_HEADROOM );
		new( &data_ptr[ data_size - 1 ] ) T( std::forward<Args>( args )... );
	}
	T PopFront()
	{
		assert( data_size && "Array is empty." );
		if( !data_size ) throw std::out_of_range( "Array is empty." );

		T ret = std::move( data_ptr[ 0 ] );
		ShiftLeft();
		return ret;
	}
	T PopBack()
	{
		assert( data_size && "Array is empty." );
		if( !data_size ) throw std::out_of_range( "Array is empty." );

		T ret = std::move( data_ptr[ data_size - 1 ] );
		ResizeNoConstruct( data_size - 1 );
		return ret;
	}

	void Reserve( size_t new_capacity, size_t headroom = 0 )
	{
		if( data_capacity < new_capacity ) {
			new_capacity += headroom;
			auto alloc_size = new_capacity + 1;
			auto new_ptr = DoAllocate( data_ptr, data_size, alloc_size );
			if( !new_ptr ) return;
			data_ptr		= new_ptr;
			data_capacity	= new_capacity;
		}
	}

	void Resize( size_t new_size, size_t headroom = 0 )
	{
		auto old_size = data_size;
		ResizeNoConstruct( new_size, headroom );
		if( old_size < new_size ) {
			ConstructRange( old_size, new_size - old_size );
		}
	}

	[[nodiscard]] T & Front() const
	{
		assert( data_size && "Array is empty." );
		if( !data_size ) throw std::out_of_range( "Array is empty." );
		return data_ptr[ 0 ];
	}
	[[nodiscard]] T & Back() const
	{
		assert( data_size && "Array is empty." );
		if( !data_size ) throw std::out_of_range( "Array is empty." );
		return data_ptr[ data_size - 1 ];
	}
	[[nodiscard]] size_t size() const
	{
		return data_size;
	}
	[[nodiscard]] T * begin() const
	{
		return &data_ptr[ 0 ];
	}
	[[nodiscard]] T * end() const
	{
		return &data_ptr[ data_size ];
	}

private:
	void ConstructRange( size_t position, size_t count )
	{
		for( size_t i = position; i < position + count; ++i ) {
			new( &data_ptr[ i ] ) T();
		}
	}
	void DestructRange( size_t position, size_t count )
	{
		for( size_t i = position; i < position + count; ++i ) {
			data_ptr[ i ].~T();
		}
	}
	void ResizeNoConstruct( size_t new_size, size_t headroom = 0 )
	{
		auto old_size = data_size;
		Reserve( new_size, headroom );
		if( old_size > new_size ) {
			// Shrinking
			DestructRange( new_size, old_size - new_size );
		}
		data_size = new_size;
	}
	void ResizeNoConstructNoDestruct( size_t new_size, size_t headroom = 0 )
	{
		Reserve( new_size, headroom );
		data_size = new_size;
	}
	void CopyOther( const vk2d::Array<T> & other )
	{
		// Destruct everything in this array and copy construct from values on other array.
		if( this != &other ) {
			DestructRange( 0, data_size );
			auto new_size = other.data_size;
			ResizeNoConstructNoDestruct( new_size );
			for( size_t i = 0; i < new_size; ++i ) {
				new( &data_ptr[ i ] ) T( other.data_ptr[ i ] );
			}
		}
	}
	void SwapOther( vk2d::Array<T> && other )
	{
		if( this != &other ) {
			std::swap( data_ptr, other.data_ptr );
			std::swap( data_size, other.data_size );
			std::swap( data_capacity, other.data_capacity );
		}
	}
	// Shifts everything right, expands the array, the first element is destructed.
	void ShiftRight()
	{
		// TODO: Should combine shift and resize to the same copy-loop
		// if new memory is being allocated at the same time.
		ResizeNoConstruct( data_size + 1, DEFAULT_HEADROOM );
		if( data_size >= 2 ) {
			// Construct the last element from previous.
			if constexpr( std::is_move_constructible_v<T> ) {
				new( &data_ptr[ data_size - 1 ] ) T( std::move( data_ptr[ data_size - 2 ] ) );
			} else {
				new( &data_ptr[ data_size - 1 ] ) T( data_ptr[ data_size - 2 ] );
			}
			// For the rest we can assign.
			for( size_t i = data_size - 2; i > 0; --i ) {
				if constexpr( std::is_move_assignable_v<T> ) {
					data_ptr[ i ] = std::move( data_ptr[ i - 1 ] );
				} else {
					data_ptr[ i ] = data_ptr[ i - 1 ];
				}
			}
			// Destruct the first element.
			DestructRange( 0, 1 );
		}
	}
	// Shifts everything left, destructs the last element, shrinks the entire array.
	void ShiftLeft()
	{
		if( data_size > 0 ) {
			// Destruct first element.
			DestructRange( 0, 1 );
			// Construct the first element from next.
			if constexpr( std::is_move_constructible_v<T> ) {
				new( &data_ptr[ 0 ] ) T( std::move( data_ptr[ 1 ] ) );
			} else {
				new( &data_ptr[ 0 ] ) T( data_ptr[ 1 ] );
			}
			// For the rest we can assign.
			for( size_t i = 1; i < data_size; ++i ) {
				if constexpr( std::is_move_assignable_v<T> ) {
					data_ptr[ i ] = std::move( data_ptr[ i + 1 ] );
				} else {
					data_ptr[ i ] = data_ptr[ i + 1 ];
				}
			}
			ResizeNoConstruct( data_size - 1, DEFAULT_HEADROOM );
		}
	}
	T * DoAllocate( T * old_ptr, size_t old_size, size_t new_size )
	{
		// Can add memory pool allocation here.
		if constexpr( std::is_trivially_constructible_v<T> ) {
			// Trivially constructible stuff we can try and realloc the
			// memory, this can be a cheaper operation than malloc.
			T * new_ptr = reinterpret_cast<T*>( std::realloc( old_ptr, new_size * sizeof( T ) ) );
			if( !new_ptr ) throw std::bad_alloc();
			return new_ptr;
		} else {
			// If type is not trivially constructible then we need to
			// allocate a new block and copy to it as we need to
			// invoke move constructor of the objects.
			T * new_ptr = reinterpret_cast<T*>( std::malloc( new_size * sizeof( T ) ) );
			if( !new_ptr ) throw std::bad_alloc();
			if constexpr( std::is_move_constructible_v<T> ) {
				for( size_t i = 0; i < old_size; ++i ) {
					new ( &new_ptr[ i ] ) T( std::move( old_ptr[ i ] ) );
				}
			} else {
				for( size_t i = 0; i < old_size; ++i ) {
					new( &new_ptr[ i ] ) T( old_ptr[ i ] );
				}
			}
			DestructRange( 0, old_size );
			DoDeallocate( old_ptr );
			return new_ptr;
		}

	}
	void DoDeallocate( T * ptr )
	{
		// Can add memory pool deallocation here.
		std::free( ptr );
	}

	static constexpr	size_t		DEFAULT_HEADROOM		= 64;
	T							*	data_ptr				= {};
	size_t							data_size				= {};
	size_t							data_capacity			= {};
};



} // vk2d
