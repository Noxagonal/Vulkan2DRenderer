
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
	ConsoleColor			text_color,
	ConsoleColor			background_color
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

	WORD text_color_as_int			= ( text_color			== ConsoleColor::DEFAULT ) ? ( ( old_attributes & WORD( 0x000F ) ) ) : ( WORD( text_color ) );
	WORD background_color_as_int	= ( background_color	== ConsoleColor::DEFAULT ) ? ( ( old_attributes & WORD( 0x00F0 ) ) ) : ( WORD( background_color ) << 4 );
	WORD new_attributes				= background_color_as_int | text_color_as_int;


	SetConsoleTextAttribute( std_out_handle, new_attributes );
	std::cout << text;
	SetConsoleTextAttribute( std_out_handle, old_attributes );
}

#elif defined( VK2D_PLATFORM_LINUX )

#include <map>



std::map<ConsoleColor, const char*> text_color_map {
	{ ConsoleColor::DEFAULT, "39" },
	{ ConsoleColor::BLACK, "30" },
	{ ConsoleColor::DARK_RED, "31" },
	{ ConsoleColor::DARK_GREEN, "32" },
	{ ConsoleColor::DARK_YELLOW, "33" },
	{ ConsoleColor::DARK_BLUE, "34" },
	{ ConsoleColor::DARK_MAGENTA, "35" },
	{ ConsoleColor::DARK_CYAN, "36" },
	{ ConsoleColor::GRAY, "37" },
	{ ConsoleColor::DARK_GRAY, "90" },
	{ ConsoleColor::RED, "91" },
	{ ConsoleColor::GREEN, "92" },
	{ ConsoleColor::YELLOW, "93" },
	{ ConsoleColor::BLUE, "94" },
	{ ConsoleColor::MAGENTA, "95" },
	{ ConsoleColor::CYAN, "96" },
	{ ConsoleColor::WHITE, "97" }
};

std::map<ConsoleColor, const char*> background_color_map {
	{ ConsoleColor::DEFAULT, "49" },
	{ ConsoleColor::BLACK, "40" },
	{ ConsoleColor::RED, "41" },
	{ ConsoleColor::GRAY, "42" },
	{ ConsoleColor::YELLOW, "43" },
	{ ConsoleColor::BLUE, "44" },
	{ ConsoleColor::MAGENTA, "45" },
	{ ConsoleColor::CYAN, "46" },
	{ ConsoleColor::GRAY, "47" },
	{ ConsoleColor::DARK_GRAY, "100" },
	{ ConsoleColor::RED, "101" },
	{ ConsoleColor::GREEN, "102" },
	{ ConsoleColor::YELLOW, "103" },
	{ ConsoleColor::BLUE, "104" },
	{ ConsoleColor::MAGENTA, "105" },
	{ ConsoleColor::CYAN, "106" },
	{ ConsoleColor::WHITE, "107" }
};

VK2D_API void VK2D_APIENTRY vk2d::ConsolePrint(
	std::string_view		text,
	ConsoleColor			text_color,
	ConsoleColor			background_color
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
