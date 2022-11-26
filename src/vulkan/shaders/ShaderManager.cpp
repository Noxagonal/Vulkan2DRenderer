
#include <core/SourceCommon.hpp>
#include "ShaderManager.hpp"

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManager::ShaderManager(
	vk2d_internal::InstanceImpl & instance
) :
	instance( instance ),
	shader_compiler( instance )
{}
