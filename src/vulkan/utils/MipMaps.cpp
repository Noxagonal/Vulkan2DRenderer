
#include <core/SourceCommon.hpp>
#include "MipMaps.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<VkExtent2D> vk2d::vulkan::CalculateMipLevels(
	VkExtent2D full_size
)
{
	std::vector<VkExtent2D> mipmap_levels;
	mipmap_levels.reserve( 16 );

	VkExtent2D last = full_size;
	mipmap_levels.push_back( last );

	while( last.width != 1 && last.height != 1 )
	{
		VkExtent2D current_dim_size	= { last.width / 2, last.height / 2 };
		if( current_dim_size.width < 1 )	current_dim_size.width = 1;
		if( current_dim_size.height < 1 )	current_dim_size.height = 1;
		last	= current_dim_size;
		mipmap_levels.push_back( current_dim_size );
	}

	return mipmap_levels;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<VkExtent3D> vk2d::vulkan::CalculateMipLevels(
	VkExtent3D full_size
)
{
	std::vector<VkExtent3D> mipmap_levels;
	mipmap_levels.reserve( 16 );

	VkExtent3D last = full_size;
	mipmap_levels.push_back( last );

	while( last.width != 1 && last.height != 1 && last.depth != 1 )
	{
		VkExtent3D current_dim_size	= { last.width / 2, last.height / 2, last.depth / 2 };
		if( current_dim_size.width < 1 )	current_dim_size.width = 1;
		if( current_dim_size.height < 1 )	current_dim_size.height = 1;
		if( current_dim_size.depth < 1 )	current_dim_size.depth = 1;
		last	= current_dim_size;
		mipmap_levels.push_back( current_dim_size );
	}

	return mipmap_levels;
}
