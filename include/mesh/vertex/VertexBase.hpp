#pragma once

#include <core/Common.hpp>

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
		ConstructAllMembers<0, MembersT...>();
	}

	~VertexBase()
	{
		DestructAllMembers<0, MembersT...>();
	}

	VertexBase & operator=( const VertexBase & other ) = default;

	template<size_t Index>
	constexpr vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...> & Get()
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		constexpr size_t offset = GetMemberOffset<Index>();
		return *reinterpret_cast<vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...>*>( &data[ offset ] );
	}

	template<size_t Index>
	constexpr const vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...> & Get() const
	{
		static_assert( Index < sizeof...( MembersT ), "Index out of range" );
		constexpr size_t offset = GetMemberOffset<Index>();
		return *reinterpret_cast<const vk2d_internal::GetTemplateTypeAtIndex<Index, MembersT...>*>( &data[ offset ] );
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
		return GetMemberOffsetImpl<0, Index, 0, MembersT...>();
	}

	static consteval size_t GetMemberCount()
	{
		return sizeof...( MembersT );
	}

	static consteval size_t GetBaseSize()
	{
		return GetBaseSizeImpl<0, MembersT...>();
	}

	static consteval size_t GetBaseAlignment()
	{
		return std::max( GetBaseAlignmentImpl<MembersT...>(), size_t( 16 ) );
	}

private:
	template<size_t CurrentOffset, typename T>
	static consteval size_t GetAlignmentForType()
	{
		return AlignOffset<CurrentOffset, alignof( T )>();
	}

	template<size_t CurrentOffset, size_t AlignmentRequirement>
	static consteval size_t AlignOffset()
	{
		return ( ( CurrentOffset - 1 ) / AlignmentRequirement + 1 ) * AlignmentRequirement;
	}

	template<size_t CurrentOffset, typename FirstT, typename ...RestT>
	constexpr void ConstructAllMembers()
	{
		constexpr size_t new_offset = GetAlignmentForType<CurrentOffset, FirstT>();
		new( reinterpret_cast<FirstT*>( &data[ new_offset ] ) ) FirstT();
		if constexpr( sizeof...( RestT ) > 0 )
		{
			ConstructAllMembers<new_offset + sizeof( FirstT ), RestT...>();
		}
	}

	template<size_t CurrentOffset, typename FirstT, typename ...RestT>
	constexpr void DestructAllMembers()
	{
		constexpr size_t new_offset = GetAlignmentForType<CurrentOffset, FirstT>();
		if constexpr( sizeof...( RestT ) > 0 )
		{
			DestructAllMembers<new_offset + sizeof( FirstT ), RestT...>();
		}
		reinterpret_cast<FirstT*>( &data[ new_offset ] )->~FirstT();
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

	template<size_t CurrentOffset, size_t GetIndex, size_t CurrentIndex, typename FirstT, typename ...RestT>
	static consteval size_t GetMemberOffsetImpl()
	{
		constexpr size_t new_offset = GetAlignmentForType<CurrentOffset, FirstT>();
		if constexpr( CurrentIndex < GetIndex )
		{
			return GetMemberOffsetImpl<new_offset + sizeof( FirstT ), GetIndex, CurrentIndex + 1, RestT...>();
		}
		return new_offset;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t GetBaseAlignmentImpl()
	{
		constexpr size_t alignment = alignof( FirstT );
		if constexpr( sizeof...( RestT ) )
		{
			return std::max<size_t>( GetBaseAlignmentImpl<RestT...>(), alignment );
		}
		return alignment;
	}

	template<size_t CurrentOffset, typename FirstT, typename ...RestT>
	static consteval size_t GetBaseSizeImpl()
	{
		constexpr size_t new_offset = GetAlignmentForType<CurrentOffset, FirstT>();
		if constexpr( sizeof...( RestT ) > 0 ) {
			return GetBaseSizeImpl<new_offset + sizeof( FirstT ), RestT...>();
		}
		return AlignOffset<new_offset + sizeof( FirstT ), GetBaseAlignment()>();
	}

	// std::tuple doesn't guarantee correct ordering of template pack types and may not
	// take into consideration the alignment so we're doing guaranteed layout manually.
	alignas( GetBaseAlignment() ) std::array<uint8_t, GetBaseSize()> data;
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
concept VertexHasVertexCoords = requires( T vertex, decltype( T::vertex_coords ) field_value )
{
	vertex.vertex_coords	= glm::vec2();
	field_value				= vertex.vertex_coords;

	requires VertexSupportedVertexCoordinateTypes<decltype( T::vertex_coords )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasUVCoords = requires( T vertex, decltype( T::uv_coords ) field_value )
{
	vertex.uv_coords	= glm::vec2();
	field_value			= vertex.uv_coords;

	requires VertexSupportedUVCoordinateTypes<decltype( T::uv_coords )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasColor = requires( T vertex, decltype( T::color ) field_value )
{
	vertex.color		= Colorf();
	field_value			= vertex.color;

	requires VertexSupportedColorTypes<decltype( T::color )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasPointSize = requires( T vertex, decltype( T::point_size ) field_value )
{
	vertex.point_size	= float();
	field_value			= vertex.point_size;

	requires VertexSupportedPointSizeTypes<decltype( T::point_size )>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
concept VertexHasSingleTextureLayer = requires( T vertex, decltype( T::single_texture_layer ) field_value )
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
