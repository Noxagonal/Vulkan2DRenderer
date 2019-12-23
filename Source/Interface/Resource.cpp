
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Resource.h"

#include <thread>



VK2D_API bool VK2D_APIENTRY vk2d::Resource::FailedToLoad() const
{
	return failed_to_load.load();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::Resource::GetLoaderThread()
{
	return loader_thread;
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsFromFile() const
{
	return is_from_file;
}

VK2D_API const std::filesystem::path VK2D_APIENTRY vk2d::Resource::GetFilePath() const
{
	return path;
}
