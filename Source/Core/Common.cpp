
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Core/Common.h"

namespace vk2d{



bool operator<( vk2d::ConsoleColor c1, vk2d::ConsoleColor c2 ){
	return uint8_t( c1 ) < uint8_t( c2 );
};



#if defined( VK2D_PLATFORM_WINDOWS )

void SetConsoleColor(
	vk2d::ConsoleColor		text_color,
	vk2d::ConsoleColor		background_color
)
{
	auto tc		= uint8_t( text_color );
	auto bc		= uint8_t( background_color );
	if( tc == uint8_t( vk2d::ConsoleColor::DEFAULT ) ) tc = uint8_t( vk2d::ConsoleColor::GRAY );
	if( bc == uint8_t( vk2d::ConsoleColor::DEFAULT ) ) bc = uint8_t( vk2d::ConsoleColor::BLACK );
	unsigned short attributes = ( (unsigned)bc << 4 ) | (unsigned)tc;
	HANDLE std_out_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( std_out_handle, attributes );
}

#elif defined( VK2D_PLATFORM_LINUX )

std::map<vk2d::ConsoleColor, const char*> text_color_map {
	{ vk2d::ConsoleColor::DEFAULT, "39" },
	{ vk2d::ConsoleColor::BLACK, "30" },
	{ vk2d::ConsoleColor::DARK_RED, "31" },
	{ vk2d::ConsoleColor::DARK_GREEN, "32" },
	{ vk2d::ConsoleColor::DARK_YELLOW, "33" },
	{ vk2d::ConsoleColor::DARK_BLUE, "34" },
	{ vk2d::ConsoleColor::DARK_MAGENTA, "35" },
	{ vk2d::ConsoleColor::DARK_CYAN, "36" },
	{ vk2d::ConsoleColor::GRAY, "37" },
	{ vk2d::ConsoleColor::DARK_GRAY, "90" },
	{ vk2d::ConsoleColor::RED, "91" },
	{ vk2d::ConsoleColor::GREEN, "92" },
	{ vk2d::ConsoleColor::YELLOW, "93" },
	{ vk2d::ConsoleColor::BLUE, "94" },
	{ vk2d::ConsoleColor::MAGENTA, "95" },
	{ vk2d::ConsoleColor::CYAN, "96" },
	{ vk2d::ConsoleColor::WHITE, "97" }
};

std::map<vk2d::ConsoleColor, const char*> background_color_map {
	{ vk2d::ConsoleColor::DEFAULT, "49" },
	{ vk2d::ConsoleColor::BLACK, "40" },
	{ vk2d::ConsoleColor::RED, "41" },
	{ vk2d::ConsoleColor::GRAY, "42" },
	{ vk2d::ConsoleColor::YELLOW, "43" },
	{ vk2d::ConsoleColor::BLUE, "44" },
	{ vk2d::ConsoleColor::MAGENTA, "45" },
	{ vk2d::ConsoleColor::CYAN, "46" },
	{ vk2d::ConsoleColor::GRAY, "47" },
	{ vk2d::ConsoleColor::DARK_GRAY, "100" },
	{ vk2d::ConsoleColor::RED, "101" },
	{ vk2d::ConsoleColor::GREEN, "102" },
	{ vk2d::ConsoleColor::YELLOW, "103" },
	{ vk2d::ConsoleColor::BLUE, "104" },
	{ vk2d::ConsoleColor::MAGENTA, "105" },
	{ vk2d::ConsoleColor::CYAN, "106" },
	{ vk2d::ConsoleColor::WHITE, "107" }
};

void SetConsoleColor(
	ConsoleColor		text_color,
	ConsoleColor		background_color
)
{
	std::cout << "\003[0;" << text_color_map[ text_color ] << ";" << background_color_map[ background_color ] << ";m";
}

#else

#error "Please add platform support!"

#endif




} // vk2d
