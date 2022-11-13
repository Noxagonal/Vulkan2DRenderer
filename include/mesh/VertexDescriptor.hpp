#pragma once

#include <core/Common.h>

#include <utility/EnumHelpers.hpp>

#include <vector>
#include <type_traits>



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file handles rudimentary reflection for VertexBase.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Determines the type of the vertex member field.
enum class VertexDescriptorMemberTypeFlags : uint32_t
{
	UNKNOWN					= 0,

	VERTEX_COORDS			= 1 << 0,
	UV_COORDS				= 1 << 1,
	COLOR					= 1 << 2,
	POINT_SIZE				= 1 << 3,
	SINGLE_TEXTURE_INDEX	= 1 << 4
};
constexpr VertexDescriptorMemberTypeFlags operator|( VertexDescriptorMemberTypeFlags first, VertexDescriptorMemberTypeFlags second )
{
	return EnumBitwiseOr( first, second );
}
constexpr VertexDescriptorMemberTypeFlags & operator|=( VertexDescriptorMemberTypeFlags & first, VertexDescriptorMemberTypeFlags second )
{
	return first = EnumBitwiseOr( first, second );
}

constexpr VertexDescriptorMemberTypeFlags operator&( VertexDescriptorMemberTypeFlags first, VertexDescriptorMemberTypeFlags second )
{
	return EnumBitwiseAnd( first, second );
}
constexpr VertexDescriptorMemberTypeFlags & operator&=( VertexDescriptorMemberTypeFlags & first, VertexDescriptorMemberTypeFlags second )
{
	return first = EnumBitwiseOr( first, second );
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexMemberDescriptor
{
	VertexDescriptorMemberTypeFlags			type						= {};
	size_t									offset						= {};
	size_t									size						= {};
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Provides vertex member field info when C++ type system isn't available anymore.
struct VertexDescriptor
{
	VertexDescriptorMemberTypeFlags			member_flags				= {};
	std::vector<VertexMemberDescriptor>		member_descriptors;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Get only the standard member types from the vertex.
///
/// @tparam		VertexT
///				Vertex type.
///
/// @return		Vertex descriptor.
template<VertexBaseDerivedType VertexT>
constexpr VertexDescriptor GetVertexDescriptorStandardMembersFromVertexType()
{
	auto ret = VertexDescriptor();
	auto temp_vertex = VertexT();

	auto VertexMemberOffset = [ &temp_vertex ]( auto member_variable ) -> size_t
	{
		return reinterpret_cast<const uint8_t*>( &member_variable ) - reinterpret_cast<const uint8_t*>( &temp_vertex );
	};

	auto VertexMemberSize = []( auto member_variable ) -> size_t
	{
		return sizeof( std::remove_reference_t<decltype( member_variable )> );
	};

	if constexpr( VertexHasVertexCoords<VertexT> )
	{
		auto description_flag		= VertexDescriptorMemberTypeFlags::VERTEX_COORDS;
		auto member_description		= VertexMemberDescriptor();
		member_description.type		= description_flag;
		member_description.offset	= VertexMemberOffset( temp_vertex.vertex_coords );
		member_description.size		= VertexMemberSize( temp_vertex.vertex_coords );

		ret.member_flags |= description_flag;
		ret.member_descriptors.push_back( member_description );
	}

	if constexpr( VertexHasUVCoords<VertexT> )
	{
		auto description_flag		= VertexDescriptorMemberTypeFlags::UV_COORDS;
		auto member_description		= VertexMemberDescriptor();
		member_description.type		= description_flag;
		member_description.offset	= VertexMemberOffset( temp_vertex.uv_coords );
		member_description.size		= VertexMemberSize( temp_vertex.uv_coords );

		ret.member_flags |= description_flag;
		ret.member_descriptors.push_back( member_description );
	}

	if constexpr( VertexHasColor<VertexT> )
	{
		auto description_flag		= VertexDescriptorMemberTypeFlags::COLOR;
		auto member_description		= VertexMemberDescriptor();
		member_description.type		= description_flag;
		member_description.offset	= VertexMemberOffset( temp_vertex.color );
		member_description.size		= VertexMemberSize( temp_vertex.color );

		ret.member_flags |= description_flag;
		ret.member_descriptors.push_back( member_description );
	}

	if constexpr( VertexHasPointSize<VertexT> )
	{
		auto description_flag		= VertexDescriptorMemberTypeFlags::POINT_SIZE;
		auto member_description		= VertexMemberDescriptor();
		member_description.type		= description_flag;
		member_description.offset	= VertexMemberOffset( temp_vertex.point_size );
		member_description.size		= VertexMemberSize( temp_vertex.point_size );

		ret.member_flags |= description_flag;
		ret.member_descriptors.push_back( member_description );
	}

	if constexpr( VertexHasSingleTextureLayer<VertexT> )
	{
		auto description_flag		= VertexDescriptorMemberTypeFlags::SINGLE_TEXTURE_INDEX;
		auto member_description		= VertexMemberDescriptor();
		member_description.type		= description_flag;
		member_description.offset	= VertexMemberOffset( temp_vertex.single_texture_layer );
		member_description.size		= VertexMemberSize( temp_vertex.single_texture_layer );

		ret.member_flags |= description_flag;
		ret.member_descriptors.push_back( member_description );
	}
}



namespace detail {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<VertexBaseDerivedType VertexT, size_t CurrentIndex = 0>
constexpr void GetVertexDescriptorFromVertexType_Extract(
	const VertexT			&	temp_vertex,
	const VertexDescriptor	&	standard_types,
	VertexDescriptor		&	out
)
{
	constexpr auto member_count		= VertexT::GetMemberCount();
	const auto & member				= temp_vertex.Get<CurrentIndex>();
	const auto member_offset		= reinterpret_cast<const uint8_t*>( &member ) - reinterpret_cast<const uint8_t*>( &temp_vertex );
	const auto member_size			= sizeof( std::remove_reference_t<decltype( member )> );

	auto member_descriptor			= VertexMemberDescriptor();
	member_descriptor.offset		= member_offset;
	member_descriptor.size			= member_size;
	std::ranges::for_each( standard_types.member_descriptors, [&member_descriptor]( VertexMemberDescriptor o )
		{
			if( o.offset == member_descriptor.offset )
			{
				member_descriptor.type = o.type;
			}
		}
	);
	out.member_descriptors.push_back( member_descriptor );

	if( CurrentIndex < VertexT::GetMemberCount() )
	{
		GetVertexDescriptorFromVertexType_Extract<VertexT, CurrentIndex + 1>(
			temp_vertex,
			standard_types,
			out
		);
	}
}



} // detail



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Get vertex descriptor struct from VertexBase derived type.
///
/// @tparam		VertexT
///				Vertex type.
///
/// @return		Vertex descriptor.
template<VertexBaseDerivedType VertexT>
constexpr VertexDescriptor GetVertexDescriptorFromVertexType()
{
	// Idea here is to first see what standard members there are in the vertex, then populate all members in order.
	// We use the standard_members to define type information to the complete list.

	const auto standard_members = GetVertexDescriptorStandardMembersFromVertexType<VertexT>();

	auto temp_vertex = VertexT();
	auto ret = VertexDescriptor();
	detail::GetVertexDescriptorFromVertexType_Extract<VertexT>(
		temp_vertex,
		standard_members,
		ret
	);

	return ret;
}



} // vk2d_internal
} // vk2d
