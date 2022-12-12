
#include <core/SourceCommon.hpp>

#include <core/Exception.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Exception::Exception(
	std::string_view message
) :
	message( message )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Exception::~Exception()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API std::string_view vk2d::Exception::GetExceptionMessage()
{
	return message;
}