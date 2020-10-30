
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/RenderTargetCommonImpl.hpp"
#include "../Header/Impl/InstanceImpl.h"

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

vk2d::Multisamples vk2d::_internal::CheckSupportedMultisampleCount(
	vk2d::_internal::InstanceImpl	*	instance,
	vk2d::Multisamples					samples
)
{
	vk2d::Multisamples max_samples = instance->GetMaximumSupportedMultisampling();
	if( uint32_t( samples ) > uint32_t( max_samples ) ) {
		std::stringstream ss;
		ss << "Parameter samples was larger than the system supports.\n"
			<< "Maximum supported amount for this system is: '" << uint32_t( max_samples ) << "'.";
		instance->Report( vk2d::ReportSeverity::WARNING, ss.str() );
		samples = max_samples;
	}

	vk2d::Multisamples supported_samples	= vk2d::Multisamples( instance->GetVulkanPhysicalDeviceProperties().limits.framebufferColorSampleCounts );
	if( !( uint32_t( samples ) & uint32_t( supported_samples ) ) ) {
		std::stringstream ss;
		ss << "This specific multisample count (" << uint32_t( samples ) << ") is not supported by this system.\n"
			<< "Reverting to sample count 1.";
		instance->Report( vk2d::ReportSeverity::WARNING, ss.str() );
		samples = vk2d::Multisamples::SAMPLE_COUNT_1;
	}

	return samples;
}
