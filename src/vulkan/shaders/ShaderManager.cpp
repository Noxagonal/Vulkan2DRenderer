
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
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto shader = shader_list.find( shader_hash );
	if( shader == shader_list.end() ) return {};

	return ShaderHandle(
		this,
		&shader->second
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
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto shader_module = shader_compiler.CreateShaderModule(
		shader_create_info
	);
	if( shader_module == VK_NULL_HANDLE ) return {};

	auto result_it = shader_list.emplace(
		shader_create_info.GetHash(),
		ShaderManagerShaderEntry {
			shader_module,
			0,
			shader_create_info.GetHash()
		}
	);

	return ShaderHandle(
		this,
		&result_it.first->second
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::IncrementReferenceCount(
	size_t shader_hash
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto it = shader_list.find( shader_hash );
	assert( it != shader_list.end() );

	it->second.reference_count += 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DecrementReferenceCount(
	size_t shader_hash
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto it = shader_list.find( shader_hash );
	assert( it != shader_list.end() );
	assert( it->second.reference_count > 0 );

	it->second.reference_count -= 1;
	if( it->second.reference_count == 0 )
	{
		DestroyShader( it );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyShader( size_t shader_hash )
{
	DestroyShader( shader_list.find( shader_hash ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyShader(
	ShaderList::iterator shader_list_iterator
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	if( shader_list_iterator != shader_list.end() )
	{
		vkDestroyShaderModule(
			vulkan_device,
			shader_list_iterator->second.vulkan_shader_module,
			nullptr
		);

		shader_list.erase( shader_list_iterator );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManager::DestroyAllShaders()
{
	for( auto & o : shader_list )
	{
		vkDestroyShaderModule(
			vulkan_device,
			o.second.vulkan_shader_module,
			nullptr
		);
	}
	shader_list.clear();
}
