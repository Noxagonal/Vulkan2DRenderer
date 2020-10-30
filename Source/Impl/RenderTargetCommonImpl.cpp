
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/RenderTargetCommonImpl.hpp"

void vk2d::_internal::RenderTargetTextureRenderCollector::Append(
	VkSubmitInfo	transfer_submit_info,
	VkSubmitInfo	render_submit_info
)
{
	collection.push_back( vk2d::_internal::RenderTargetTextureRenderCollector::Collection {
		transfer_submit_info,
		render_submit_info
	} );
}

vk2d::_internal::RenderTargetTextureRenderCollector::Collection & vk2d::_internal::RenderTargetTextureRenderCollector::operator[](
	size_t	index
)
{
	assert( index < std::size( collection ) );
	return collection[ index ];
}

vk2d::_internal::RenderTargetTextureRenderCollector::Collection * vk2d::_internal::RenderTargetTextureRenderCollector::begin()
{
	// TODO: Do proper iterator for RenderTargetTextureRenderCollector.
	return collection.data();
}

vk2d::_internal::RenderTargetTextureRenderCollector::Collection * vk2d::_internal::RenderTargetTextureRenderCollector::end()
{
	return collection.data() + std::size( collection );
}

size_t vk2d::_internal::RenderTargetTextureRenderCollector::size()
{
	return collection.size();
}
