#pragma once

#include <core/Common.h>

#include "VertexDescriptor.hpp"
#include "VertexBase.hpp"
#include "StandardVertex.hpp"

#include <vector>
#include <span>



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RawVertexData
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RawVertexData() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<VertexBaseOrDerivedType VertexT>
	explicit RawVertexData(
		std::span<const VertexT> vertices
	)
	{
		vertex_descriptor	= GetVertexDescriptorFromVertexType<VertexT>();
		vertex_count		= vertices.size();
		vertex_stride		= vertex_descriptor.size;
		assert( ( vertex_stride % vertex_descriptor.size ) == 0 );

		vertex_data.resize( vertices.size() * vertex_stride, 0 );

		size_t offset = 0;
		for( auto & v : vertices ) {
			CopyMembers( offset, v );
			offset += vertex_stride;
		}
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<
		VertexBaseOrDerivedType		VertexT,
		size_t						CurrentIndex = 0
	>
	constexpr void					CopyMembers(
		size_t						current_offset,
		const VertexT			&	vertex
	)
	{
		static_assert( VertexT::GetMemberCount() > 0, "Vertex must have at least one member" );

		if constexpr( CurrentIndex < VertexT::GetMemberCount() )
		{
			assert( current_offset < vertex_data.size() );
			CopyMemberData( current_offset + VertexT::template GetMemberOffset<CurrentIndex>(), vertex.Get<CurrentIndex>());
			CopyMembers<VertexT, CurrentIndex + 1>( current_offset, vertex );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<
		typename					MemberT
	>
	constexpr void					CopyMemberData(
		size_t						current_offset,
		const MemberT			&	data
	)
	{
		*reinterpret_cast<MemberT*>( &vertex_data[ current_offset ] ) = data;
	}

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VertexDescriptor					vertex_descriptor;
	size_t								vertex_count				= {};
	size_t								vertex_stride				= {};
	std::vector<uint8_t>				vertex_data;
};



} // vk2d_internal
} // vk2d
