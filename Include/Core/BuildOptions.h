#pragma once

// Enable general debugging.
// Setting this to 0 will disable all
// debugging, including Vulkan validation.
#define VK2D_BUILD_OPTION_DEBUG_ENABLE									1

// Use Vulkan validation if
// VK2D_BUILD_OPTION_DEBUG_ENABLE == 1
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION								1

// Use Vulkan GPU assisted validation
#define VK2D_BUILD_OPTION_VULKAN_GPU_ASSISTED_VALIDATION				1

// NVIDIA GPUs ONLY!
// Use checkpoints in command buffer to try and narrow down persistent
// problems that can occurr while executing the command buffer.
// Helps narrow down VK_ERROR_DEVICE_LOST problems.
// This utilises Nvidia specific extension.
#define VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS				1

// For finishing touches and to catch possible
// performance issues. Prints more information about
// what could be done better or in a better way.
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_BEST_PRACTICES				0

// If using Vulkan validation, show verbose messages
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_VERBOSE			0

// If using Vulkan validation, show info messages
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_INFO				0

// If using Vulkan validation, show warning messages
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_WARNING			1

// If using Vulkan validation, show error messages
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_ERROR				1

// Mesh buffer object handles getting meshes into the GPU.
// This system handles meshes in batches. These defines
// control the allocation batch size that is Allocated
// and bound at one time, if mesh is bigger than these,
// a larger buffer is automatically allocated so these
// really are just the minimum sizes.
// Vertex buffer is by default 64 Mb.
// Index buffer is by default 16 Mb.
// TODO: This ratio should be changed at least.
// TODO: Ratio could be automated in future allocations.
// TODO: If automated, replace these with total buffer size.
// TEST THESE;
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE					8 /*( 64	* 1024 * 1024 )*/
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE					8 /*( 8		* 1024 * 1024 )*/
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_TEXTURE_CHANNEL_SIZE		8 /*( 8		* 1024 * 1024 )*/
