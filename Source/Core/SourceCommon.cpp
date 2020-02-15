
#include "../Header/Core/SourceCommon.h"

#include "../Header/Impl/InstanceImpl.h"



#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE



PFN_vkCmdSetCheckpointNV							fp_vkCmdSetCheckpointNV						= nullptr;
PFN_vkGetQueueCheckpointDataNV						fp_vkGetQueueCheckpointDataNV				= nullptr;

VkQueue												checkpoint_queue							= VK_NULL_HANDLE;
std::mutex										*	checkpoint_queue_mutex						= nullptr;
vk2d::_internal::CommandBufferCheckpointData	*	previous_command_buffer_checkpoint_data		= nullptr;



void vk2d::_internal::SetCommandBufferCheckpointQueue(
	VkDevice							device,
	VkQueue								queue,
	std::mutex						*	queue_mutex
)
{
	checkpoint_queue		= queue;
	checkpoint_queue_mutex	= queue_mutex;
	assert( checkpoint_queue );
	assert( checkpoint_queue_mutex );

	fp_vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)vkGetDeviceProcAddr(
		device,
		"vkCmdSetCheckpointNV"
	);
	assert( fp_vkCmdSetCheckpointNV );

	fp_vkGetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)vkGetDeviceProcAddr(
		device,
		"vkGetQueueCheckpointDataNV"
	);
	assert( fp_vkGetQueueCheckpointDataNV );
}

void vk2d::_internal::CmdInsertCommandBufferCheckpoint(
	VkCommandBuffer					command_buffer,
	std::string						name,
	CommandBufferCheckpointType		type
)
{
	auto new_command_buffer_checkpoint_data = new CommandBufferCheckpointData(
		name,
		type,
		previous_command_buffer_checkpoint_data
	);

	previous_command_buffer_checkpoint_data	= new_command_buffer_checkpoint_data;

	fp_vkCmdSetCheckpointNV(
		command_buffer,
		new_command_buffer_checkpoint_data
	);
}

std::vector<VkCheckpointDataNV> vk2d::_internal::GetCommandBufferCheckpoints()
{
	assert( checkpoint_queue_mutex );

	std::lock_guard<std::mutex> queue_lock(
		*checkpoint_queue_mutex );

	uint32_t checkpoint_count = 0;
	fp_vkGetQueueCheckpointDataNV(
		checkpoint_queue,
		&checkpoint_count,
		nullptr
	);
	std::vector<VkCheckpointDataNV> checkpoint_data( checkpoint_count );
	for( auto & c : checkpoint_data ) {
		c.sType				= VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
		c.pNext				= nullptr;
		c.stage				= VkPipelineStageFlagBits( 0 );
		c.pCheckpointMarker	= 0;
	}
	fp_vkGetQueueCheckpointDataNV(
		checkpoint_queue,
		&checkpoint_count,
		checkpoint_data.data()
	);
	return checkpoint_data;
}

std::string vk2d::_internal::CommandBufferCheckpointTypeToString( CommandBufferCheckpointType type )
{
	switch( type ) {
	case vk2d::_internal::CommandBufferCheckpointType::BEGIN_COMMAND_BUFFER:
		return "BEGIN_COMMAND_BUFFER";
	case vk2d::_internal::CommandBufferCheckpointType::END_COMMAND_BUFFER:
		return "END_COMMAND_BUFFER";
	case vk2d::_internal::CommandBufferCheckpointType::BEGIN_RENDER_PASS:
		return "BEGIN_RENDER_PASS";
	case vk2d::_internal::CommandBufferCheckpointType::END_RENDER_PASS:
		return "END_RENDER_PASS";
	case vk2d::_internal::CommandBufferCheckpointType::BIND_PIPELINE:
		return "BIND_PIPELINE";
	case vk2d::_internal::CommandBufferCheckpointType::BIND_DESCRIPTOR_SET:
		return "BIND_DESCRIPTOR_SET";
	case vk2d::_internal::CommandBufferCheckpointType::BIND_INDEX_BUFFER:
		return "BIND_INDEX_BUFFER";
	case vk2d::_internal::CommandBufferCheckpointType::BIND_VERTEX_BUFFER:
		return "BIND_VERTEX_BUFFER";
	case vk2d::_internal::CommandBufferCheckpointType::BEGIN_DRAW:
		return "BEGIN_DRAW";
	case vk2d::_internal::CommandBufferCheckpointType::END_DRAW:
		return "END_DRAW";
	case vk2d::_internal::CommandBufferCheckpointType::GENERIC:
		return "GENERIC";
	default:
		return "<UNKNOWN CommandBufferCheckpointType>";
	}
}

#endif



std::string vk2d::_internal::VkResultToString( VkResult result )
{
	switch( result ) {
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
		return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR";
	default:
		return "<UNKNOWN VkResult>";
	}
}

std::string vk2d::_internal::VkPipelineStageFlagBitsToString( VkPipelineStageFlagBits flags )
{
	switch( flags ) {
	case VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:
		return "VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT";
	case VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT:
		return "VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT";
	case VK_PIPELINE_STAGE_VERTEX_INPUT_BIT:
		return "VK_PIPELINE_STAGE_VERTEX_INPUT_BIT";
	case VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		return "VK_PIPELINE_STAGE_VERTEX_SHADER_BIT";
	case VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT:
		return "VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT";
	case VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT:
		return "VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT";
	case VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT:
		return "VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT";
	case VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT:
		return "VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT";
	case VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
		return "VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT";
	case VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT:
		return "VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT";
	case VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
		return "VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT";
	case VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT:
		return "VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT";
	case VK_PIPELINE_STAGE_TRANSFER_BIT:
		return "VK_PIPELINE_STAGE_TRANSFER_BIT";
	case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:
		return "VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT";
	case VK_PIPELINE_STAGE_HOST_BIT:
		return "VK_PIPELINE_STAGE_HOST_BIT";
	case VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT:
		return "VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT";
	case VK_PIPELINE_STAGE_ALL_COMMANDS_BIT:
		return "VK_PIPELINE_STAGE_ALL_COMMANDS_BIT";
	case VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT:
		return "VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT";
	case VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT:
		return "VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT";
	case VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX:
		return "VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX";
	case VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV:
		return "VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV";
	case VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV:
		return "VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV";
	case VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV:
		return "VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV";
	case VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV:
		return "VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV";
	case VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV:
		return "VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV";
	case VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT:
		return "VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT";
	case VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM:
		return "VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM";
	default:
		return "<UNKNOWN VkPipelineStageFlagBits>";
	}
}
