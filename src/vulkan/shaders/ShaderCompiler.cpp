
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::ShaderCompiler::ShaderCompiler(
	vk2d_internal::InstanceImpl	&	instance
) :
	instance( instance )
{
	is_good = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vulkan::ShaderCompiler::CreateShaderModule(
	ShaderStage					stage,
	const ShaderText		&	user_shader_text
)
{
	auto spir_v_code = CompileSpirV(
		stage,
		user_shader_text
	);

	VkShaderModuleCreateInfo create_info {};
	create_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext		= nullptr;
	create_info.flags		= 0;
	create_info.codeSize	= spir_v_code.size() * sizeof( decltype( spir_v_code )::value_type );
	create_info.pCode		= spir_v_code.data();
	VkShaderModule shader_module = {};
	auto result = vkCreateShaderModule(
		instance.GetVulkanDevice(),
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
auto temp_build_in_glsl = std::string();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<uint32_t> vk2d::vulkan::ShaderCompiler::CompileSpirV(
	ShaderStage				stage,
	const ShaderText	&	user_shader_text
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto ShaderStageToEShLanguage = []( ShaderStage stage ) -> EShLanguage
	{
		switch( stage )
		{
		case vk2d::vulkan::ShaderStage::VERTEX:
			return EShLanguage::EShLangVertex;
		case vk2d::vulkan::ShaderStage::TESSELLATION_CONTROL:
			return EShLanguage::EShLangTessControl;
		case vk2d::vulkan::ShaderStage::TESSELLATION_EVALUATION:
			return EShLanguage::EShLangTessEvaluation;
		case vk2d::vulkan::ShaderStage::GEOMETRY:
			return EShLanguage::EShLangGeometry;
		case vk2d::vulkan::ShaderStage::FRAGMENT:
			return EShLanguage::EShLangFragment;
		case vk2d::vulkan::ShaderStage::COMPUTE:
			return EShLanguage::EShLangCompute;
		default:
			assert( 0 && "Invalid stage." );
			return {};
		}
	};

	// glslang::GetEsslVersionString();
	// glslang::GetGlslVersionString();

	struct {
		std::vector<const char*>	texts;
		std::vector<int>			lenghts;
		std::vector<const char*>	names;
		size_t						size;

		std::vector<std::string>	names_storage;
	} glsl_sources;

	auto AddGLSLSource = [ &glsl_sources ](
		std::string_view		code_text,
		std::string_view		code_name
	)
	{
		glsl_sources.texts.push_back( code_text.data() );
		glsl_sources.lenghts.push_back( code_text.length() );

		glsl_sources.names_storage.push_back( std::string( code_name ) );
		glsl_sources.names.push_back( glsl_sources.names_storage.back().c_str() );

		++glsl_sources.size;
	};

	// You may add build-in glsl include sources here. Just add a call to AddGLSLInclude().
	// Order matters, first included codes should also be added first.
	// 
	// !!! WARNING !!! GLSL CODE MUST BE STORED OUTSIDE OF THIS FUNCTION CALL.
	// GLSL source code is not copied to internal storage.
	// 
	// Name is copied to internal storage and may be temporary.
	AddGLSLSource( temp_build_in_glsl, "<build_in_glsl>" );
	AddGLSLSource( user_shader_text.GetCode(), user_shader_text.GetName() );



	// Automate InitializeProcess() and FinalizeProcess().
	struct GLSLANG
	{
		GLSLANG() { glslang::InitializeProcess(); }
		~GLSLANG() { glslang::FinalizeProcess(); }
	} glslang_instance;

	auto build_in_resource = glslang::DefaultTBuiltInResource;

	auto shader = glslang::TShader(
		ShaderStageToEShLanguage( stage )
	);
	shader.setStringsWithLengthsAndNames(
		glsl_sources.texts.data(),
		glsl_sources.lenghts.data(),
		glsl_sources.names.data(),
		glsl_sources.size
	);
	shader.setEnvInput( glslang::EShSource::EShSourceGlsl, shader.getStage(), glslang::EShClient::EShClientVulkan, 450 );
	shader.setEnvClient( glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2 );
	shader.setEnvTarget( glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0 );

	auto compile_success = shader.parse( &build_in_resource, 110, false, EShMessages::EShMsgDefault );

	if( shader.getInfoLog() && std::strlen( shader.getInfoLog() ) ) {
		instance.Report(
			ReportSeverity::INFO,
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
			ReportSeverity::INFO,
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
