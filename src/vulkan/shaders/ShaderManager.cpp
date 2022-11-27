
#include <core/SourceCommon.hpp>
#include "ShaderManager.hpp"

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManager::ShaderManager(
	vk2d_internal::InstanceImpl		&	instance,
	Device							&	vulkan_device
) :
	instance( instance ),
	vulkan_device( vulkan_device ),
	shader_compiler( instance, vulkan_device )
{}

vk2d::vulkan::ShaderManager::~ShaderManager()
{
	DestroyShaders();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderManager::FindShader(
	const ShaderInfo & shader_info
)
{
	return FindShader( shader_info.GetHash() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderManager::FindShader(
	size_t shader_hash
)
{
	auto shader = shader_list.find( shader_hash );
	if( shader == shader_list.end() ) return {};
	return shader->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderManager::CreateShader(
	const ShaderInfo	&	user_shader_info
)
{
	auto shader_module = shader_compiler.CreateShaderModule(
		user_shader_info
	);
	if( shader_module == VK_NULL_HANDLE ) return {};

	shader_list.emplace( user_shader_info.GetHash(), shader_module );

	return shader_module;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyShader(
	VkShaderModule shader_module
)
{
	if( shader_module != VK_NULL_HANDLE )
	{
		for( auto it = shader_list.begin(); it != shader_list.end(); )
		{
			if( it->second == shader_module )
			{
				shader_list.erase( it );

				vkDestroyShaderModule(
					vulkan_device,
					shader_module,
					nullptr
				);

				return;
			}

			++it;
		}
	}
}

void vk2d::vulkan::ShaderManager::DestroyShaders()
{
	for( auto & o : shader_list )
	{
		vkDestroyShaderModule(
			vulkan_device,
			o.second,
			nullptr
		);
	}
	shader_list.clear();
}
