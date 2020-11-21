#pragma once

// Required Vulkan API version.
#define VK2D_BUILD_OPTION_VULKAN_MINIMUM_REQUIRED_VERSION				VK_API_VERSION_1_2

// Enable general debugging.
// Setting this to 0 will disable all
// debugging, including Vulkan validation.
// THIS IS SET BY CMake
// #define VK2D_DEBUG_ENABLE											1

// Draws all solid polygons with overlayed triangle mesh, can help with visual debugging.
#define VK2D_BUILD_OPTION_DEBUG_ALWAYS_DRAW_TRIANGLES_WIREFRAME			0

// Use Vulkan validation.
// Used only if VK2D_DEBUG_ENABLE == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION								0

// Dump Vulkan API calls
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP					0

// Vulkan API dump file path and name, relation to working folder or folder
// containing the executable. No need to put in the file extension.
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP_FILE_PATH			"vulkan_api_dump"

// Which format to use if using api dump.
// Supported formats are: "txt", "html", "json"
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP_FILE_EXTENSION		"txt"

// Use Vulkan GPU assisted validation
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1 and if VK2D_DEBUG_ENABLE == 1
#define VK2D_BUILD_OPTION_VULKAN_GPU_ASSISTED_VALIDATION				1

// NVIDIA GPUs ONLY!
// Use checkpoints in command buffer to try and narrow down persistent
// problems that can occurr while executing the command buffer.
// Helps narrow down VK_ERROR_DEVICE_LOST problems.
// This utilises Nvidia specific extension.
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1 and if VK2D_DEBUG_ENABLE == 1
#define VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS				1

// For finishing touches and to catch possible
// performance issues. Prints more information about
// what could be done better or in a better way.
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_BEST_PRACTICES				0

// If using Vulkan validation, show verbose messages
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_VERBOSE			0

// If using Vulkan validation, show info messages
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_INFO				0

// If using Vulkan validation, show warning messages
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_WARNING			1

// If using Vulkan validation, show error messages
// Used only if VK2D_BUILD_OPTION_VULKAN_VALIDATION == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_ERROR				1

// Mesh buffer object handles getting meshes into the GPU.
// This system handles meshes in batches. These defines
// control the allocation batch size that is Allocated
// and bound at one time, if mesh is bigger than these,
// a larger buffer is automatically allocated so these
// really are just the minimum sizes.
// Vertex buffer is by default 64 Mb.
// Index buffer is by default 16 Mb.
// Texture channel buffer is by default 16 Mb.
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE					( 64	* 1024 * 1024 )
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE					( 16	* 1024 * 1024 )
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_TEXTURE_CHANNEL_SIZE		( 16	* 1024 * 1024 )
