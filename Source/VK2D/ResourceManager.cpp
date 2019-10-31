
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/ResourceManager.h"
#include "../Header/ResourceManagerImpl.h"

#include <memory>



namespace vk2d {



ResourceManager::ResourceManager(
	_internal::RendererImpl			*	parent_renderer
)
{
	impl = std::make_unique<_internal::ResourceManagerImpl>( parent_renderer );
	if( !impl )				return;
	if( !impl->IsGood() )	return;

	is_good					= true;
}

ResourceManager::~ResourceManager()
{
	
}



}

