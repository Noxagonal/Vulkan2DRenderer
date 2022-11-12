#pragma once

#include <core/Common.h>

#include <utility/ParameterPackTypeInfo.hpp>

#include <array>



namespace vk2d {



template<typename ...ArgsT>
class VertexBase {
public:

	using IsVertexBase = std::true_type;

	constexpr VertexBase()
	{
		InitAllMembers<ArgsT...>();
	}

	constexpr ~VertexBase()
	{
		DeinitAllMembers<ArgsT...>();
	}

	template<size_t Index>
	constexpr vk2d_internal::GetTemplateTypeAtIndex<Index, ArgsT...> & Get()
	{
		static_assert( Index < sizeof...( ArgsT ), "Index out of range." );
		return *reinterpret_cast<vk2d_internal::GetTemplateTypeAtIndex<Index, ArgsT...>*>( &data[ GetOffsetFor<Index, 0, ArgsT...>() ] );
	}

private:
	template<typename T>
	static constexpr size_t GetMemberOffset( size_t current_offset )
	{
		return ( ( current_offset - 1 ) / alignof( T ) + 1 ) * alignof( T );
	}

	template<typename FirstT, typename ...RestT>
	constexpr void InitAllMembers( size_t offset = 0 )
	{
		offset = GetMemberOffset<FirstT>( offset );
		new( reinterpret_cast<FirstT*>( &data[ offset ] ) ) FirstT();
		if constexpr( sizeof...( RestT ) > 0 ) {
			InitAllMembers<RestT...>( offset + sizeof( FirstT ) );
		}
	}
	
	template<typename FirstT, typename ...RestT>
	constexpr void DeinitAllMembers( size_t offset = 0 )
	{
		offset = GetMemberOffset<FirstT>( offset );
		if constexpr( sizeof...( RestT ) > 0 ) {
			DeinitAllMembers<RestT...>( offset + sizeof( FirstT ) );
		}
		reinterpret_cast<FirstT*>( &data[ offset ] )->~FirstT();
		data[ offset ] = 0xDD;
	}

	template<size_t GetIndex, size_t CurrentIndex, typename FirstT, typename ...RestT>
	static consteval size_t GetOffsetFor( size_t offset = 0 )
	{
		offset = GetMemberOffset<FirstT>( offset );
		if constexpr( CurrentIndex < GetIndex ) {
			return GetOffsetFor<GetIndex, CurrentIndex + 1, RestT...>( offset + sizeof( FirstT ) );
		}
		return offset;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateMyAlignment()
	{
		size_t alignment = alignof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 ) {
			size_t other_alignment = CalculateMyAlignment<RestT...>();
			alignment = ( other_alignment > alignment ) ? other_alignment : alignment;
		}
		return alignment;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateMySize( size_t offset = 0 )
	{
		offset = GetMemberOffset<FirstT>( offset );
		offset += sizeof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 ) {
			offset = CalculateMySize<RestT...>( offset );
		}
		return offset;
	}

	// std::tuple doesn't guarantee correct ordering of template pack types and may not
	// take into consideration the alignment so we're doing guaranteed layout manually.
	alignas( CalculateMyAlignment<ArgsT...>() ) std::array<uint8_t, CalculateMySize<ArgsT...>()> data;
};



namespace vk2d_internal {



template<typename T>
concept IsVertexBaseDerived = requires
{
	typename T::Base;
	typename T::Base::IsVertexBase;
};



} // vk2d_internal
} // vk2d
