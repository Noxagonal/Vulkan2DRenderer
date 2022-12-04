
#include <core/SourceCommon.hpp>
#include "ShaderCompiler.hpp"
#include "ShaderManager.hpp"
#include <vulkan/Device.hpp>

#include <interface/instance/InstanceImpl.hpp>

#include <span>

#include <glslang/Public/ShaderLang.h>
#include <glslang/StandAlone/ResourceLimits.h>	// To get glslang::DefaultTBuiltInResource so we don't have to make one ourselves
#include <glslang/SPIRV/Logger.h>
#include <glslang/SPIRV/SpvTools.h>
#include <glslang/SPIRV/GLSL.std.450.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "glsl/GLSLGenerators.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderCompiler::ShaderCompiler(
	vk2d_internal::InstanceImpl		&	instance,
	Device							&	vulkan_device
) :
	instance( instance ),
	vulkan_device( vulkan_device )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderCompiler::CreateShaderModule(
	const ShaderCreateInfo		&	shader_create_info
)
{
	auto spir_v_code = CompileSpirV(
		shader_create_info
	);
	if( spir_v_code.empty() ) return {};

	VkShaderModuleCreateInfo create_info {};
	create_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext		= nullptr;
	create_info.flags		= 0;
	create_info.codeSize	= spir_v_code.size() * sizeof( decltype( spir_v_code )::value_type );
	create_info.pCode		= spir_v_code.data();
	VkShaderModule shader_module = {};
	auto result = vkCreateShaderModule(
		vulkan_device,
		&create_info,
		nullptr,
		&shader_module
	);
	if( result != VK_SUCCESS ) {
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Could not create vulkan shader module." );
		return {};
	}
	return shader_module;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<uint32_t> vk2d::vulkan::ShaderCompiler::CompileSpirV(
	const ShaderCreateInfo & shader_create_info
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto ShaderStageToEShLanguage = []( ShaderStage stage ) -> EShLanguage
	{
		switch( stage )
		{
		case vk2d::ShaderStage::VERTEX:
			return EShLanguage::EShLangVertex;
		case vk2d::ShaderStage::TESSELLATION_CONTROL:
			return EShLanguage::EShLangTessControl;
		case vk2d::ShaderStage::TESSELLATION_EVALUATION:
			return EShLanguage::EShLangTessEvaluation;
		case vk2d::ShaderStage::GEOMETRY:
			return EShLanguage::EShLangGeometry;
		case vk2d::ShaderStage::FRAGMENT:
			return EShLanguage::EShLangFragment;
		case vk2d::ShaderStage::COMPUTE:
			return EShLanguage::EShLangCompute;
		default:
			assert( 0 && "Invalid stage." );
			return {};
		}
	};

	// glslang::GetEsslVersionString();
	// glslang::GetGlslVersionString();

	struct {
		std::vector<const char*>	codes;
		std::vector<int>			code_lenghts;
		std::vector<const char*>	names;
		size_t						size				= {};

		std::vector<std::string>	codes_storage;
		std::vector<std::string>	names_storage;

		void Finalize()
		{
			codes.resize( size );
			code_lenghts.resize( size );
			names.resize( size );
			for( size_t i = 0; i < size; ++i ) {
				codes[ i ]			= codes_storage[ i ].c_str();
				code_lenghts[ i ]	= codes_storage[ i ].length();
				names[ i ]			= names_storage[ i ].c_str();
			}
		}
	} glsl_sources;

	auto AddGLSLSource = [ &glsl_sources ](
		std::string_view		code,
		std::string_view		name		= "<build-in-glsl>"
	)
	{
		glsl_sources.codes_storage.push_back( std::string( code ) );
		glsl_sources.names_storage.push_back( std::string( name ) );
		++glsl_sources.size;
	};

	auto AddGLSLCommonSources = [ AddGLSLSource ]()
	{
		AddGLSLSource( glsl::GenerateInterfaceVersion( 450 ) );
		AddGLSLSource( glsl::GenerateInterfaceVertex() );
	};

	auto AddGLSLVertexShaderSpecificSources = [ AddGLSLSource ]()
	{
		AddGLSLSource( glsl::GenerateInterfaceWindowFrameData() );
		AddGLSLSource( glsl::GenerateInterfaceTransformationBuffer() );
		AddGLSLSource( glsl::GenerateInterfaceVertexBuffer() );
		AddGLSLSource( glsl::GenerateInterfacePushConstants() );
		AddGLSLSource( glsl::GenerateInterfaceVertexOutput() );
	};

	auto AddGLSLFragmentShaderSpecificSources = [ AddGLSLSource ]()
	{
		AddGLSLSource( glsl::GenerateInterfaceIndexBuffer() );
		AddGLSLSource( glsl::GenerateInterfaceVertexBuffer() );
		AddGLSLSource( glsl::GenerateInterfaceSampler() );
		AddGLSLSource( glsl::GenerateInterfaceSampledImage() );
		AddGLSLSource( glsl::GenerateInterfacePushConstants() );
		AddGLSLSource( glsl::GenerateInterfaceFragmentInput() );
		AddGLSLSource( glsl::GenerateInterfaceFragmentOutput() );
	};

	// You may add build-in glsl include sources here. Just add a call to AddGLSLInclude().
	// Order matters, first included codes should also be added first.
	AddGLSLCommonSources();
	if( shader_create_info.GetStage() == ShaderStage::VERTEX ) AddGLSLVertexShaderSpecificSources();
	if( shader_create_info.GetStage() == ShaderStage::FRAGMENT ) AddGLSLFragmentShaderSpecificSources();
	AddGLSLSource( shader_create_info.GetCode(), shader_create_info.GetName() );



	glsl_sources.Finalize();

	#if VK2D_BUILD_OPTION_VULKAN_PRINT_GLSL_BEFORE_COMPILATION && VK2D_DEBUG_ENABLE
	{
		auto debug_print = std::string( "GLSL debug printout: \n" );
		for( size_t i = 0; i < glsl_sources.size; ++i ) {
			debug_print += glsl_sources.codes[ i ];
		}
		debug_print += std::string( "\nGLSL debug printout end." );
		instance.Report( ReportSeverity::DEBUG, debug_print );
	}
	#endif

	// Automate InitializeProcess() and FinalizeProcess().
	struct GLSLANG
	{
		GLSLANG() { glslang::InitializeProcess(); }
		~GLSLANG() { glslang::FinalizeProcess(); }
	} glslang_instance;

	auto build_in_resource = glslang::DefaultTBuiltInResource;

	auto shader = glslang::TShader(
		ShaderStageToEShLanguage( shader_create_info.GetStage() )
	);
	shader.setStringsWithLengthsAndNames(
		glsl_sources.codes.data(),
		glsl_sources.code_lenghts.data(),
		glsl_sources.names.data(),
		glsl_sources.size
	);
	shader.setEnvInput( glslang::EShSource::EShSourceGlsl, shader.getStage(), glslang::EShClient::EShClientVulkan, 450 );
	shader.setEnvClient( glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2 );
	shader.setEnvTarget( glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0 );

	auto compile_success = shader.parse( &build_in_resource, 110, false, EShMessages::EShMsgDefault );

	if( shader.getInfoLog() && std::strlen( shader.getInfoLog() ) ) {
		instance.Report(
			compile_success ? ReportSeverity::WARNING : ReportSeverity::NON_CRITICAL_ERROR,
			std::string( "Shader compiler: " ) + shader.getInfoLog()
		);
	}
	if( shader.getInfoDebugLog() && std::strlen( shader.getInfoDebugLog() ) ) {
		instance.Report(
			ReportSeverity::WARNING,
			std::string( "Shader compiler: " ) + shader.getInfoDebugLog()
		);
	}

	if( !compile_success ) {
		instance.Report(
			ReportSeverity::NON_CRITICAL_ERROR,
			"Shader compilation failed."
		);
		return {};
	}

	glslang::TProgram program;
	program.addShader( &shader );

	auto link_success =
		program.link( EShMessages::EShMsgDefault ) &&
		program.mapIO();

	if( program.getInfoLog() && std::strlen( program.getInfoLog() ) ) {
		instance.Report(
			link_success ? ReportSeverity::WARNING : ReportSeverity::NON_CRITICAL_ERROR,
			std::string( "Shader linker: " ) + program.getInfoLog()
		);
	}
	if( program.getInfoDebugLog() && std::strlen( program.getInfoDebugLog() ) ) {
		instance.Report(
			ReportSeverity::WARNING,
			std::string( "Shader linker: " ) + program.getInfoDebugLog()
		);
	}

	if( !link_success ) {
		instance.Report(
			ReportSeverity::NON_CRITICAL_ERROR,
			"Shader linking failed."
		);
		return {};
	}

	// program.buildReflection();
	// program.dumpReflection();

	if( program.getIntermediate( shader.getStage() ) ) {
		std::vector<uint32_t>			spirv;
		spv::SpvBuildLogger				logger;

		glslang::SpvOptions				spv_options;
		spv_options.generateDebugInfo	= false;
		spv_options.disableOptimizer	= false;
		spv_options.optimizeSize		= true;
		spv_options.disassemble			= false;
		spv_options.validate			= false;
		glslang::GlslangToSpv( *program.getIntermediate( shader.getStage() ), spirv, &logger, &spv_options );
		auto logger_messages = logger.getAllMessages();
		if( logger_messages.size() ) {
			instance.Report(
				ReportSeverity::INFO,
				std::string( "Spir-v conversion: " ) + logger.getAllMessages()
			);
		}

		return spirv;
	}

	return {};
}
