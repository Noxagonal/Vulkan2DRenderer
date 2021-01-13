#pragma once

#include "Core/SourceCommon.h"



namespace vk2d {
namespace _internal {



enum class ImageFormatConverterColorSwizzle : uint32_t
{
	RED			= 0,
	GREEN		= 1,
	BLUE		= 2,
	ALPHA		= 3,
	CONSTANT	= 4,
};



/// @brief		Basic image format conversion tool
/// @tparam		DestinationT
///				Desination format per channel. eg. Image format RGBA where each channel is uint8_t then DestinationT should be uint8_t.
/// @tparam		SourceT
///				Source format per channel. See DestinationT for more info.
/// @param		size
///				Size of the image in texels.
/// @param		destination_image_data
///				Raw data where final image will be written to. Must be large enough to contain the image, no checks can be made inside
///				this function to make sure it actually is large enough to store the data.
///				Must be "width" * "height" * "channel count" * "sizeof( DestinationT )" in byte size.
/// @param		destination_image_channel_count
///				How many color channels are in the destination image.
/// @param		source_image_data
///				Same as destination_image_data but for source data.
/// @param		source_image_channel_count
///				Same as destination_image_channel_count buf for source data.
/// @param		multiplier
///				Multiply source color by this before copying it into destination.
/// @param		swizzles
///				Tells which channels in source image we use for each destination channel.
/// @param		constants
///				If a swizzle points to a constant for a specific channel, then the corresponding constant value is taken from here
///				without using the multiplier.
/// @return		true on success, false on failure.
template<typename DestinationT, typename SourceT>
void ImageFormatConverter(
	glm::uvec2													size,
	DestinationT											*	destination_image_data,
	uint32_t													destination_image_channel_count,
	SourceT													*	source_image_data,
	uint32_t													source_image_channel_count,
	float														multiplier,
	const std::array<ImageFormatConverterColorSwizzle, 4>	&	swizzles,
	const std::array<DestinationT, 4>						&	constants
)
{
	assert( destination_image_channel_count >= 1 && destination_image_channel_count <= 4 );
	assert( source_image_channel_count >= 1 && source_image_channel_count <= 4 );

	size_t total_texels = size_t( size.x ) * size_t( size.y );

	for( size_t i = 0; i < total_texels; ++i ) {

		for( size_t c = 0; c < destination_image_channel_count; ++c ) {
			destination_image_data[ c ] =
				( swizzles[ c ] == vk2d::_internal::ImageFormatConverterColorSwizzle::CONSTANT ) ?
				DestinationT( constants[ c ] ) :
				DestinationT( float( source_image_data[ uint32_t( swizzles[ c ] ) ] ) * multiplier );
		}

		destination_image_data	+= destination_image_channel_count;
		source_image_data		+= source_image_channel_count;
	}
}



} // _internal
} // vk2d
