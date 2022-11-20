
#include <core/SourceCommon.hpp>

#include "CursorImpl.hpp"
#include "InstanceImpl.hpp"

#include <containers/Color.hpp>

#include <stb_image.h>
#include <stb_image_write.h>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::CursorImpl::CursorImpl(
	InstanceImpl					&	instance,
	const std::filesystem::path		&	image_path,
	glm::ivec2							hot_spot
) :
	instance( instance )
{
	int x = 0, y = 0, channels = 0;
	auto stbiData = stbi_load( image_path.string().c_str(), &x, &y, &channels, 4 );
	if( stbiData )
	{
		std::vector<Color8> data( x * y );
		std::memcpy( data.data(), stbiData, data.size() * sizeof( Color8 ) );
		free( stbiData );
		CursorImpl(
			instance,
			{ uint32_t( x ), uint32_t( y ) },
			data,
			hot_spot
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::CursorImpl::CursorImpl(
	InstanceImpl					&	instance,
	glm::uvec2							image_size,
	const std::vector<Color8>		&	image_data,
	glm::ivec2							hot_spot
) :
	instance( instance )
{
	if( size_t( image_size.x ) * size_t( image_size.y ) != image_data.size() )
	{
		std::stringstream ss;
		ss << "Cannot create cursor, image dimensions do not match required texel count.\n"
			<< "Cursor dimensions are: X=" << image_size.x << " Y=" << image_size.y << ".";
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, ss.str() );
		return;
	}

	pixel_data.resize( size_t( image_size.x ) * size_t( image_size.y ) * sizeof( Color8 ) );
	std::memcpy( pixel_data.data(), image_data.data(), pixel_data.size() * sizeof( Color8 ) );

	GLFWimage glfwImage {};
	glfwImage.width = image_size.x;
	glfwImage.height = image_size.y;
	glfwImage.pixels = (uint8_t*)pixel_data.data();
	cursor = glfwCreateCursor( &glfwImage, int( hot_spot.x ), int( hot_spot.y ) );
	if( cursor )
	{
		hotSpot = { hot_spot.x, hot_spot.y };
		extent = { image_size.x, image_size.y };
	}
	else
	{
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
		return;
	}

	is_good = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::CursorImpl::~CursorImpl()
{
	glfwDestroyCursor( cursor );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::CursorImpl::IsGood()
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::InstanceImpl & vk2d::vk2d_internal::CursorImpl::GetInstance()
{
	return instance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<vk2d::Color8> & vk2d::vk2d_internal::CursorImpl::GetTexelData()
{
	return pixel_data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLFWcursor * vk2d::vk2d_internal::CursorImpl::GetGLFWcursor()
{
	return cursor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::uvec2 vk2d::vk2d_internal::CursorImpl::GetSize()
{
	return { extent.width, extent.height };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::ivec2 vk2d::vk2d_internal::CursorImpl::GetHotSpot()
{
	return { hotSpot.x, hotSpot.y };
}
