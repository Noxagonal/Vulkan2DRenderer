
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/Resource.h"

#include <thread>


namespace vk2d {



VK2D_API bool VK2D_APIENTRY Resource::IsLoaded() const
{
	return is_loaded.load();
}

VK2D_API bool VK2D_APIENTRY Resource::FailedToLoad() const
{
	return failed_to_load.load();
}

VK2D_API bool VK2D_APIENTRY Resource::WaitUntilLoaded() const
{
	while( !IsLoaded() && !FailedToLoad() ) {
		// We'll just do a semi-busy loop for now.
		std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
	}
	if( IsLoaded() ) return true;
	return false;
}

VK2D_API bool VK2D_APIENTRY Resource::IsFromFile() const
{
	return is_from_file;
}

VK2D_API const std::filesystem::path VK2D_APIENTRY Resource::GetFilePath() const
{
	return path;
}

}
