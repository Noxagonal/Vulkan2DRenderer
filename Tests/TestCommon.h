#pragma once

#include <VK2D.h>

#include <filesystem>
#include <vector>
#include <array>

namespace fs = std::filesystem;

enum class ExitCodes : int
{
	SUCCESS													= 0,
	CANNOT_CREATE_INSTANCE,
	CANNOT_CREATE_WINDOW,
	CANNOT_BEGIN_RENDER,
	CANNOT_END_RENDER,
	CANNOT_WRITE_FILE,
	CANNOT_READ_FILE,
	RENDER_SAMPLE_OUT_OF_BOUNDS,
	RENDER_DOES_NOT_MATCH_EXPECTED_RESULT,
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

bool									VerifyImageWithSingleSample(
	const ColorPoint				&	sample,
	const vk2d::ImageData			&	image_data,
	uint8_t								tolerance					= 5,
	float								minimum_pass				= 0.98f );

// Checks samples with image data, returns true if check passes and false if samples
// do not match image data, or if sample coordinates are outside of image data.
// tolerance means how much the image color value can differ from test sample color.
// If tolerance is 0, then every sample must match 100%, if tolerance is 255 then
// every pixel will pass. minimum_pass tells how many of the test samples must pass
// for the whole check to pass. Tolerance encounters situations where different GPUs
// might render colors sligly differently. minimum_pass is meant for situations where
// different GPUs might render edges sligly differently.
bool												VerifyImageWithSamples(
	const std::vector<ColorPoint>				&	samples,
	const vk2d::ImageData						&	image_data,
	uint8_t											tolerance				= 5,
	float											minimum_pass			= 0.98f );

template<size_t SampleCount>
bool												VerifyImageWithSamples(
	const std::array<ColorPoint, SampleCount>	&	samples,
	const vk2d::ImageData						&	image_data,
	uint8_t											tolerance				= 5,
	float											minimum_pass			= 0.98f )
{
	std::vector<ColorPoint> samples_vector;
	samples_vector.insert( samples_vector.begin(), samples.begin(), samples.end() );
	return VerifyImageWithSamples(
		samples_vector,
		image_data,
		tolerance,
		minimum_pass
	);
}

class CoordGrid
{
public:
	CoordGrid(
		vk2d::Vector2f			draw_area,
		vk2d::Vector2f			spacing );

	CoordGrid(
		vk2d::Vector2u			draw_area,
		vk2d::Vector2f			spacing );

	vk2d::Vector2f				InsertTopLeft();
	vk2d::Vector2f				InsertBottomRight();
	void						Reset();

private:
	vk2d::Vector2f				draw_area			= {};
	vk2d::Vector2f				spacing				= {};
	vk2d::Vector2i				location			= {};
};

#define Coords( grid_object ) \
	grid_object.InsertTopLeft(), \
	grid_object.InsertBottomRight()
