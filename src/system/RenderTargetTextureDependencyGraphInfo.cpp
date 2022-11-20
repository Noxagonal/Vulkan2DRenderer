
#include <core/SourceCommon.hpp>

#include <system/RenderTargetTextureDependecyGraphInfo.hpp>

#include <interface/InstanceImpl.hpp>



void vk2d::vk2d_internal::RenderTargetTextureRenderCollector::Append(
	VkSubmitInfo	*	transfer_submit_info,
	VkSubmitInfo	*	render_submit_info
)
{
	collection.push_back( RenderTargetTextureRenderCollector::Collection {
		transfer_submit_info,
		render_submit_info
	} );
}

vk2d::vk2d_internal::RenderTargetTextureRenderCollector::Collection & vk2d::vk2d_internal::RenderTargetTextureRenderCollector::operator[](
	size_t	index
)
{
	assert( index < std::size( collection ) );
	return collection[ index ];
}

vk2d::vk2d_internal::RenderTargetTextureRenderCollector::Collection * vk2d::vk2d_internal::RenderTargetTextureRenderCollector::begin()
{
	// TODO: Do proper iterator for RenderTargetTextureRenderCollector.
	return collection.data();
}

vk2d::vk2d_internal::RenderTargetTextureRenderCollector::Collection * vk2d::vk2d_internal::RenderTargetTextureRenderCollector::end()
{
	return collection.data() + std::size( collection );
}

size_t vk2d::vk2d_internal::RenderTargetTextureRenderCollector::size()
{
	return collection.size();
}

vk2d::Multisamples vk2d::vk2d_internal::CheckSupportedMultisampleCount(
	InstanceImpl	&	instance,
	Multisamples		samples
)
{
	Multisamples max_samples = instance.GetMaximumSupportedMultisampling();
	if( uint32_t( samples ) > uint32_t( max_samples ) ) {
		std::stringstream ss;
		ss << "Parameter samples was larger than the system supports.\n"
			<< "Maximum supported amount for this system is: '" << uint32_t( max_samples ) << "'.";
		instance.Report( ReportSeverity::WARNING, ss.str() );
		samples = max_samples;
	}

	Multisamples supported_samples	= Multisamples( instance.GetVulkanDevice().GetVulkanPhysicalDeviceProperties().limits.framebufferColorSampleCounts);
	if( !( uint32_t( samples ) & uint32_t( supported_samples ) ) ) {
		std::stringstream ss;
		ss << "This specific multisample count (" << uint32_t( samples ) << ") is not supported by this system.\n"
			<< "Reverting to sample count 1.";
		instance.Report( ReportSeverity::WARNING, ss.str() );
		samples = Multisamples::SAMPLE_COUNT_1;
	}

	return samples;
}
