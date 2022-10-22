
#include "core/SourceCommon.h"

#include "core/SystemConsole.h"

#include <iostream>



// Mutex needed to make sense of messages if multiple
// threads are printing messages at the same time.
std::mutex print_mutex;



#if defined( VK2D_PLATFORM_WINDOWS )

#include <Windows.h>



VK2D_API void VK2D_APIENTRY vk2d::ConsolePrint(
	std::string_view		text,
	vk2d::ConsoleColor		text_color,
	vk2d::ConsoleColor		background_color
)
{
	std::lock_guard<std::mutex> lock_guard( print_mutex );

	HANDLE std_out_handle = GetStdHandle( STD_OUTPUT_HANDLE );

	// Get old attributes.
	WORD old_attributes {};
	{
		CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_info {};
		if( GetConsoleScreenBufferInfo( std_out_handle, &console_screen_buffer_info ) ) {
			old_attributes = console_screen_buffer_info.wAttributes;
		} else {
			old_attributes = 7;		// Black background, Gray color text.
		}
	}

	WORD text_color_as_int			= ( text_color			== vk2d::ConsoleColor::DEFAULT ) ? ( ( old_attributes & WORD( 0x000F ) ) ) : ( WORD( text_color ) );
	WORD background_color_as_int	= ( background_color	== vk2d::ConsoleColor::DEFAULT ) ? ( ( old_attributes & WORD( 0x00F0 ) ) ) : ( WORD( background_color ) << 4 );
	WORD new_attributes				= background_color_as_int | text_color_as_int;


	SetConsoleTextAttribute( std_out_handle, new_attributes );
	std::cout << text;
	SetConsoleTextAttribute( std_out_handle, old_attributes );
}

#elif defined( VK2D_PLATFORM_LINUX )

#include <map>



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

void vk2d::ConsolePrint(
	std::string_view		text,
	vk2d::ConsoleColor		text_color,
	vk2d::ConsoleColor		background_color
)
{
	// TODO: Test ConsolePrint on Linux machine.

	std::lock_guard<std::mutex> lock_guard( print_mutex );

	std::cout << "\033[" << text_color_map[ text_color ] << ";" << background_color_map[ background_color ] << "m"
		<< text
		<< "\033[0m";
}

#else

#error "Please add platform support!"

#endif
