#pragma once

#include <VK2D.h>

#include <filesystem>

namespace fs = std::filesystem;

enum class ExitCodes : int
{
	SUCCESS								= 0,
	CANNOT_WRITE_FILE,
};

struct ColorPoint {
	vk2d::Vector2u						position			= {};
	vk2d::Color8						color				= {};
};

void									ExitWithCode(
	ExitCodes							code );

std::vector<ColorPoint>					GenerateSamples(
	const vk2d::ImageData			&	image_data,
	uint32_t							sample_count );

void									SaveSamplesToHeaderFile(
	const std::vector<ColorPoint>	&	samples,
	fs::path							save_path,
	std::string							sample_array_name );

// Checks samples with image data, returns true if check passes and false if samples
// do not match image data, or if sample coordinates are outside of image data.
// tolerance means how much the source color value can differ from sample color value.
// If tolerance is 0, then every sample must match 100%, if tolerance is 255 then
// every pixel will pass. If any sample is not within tolerance then this function
// will return false.
bool									CheckSamplesWithImage(
	const std::vector<ColorPoint>	&	samples,
	const vk2d::ImageData			&	image_data,
	uint8_t								tolerance				= 5 );
