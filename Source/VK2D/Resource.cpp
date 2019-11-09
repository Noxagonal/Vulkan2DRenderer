
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/Resource.h"

#include <thread>


namespace vk2d {



VK2D_API bool VK2D_APIENTRY Resource::FailedToLoad() const
{
	return failed_to_load.load();
}

VK2D_API uint32_t VK2D_APIENTRY Resource::GetLoaderThread()
{
	return loader_thread;
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
