
#include "TestCommon.h"

#include <random>
#include <functional>
#include <fstream>


using RandomEngine			= std::default_random_engine;
using RandomDistribution	= std::uniform_int_distribution<uint32_t>;

void ExitWithCode(
	ExitCodes		code
)
{
	std::exit( static_cast<int>( code ) );
}

std::vector<ColorPoint> GenerateSamples(
	const vk2d::ImageData		&	image_data,
	uint32_t						sample_count
)
{
	auto rnde = RandomEngine();

	auto rand_dis_x = RandomDistribution( 0, image_data.size.x - 1 );
	auto rand_dis_y = RandomDistribution( 0, image_data.size.y - 1 );

	std::vector<ColorPoint> ret; ret.reserve( sample_count );
	for( uint32_t i = 0; i < sample_count; ++i ) {
		auto x = rand_dis_x( rnde );
		auto y = rand_dis_y( rnde );
		ColorPoint cp;
		cp.position = { x, y };
		cp.color = image_data.data[ size_t( image_data.size.x ) * y + x ];
		ret.push_back( cp );
	}

	return ret;
}

void SaveSamplesToHeaderFile(
	const std::vector<ColorPoint>	&	samples,
	fs::path							save_path,
	std::string							sample_array_name
)
{
	assert( !save_path.empty() );

	std::ofstream file( save_path );
	if( !file.is_open() ) ExitWithCode( ExitCodes::CANNOT_WRITE_FILE );

	file <<
		"#pragma once\n"
		"\n"
		"#include \"TestCommon.h\"\n"
		"\n"
		"#include <stdint.h>\n"
		"#include <array>\n"
		"\n"
		"std::array<ColorPoint, " << samples.size() << "> " << sample_array_name << " { {\n";

	for( auto & s : samples ) {
		file <<
			"\t{ { " << s.position.x << ", " << s.position.y << " },"
			" { " << (uint32_t)s.color.r << ", " << (uint32_t)s.color.g << ", " << (uint32_t)s.color.b << ", " << (uint32_t)s.color.a << " } },\n";
	}

	file <<
		"} };\n";
}

bool VerifyImageWithSingleSample(
	const ColorPoint				&	sample,
	const vk2d::ImageData			&	image_data,
	uint8_t								tolerance,
	float								minimum_pass
)
{
	if( sample.position.x >= image_data.size.x ||
		sample.position.y >= image_data.size.y ) {
		ExitWithCode( ExitCodes::RENDER_SAMPLE_OUT_OF_BOUNDS );
	}
	auto ic					= image_data.data[ size_t( image_data.size.x ) * sample.position.y + sample.position.x ];
	auto ec					= sample.color;
	auto diff_r				= float( ic.r ) - float( ec.r );
	auto diff_g				= float( ic.g ) - float( ec.g );
	auto diff_b				= float( ic.b ) - float( ec.b );
	auto diff_a				= float( ic.a ) - float( ec.a );

	if( std::abs( diff_r ) > float( tolerance ) ||
		std::abs( diff_g ) > float( tolerance ) ||
		std::abs( diff_b ) > float( tolerance ) ||
		std::abs( diff_a ) > float( tolerance ) ) {
		return false;
	} else {
		return true;
	}
}

bool VerifyImageWithSamples(
	const std::vector<ColorPoint>	&	samples,
	const vk2d::ImageData			&	image_data,
	uint8_t								tolerance,
	float								minimum_pass
)
{
	uint32_t passed			= 0;
	uint32_t missed			= 0;

	for( auto & s : samples ) {
		if( VerifyImageWithSingleSample(
			s,
			image_data,
			tolerance,
			minimum_pass
		) ) {
			++passed;
		} else {
			++missed;
		}
	}

	if( ( float( passed ) / samples.size() ) < minimum_pass ) {
		return false;
	} else {
		return true;
	}
}

CoordGrid::CoordGrid(
	glm::vec2		draw_area,
	glm::vec2		spacing )
{
	this->draw_area		= draw_area;
	this->spacing		= spacing;
	this->location		= {};
}

CoordGrid::CoordGrid( glm::uvec2 draw_area, glm::vec2 spacing )
{
	this->draw_area		= glm::vec2( float( draw_area.x ), float( draw_area.y ) );
	this->spacing		= spacing;
	this->location		= {};
}

glm::vec2 CoordGrid::InsertTopLeft()
{
	glm::vec2 coords {};
	coords.x		= location.x * spacing.x;
	coords.y		= location.y * spacing.y;

	++location.x;
	if( coords.x + spacing.x + spacing.x > draw_area.x ) {
		location.x = 0;
		++location.y;
	}

	return coords;
}

glm::vec2 CoordGrid::InsertBottomRight()
{
	glm::vec2 coords {};
	coords.x		= location.x * spacing.x + spacing.x;
	coords.y		= location.y * spacing.y + spacing.y;
	return coords;
}

void CoordGrid::Reset()
{
	location	= {};
}
