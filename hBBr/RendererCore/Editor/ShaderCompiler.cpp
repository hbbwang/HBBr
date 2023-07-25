#include "ShaderCompiler.h"
#include "FileSystem.h"
#if IS_EDITOR
#pragma comment(lib,"./shaderc/shaderc_shared.lib")
#include <fstream>
#include "ConsoleDebug.h"
#include "glm/glm.hpp"
#include "shaderc/shaderc.hpp"

void Shaderc::ShaderCompiler::CompileAllShaders(const char* srcShaderPath)
{
	auto allFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath , "fx");
	auto allvFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "vfx");
	auto allpFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "pfx");
	auto allcFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "cfx");
	//fx
	if (allFxShaders.size() > 0)
	{
		for (auto i : allFxShaders)
		{
			CompileShader(i.absPath.c_str(), "VSMain", CompileShaderType::VertexShader);
			CompileShader(i.absPath.c_str(), "PSMain", CompileShaderType::PixelShader);
		}
	}
	//vfx
	if (allvFxShaders.size() > 0)
	{
		for (auto i : allvFxShaders)
		{
			CompileShader(i.absPath.c_str(), "VSMain", CompileShaderType::VertexShader);
		}
	}
	//pfx
	if (allpFxShaders.size() > 0)
	{
		for (auto i : allpFxShaders)
		{
			CompileShader(i.absPath.c_str(), "PSMain", CompileShaderType::PixelShader);
		}
	}
	//cfx
	if (allcFxShaders.size() > 0)
	{
		for (auto i : allcFxShaders)
		{
			CompileShader(i.absPath.c_str(), "CSMain", CompileShaderType::ComputeShader);
		}
	}
}

class ShaderIncluder : public  shaderc::CompileOptions::IncluderInterface
{
public:
	ShaderIncluder(HString currentShaderPath) :shaderc::CompileOptions::IncluderInterface()
	{
		_currentShaderPath = currentShaderPath;
	}
private:
	HString _currentShaderPath;
	HString _shaderName;
	HString _codeContents;
	//std::unique_ptr<char> str_contents = NULL;
	shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)override
	{
		HString path = _currentShaderPath + requested_source;
		path.CorrectionPath();
		std::ifstream t(path.c_str());
		std::string contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		_codeContents = contents.c_str();
		//std::ifstream file(path.c_str(), std::ios::ate);
		//size_t fileSize = static_cast<size_t>(file.tellg());
		//str_contents.reset( new char[fileSize]);
		//file.seekg(0);
		//file.read(str_contents.get(), fileSize);
		//file.close();
		//ConsoleDebug::print_endl(contents.c_str());
		_shaderName = path.GetFileName();
		//data->user_data = useData->data();
		shaderc_include_result* _data = new shaderc_include_result;
		_data->source_name = _shaderName.c_str();
		_data->source_name_length = _shaderName.Length();
		_data->content = _codeContents.c_str();
		_data->content_length = _codeContents.Length();
		return _data;
	};
	void ReleaseInclude(shaderc_include_result* data) override
	{
		delete data;
		//delete[]str_contents;
	};
};

void Shaderc::ShaderCompiler::CompileShader(const char* srcShaderFileFullPath, const char* entryPoint, CompileShaderType shaderType, ShaderCompileFlags flags)
{
	{
		//导入shader源码
		std::ifstream shaderSrcFile(srcShaderFileFullPath);
		std::string shaderSrcCode((std::istreambuf_iterator<char>(shaderSrcFile)), std::istreambuf_iterator<char>());
		HString _shaderSrcCode = shaderSrcCode.c_str();
		//
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		//
		if ((flags & EnableShaderDebug)) {
			options.SetOptimizationLevel(shaderc_optimization_level_zero);
			options.SetGenerateDebugInfo();
		}
		else {
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		options.SetSourceLanguage(shaderc_source_language_hlsl);
		options.SetHlslIoMapping(true);
		//options.SetTargetSpirv(shaderc_spirv_version_1_3);
		HString fileName = srcShaderFileFullPath;
		HString filePath = fileName.GetFilePath();
		HString ResourcePath = HString::GetExePathWithoutFileName() + "Resource/";
		ResourcePath.CorrectionPath();
		options.SetIncluder(std::make_unique<ShaderIncluder>(ResourcePath));
		HString dTarget;
		auto kind = shaderc_vertex_shader;
		if (shaderType == CompileShaderType::VertexShader)
		{
			dTarget = "vs_6_5";
			kind = shaderc_vertex_shader;
		}
		else if (shaderType == CompileShaderType::PixelShader)
		{
			dTarget = "ps_6_5";
			kind = shaderc_fragment_shader;
		}
		else if (shaderType == CompileShaderType::ComputeShader)
		{
			dTarget = "cs_6_5";
			kind = shaderc_compute_shader;
		}
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(_shaderSrcCode.c_str(), _shaderSrcCode.Length(), kind, fileName.GetBaseName().c_str(), entryPoint, options);
		auto resultStatus = result.GetCompilationStatus();
		if (resultStatus != shaderc_compilation_status_success)
		{
			std::cerr << result.GetErrorMessage();
			MessageOut(result.GetErrorMessage().c_str(), false, true, "255,0,0");
			return;
		}
		else
		{
			std::vector<uint32_t> resultChar = { result.cbegin(), result.cend() };
			HString cachePath = (FileSystem::GetShaderCacheAbsPath()).c_str();
			cachePath += fileName.GetBaseName();

			if (shaderType == CompileShaderType::VertexShader)
			{
				cachePath += ("@vs");
			}
			else if (shaderType == CompileShaderType::PixelShader)
			{
				cachePath += ("@ps");
			}
			else if (shaderType == CompileShaderType::ComputeShader)
			{
				cachePath += ("@cs");
			}
			cachePath += TEXT(".spv");
			std::ofstream out(cachePath.c_str(), std::ios::binary);
			out.write((char*)resultChar.data(), resultChar.size() * sizeof(glm::uint));
			out.close();
			HString compileResultStr = TEXT("Compile shader [");
			compileResultStr += srcShaderFileFullPath;
			compileResultStr += TEXT("] successful.");
			ConsoleDebug::print_endl(compileResultStr, TEXT("0,255,50"));
		}
	}
}




#endif