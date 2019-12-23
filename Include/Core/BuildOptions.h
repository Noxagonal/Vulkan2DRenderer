#pragma once

// Enable debugging
#define VK2D_BUILD_OPTION_DEBUG_ENABLE									1

// Use Vulkan validation
#define VK2D_BUILD_OPTION_VULKAN_VALIDATION								1

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
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_VERTEX_SIZE					( 56	* 1024 * 1024 )
#define VK2D_BUILD_OPTION_MESH_BUFFER_BLOCK_INDEX_SIZE					( 8		* 1024 * 1024 )
