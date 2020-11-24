
#include "../Core/SourceCommon.h"

#include "../../Include/Types/Vector2.hpp"

#include "CommonTools.h"

std::vector<VkExtent2D> vk2d::_internal::GenerateMipSizes(
	vk2d::Vector2u		full_size
)
{
	std::vector<VkExtent2D> mipmap_levels;
	mipmap_levels.reserve( 16 );

	VkExtent2D last { full_size.x, full_size.y };
	mipmap_levels.push_back( last );

	while( last.width != 1 && last.height != 1 ) {
		VkExtent2D current_dim_size	= { last.width / 2, last.height / 2 };
		if( current_dim_size.width < 1 )	current_dim_size.width = 1;
		if( current_dim_size.height < 1 )	current_dim_size.height = 1;
		last	= current_dim_size;
		mipmap_levels.push_back( current_dim_size );
	}

	return mipmap_levels;
}
