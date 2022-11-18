#pragma once

#include <core/Common.h>

#include <utility/ParameterPackTypeInfo.hpp>
#include <containers/Color.hpp>

#include <array>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		A collection of data which defines a single corner of a triangle, end of a line or a point in space.
///
///				A vertex is a single point in space defining different parameters for that location, like color and size of this
///				point <br>
///				A vertex is just a point in space but it can be connected to other points via the index list, which would then
///				form a triangle or a line between different vertices.
///
/// @tparam		MembersT
///				Template parameter list describing types of each field this vertex has, these fields will define members that
///				are used in the shader.
template<typename ...MembersT>
class VertexBase {
public:

	using IsVertexBase = std::true_type;

	constexpr VertexBase()
	{
		ConstructAllMembers<MembersT...>();
	}

	constexpr ~VertexBase()
	{
		DestructAllMembers<MembersT...>();
	}

	template<size_t Index>
	constexpr vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...> & Get()
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		size_t offset = GetMemberOffset<Index>();
		return *reinterpret_cast<vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...>*>( &data[ offset ] );
	}
	
	template<size_t Index>
	constexpr const vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...> & Get() const
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		size_t offset = GetMemberOffset<Index>();
		return *reinterpret_cast<vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...>*>( &data[ offset ] );
	}

	template<size_t Index>
	static consteval size_t GetMemberSize()
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		return GetMemberSizeImpl<Index, 0, MembersT...>();
	}

	template<size_t Index>
	static consteval size_t GetMemberOffset()
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		return GetMemberOffsetImpl<Index, 0, MembersT...>();
	}

	static consteval size_t GetMemberCount()
	{
		return sizeof...( MembersT );
	}

private:
	template<typename T>
	static constexpr size_t GetTypeAlignmentFromOffset( size_t current_offset )
	{
		return ( ( current_offset - 1 ) / alignof( T ) + 1 ) * alignof( T );
	}

	template<typename FirstT, typename ...RestT>
	constexpr void ConstructAllMembers( size_t offset = 0 )
	{
		offset = GetTypeAlignmentFromOffset<FirstT>( offset );
		new( reinterpret_cast<FirstT*>( &data[ offset ] ) ) FirstT();
		if constexpr( sizeof...( RestT ) > 0 )
		{
			ConstructAllMembers<RestT...>( offset + sizeof( FirstT ) );
		}
	}
	
	template<typename FirstT, typename ...RestT>
	constexpr void DestructAllMembers( size_t offset = 0 )
	{
		offset = GetTypeAlignmentFromOffset<FirstT>( offset );
		if constexpr( sizeof...( RestT ) > 0 )
		{
			DestructAllMembers<RestT...>( offset + sizeof( FirstT ) );
		}
		reinterpret_cast<FirstT*>( &data[ offset ] )->~FirstT();
	}

	template<size_t GetIndex, size_t CurrentIndex, typename FirstT, typename ...RestT>
	static consteval size_t GetMemberSizeImpl()
	{
		if constexpr( CurrentIndex < GetIndex )
		{
			return GetMemberSizeImpl<GetIndex, CurrentIndex + 1, RestT...>();
		}
		return sizeof( FirstT );
	}
	
	template<size_t GetIndex, size_t CurrentIndex, typename FirstT, typename ...RestT>
	static consteval size_t GetMemberOffsetImpl( size_t offset = 0 )
	{
		offset = GetTypeAlignmentFromOffset<FirstT>( offset );
		if constexpr( CurrentIndex < GetIndex )
		{
			return GetMemberOffsetImpl<GetIndex, CurrentIndex + 1, RestT...>( offset + sizeof( FirstT ) );
		}
		return offset;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateMyAlignment()
	{
		size_t alignment = alignof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 )
		{
			size_t other_alignment = CalculateMyAlignment<RestT...>();
			alignment = ( other_alignment > alignment ) ? other_alignment : alignment;
		}
		return alignment;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateMySize( size_t offset = 0 )
	{
		offset = GetTypeAlignmentFromOffset<FirstT>( offset );
		offset += sizeof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 ) {
			offset = CalculateMySize<RestT...>( offset );
		}
		return offset;
	}

	// std::tuple doesn't guarantee correct ordering of template pack types and may not
	// take into consideration the alignment so we're doing guaranteed layout manually.
	alignas( CalculateMyAlignment<MembersT...>() ) std::array<uint8_t, CalculateMySize<MembersT...>()> data;
};



namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Concepts to test a type is a vertex type or derived from it.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexBaseType = requires
{
	typename T::IsVertexBase;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexBaseDerivedType = requires
{
	typename T::Base;
	typename T::Base::IsVertexBase;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexBaseOrDerivedType =
VertexBaseType<T> ||
VertexBaseDerivedType<T>;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Concepts to test a vertex has the correct member variable types.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexSupportedVertexCoordinateTypes =
std::is_same_v<T, glm::vec2&> ||
std::is_same_v<T, glm::vec3&> ||
std::is_same_v<T, glm::vec4&>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexSupportedUVCoordinateTypes =
std::is_same_v<T, glm::vec2&>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexSupportedColorTypes =
std::is_same_v<T, Colorf&>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexSupportedPointSizeTypes =
std::is_same_v<T, float&>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexSupportedSingleTextureLayerTypes =
std::is_same_v<T, uint32_t&>;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Concepts to test if a vertex has any of the standard vertex fields.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasVertexCoords = requires( T vertex, auto field_value )
{
	vertex.vertex_coords	= glm::vec2();
	field_value				= vertex.vertex_coords;

	requires VertexSupportedVertexCoordinateTypes<decltype( T::vertex_coords )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasUVCoords = requires( T vertex, auto field_value )
{
	vertex.uv_coords	= glm::vec2();
	field_value			= vertex.uv_coords;

	requires VertexSupportedUVCoordinateTypes<decltype( T::uv_coords )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasColor = requires( T vertex, auto field_value )
{
	vertex.color		= Colorf();
	field_value			= vertex.color;

	requires VertexSupportedColorTypes<decltype( T::color )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasPointSize = requires( T vertex, auto field_value )
{
	vertex.point_size	= float();
	field_value			= vertex.point_size;

	requires VertexSupportedPointSizeTypes<decltype( T::point_size )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasSingleTextureLayer = requires( T vertex, auto field_value )
{
	vertex.single_texture_layer	= uint32_t();
	field_value					= vertex.single_texture_layer;

	requires VertexSupportedSingleTextureLayerTypes<decltype( T::single_texture_layer )>;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Evaluates to true if vertex is standard vertex or compatible.
///
/// @see		vk2d::StandardVertex
template<typename T>
concept VertexIsStandardVertexCompatible =
VertexHasVertexCoords<T> &&
VertexHasUVCoords<T> &&
VertexHasColor<T> &&
VertexHasPointSize<T> &&
VertexHasSingleTextureLayer<T>;



} // vk2d_internal
} // vk2d
