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



namespace p1 {



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
	inline uint32_t									GetVertexRawDataStride() const
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
	inline uint32_t									GetUserDataPerVertexRawSize() const
	{
		return user_data_per_vertex_byte_size;
	}
	inline void										SetUserDataPerDrawRaw(
		void									*	user_data_raw,
		uint32_t									user_data_byte_size )
	{
		// TODO: Might be a good idea to move per draw data into when we're actually drawing the mesh.
		// This would allow reusing mesh data with different per draw user data per instance.
		user_data_per_draw.resize( user_data_byte_size );
		std::memcpy( user_data_per_draw.data(), user_data_raw, user_data_byte_size );
	}
	inline const std::vector<uint8_t>			&	GetUserDataPerDrawRawData() const
	{
		return user_data_per_draw;
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

	std::vector<uint8_t>							user_data_per_draw				= {};

	bool											generated						= false;							///< Tells if this mesh was created by a mesh generator.
	vk2d::MeshType									generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;	///< Tells the original mesh type if it was created by a mesh generator.
	vk2d::MeshType									mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;	///< Current mesh type.
};



namespace _internal {



/// @brief		Gets the expected vertex byte for specific template parameters.
///				Used for compile-time error checking.
/// @tparam		UVCountPerVertex
///				Number of UV coordinates per vertex.
/// @tparam		ColorCountPerVertex
///				Number of colors per vertex.
/// @tparam		UserDataPerVertexT
///				User data type per vertex.
template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
static constexpr size_t ExpectedVertexSize = sizeof( glm::vec2 ) + sizeof( glm::vec2 ) * UVCountPerVertex + sizeof( vk2d::Colorf ) * ColorCountPerVertex + vk2d::TypeSizeVoid0<UserDataPerVertexT>;



} // _internal



template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<glm::vec2, UVCountPerVertex>		uv_coords		= {};
	alignas( 4 ) std::array<vk2d::Colorf, ColorCountPerVertex>	colors			= {};
	alignas( 4 ) UserDataPerVertexT								user_data		= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		ColorCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex<0, ColorCountPerVertex, UserDataPerVertexT>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<vk2d::Colorf, ColorCountPerVertex>	colors			= {};
	alignas( 4 ) UserDataPerVertexT								user_data		= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<0, ColorCountPerVertex, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex,
	typename	UserDataPerVertexT
>
struct Vertex<UVCountPerVertex, 0, UserDataPerVertexT>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<glm::vec2, UVCountPerVertex>		uv_coords		= {};
	alignas( 4 ) UserDataPerVertexT								user_data		= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<UVCountPerVertex, 0, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	typename	UserDataPerVertexT
>
struct Vertex<0, 0, UserDataPerVertexT>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) UserDataPerVertexT								user_data		= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<0, 0, UserDataPerVertexT>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex,
	size_t		ColorCountPerVertex
>
struct Vertex<UVCountPerVertex, ColorCountPerVertex, void>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<glm::vec2, UVCountPerVertex>		uv_coords		= {};
	alignas( 4 ) std::array<vk2d::Colorf, ColorCountPerVertex>	colors			= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<UVCountPerVertex, ColorCountPerVertex, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		ColorCountPerVertex
>
struct Vertex<0, ColorCountPerVertex, void>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<vk2d::Colorf, ColorCountPerVertex>	colors			= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<0, ColorCountPerVertex, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<
	size_t		UVCountPerVertex
>
struct Vertex<UVCountPerVertex, 0, void>
{
	alignas( 4 ) glm::vec2										coords			= {};
	alignas( 4 ) std::array<glm::vec2, UVCountPerVertex>		uv_coords		= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<UVCountPerVertex, 0, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};

template<>
struct Vertex<0, 0, void>
{
	alignas( 4 ) glm::vec2										coords			= {};

	#if VK2D_DEBUG_ENABLE
	Vertex()
	{
		static_assert( sizeof( *this ) ==
			vk2d::p1::_internal::ExpectedVertexSize<0, 0, void>,
			"Internal error, Vertex size does not match expected." );
	}
	#endif
};



namespace _internal {



class MeshInterfaceForVertexBase
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
class MeshInterfaceForVertex :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<0, ColorCountPerVertex, UserDataPerVertexT> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<UVCountPerVertex, 0, UserDataPerVertexT> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<0, 0, UserDataPerVertexT> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<UVCountPerVertex, ColorCountPerVertex, void> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<0, ColorCountPerVertex, void> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<UVCountPerVertex, 0, void> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
class MeshInterfaceForVertex<0, 0, void> :
	protected MeshInterfaceForVertexBase
{
public:
	MeshInterfaceForVertex( MeshData * ptr_to_self ) { this->ptr_to_self = ptr_to_self; };

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
public:
	MeshInterfaceForUserDataPerDraw( MeshData * ptr_to_self ) : ptr_to_self( ptr_to_self ) {};

	void SetUserDataPerDraw( const UserDataPerDrawT & user_data )
	{
		
	}

private:
	vk2d::p1::MeshData		*	ptr_to_self = nullptr;
};

/// @brief		Mesh specialization for user data per draw = void.
/// @see		vk2d::_internal::MeshInterfaceForUserDataPerDraw
template<>
class MeshInterfaceForUserDataPerDraw<void>
{
public:
	MeshInterfaceForUserDataPerDraw( MeshData * ptr_to_self ) {};
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
	public vk2d::p1::MeshData,
	public vk2d::p1::_internal::MeshInterfaceForVertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>,
	public vk2d::p1::_internal::MeshInterfaceForUserDataPerDraw<UserDataPerDrawT>
{
	static_assert( std::is_void_v<UserDataPerVertexT> || std::is_pod_v<UserDataPerVertexT>, "User data per vertex must be plain-old-data struct." );
	static_assert( std::is_void_v<UserDataPerVertexT> || std::is_trivially_copyable_v<UserDataPerVertexT>, "User data per vertex must be trivially constructible." );
	static_assert( std::is_void_v<UserDataPerVertexT> || vk2d::TypeSizeVoid0<UserDataPerVertexT> % 4 == 0, "User data per vertex size must be integer multiple of 4 bits." );
	static_assert( std::is_void_v<UserDataPerDrawT> || std::is_pod_v<UserDataPerDrawT>, "User data per draw must be plain-old-data struct." );
	static_assert( std::is_void_v<UserDataPerDrawT> || std::is_trivially_copyable_v<UserDataPerDrawT>, "User data per draw must be trivially constructible." );
	static_assert( std::is_void_v<UserDataPerDrawT> || vk2d::TypeSizeVoid0<UserDataPerVertexT> % 4 == 0, "User data per draw size must be integer multiple of 4 bits." );

public:
	using VertexType = Vertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>;

	inline Mesh(
		size_t	initial_vertex_count	= 0,
		size_t	initial_index_count		= 0
	) :
		vk2d::p1::MeshData(
			UVCountPerVertex,
			ColorCountPerVertex,
			vk2d::TypeSizeVoid0<UserDataPerVertexT>,
			vk2d::TypeSizeVoid0<UserDataPerDrawT>,
			initial_vertex_count,
			initial_index_count
		),
		vk2d::p1::_internal::MeshInterfaceForVertex<UVCountPerVertex, ColorCountPerVertex, UserDataPerVertexT>( this )
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
