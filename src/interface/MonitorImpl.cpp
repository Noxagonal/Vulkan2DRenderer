
#include <core/SourceCommon.hpp>

#include "MonitorImpl.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::MonitorImpl::MonitorImpl(
	GLFWmonitor								*	monitor,
	VkOffset2D									position,
	VkExtent2D									physical_size,
	std::string									name,
	MonitorVideoMode							current_video_mode,
	const std::vector<MonitorVideoMode>		&	video_modes
) :
	monitor( monitor ),
	position( position ),
	physical_size( physical_size ),
	name( name ),
	current_video_mode( current_video_mode ),
	video_modes( video_modes )
{
	assert( this->monitor );

	is_good = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::MonitorVideoMode & vk2d::vk2d_internal::MonitorImpl::GetCurrentVideoMode() const
{
	return current_video_mode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<vk2d::MonitorVideoMode> & vk2d::vk2d_internal::MonitorImpl::GetVideoModes() const
{
	return video_modes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::MonitorImpl::SetGamma(
	float		gamma
)
{
	glfwSetGamma(
		monitor,
		gamma
	);
}



namespace vk2d {
namespace vk2d_internal {
constexpr float GAMMA_MULTIPLIER = float( 65536 - 256 );
} // vk2d_internal
} // vk2d



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<vk2d::GammaRampNode> vk2d::vk2d_internal::MonitorImpl::GetGammaRamp()
{
	auto glfwRamp = glfwGetGammaRamp( monitor );
	std::vector<GammaRampNode>	ret {};
	ret.reserve( glfwRamp->size );
	for( size_t i = 0; i < size_t( glfwRamp->size ); ++i )
	{
		ret.push_back(
			{
				glfwRamp->red[ i ] / GAMMA_MULTIPLIER,
				glfwRamp->green[ i ] / GAMMA_MULTIPLIER,
				glfwRamp->blue[ i ] / GAMMA_MULTIPLIER
			}
		);
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::MonitorImpl::SetGammaRamp(
	std::span<const GammaRampNode>  ramp
)
{
	if( ramp.size() < 2 ) return;

	size_t glfw_ramp_node_count = 0;
	{
		auto glfw_original_ramp = glfwGetGammaRamp( monitor );
		if( !glfw_original_ramp )
		{
			return;
		}
		glfw_ramp_node_count = glfw_original_ramp->size;

		if( glfw_ramp_node_count < 2 ) return;
	}
	std::vector<uint16_t> glfw_ramp_red( glfw_ramp_node_count );
	std::vector<uint16_t> glfw_ramp_green( glfw_ramp_node_count );
	std::vector<uint16_t> glfw_ramp_blue( glfw_ramp_node_count );

	// Need to match the original node count regardless of input
	// ramp node count so we do some linear interpolation here.
	{
		auto ramp_nodes = ramp.size();
		auto monitor_nodes = glfw_ramp_node_count;

		float difference = float( ramp_nodes - 1 ) / float( monitor_nodes - 1 );

		for( int i = 0; i < monitor_nodes - 1; ++i )
		{
			float	offset = i * difference;
			size_t	node_index = size_t( std::floor( offset ) );
			float	local_offset = offset - float( node_index );
			auto	node_0 = ramp[ node_index ];
			auto	node_1 = ramp[ std::min( node_index + 1, ramp_nodes - 1 ) ];

			glfw_ramp_red[ i ] = uint16_t( GAMMA_MULTIPLIER * ( ( 1.0f - local_offset ) * node_0.red + local_offset * node_1.red ) );
			glfw_ramp_green[ i ] = uint16_t( GAMMA_MULTIPLIER * ( ( 1.0f - local_offset ) * node_0.green + local_offset * node_1.green ) );
			glfw_ramp_blue[ i ] = uint16_t( GAMMA_MULTIPLIER * ( ( 1.0f - local_offset ) * node_0.blue + local_offset * node_1.blue ) );
		}
		glfw_ramp_red.back() = uint16_t( GAMMA_MULTIPLIER * ramp.back().red );
		glfw_ramp_green.back() = uint16_t( GAMMA_MULTIPLIER * ramp.back().green );
		glfw_ramp_blue.back() = uint16_t( GAMMA_MULTIPLIER * ramp.back().blue );
	}

	GLFWgammaramp glfw_gamma_ramp {};
	glfw_gamma_ramp.size = (unsigned int)glfw_ramp_node_count;
	glfw_gamma_ramp.red = glfw_ramp_red.data();
	glfw_gamma_ramp.green = glfw_ramp_green.data();
	glfw_gamma_ramp.blue = glfw_ramp_blue.data();

	glfwSetGammaRamp( monitor, &glfw_gamma_ramp );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::MonitorImpl::IsGood()
{
	return is_good;
}
