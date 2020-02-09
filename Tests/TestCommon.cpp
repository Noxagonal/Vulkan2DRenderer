
#include "TestCommon.h"

#include <random>
#include <functional>
#include <fstream>


auto RandomEngine = std::default_random_engine();
using RandomDistribution = std::uniform_int_distribution<uint32_t>;

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
	auto rand_dis_x = RandomDistribution( 0, image_data.size.x );
	auto rand_dis_y = RandomDistribution( 0, image_data.size.y );
	auto RandPixelX = std::bind( rand_dis_x, RandomEngine );
	auto RandPixelY = std::bind( rand_dis_y, RandomEngine );

	std::vector<ColorPoint> ret; ret.reserve( sample_count );
	for( uint32_t i = 0; i < sample_count; ++i ) {
		auto x = RandPixelX();
		auto y = RandPixelY();
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
		"\n"
		"#include <TestCommon.h>\n"
		"\n"
		"#include <stdint.h>\n"
		"#include <array>\n"
		"\n"
		"array<ColorPoint, " << samples.size() << "> " << sample_array_name << " {\n";

	for( auto & s : samples ) {
		file <<
			"\t{ { " << s.position.x << ", " << s.position.y << " },"
			" { " << s.color.r << ", " << s.color.g << ", " << s.color.b << ", " << s.color.r << " } } ),";
	}

	file <<
		"\n"
		"}\n";
}
