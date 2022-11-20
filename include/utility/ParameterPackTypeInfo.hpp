#pragma once

#include <core/Common.hpp>

#include <stdint.h>



namespace vk2d {
namespace vk2d_internal {
namespace details {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t Index, typename T>
struct GetTemplateTypeAtIndex_Indexed { using type = T; };

template <typename Is, typename ...ArgsT>
struct GetTemplateTypeAtIndex_Indexer;

template <size_t ...Is, typename ...ArgsT>
struct GetTemplateTypeAtIndex_Indexer<std::index_sequence<Is...>, ArgsT...>
	: GetTemplateTypeAtIndex_Indexed<Is, ArgsT>...
{ };

template <size_t Index, typename T>
static GetTemplateTypeAtIndex_Indexed<Index, T> Select( GetTemplateTypeAtIndex_Indexed<Index, T> );



} // details



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Get parameter pack element type by index.
///
/// @tparam		Index
///				Index from 0 to sizeof...(ArgsT)-1;
///
/// @tparam		ArgsT
///				Template parameter pack of which to get a type from.
template <std::size_t Index, typename ...ArgsT>
using GetTemplateTypeAtIndex = typename decltype( details::Select<Index>(
	details::GetTemplateTypeAtIndex_Indexer<std::index_sequence_for<ArgsT...>, ArgsT...>{}
) )::type;



} // vk2d_internal
} // vk2d
