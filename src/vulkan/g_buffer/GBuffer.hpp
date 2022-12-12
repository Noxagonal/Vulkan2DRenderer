#pragma once

#include <core/SourceCommon.hpp>
#include "GBufferCreateInfo.hpp"

#include <vulkan/utils/VulkanMemoryManagement.hpp>



namespace vk2d {
namespace vk2d_internal {

class InstanceImpl;

} // vk2d_internal

namespace vulkan {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Graphics Buffer.
///
///				GBuffers are used for drawing each frame, and used as a source for further realtime image manipulation.
class GBuffer
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer() = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer(
		const GBuffer							&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer(
		GBuffer									&&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer(
		vk2d_internal::InstanceImpl				&	instance,
		GBufferCreateInfo						&	create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~GBuffer();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer										&	operator=(
		const GBuffer							&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GBuffer										&	operator=(
		GBuffer									&&	other
	) = default;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl				&	instance;
	GBufferCreateInfo							create_info;

	std::vector<CompleteImageResource>			image_resources;
};



} // vulkan
} // vk2d
