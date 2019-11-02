
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/Resource.h"



namespace vk2d {



VK2D_API bool VK2D_APIENTRY Resource::IsLoaded() const
{
	return is_loaded.load();
}

VK2D_API bool VK2D_APIENTRY Resource::FailedToLoad() const
{
	return failed_to_load.load();
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
