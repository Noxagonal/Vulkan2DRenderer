#pragma once

#define VK2D_LIBRARY_EXPORT				1

#include "BuildOptions.h"

#include "Core/PreCompiledHeader.h"
#include "Core/Common.h"

#include <vulkan/vulkan.h>

namespace vk2d {

namespace _internal {



constexpr float KINDA_SMALL_VALUE			= 0.001f;



std::string				VkResultToString( VkResult result );
std::string				VkPipelineStageFlagBitsToString( VkPipelineStageFlagBits flags );



enum class CommandBufferCheckpointType : uint32_t {
	BEGIN_COMMAND_BUFFER,
	END_COMMAND_BUFFER,
	BEGIN_RENDER_PASS,
	END_RENDER_PASS,
	BIND_PIPELINE,
	BIND_DESCRIPTOR_SET,
	BIND_INDEX_BUFFER,
	BIND_VERTEX_BUFFER,
	DRAW,
	GENERIC,
};

struct CommandBufferCheckpointData {
	CommandBufferCheckpointData(
		std::string						name,
		CommandBufferCheckpointType		type,
		CommandBufferCheckpointData	*	previous
	) :
		name( name ),
		type( type ),
		previous( previous )
	{}

	std::string							name			= {};
	CommandBufferCheckpointType			type			= {};
	CommandBufferCheckpointData		*	previous		= {};
};

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE

void									SetCommandBufferCheckpointQueue(
	VkDevice							device,
	VkQueue								queue,
	std::mutex						*	queue_mutex );

void									CmdInsertCommandBufferCheckpoint(
	VkCommandBuffer						command_buffer,
	std::string							name,
	CommandBufferCheckpointType			type );

std::vector<VkCheckpointDataNV>			GetCommandBufferCheckpoints();

std::string								CommandBufferCheckpointTypeToString( CommandBufferCheckpointType type );

#else

inline void								CmdInsertCommandBufferCheckpoint(
	VkCommandBuffer						command_buffer,
	std::string							name,
	CommandBufferCheckpointType			type )
{};

#endif



#if VK2D_DEBUG_ENABLE

#define VK2D_ASSERT_MAIN_THREAD( m_p_instance ) assert( m_p_instance->IsThisThreadCreatorThread() );



class ThreadAccessScopeTracker
{
public:
	ThreadAccessScopeTracker( std::string file, std::string function_name, size_t line );
	~ThreadAccessScopeTracker();

	std::string		key;
	std::thread::id	thread_id;
};
#define VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE() ThreadAccessScopeTracker m_thread_scope_access_tracker_##__LINE__( __FILE__, __FUNCTION__, __LINE__ )

#else

#define VK2D_ASSERT_MAIN_THREAD( m_p_instance )
#define VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE()

#endif


} // _internal

} // vk2d
