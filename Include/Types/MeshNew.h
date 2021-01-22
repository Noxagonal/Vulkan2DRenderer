#pragma once

#include "Core/Common.h"

#include "Types/MeshPrimitives.hpp"
#include "Types/Rect2.hpp"
#include "Types/Color.hpp"

#include <vector>
#include <string>

#include <iostream>

namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
}

class FontResource;
class Sampler;
class Texture;



namespace tr1 {



class MeshData
{
public:
	inline											MeshData(
		uint32_t									uv_count,
		uint32_t									color_count,
		uint32_t									user_data_per_vertex_byte_size,
		uint32_t									user_data_per_draw_byte_size,
		size_t										vertex_count					= 0,
		size_t										index_count						= 0
	) :
		uv_count( uv_count ),
		color_count( color_count ),
		user_data_per_vertex_byte_size( user_data_per_vertex_byte_size ),
		user_data_per_draw_byte_size( user_data_per_draw_byte_size ),
		vertex_count( uint32_t( vertex_count ) )
	{
		vertex_stride = sizeof( glm::vec2 ) +
			sizeof( glm::vec2 ) * uv_count +
			sizeof( vk2d::Colorf ) * color_count +
			user_data_per_vertex_byte_size;

		raw_vertex_data.resize( size_t( vertex_count ) * size_t( vertex_stride ) );
		indices.resize( index_count );
	}

	inline void										ResizeVertices(
		size_t										new_size
	)
	{
		raw_vertex_data.resize( size_t( new_size ) * size_t( vertex_stride ) );
		vertex_count = new_size;
	}

	inline void										ResizeIndices(
		size_t										new_size
	)
	{
		indices.resize( new_size );
	}

	inline void										AppendVertexRaw(
		void									*	ptr
	)
	{
		auto position = vertex_count * vertex_stride;
		ResizeVertices( vertex_count + 1 );
		std::memcpy( &raw_vertex_data[ position ], ptr, vertex_stride );
	}
	inline void										AppendIndices(
		vk2d::VertexIndex_2							line_indices
	)
	{
		indices.push_back( line_indices.indices[ 0 ] );
		indices.push_back( line_indices.indices[ 1 ] );
	}
	inline void										AppendIndices(
		vk2d::VertexIndex_3							triangle_indices
	)
	{
		indices.push_back( triangle_indices.indices[ 0 ] );
		indices.push_back( triangle_indices.indices[ 1 ] );
		indices.push_back( triangle_indices.indices[ 2 ] );
	}

	inline std::vector<uint8_t>					&	GetVertexRawData()
	{
		return raw_vertex_data;
	}
	inline void									*	GetVertexRaw(
		size_t										index
	)
	{
		assert( index < vertex_count );
		return raw_vertex_data.data() + index * vertex_stride;
	}
	inline uint32_t									GetVertexStride() const
	{
		return vertex_stride;
	}
	inline uint32_t									GetVertexRawDataCoordsPosition() const
	{
		return 0;
	}
	inline uint32_t									GetVertexRawDataUVsPosition() const
	{
		return GetVertexRawDataCoordsPosition() + sizeof( glm::vec2 );
	}
	inline uint32_t									GetVertexRawDataColorsPosition() const
	{
		return GetVertexRawDataUVsPosition() + sizeof( glm::vec2 ) * uv_count;
	}
	inline uint32_t									GetVertexRawDataUserDataPosition() const
	{
		return GetVertexRawDataColorsPosition() + sizeof( vk2d::Colorf ) * color_count;
	}
	inline uint32_t									GetVertexUVCount() const
	{
		return uv_count;
	}
	inline uint32_t									GetVertexColorCount() const
	{
		return color_count;
	}
	inline uint32_t									GetUserDataPerVertexByteSize() const
	{
		return user_data_per_vertex_byte_size;
	}
	inline uint32_t									GetUserDataPerDrawByteSize() const
	{
		return user_data_per_draw_byte_size;
	}
	inline std::vector<uint32_t>				&	GetIndices()
	{
		return indices;
	}

protected:
	vk2d::Rect2f									aabb							= {};

