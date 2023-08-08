#include "ShaderCompiler.h"
#include "FileSystem.h"
#if IS_EDITOR
#include <fstream>
#include "ConsoleDebug.h"
#include "glm/glm.hpp"
#include "shaderc/shaderc.hpp"
#include <string>  
#include <iostream> 

#pragma comment(lib,"./shaderc/shaderc_shared.lib")

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
std::vector<HString> samePath;
class ShaderIncluder : public  shaderc::CompileOptions::IncluderInterface 
{
public:
	std::string content;
	shaderc_include_result* GetInclude(const char* requested_source,
		shaderc_include_type type,
		const char* requesting_source,
		size_t include_depth) override {
		// 创建一个新的 shaderc_include_result 结构体来保存结果。
		auto* result = new shaderc_include_result;
		// 读取文件内容。
		HString IncludePath = FileSystem::GetShaderIncludeAbsPath() + requested_source;
		IncludePath.CorrectionPath();
		std::ifstream file(IncludePath.c_str());
		if (file.good()) {
			if (std::find(samePath.begin(), samePath.end(), IncludePath) != samePath.end())
			{
				MessageOut(HString(HString(requesting_source) + (": Shader Include exist.")).c_str(), false, false, "255,255,0");
				delete result;
				result = nullptr;
				return result;
			}
			samePath.push_back(IncludePath);
			content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str();
			result->content = content.c_str();
			result->content_length = content.size();
			result->source_name = requested_source;
			result->source_name_length = strlen(requested_source);
			result->user_data = nullptr;
		}
		else {
			// 如果无法打开文件，你可能需要处理错误。
			// 在这个例子中，我们简单地删除了结果并返回 nullptr。
			MessageOut(HString(HString(requesting_source) + (": Shader Include Error.")).c_str(),false,true,"255,0,0");
			delete result;
			result = nullptr;
		}

		return result;
	}

	void ReleaseInclude(shaderc_include_result* data) override {
		// 释放我们在 GetInclude 中分配的内存。
		delete data;
	}
};

void Shaderc::ShaderCompiler::CompileShader(const char* srcShaderFileFullPath, const char* entryPoint, CompileShaderType shaderType)
{
	{
		//导入shader源码
		std::ifstream shaderSrcFile(srcShaderFileFullPath);
		std::string shaderSrcCode((std::istreambuf_iterator<char>(shaderSrcFile)), std::istreambuf_iterator<char>());
		HString _shaderSrcCode = shaderSrcCode.c_str();
		//
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		ShaderCacheHeader header = {};
		//自定义的信息进去
		{
			auto line = _shaderSrcCode.Split("\n");
			bool bSearchVertexInputLayout = false;
			for (auto s : line)
			{
				s.ClearSpace();
				HString setting = s;
				auto settings = setting.Split("]");
				if (settings[0].IsSame("//[Flags"))
				{
					auto flagStr = settings[1].Split(";");
					for (auto i : flagStr)
					{
						if (i.IsSame("EnableShaderDebug"))
						{
							header.flags |= EnableShaderDebug;
						}
					}
				}
				else if (settings[0].IsSame("//[InputLayout"))
				{
					bSearchVertexInputLayout = true;
				}
				else if (bSearchVertexInputLayout)
				{
					//查找类型符号
					//[0]pos:POSITION , [1]nor:NORMAL , [2]tar:TANGENT , [3]col:COLOR , [4]uv01:TEXCOORD0 , [5]uv23:TEXCOORD1
					bool bFind = false;
					int size = 0;
					if (s.Contains("float3"))
					{
						bFind = true;
						size = 3;
					}
					else if (s.Contains("float4"))
					{
						bFind = true;
						size = 4;
					}
					if (bFind)
					{
						auto sem = s.Split(":");
						if (sem.size() > 1)
						{
							if (sem[1].Contains("POSITION"))
								header.vertexInput[0] = size;
							else if (sem[1].Contains("NORMAL"))
								header.vertexInput[1] = size;
							else if (sem[1].Contains("TANGENT"))
								header.vertexInput[2] = size;
							else if (sem[1].Contains("COLOR"))
								header.vertexInput[3] = size;
							else if (sem[1].Contains("TEXCOORD0"))
								header.vertexInput[4] = size;
							else if (sem[1].Contains("TEXCOORD1"))
								header.vertexInput[5] = size;
						}
					}
					//结构体结束
					if (s.Contains("};"))
					{
						bSearchVertexInputLayout = false;
					}
				}
			}
		}
		//
		if ((header.flags & EnableShaderDebug)) {
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
		HString ResourcePath = FileSystem::GetShaderIncludeAbsPath();
		ResourcePath.CorrectionPath();
		options.SetIncluder(std::make_unique<ShaderIncluder>());
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
			HString cacheOnlyPath = (FileSystem::GetShaderCacheAbsPath()).c_str();
			HString cachePath = cacheOnlyPath + fileName.GetBaseName();
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
			//Cache 路径不存在
			if (!FileSystem::FileExist(cacheOnlyPath.c_str()))
			{
				std::filesystem::create_directory(cacheOnlyPath.c_str());
			}
			std::ofstream out(cachePath.c_str(), std::ios::binary );
			if (out.is_open())
			{
				//Header
				out.write((char*)&header, sizeof(ShaderCacheHeader));
				//ShaderCache
				out.write((char*)resultChar.data(), resultChar.size() * sizeof(glm::uint));
				out.close();
				//Log
				HString compileResultStr = TEXT("Compile shader [");
				compileResultStr += srcShaderFileFullPath;
				compileResultStr += TEXT("] successful.");
				ConsoleDebug::print_endl(compileResultStr, TEXT("0,255,50"));
			}
			else
			{
			}
		}
		samePath.clear();
	}
}




#endif