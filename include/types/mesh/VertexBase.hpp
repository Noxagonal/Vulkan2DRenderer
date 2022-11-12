#pragma once

#include <core/Common.h>

#include <utility/ParameterPackTypeInfo.hpp>

#include <array>



namespace vk2d {
namespace vk2d_internal {
namespace details {



} // details



template<typename ...ArgsT>
class VertexBase {
public:

	template<size_t Index>
	GetTemplateTypeAtIndex<Index, ArgsT...> & Get()
	{
		static_assert( Index < sizeof...( ArgsT ), "Index out of range." );
		return *reinterpret_cast<GetTemplateTypeAtIndex<Index, ArgsT...>*>( &data[ GetOffset<Index, 0, ArgsT...>() ] );
	}

// private:
	template<size_t GetIndex, size_t CurrentIndex, typename FirstT, typename ...RestT>
	static consteval size_t GetOffset( size_t offset = 0 )
	{
		offset = ( ( offset - 1 ) / alignof( FirstT ) + 1 ) * alignof( FirstT );
		if constexpr( CurrentIndex < GetIndex ) {
			return GetOffset<GetIndex, CurrentIndex + 1, RestT...>( offset + sizeof( FirstT ) );
		}
		return offset;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateAlignment()
	{
		size_t alignment = alignof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 ) {
			size_t other_alignment = CalculateAlignment<RestT...>();
			alignment = ( other_alignment > alignment ) ? other_alignment : alignment;
		}
		return alignment;
	}

	template<typename FirstT, typename ...RestT>
	static consteval size_t CalculateSize( size_t offset = 0 )
	{
		offset = ( ( offset - 1 ) / alignof( FirstT ) + 1 ) * alignof( FirstT );
		offset += sizeof( FirstT );
		if constexpr( sizeof...( RestT ) > 0 ) {
			offset = CalculateSize<RestT...>( offset );
		}
		return offset;
	}

	// std::tuple doesn't guarantee correct ordering of template pack types and may not
	// take into consideration the alignment so we're doing guaranteed layout manually.
	alignas( CalculateAlignment<ArgsT...>() ) std::array<uint8_t, CalculateSize<ArgsT...>()> data;
};



// TEST!
class StandardVertex : public VertexBase<glm::vec2, glm::vec2, float> {
public:
	struct {
		auto operator=(glm::vec2 other) {
			return Get<0>() = other;
		};
	} position;
	auto & Position() { return Get<0>(); };
};

static_assert( sizeof( StandardVertex ) == sizeof( VertexBase<glm::vec2, glm::vec2, float> ) );



}
}