	uint32_t										uv_count						= {};
	uint32_t										color_count						= {};
	uint32_t										user_data_per_vertex_byte_size	= {};

	uint32_t										vertex_count					= {};
	uint32_t										vertex_stride					= {};

	std::vector<uint8_t>							raw_vertex_data					= {};
	std::vector<uint32_t>							indices							= {};

	std::vector<vk2d::Sampler*>						samplers						= {};
	std::vector<vk2d::Texture*>						textures						= {};

	uint32_t										user_data_per_draw_byte_size	= {};
	std::vector<uint8_t>							user_data_per_draw				= {};

	bool											generated						= false;							///< Tells if this mesh was created by a mesh generator.
	vk2d::MeshType									generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;	///< Tells the original mesh type if it was created by a mesh generator.
	vk2d::MeshType									mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;	///< Current mesh type.
};



namespace _internal {



// Data-type byte size with exception for void which returns 0 size.
template<typename T> static constexpr size_t TypeSizeVoid0 = sizeof( T );
template<> static constexpr size_t TypeSizeVoid0<void> = 0;



// Vertex expected byte size
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
static constexpr size_t VertexExpectedSize = sizeof( glm::vec2 ) + sizeof( glm::vec2 ) * UVCountPerVertex + sizeof( vk2d::Colorf ) * ColorCountPerVertex + vk2d::tr1::_internal::TypeSizeVoid0<UserDataPerVertexT>;



struct VertexInterfaceBase
{
	alignas( 4 ) glm::vec2										coords			= {};
};

template<size_t UVCountPerVertex>
struct VertexInterfaceUVs
{
	alignas( 4 ) std::array<glm::vec2, UVCountPerVertex>		uv_coords		= {};
};
template<> struct VertexInterfaceUVs<0> {};

template<size_t ColorCountPerVertex>
struct VertexInterfaceColors
{
	alignas( 4 ) std::array<vk2d::Colorf, ColorCountPerVertex>	colors			= {};
};
template<> struct VertexInterfaceColors<0> {};

template<typename UserDataPerVertexT>
struct VertexInterfaceUserData
{
	alignas( 4 ) UserDataPerVertexT								user_data		= {};
};
template<> struct VertexInterfaceUserData<void> {};



} // _internal



/*
// Annoyingly MSVC multiple inheritance empty base optimization is not working like other compilers, otherwise this code would be all that's needed.
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUVs<UVCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceColors<ColorCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceUserData<UserDataPerVertexT>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		// Relaying on empty base class optimization here... Consider moving to C++20 so we could try [[no_unique_address]]
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};
*/



template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUVs<UVCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceColors<ColorCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceUserData<UserDataPerVertexT>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex<0, ColorCountPerVertex, UserDataPerVertexT> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceColors<ColorCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceUserData<UserDataPerVertexT>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<0, ColorCountPerVertex, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex<UVCountPerVertex, 0, UserDataPerVertexT> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUVs<UVCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceUserData<UserDataPerVertexT>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<UVCountPerVertex, 0, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	typename	UserDataPerVertexT
>
struct Vertex<0, 0, UserDataPerVertexT> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUserData<UserDataPerVertexT>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<0, 0, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex
>
struct Vertex<UVCountPerVertex, ColorCountPerVertex, void> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUVs<UVCountPerVertex>,
	public vk2d::tr1::_internal::VertexInterfaceColors<ColorCountPerVertex>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<UVCountPerVertex, ColorCountPerVertex, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		ColorCountPerVertex
>
struct Vertex<0, ColorCountPerVertex, void> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceColors<ColorCountPerVertex>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<0, ColorCountPerVertex, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex
>
struct Vertex<UVCountPerVertex, 0, void> :
	public vk2d::tr1::_internal::VertexInterfaceBase,
	public vk2d::tr1::_internal::VertexInterfaceUVs<UVCountPerVertex>
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<UVCountPerVertex, 0, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<>
struct Vertex<0, 0, void> :
	public vk2d::tr1::_internal::VertexInterfaceBase
{
	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::tr1::_internal::VertexExpectedSize<0, 0, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};



namespace _internal {



class MeshInterfaceForUserDataPerVertexBase
{
protected:
	MeshData * ptr_to_self = {};
};

/// @brief		Mesh template base used to improve library user interface.
/// @tparam		UVCountPerVertex
///				Number of uv coordinates provided per vertex.
/// @tparam		ColorCountPerVertex
///				Number of colors provided per vertex.
/// @tparam		UserDataPerVertexT
///				User data type sent to shader for each vertex.
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT		= void
>
class MeshInterfaceForUserDataPerVertex :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<glm::vec2>					uv_coordinates,
		std::initializer_list<vk2d::Colorf>					vertex_colors,
		const UserDataPerVertexT						&	user_data
	)
	{
		assert( std::size( uv_coordinates ) == UVCountPerVertex && "uv_coordinates size must match amount of vertex uv coordinates" );
		assert( std::size( vertex_colors ) == ColorCountPerVertex && "vertex_colors size must match amount of vertex colors" );

		Vertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( uv_coordinates ), std::end( uv_coordinates ), std::begin( vertex.uv_coords ) );
		std::copy( std::begin( vertex_colors ), std::end( vertex_colors ), std::begin( vertex.colors ) );
		vertex.user_data	= user_data;
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for UV count 0.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
class MeshInterfaceForUserDataPerVertex<0, ColorCountPerVertex, UserDataPerVertexT> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<vk2d::Colorf>					vertex_colors,
		const UserDataPerVertexT						&	user_data
	)
	{
		assert( std::size( vertex_colors ) == ColorCountPerVertex && "vertex_colors size must match amount of vertex colors" );

		Vertex<0, ColorCountPerVertex, UserDataPerVertexT> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( vertex_colors ), std::end( vertex_colors ), std::begin( vertex.colors ) );
		vertex.user_data	= user_data;
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for color count 0.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	size_t		UVCountPerVertex,
	typename	UserDataPerVertexT
