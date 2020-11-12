
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <thread>

#include <glslang/Public/ShaderLang.h>
#include <StandAlone/ResourceLimits.h>	// To get glslang::DefaultTBuiltInResource so we don't have to make one ourselves
#include <SPIRV/Logger.h>
#include <SPIRV/SpvTools.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>

using namespace std;
namespace fs = std::filesystem;


vector<string> entrypoint_names {};

fs::path temp_path						= "temp";
fs::path entrypoint_names_file_name		= "EntrypointNames.txt";


struct FileEntry {
	EShLanguage			stage				= {};
	string				text				= {};
	string				entrypoint_name		= {};
	fs::path			source_file			= {};
	fs::path			destination_file	= {};
};

enum class Options : uint64_t {
	DebugInfo
};
Options operator&( Options o1, Options o2 )
{
	return Options( uint64_t( o1 ) & uint64_t( o2 ) );
}
Options operator|( Options o1, Options o2 )
{
	return Options( uint64_t( o1 ) | uint64_t( o2 ) );
}
Options operator&=( Options & o1, Options o2 )
{
	auto o = Options( uint64_t( o1 ) & uint64_t( o2 ) );
	return o1 = o;
}
Options operator|=( Options & o1, Options o2 )
{
	auto o = Options( uint64_t( o1 ) | uint64_t( o2 ) );
	return o1 = o;
}

Options		options			= {};
fs::path	glsl_location	= {};


void			GetEntrypointNames();
bool			IsEntrypointInFile( fs::path path, string entrypoint );
FileEntry		GetGLSLFileEntryForEntrypoint( fs::path path, string entrypoint );
void			ExecuteGlslang( FileEntry file_entry );



int main( int argc, char * argv[] )
{
	glsl_location = fs::current_path();

	{
		vector<string> args;
		for( int i = 0; i < argc; ++i ) {
			args.push_back( argv[ i ] );
		}
		auto argit = args.begin();
		while( argit != args.end() ) {
			string a = *argit;
			transform( a.begin(), a.end(), a.begin(), []( unsigned char c )
				{
					return tolower( c );
				} );

			if( a == "-debuginfo" ) {
				options |= Options::DebugInfo;
			} else if( a == "-shaderpath" ) {
				++argit;
				if( argit == args.end() ) {
					cout << "-ShaderPath option needs to be followed with a directory path where the GLSL shaders reside.\n";
					exit( -1 );
				}
				glsl_location = *argit;
			}

			++argit;
		}
	}

	cout << "Running GLSL to SPV header compiler.\n"
		<< "    GLSL shader search path: " << glsl_location << "\n\n";

	GetEntrypointNames();

	vector<FileEntry> file_entries;

	for( auto & entrypoint : entrypoint_names ) {
		for( auto & file : fs::directory_iterator( glsl_location ) ) {
			if( file.path().extension() == ".vert" ||
				file.path().extension() == ".tesc" ||
				file.path().extension() == ".tese" ||
				file.path().extension() == ".geom" ||
				file.path().extension() == ".frag" ||
				file.path().extension() == ".comp" ) {
				if( IsEntrypointInFile( file, entrypoint ) ) {
					file_entries.push_back( std::move( GetGLSLFileEntryForEntrypoint( file, entrypoint ) ) );
				}
			}
		}
	}

	// This would be the perfect spot for multithreading, if only glslang would allow it...
	// Might be worth looking into making this a multiple program call instead.
	for( auto & fe : file_entries ) {
		ExecuteGlslang( fe );
	}

	// Create header file that includes every shader
	auto include_file_path = glsl_location / "Spir-V" / "IncludeAllShaders.h";
	auto include_file = ofstream( include_file_path );
	include_file << "#pragma once\n\n";
	for( auto & e : file_entries ) {
		auto filename = e.destination_file.filename();
		include_file << "#include " << filename << "\n";
	}

	cout << "GLSL to SPV header compiler completed successfully." << endl;

	return 0;
}


void GetEntrypointNames()
{
	auto entrypoint_names_file = glsl_location / entrypoint_names_file_name;
	auto f = ifstream( entrypoint_names_file );
	if( !f.is_open() ) {
		cout << "Couldn't open entrypoint names file: " << entrypoint_names_file << "\n";
		exit( -1 );
	}
	string line;
	while( getline( f, line ) ) {
		// Remove comments.
		{
			auto comment_begin = line.find( "//" );
			if( comment_begin != string::npos ) {
				line.replace( comment_begin, string::npos, "" );
			}
		}
		// Remove whitespace.
		line.erase( std::remove_if( line.begin(), line.end(), ::isspace ), line.end() );

		// Get entrypoint names if any left from comment and whitespace removal.
		if( line.size() ) {
			entrypoint_names.push_back( line );
		}
	}
}


bool IsEntrypointInFile( fs::path path, string entrypoint )
{
	auto file = ifstream( path );
	std::string line;
	while( std::getline( file, line ) ) {
		auto pos = line.find( entrypoint );
		if( pos != string::npos ) {
			// Found a match
			return true;
		}
	}
	return false;
}



