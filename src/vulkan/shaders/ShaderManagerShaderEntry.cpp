
#include <core/SourceCommon.hpp>
#include "ShaderManagerShaderEntry.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManagerShaderEntry::ShaderManagerShaderEntry(
	VkShaderModule		vulkan_shader_module,
	size_t				hash
) :
	vulkan_shader_module( vulkan_shader_module ),
	hash( hash )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManagerShaderEntry::ShaderManagerShaderEntry(
	ShaderManagerShaderEntry && other
) noexcept
{
	MoveOther( std::move( other ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManagerShaderEntry::~ShaderManagerShaderEntry()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderManagerShaderEntry & vk2d::vulkan::ShaderManagerShaderEntry::operator=(
	ShaderManagerShaderEntry && other
	) noexcept
{
	MoveOther( std::move( other ) );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vulkan::ShaderManagerShaderEntry::operator==(
	const ShaderManagerShaderEntry & other
	)
{
	return vulkan_shader_module == other.vulkan_shader_module;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderManagerShaderEntry::GetVulkanShaderModule()
{
	return vulkan_shader_module;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::vulkan::ShaderManagerShaderEntry::GetHash()
{
	return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::ShaderManagerShaderEntry::MoveOther(
	ShaderManagerShaderEntry && other
) noexcept
{
	if( other == *this ) return;

	// Mutex cannot be moved so we only allow moving if the reference count is 0 for both,
	// which means that both entries aren't in use.
	#if VK2D_DEBUG_ENABLE
	reference_count(
		[]( size_t & count )
		{
			assert( count == 0 );
		}
	);
	other.reference_count(
		[]( size_t & count )
		{
			assert( count == 0 );
		}
	);
	#endif

	std::swap( vulkan_shader_module, other.vulkan_shader_module );
	std::swap( hash, other.hash );
}
