
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManager::~ShaderManager()
{
	DestroyAllShaders();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderHandle vk2d::vulkan::ShaderManager::FindShader(
	const ShaderCreateInfo & shader_create_info
)
{
	return FindShader( shader_create_info.GetHash() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderHandle vk2d::vulkan::ShaderManager::FindShader(
	size_t shader_hash
)
{
	return shader_list(
		[ this, shader_hash ]( ShaderList & list ) -> ShaderHandle
		{
			auto shader = list.find( shader_hash );
			if( shader == list.end() ) return {};

			return ShaderHandle(
				this,
				&shader->second
			);
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderHandle vk2d::vulkan::ShaderManager::GetShader(
	const ShaderCreateInfo & shader_create_info
)
{
	auto existing_shader = FindShader( shader_create_info );
	if( existing_shader ) return existing_shader;

	return CreateShader( shader_create_info );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderHandle vk2d::vulkan::ShaderManager::CreateShader(
	const ShaderCreateInfo	&	shader_create_info
)
{
	return shader_list(
		[ this, &shader_create_info ]( ShaderList & list ) -> ShaderHandle
		{
			auto shader_module = shader_compiler.CreateShaderModule(
				shader_create_info
			);
			if( shader_module == VK_NULL_HANDLE ) return {};

			auto result_it = list.emplace(
				shader_create_info.GetHash(),
				ShaderManagerShaderEntry(
					shader_module,
					shader_create_info.GetHash()
				)
			);

			return ShaderHandle(
				this,
				&result_it.first->second
			);
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::IncrementReferenceCount(
	ShaderManagerShaderEntry * shader_entry
)
{
	shader_entry->reference_count(
		[]( size_t & count )
		{
			++count;
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DecrementReferenceCount(
	ShaderManagerShaderEntry * shader_entry
)
{
	bool destruct = false;

	shader_entry->reference_count(
		[ this, &destruct ]( size_t & count )
		{
			--count;
			if( count == 0 ) destruct = true;
		}
	);

	if( destruct ) DestroyShader( shader_entry->GetHash() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyShader(
	size_t shader_hash
)
{
	shader_list(
		[ this, shader_hash ]( ShaderList & list )
		{
			auto it = list.find( shader_hash );
			if( it == list.end() ) return;

			vkDestroyShaderModule(
				vulkan_device,
				it->second.GetVulkanShaderModule(),
				nullptr
			);

			list.erase( it );
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyAllShaders()
{
	shader_list(
		[ this ]( ShaderList & list )
		{
			for( auto & p : list )
			{
				vkDestroyShaderModule(
					vulkan_device,
					p.second.GetVulkanShaderModule(),
					nullptr
				);
			}

			list.clear();
		}
	);
}