>
class MeshInterfaceForUserDataPerVertex<UVCountPerVertex, 0, UserDataPerVertexT> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<glm::vec2>					uv_coordinates,
		const UserDataPerVertexT						&	user_data
	)
	{
		assert( std::size( uv_coordinates ) == UVCountPerVertex && "uv_coordinates size must match amount of vertex uv coordinates" );

		Vertex<UVCountPerVertex, 0, UserDataPerVertexT> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( uv_coordinates ), std::end( uv_coordinates ), std::begin( vertex.uv_coords ) );
		vertex.user_data	= user_data;
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for UV and color count 0.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	typename	UserDataPerVertexT
>
class MeshInterfaceForUserDataPerVertex<0, 0, UserDataPerVertexT> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		const UserDataPerVertexT						&	user_data
	)
	{
		Vertex<0, 0, UserDataPerVertexT> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		vertex.user_data	= user_data;
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for user data per vertex = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex
>
class MeshInterfaceForUserDataPerVertex<UVCountPerVertex, ColorCountPerVertex, void> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<glm::vec2>					uv_coordinates,
		std::initializer_list<vk2d::Colorf>					vertex_colors
	)
	{
		assert( std::size( uv_coordinates ) == UVCountPerVertex && "uv_coordinates size must match amount of vertex uv coordinates" );
		assert( std::size( vertex_colors ) == ColorCountPerVertex && "vertex_colors size must match amount of vertex colors" );

		Vertex<UVCountPerVertex, ColorCountPerVertex, void> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( uv_coordinates ), std::end( uv_coordinates ), std::begin( vertex.uv_coords ) );
		std::copy( std::begin( vertex_colors ), std::end( vertex_colors ), std::begin( vertex.colors ) );
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for UV count 0, user data per vertex = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	size_t		ColorCountPerVertex
>
class MeshInterfaceForUserDataPerVertex<0, ColorCountPerVertex, void> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<vk2d::Colorf>					vertex_colors
	)
	{
		assert( std::size( vertex_colors ) == ColorCountPerVertex && "vertex_colors size must match amount of vertex colors" );

		Vertex<0, ColorCountPerVertex, void> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( vertex_colors ), std::end( vertex_colors ), std::begin( vertex.colors ) );
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for color count 0, user data per vertex = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<
	size_t		UVCountPerVertex