FileEntry GetGLSLFileEntryForEntrypoint( fs::path path, string entrypoint )
{
	FileEntry		file_entry	= {};
	ifstream		file		= ifstream( path );

	auto file_extension = path.extension();
	if( file_extension == ".vert" ) {
		file_entry.stage = EShLanguage::EShLangVertex;
	} else if( file_extension == ".tesc" ) {
		file_entry.stage = EShLanguage::EShLangTessControl;
	} else if( file_extension == ".tese" ) {
		file_entry.stage = EShLanguage::EShLangTessEvaluation;
	} else if( file_extension == ".geom" ) {
		file_entry.stage = EShLanguage::EShLangGeometry;
	} else if( file_extension == ".frag" ) {
		file_entry.stage = EShLanguage::EShLangFragment;
	} else if( file_extension == ".comp" ) {
		file_entry.stage = EShLanguage::EShLangCompute;
	} else {
		cout << "Invalid file extension. Shouldn't get this.\n";
		exit( -1 );
	}

	file_entry.entrypoint_name		= entrypoint;
	file_entry.source_file			= path;
	file_entry.destination_file		= path;
	file_entry.destination_file.remove_filename();
	file_entry.destination_file		/= fs::path( "Spir-V" ) / entrypoint;
	file_entry.destination_file		+= path.extension();
	file_entry.destination_file		+= ".spv.h";

	string line;
	while( getline( file, line ) ) {
		auto pos = line.find( entrypoint );
		if( pos != string::npos ) {
			line.replace( pos, entrypoint.size(), "main" );
		}
		file_entry.text += line + "\n";
	}

	return file_entry;
}


void ExecuteGlslang( FileEntry file_entry )
{
	glslang::InitializeProcess();

	{
		glslang::GetEsslVersionString();
		glslang::GetGlslVersionString();

		TBuiltInResource build_in_resource	= glslang::DefaultTBuiltInResource;

		glslang::TShader shader( file_entry.stage );
		auto text			= file_entry.text.c_str();
		auto text_lenght	= int( file_entry.text.length() );
		auto name_str		= file_entry.source_file.extension().string();
		auto name			= name_str.c_str();
		shader.setStringsWithLengthsAndNames( &text, &text_lenght, &name, 1 );
		shader.setEnvInput( glslang::EShSource::EShSourceGlsl, file_entry.stage, glslang::EShClient::EShClientVulkan, 450 );
		shader.setEnvClient( glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2 );
		shader.setEnvTarget( glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0 );

		bool compile_failed		= false;
		if( !shader.parse( &build_in_resource, 110, false, EShMessages::EShMsgDefault ) ) {
			compile_failed		= true;
		}

		if( shader.getInfoLog() && std::strlen( shader.getInfoLog() ) ) {
			string info_log	= shader.getInfoLog();

			vector<string> suppressed_warnings {
				"extension not supported: GL_KHR_vulkan_glsl"
			};

			if( none_of( suppressed_warnings.begin(), suppressed_warnings.end(), [&info_log]( string & s )
				{
					return info_log.find( s ) != string::npos;
				} ) ) {

				// No suppressed warnings, display it.
				cout << "Shader info log: " << shader.getInfoLog() << endl;
			}
		}
		if( shader.getInfoDebugLog() && std::strlen( shader.getInfoDebugLog() ) ) {
			cout << "Shader info debug log: " << shader.getInfoDebugLog() << endl;
		}

		if( compile_failed ) {
			cout << "Compiling failed: " << file_entry.destination_file << "\n";
			exit( -1 );
		}

		glslang::TProgram program;
		program.addShader( &shader );

		bool link_failed		= false;
		if( !program.link( EShMessages::EShMsgDefault ) ) {
			link_failed			= true;
		}
		if( !program.mapIO() ) {
			link_failed			= true;
		}

		if( program.getInfoLog() && std::strlen( program.getInfoLog() ) ) {
			cout << "Program info log: " << program.getInfoLog() << endl;
		}
		if( program.getInfoDebugLog() && std::strlen( program.getInfoDebugLog() ) ) {
			cout << "Program info debug log: " << program.getInfoDebugLog() << endl;
		}

		if( link_failed ) {
			cout << "Linking failed: " << file_entry.destination_file << "\n";
			exit( -1 );
		}

//		program.buildReflection();
//		program.dumpReflection();

		if( program.getIntermediate( file_entry.stage ) ) {
			std::vector<unsigned int>		spirv;
			spv::SpvBuildLogger				logger;

			glslang::SpvOptions				spv_options;
			spv_options.generateDebugInfo	= bool( options & Options::DebugInfo );
			spv_options.disableOptimizer	= false;
			spv_options.optimizeSize		= true;
			spv_options.disassemble			= false;
			spv_options.validate			= false;
			glslang::GlslangToSpv( *program.getIntermediate( file_entry.stage ), spirv, &logger, &spv_options );
			auto logger_messages = logger.getAllMessages();
			if( logger_messages.size() ) {
				cout << "Logger messages: \n" << logger.getAllMessages() << endl;
			}

			auto extension_name		= file_entry.source_file.extension().string();
			extension_name			= extension_name.substr( 1 );

			// Save to a header file
			ofstream header_file = ofstream( file_entry.destination_file );
			header_file
				<< "#pragma once\n"
				<< "#include <array>\n"
				<< "#include <stdint.h>\n"
				<< "std::array<uint32_t, " << spirv.size() << "> "
				<< file_entry.entrypoint_name << "_" << extension_name << "_shader_data {";

			for( size_t i = 0; i < spirv.size(); ++i ) {
				stringstream w;
				if( !( i % 10 ) ) {
					w << "\n\t";
				}
				w << "0x" << std::uppercase << std::setfill( '0' ) << std::setw( 8 ) << std::hex << spirv[ i ];
				if( i != spirv.size() - 1 ) {
					w << ", ";
				}
				header_file << w.str();
			}

			header_file << "\n};\n";
		}
	}
	glslang::FinalizeProcess();

	cout << "Successfully converted shader entrypoint:\n"
		<< "    " << file_entry.entrypoint_name << "\n"
		<< "    From source file: " << file_entry.source_file << "\n"
		<< "    To header file: " << file_entry.destination_file << "\n" << endl;
}
