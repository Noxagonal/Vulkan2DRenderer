#pragma once

#include "Core/Common.h"

#include "Interface/Resources/ResourceBase.h"

#include <filesystem>

namespace vk2d {

namespace _internal {

class ResourceManagerImpl;
class GraphicsPipelineResourceImpl;

} // _internal



class GraphicsPipelineResource :
	public vk2d::ResourceBase
{
	VK2D_API																					GraphicsPipelineResource(
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread_index,
		vk2d::ResourceBase								*	parent_resource,
		const std::filesystem::path						&	file_path );

public:
	VK2D_API																					~GraphicsPipelineResource();

private:
	std::unique_ptr<vk2d::_internal::GraphicsPipelineResourceImpl>
															impl;
};



} // vk2d