>
class MeshInterfaceForUserDataPerVertex<UVCountPerVertex, 0, void> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates,
		std::initializer_list<glm::vec2>					uv_coordinates
	)
	{
		assert( std::size( uv_coordinates ) == UVCountPerVertex && "uv_coordinates size must match amount of vertex uv coordinates" );

		Vertex<UVCountPerVertex, 0, void> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		std::copy( std::begin( uv_coordinates ), std::end( uv_coordinates ), std::begin( vertex.uv_coords ) );
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};

/// @brief		Mesh specialization for UV and color count 0, user data per vertex = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerVertex
template<>
class MeshInterfaceForUserDataPerVertex<0, 0, void> :
	protected MeshInterfaceForUserDataPerVertexBase
{
public:
	MeshInterfaceForUserDataPerVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

	inline void												AppendVertex(
		const glm::vec2									&	vertex_coordinates
	)
	{
		Vertex<0, 0, void> vertex {};
		assert( sizeof( vertex ) == size_t( ptr_to_self->GetVertexStride() ) );

		vertex.coords		= vertex_coordinates;
		ptr_to_self->AppendVertexRaw( &vertex );
	}
};



/// @brief		Mesh template base used to improve library user interface.
/// @tparam		UserDataPerDrawT
///				User data type sent to shader per draw call.
template<typename UserDataPerDrawT>
class MeshInterfaceForUserDataPerDraw
{

};

/// @brief		Mesh specialization for user data per draw = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerDraw
template<>
class MeshInterfaceForUserDataPerDraw<void>
{

};



} // _internal



/// @brief		A mesh represents a collection of vertices and their relations
///				forming a shape that can be drawn onto a surface.
/// 
///				- A mesh can be created using any of the mesh generator functions,
///				or constructing it manually.
///				- A mesh can be modified using any of the modifier functions or
///				by manually modifying the vertices and indices.
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT		= void,
	typename	UserDataPerDrawT		= void
>
class Mesh :
	public vk2d::tr1::MeshData,
	public vk2d::tr1::_internal::MeshInterfaceForUserDataPerVertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>,
	public vk2d::tr1::_internal::MeshInterfaceForUserDataPerDraw<UserDataPerDrawT>
{
	static_assert( std::is_void_v<UserDataPerVertexT> || std::is_pod_v<UserDataPerVertexT>, "User data per vertex must be plain-old-data struct." );
	static_assert( std::is_void_v<UserDataPerVertexT> || std::is_trivially_copyable_v<UserDataPerVertexT>, "User data per vertex must be trivially constructible." );
	static_assert( std::is_void_v<UserDataPerDrawT> || std::is_pod_v<UserDataPerDrawT>, "User data per draw must be plain-old-data struct." );
	static_assert( std::is_void_v<UserDataPerDrawT> || std::is_trivially_copyable_v<UserDataPerDrawT>, "User data per draw must be trivially constructible." );

	using VertexType = Vertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>;
public:

	inline Mesh(
		size_t	initial_vertex_count	= 0,
		size_t	initial_index_count		= 0
	) :
		vk2d::tr1::MeshData(
			UVCountPerVertex,
			ColorCountPerVertex,
			vk2d::tr1::_internal::TypeSizeVoid0<UserDataPerVertexT>,
			vk2d::tr1::_internal::TypeSizeVoid0<UserDataPerDrawT>,
			initial_vertex_count,
			initial_index_count
		),
		vk2d::tr1::_internal::MeshInterfaceForUserDataPerVertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>( this )
	{}

	inline VertexType & GetVertex( size_t index )
	{
		assert( index < vertex_count && "Out of range." );
		return *reinterpret_cast<VertexType*>( raw_vertex_data.data() + vertex_stride * index );
	}

	inline VertexType & operator[]( size_t index )
	{
		return GetVertex( index );
	}
};



} // t1



} // vk2d
