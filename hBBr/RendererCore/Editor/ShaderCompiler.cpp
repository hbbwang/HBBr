#include "ShaderCompiler.h"
#include "FileSystem.h"
#if IS_EDITOR
#include <fstream>
#include "ConsoleDebug.h"
#include "glm/glm.hpp"
#include "ThirdParty/shaderc/shaderc.hpp"
#include "Asset/Material.h"

#include <string>  
#include <iostream> 

#pragma comment(lib,"shaderc/shaderc_shared.lib")

#ifdef _DEBUG
#include "spirv_cross/spirv_reflect.hpp"
#pragma comment(lib,"vulkan/Debug/spirv-cross-cd.lib")
#pragma comment(lib,"vulkan/Debug/spirv-cross-cored.lib")
#pragma comment(lib,"vulkan/Debug/spirv-cross-cppd.lib")
#pragma comment(lib,"vulkan/Debug/spirv-cross-glsld.lib")
#pragma comment(lib,"vulkan/Debug/spirv-cross-hlsld.lib")
#pragma comment(lib,"vulkan/Debug/spirv-cross-reflectd.lib")

#else

#include "spirv_cross/spirv_reflect.hpp"
#pragma comment(lib,"vulkan/Release/spirv-cross-c.lib")
#pragma comment(lib,"vulkan/Release/spirv-cross-core.lib")
#pragma comment(lib,"vulkan/Release/spirv-cross-cpp.lib")
#pragma comment(lib,"vulkan/Release/spirv-cross-glsl.lib")
#pragma comment(lib,"vulkan/Release/spirv-cross-hlsl.lib")
#pragma comment(lib,"vulkan/Release/spirv-cross-reflect.lib")

#endif

int Shaderc::ShaderCompiler::_bEnableShaderDebug = 0;

void Shaderc::ShaderCompiler::CompileAllShaders(const char* srcShaderPath)
{
	auto allFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath , "fx");
	auto allvFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "vfx");
	auto allpFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "pfx");
	auto allcFxShaders = FileSystem::GetFilesBySuffix(srcShaderPath, "cs");
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
	//cs
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
	std::vector<HString> samePath;
	std::vector<std::string>contents;
	std::vector<std::string>source_names;
	//这是个循环函数,content必须要存起来才能正常处理嵌套关系
	shaderc_include_result* GetInclude(const char* requested_source,
		shaderc_include_type type,
		const char* requesting_source,
		size_t include_depth) override {
		auto* result = new shaderc_include_result;
		// 读取文件内容。
		HString IncludePath = FileSystem::GetShaderIncludeAbsPath() + requested_source;
		FileSystem::CorrectionPath(IncludePath);
		std::ifstream file(IncludePath.c_str());
		if (file.good()) {
			contents.push_back(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str());
			source_names.push_back(requested_source);
			result->content = contents[contents.size() - 1].c_str();
			result->content_length = contents[contents.size() - 1].size();
			result->source_name = source_names[source_names.size() - 1].c_str();
			result->source_name_length = source_names[source_names.size() - 1].size();
			result->user_data = nullptr;
			if (std::find(samePath.begin(), samePath.end(), IncludePath) != samePath.end())
			{
				MessageOut(HString(HString(requesting_source) + (": Shader Include exist.")), false, false, "255,255,0");
				result->content = " \n\n ";
				result->content_length = strlen(result->content);
			}
			else
			{
				samePath.push_back(IncludePath);
			}
		}
		//MessageOut(contents[contents.size() - 1], false, false);
		return result;
	}

	void ReleaseInclude(shaderc_include_result* data) override {
		// 释放我们在 GetInclude 中分配的内存。
		delete data;
		data = nullptr;
	}
};

HString GetTextureNameFromShaderCodeLine(HString line)
{
	auto setting = line.ClearSpace();
	auto splitTex = setting.Split(">");
	HString texName;
	if (splitTex.size() <= 1)
	{
		texName = setting;
		//texture1/2/3D
		if (setting[7] == '1' && setting[8] == 'D'
			|| setting[7] == '2' && setting[8] == 'D'
			|| setting[7] == '3' && setting[8] == 'D')
		{
			texName.Remove(0, 9);
		}
		else if (setting[9] == 'b' && setting[10] == 'e')//cube
		{
			texName.Remove(0, 11);
		}
		else if (setting[12] == 'a' && setting[13] == 'y')//texture(1/2/3D)Array
		{
			texName.Remove(0, 14);
		}
	}
	else
	{
		texName = splitTex[1];
	}

	//Clear ";"
	texName = texName.Split(";")[0];

	return texName;
}

void ExecCompileShader(HString& _shaderSrcCode, HString& fileName, const char* entryPoint, CompileShaderType shaderType
	, ShaderCacheHeader& header, std::vector<ShaderParameterInfo>& shaderParamInfos, std::vector<ShaderTextureInfo>& shaderTextureInfos, const char* srcShaderFileFullPath, std::vector<ShaderVarientGroup>&varients, std::vector<HString>& macroDefines)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	if ((header.flags & EnableShaderDebug)) {
		options.SetOptimizationLevel(shaderc_optimization_level_zero);
		options.SetGenerateDebugInfo();
	}
	else {
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
	}
	options.SetSourceLanguage(shaderc_source_language_hlsl);
	options.SetHlslIoMapping(true);
	options.SetIncluder(std::make_unique<ShaderIncluder>());
	//options.SetTargetSpirv(shaderc_spirv_version_1_3);

	//Pre macro defines
	for (auto& m: macroDefines)
	{
		options.AddMacroDefinition(m.c_str(), "1");
	}

	//Platform
	if (VulkanManager::GetManager()->GetPlatform() == EPlatform::Windows)//Default
	{
		options.AddMacroDefinition("PLATFORM_WINDOWS","1");
	}
	else if (VulkanManager::GetManager()->GetPlatform() == EPlatform::Android)
	{
		options.AddMacroDefinition("PLATFORM_ANDROID", "1");
	}
	else if (VulkanManager::GetManager()->GetPlatform() == EPlatform::Linux)
	{
		options.AddMacroDefinition("PLATFORM_LINUX", "1");
	}

	for (int i = 0; i < varients.size(); i++)
	{
		options.AddMacroDefinition(varients[i].name, std::string(1, varients[i].defaultValue));
	}

	shaderc_shader_kind kind = shaderc_vertex_shader;
	if (shaderType == CompileShaderType::VertexShader)
	{
		kind = shaderc_vertex_shader;
	}
	else if (shaderType == CompileShaderType::PixelShader)
	{
		kind = shaderc_fragment_shader;
	}
	else if (shaderType == CompileShaderType::ComputeShader)
	{
		kind = shaderc_compute_shader;
	}

	//OutputDebugStringA(_shaderSrcCode.c_str());
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(_shaderSrcCode.c_str(), _shaderSrcCode.Length(), kind, fileName.GetBaseName().c_str(), entryPoint, options);
	auto resultStatus = result.GetCompilationStatus();
	if (resultStatus != shaderc_compilation_status_success)
	{
		std::cerr << result.GetErrorMessage();
		MessageOut(result.GetErrorMessage(), false, true, "255,0,0");
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

		HString varientString;
		if (varients.size() > 0)
		{
			for (int v = 0; v < varients.size(); v++)
			{
				varientString += varients[v].defaultValue;
			}
		}
		else
		{
			varientString = "0";
		}
		cachePath += TEXT("@") + varientString;
		cachePath += TEXT(".spv");
		//Cache 路径不存在
		if (!FileSystem::FileExist(cacheOnlyPath.c_str()))
		{
			std::filesystem::create_directory(cacheOnlyPath.c_str());
		}
		std::ofstream out(cachePath.c_str(), std::ios::binary);
		if (out.is_open())
		{
			//通过spirv-cross获取shader信息，二次处理shaderCache
			Shaderc::ShaderCompiler::PostProcessShaderCache(resultChar, header, shaderParamInfos, shaderTextureInfos);
			//Header 存入
			out.write((char*)&header, sizeof(ShaderCacheHeader));
			//保存变体名
			for (int v = 0; v < varients.size(); v++)
			{
				out.write((char*)&varients[v], sizeof(ShaderVarientGroup));
			}
			//Shader 参数信息存入
			for (auto s : shaderParamInfos)
			{
				out.write((char*)&s, sizeof(ShaderParameterInfo));
			}
			for (auto s : shaderTextureInfos)
			{
				out.write((char*)&s, sizeof(ShaderTextureInfo));
			}
			//ShaderCache
			out.write((char*)resultChar.data(), resultChar.size() * sizeof(glm::uint));
			out.close();
			//Log
			HString compileResultStr = TEXT("Compile shader [");
			compileResultStr += srcShaderFileFullPath;
			compileResultStr += TEXT("] successful.");
			MessageOut(compileResultStr, false, false, "0,255,50");
		}
		else
		{
		}
	}
}

//编译所有排列组合
void ProcessCombination(uint32_t bits, int count,
	HString& _shaderSrcCode, HString& fileName, const char* entryPoint, CompileShaderType shaderType
	, ShaderCacheHeader& header, std::vector<ShaderParameterInfo>& shaderParamInfos,
	std::vector<ShaderTextureInfo>& shaderTextureInfos, const char* srcShaderFileFullPath, std::vector<ShaderVarientGroup>& varients, std::vector<HString>& macroDefines)
{
	std::string out = "Bit ";
	if (count == 0)
		out += ":0";
	else
	{
	}
	for (int i = 0; i < count; ++i)
	{
		bool value = (bits & (1 << i)) != 0;
		out += ":" + std::to_string(value);
		if (value) 
			varients[i].defaultValue = '1';
		else
			varients[i].defaultValue = '0';
		std::string varientMsg(1, varients[i].defaultValue);
		varientMsg = std::string("Varient : ") + varients[i].name + std::string(":") + varientMsg;
		MessageOut(varientMsg, false, false, "50,125,80");
	}
	MessageOut(out, false, false);
	ExecCompileShader(_shaderSrcCode, fileName, entryPoint, shaderType, header, shaderParamInfos, shaderTextureInfos, srcShaderFileFullPath, varients, macroDefines);
}

void GenerateCombinations(int count, uint32_t bits, int bitIndex, 
	HString& _shaderSrcCode, HString& fileName, const char* entryPoint, CompileShaderType shaderType
	, ShaderCacheHeader& header, std::vector<ShaderParameterInfo>& shaderParamInfos, 
	std::vector<ShaderTextureInfo>& shaderTextureInfos, const char* srcShaderFileFullPath, std::vector<ShaderVarientGroup>& varients, std::vector<HString>&macroDefines )
{
	if (bitIndex >= count)
		ProcessCombination(bits, count,
			_shaderSrcCode, fileName, entryPoint, shaderType, header, shaderParamInfos, shaderTextureInfos, srcShaderFileFullPath, varients, macroDefines);
	else
	{
		// 将当前位设置为 0 并递归
		GenerateCombinations(count, bits & ~(1 << bitIndex), bitIndex + 1,
			_shaderSrcCode, fileName, entryPoint, shaderType, header, shaderParamInfos, shaderTextureInfos, srcShaderFileFullPath, varients, macroDefines);
		// 将当前位设置为 1 并递归
		GenerateCombinations(count, bits | (1 << bitIndex), bitIndex + 1,
			_shaderSrcCode, fileName, entryPoint, shaderType, header, shaderParamInfos, shaderTextureInfos, srcShaderFileFullPath, varients, macroDefines);
	}
}

void Shaderc::ShaderCompiler::CompileShader(const char* srcShaderFileFullPath, const char* entryPoint, CompileShaderType shaderType)
{
	//导入shader源码
	std::ifstream shaderSrcFile(srcShaderFileFullPath);
	std::string shaderSrcCode((std::istreambuf_iterator<char>(shaderSrcFile)), std::istreambuf_iterator<char>());
	HString _shaderSrcCode = shaderSrcCode.c_str();
	//
	ShaderCacheHeader header = {} ;
	std::vector<ShaderParameterInfo> shaderParamInfosPS;
	std::vector<ShaderParameterInfo> shaderParamInfosVS;
	std::vector<ShaderTextureInfo> shaderTextureInfos;
	std::vector<HString> macroDefines;
	//
	std::vector<ShaderVarientGroup>varients;
	//自定义的信息进去
	auto line = _shaderSrcCode.Split("\n");
	{
		bool bSearchVertexInputLayout = false;
		bool bCollectMaterialParameter = false;
		bool bCollectMaterialParameterVS = false;
		bool bCollectMaterialTexture = false;
		bool bCollectMaterialFlags = false;
		bool bCollectMaterialVarients = false;
		int CollectMaterialParameterIndex = 0;
		int cBufferMaterialRegisterLineIndexPS = -1;
		int cBufferMaterialRegisterLineIndexVS = -1;
		std::vector<int> textureRegisterLineIndex;
		for (int s = 0; s < line.size(); s++)
		{
			HString setting = line[s].ClearSpace();
			//Flag
			if (setting[0] == '[' && setting[1] == 'F' && setting[2] == 'l' && setting[3] == 'a'
				&& setting[4] == 'g' && setting[5] == ']')
			{
				line[s] = "//" + line[s];
				bCollectMaterialFlags = true;
			}
			else if (bCollectMaterialFlags)
			{
				auto flagStr = setting.Split(";");
				for (auto i : flagStr)
				{
					if (i.Length() > 0 && i.IsSame("EnableShaderDebug"))
					{
						header.flags |= EnableShaderDebug;
					}
					else if (i.IsSame("NativeHLSL"))
					{
						header.flags |= NativeHLSL;
					}
					else if (i.IsSame("HideInEditor"))
					{
						header.flags |= HideInEditor;
					}
				}
				line[s] = "//" + line[s];
				if (setting.Contains("}"))
				{
					bCollectMaterialFlags = false;
					if (header.flags & NativeHLSL)//原生HLSL,不进行拓展处理
					{
						break;
					}
				}
			}
			//Varient
			else if (setting[0] == '[' && setting[1] == 'V' && setting[2] == 'a' && setting[3] == 'r'
				&& setting[4] == 'i' && setting[5] == 'e' && setting[6] == 'n' && setting[7] == 't' && setting[8] == ']')
			{
				line[s] = "//" + line[s];
				bCollectMaterialVarients = true;
			}
			else if (bCollectMaterialVarients)
			{
				auto varientLine = setting.Split(";");
				for (auto v : varientLine)
				{
					auto varientStr = v.Split("=");
					if (varientStr.size() == 2)
					{
						ShaderVarientGroup newVarient;
						//
						std::string name = varientStr[0].c_str();
						name.copy(newVarient.name, sizeof(newVarient.name));
						newVarient.name[sizeof(newVarient.name)] = '\0';
						//
						//if (varientStr[1].IsSame("true", false) || varientStr[1].IsSame("false", false))
						//{
						//	newVarient.type = ShaderVarientType::Bool;
						//}
						//else if (varientStr[1].IsNumber())
						//{
						//	newVarient.type = ShaderVarientType::Int;
						//}
						varients.push_back(newVarient);
						header.varientCount += 1;
						line[s] = "//" + line[s];
					}
					else
					{
						line[s] = "//" + line[s];
					}
				}
				if (setting.Contains("}"))
				{
					bCollectMaterialVarients = false;
				}
			}
			//InputLayout
			else if (setting[0] == 's' && setting[1] == 't' && setting[2] == 'r' && setting[3] == 'u'
				&& setting[4] == 'c' && setting[5] == 't' && setting[6] == 'V' && setting[7] == 'S'
				&& setting[8] == 'I' && setting[9] == 'n' && setting[10] == 'p' && setting[11] == 'u' && setting[12] == 't')
			{
				//line[s] = "//" + line[s];
				bSearchVertexInputLayout = true;
			}
			else if (bSearchVertexInputLayout)
			{
				//查找类型符号
				//[0]pos:POSITION , [1]nor:NORMAL , [2]tar:TANGENT , [3]col:COLOR , [4]uv01:TEXCOORD0 , [5]uv23:TEXCOORD1
				bool bFind = false;
				int size = 0;
				if (line[s].Contains("float2"))
				{
					bFind = true;
					size = 2;
				}
				else if (line[s].Contains("float3"))
				{
					bFind = true;
					size = 3;
				}
				else if (line[s].Contains("float4"))
				{
					bFind = true;
					size = 4;
				}
				if (bFind)
				{
					auto sem = line[s].Split(":");
					if (sem.size() > 1)
					{
						HString sem_clearSpace = sem[0].ClearSpace();
						if (sem_clearSpace[0] == '/' && sem_clearSpace[1] == '/')
							continue;
						//if (sem[1].Contains("POSITION"))
						//{
						//	macroDefines.push_back("USE_VERTEX_INPUT_POSITION");
						//	header.vertexInput[0] = size;
						//}
						if (sem[1].Contains("NORMAL"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_NORMAL");
							header.vertexInput[1] = 3;
						}
						else if (sem[1].Contains("TANGENT"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_TANGENT");
							header.vertexInput[2] = 3;
						}
						else if (sem[1].Contains("COLOR"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_COLOR");
							header.vertexInput[3] = 4;
						}
						else if (sem[1].Contains("TEXCOORD0"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_TEXCOORD0");
							header.vertexInput[4] = 4;
						}
						else if (sem[1].Contains("TEXCOORD1"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_TEXCOORD1");
							header.vertexInput[5] = 4;
						}
						else if (sem[1].Contains("TEXCOORD2"))
						{
							macroDefines.push_back("USE_VERTEX_INPUT_TEXCOORD2");
							header.vertexInput[6] = 4;
						}
					}
				}
				//结构体结束
				if (line[s].Contains("}"))
				{
					bSearchVertexInputLayout = false;
				}
			}
			//VS MaterialParameter
			else if (setting[0] == 'c' && setting[1] == 'b' && setting[2] == 'u' && setting[3] == 'f'
				&& setting[4] == 'f' && setting[5] == 'e' && setting[6] == 'r' && setting[7] == 'M'
				&& setting[8] == 'a' && setting[9] == 't' && setting[10] == 'e' && setting[11] == 'r'
				&& setting[12] == 'i' && setting[13] == 'a' && setting[14] == 'l' && setting[15] == 'V' && setting[16] == 'S')
				{
					bCollectMaterialParameterVS = true;
					//下一行必须是  cbuffer 的register
					//cBufferMaterialRegisterLineIndex = s + 1;
					//line[s] = "//" + line[s];
					cBufferMaterialRegisterLineIndexVS = s;
			}
			else if (bCollectMaterialParameterVS)
			{
				if (setting.Contains("}"))
				{
					bCollectMaterialParameterVS = false;
				}
				if (setting[0] == 'f' && setting[1] == 'l' && setting[2] == 'o' && setting[3] == 'a' && setting[4] == 't')
				{
					shaderParamInfosVS.push_back(ShaderParameterInfo({}));
					//提取param
					std::stringstream param(line[s].c_str());
					std::string type, name;
					param >> type >> name;
					size_t shaderParamSize = shaderParamInfosVS.size();
					if (name[name.size() - 1] == ';')
					{
						name.erase(name.begin() + name.size() - 1);
					}
					name.copy(shaderParamInfosVS[shaderParamSize - 1].name, sizeof(shaderParamInfosVS[shaderParamSize - 1].name) - 1);
					shaderParamInfosVS[shaderParamSize - 1].name[sizeof(shaderParamInfosVS[shaderParamSize - 1].name) - 1] = '\0';
					//
					if (type.compare("float4") == 0)
					{
						shaderParamInfosVS[shaderParamSize - 1].type = MPType::VSFloat4;
						shaderParamInfosVS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 4;
					}

					else if (type.compare("float3") == 0)
					{
						shaderParamInfosVS[shaderParamSize - 1].type = MPType::VSFloat3;
						shaderParamInfosVS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 3;
					}
					else if (type.compare("float2") == 0)
					{
						shaderParamInfosVS[shaderParamSize - 1].type = MPType::VSFloat2;
						shaderParamInfosVS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 2;
					}
					else if (type.compare("float") == 0)
					{
						shaderParamInfosVS[shaderParamSize - 1].type = MPType::VSFloat;
						shaderParamInfosVS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 1;
					}
					header.shaderParameterCount_vs++;
					auto lastLineStr = line[s - 1].ClearSpace();
					if (lastLineStr[0] == '[')
					{
						//提取每个属性的拓展信息
						lastLineStr.Replace("[", "");
						lastLineStr.Replace("]", "");
						auto paramProperty = lastLineStr.Split(";");//获取上一行的代码
						line[s - 1] = "//" + line[s - 1];//拓展信息非着色器正式代码,填充注释防止编译错误
						for (auto i : paramProperty)
						{
							//提取属性里的值
							auto value = i.Split("=");
							if (value.size() > 1)
							{
								if (value[0].Contains("Default", false))
								{
									auto p_values = value[1].Split(",");
									auto maxCount = std::min((int)p_values.size(), 4);
									for (int vv = 0; vv < maxCount; vv++)
										shaderParamInfosVS[shaderParamSize - 1].defaultValue[vv] = (float)HString::ToDouble(p_values[vv]);
								}
								else if (value[0].Contains("Name", false))
								{
									std::string p_name = value[1].c_str();
									size_t numCharsToCopy = std::min(p_name.size(), sizeof(shaderParamInfosVS[shaderParamSize - 1].name) - 1);
									p_name.copy(shaderParamInfosVS[shaderParamSize - 1].name, numCharsToCopy);
									shaderParamInfosVS[shaderParamSize - 1].name[numCharsToCopy] = '\0';
								}
								else if (value[0].Contains("Group", false))
								{
									std::string p_group = value[1].c_str();
									size_t numCharsToCopy = std::min(p_group.size(), sizeof(shaderParamInfosVS[shaderParamSize - 1].group) - 1);
									p_group.copy(shaderParamInfosVS[shaderParamSize - 1].group, numCharsToCopy);
									shaderParamInfosVS[shaderParamSize - 1].group[numCharsToCopy] = '\0';
								}
							}
						}
					}
				}
			}
			//MaterialParameter PS
			//else if (setting[0] == '[' && setting[1] == 'M' && setting[2] == 'a' && setting[3] == 't'
			//	&& setting[4] == 'e' && setting[5] == 'r' && setting[6] == 'i' && setting[7] == 'a' 
			//	&& setting[8] == 'l' && setting[9] == 'P' && setting[10] == 'a' && setting[11] == 'r'
			//	&& setting[12] == 'a' && setting[13] == 'm' && setting[14] == 'e' && setting[15] == 't' && setting[16] == 'e' && setting[17] == 'r' && setting[18] == ']')
			else if (setting[0] == 'c' && setting[1] == 'b' && setting[2] == 'u' && setting[3] == 'f'
				&& setting[4] == 'f' && setting[5] == 'e' && setting[6] == 'r' && setting[7] == 'M'
				&& setting[8] == 'a' && setting[9] == 't' && setting[10] == 'e' && setting[11] == 'r'
				&& setting[12] == 'i' && setting[13] == 'a' && setting[14] == 'l' && setting[15] == 'P' && setting[16] == 'S')
			{
				bCollectMaterialParameter = true;
				//下一行必须是  cbuffer 的register
				//cBufferMaterialRegisterLineIndex = s + 1;
				//line[s] = "//" + line[s];
				cBufferMaterialRegisterLineIndexPS = s;
			}
			else if (bCollectMaterialParameter)
			{
				if (setting.Contains("}"))
				{
					bCollectMaterialParameter = false;
				}
				if (setting[0] == 'f' && setting[1] == 'l' && setting[2] == 'o' && setting[3] == 'a' && setting[4] == 't')
				{
					shaderParamInfosPS.push_back(ShaderParameterInfo({}));
					//提取param
					std::stringstream param(line[s].c_str());
					std::string type, name;
					param >> type >> name;
					size_t shaderParamSize = shaderParamInfosPS.size();
					if (name[name.size() - 1] == ';')
					{
						name.erase(name.begin() + name.size() - 1);
					}
					name.copy(shaderParamInfosPS[shaderParamSize - 1].name, sizeof(shaderParamInfosPS[shaderParamSize - 1].name) - 1);
					shaderParamInfosPS[shaderParamSize - 1].name[sizeof(shaderParamInfosPS[shaderParamSize - 1].name) - 1] = '\0';
					//
					if (type.compare("float4") == 0)
					{
						shaderParamInfosPS[shaderParamSize - 1].type = MPType::PSFloat4;
						shaderParamInfosPS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 4;
					}

					else if (type.compare("float3") == 0)
					{
						shaderParamInfosPS[shaderParamSize - 1].type = MPType::PSFloat3;
						shaderParamInfosPS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 3;
					}
					else if (type.compare("float2") == 0)
					{
						shaderParamInfosPS[shaderParamSize - 1].type = MPType::PSFloat2;
						shaderParamInfosPS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 2;
					}
					else if (type.compare("float") == 0)
					{
						shaderParamInfosPS[shaderParamSize - 1].type = MPType::PSFloat;
						shaderParamInfosPS[shaderParamSize - 1].index = CollectMaterialParameterIndex;
						CollectMaterialParameterIndex += 1;
					}
					header.shaderParameterCount_ps++;
					auto lastLineStr = line[s - 1].ClearSpace();
					if (lastLineStr[0] == '[')
					{
						//提取每个属性的拓展信息
						lastLineStr.Replace("[", "");
						lastLineStr.Replace("]", "");
						auto paramProperty = lastLineStr.Split(";");//获取上一行的代码
						line[s - 1] = "//" + line[s - 1];//拓展信息非着色器正式代码,填充注释防止编译错误
						for (auto i : paramProperty)
						{
							//提取属性里的值
							auto value = i.Split("=");
							if (value.size() > 1)
							{
								if (value[0].Contains("Default", false))
								{
									auto p_values = value[1].Split(",");
									auto maxCount = std::min((int)p_values.size(), 4);
									for (int vv = 0; vv < maxCount; vv++)
										shaderParamInfosPS[shaderParamSize - 1].defaultValue[vv] = (float)HString::ToDouble(p_values[vv]);
								}
								else if (value[0].Contains("Name", false))
								{
									std::string p_name = value[1].c_str();
									size_t numCharsToCopy = std::min(p_name.size(), sizeof(shaderParamInfosPS[shaderParamSize - 1].name) - 1);
									p_name.copy(shaderParamInfosPS[shaderParamSize - 1].name, numCharsToCopy);
									shaderParamInfosPS[shaderParamSize - 1].name[numCharsToCopy] = '\0';
								}
								else if (value[0].Contains("Group", false))
								{
									std::string p_group = value[1].c_str();
									size_t numCharsToCopy = std::min(p_group.size(), sizeof(shaderParamInfosPS[shaderParamSize - 1].group) - 1);
									p_group.copy(shaderParamInfosPS[shaderParamSize - 1].group, numCharsToCopy);
									shaderParamInfosPS[shaderParamSize - 1].group[numCharsToCopy] = '\0';
								}
							}
						}
					}
				}
			}
			//Texture
			else if (setting[0] == 'T' && setting[1] == 'e' && setting[2] == 'x' && setting[3] == 't'
				&& setting[4] == 'u' && setting[5] == 'r' && setting[6] == 'e')
			{
				textureRegisterLineIndex.push_back(s);
				shaderTextureInfos.push_back(ShaderTextureInfo({}));
				//
				auto infoIndex = shaderTextureInfos.size() - 1 ;
				//Get index
				shaderTextureInfos[infoIndex].index = header.shaderTextureCount;
				header.shaderTextureCount++;
				bCollectMaterialTexture = true;
				//Get Name
				std::string name = GetTextureNameFromShaderCodeLine(line[s]).c_str();
				name.copy(shaderTextureInfos[infoIndex].name, sizeof(shaderTextureInfos[infoIndex].name) - 1);
				shaderTextureInfos[infoIndex].name[sizeof(shaderTextureInfos[infoIndex].name) - 1] = '\0';
				//GetType
				shaderTextureInfos[infoIndex].type = MTType::Texture2D;
				if (setting[0] == 'T' && setting[1] == 'e' && setting[2] == 'x' && setting[3] == 't'
					&& setting[4] == 'u' && setting[5] == 'r' && setting[6] == 'e' && setting[7] == 'C' && setting[8] == 'u' && setting[9] == 'b' && setting[10] == 'e')
				{
					shaderTextureInfos[infoIndex].type = MTType::TextureCube;
				}
				else if (setting[0] == 'T' && setting[1] == 'e' && setting[2] == 'x' && setting[3] == 't'
					&& setting[4] == 'u' && setting[5] == 'r' && setting[6] == 'e' && setting[7] == '2' && setting[8] == 'D' && setting[9] == 'A' && setting[10] == 'r' && setting[11] == 'r' && setting[12] == 'a' && setting[13] == 'y')
				{
					shaderTextureInfos[infoIndex].type = MTType::TextureArray;
				}
			}
			else if (bCollectMaterialTexture)
			{
				if (setting.Contains("}"))
				{
					bCollectMaterialTexture = false;
				}
				auto infoSize = shaderTextureInfos.size();
				//提取纹理属性
				auto paramProperty = setting.Split(";");//移除分号
				line[s] = "//" + line[s];//拓展信息非着色器正式代码,填充注释防止编译错误
				for (auto i : paramProperty)
				{
					//提取属性里的值
					auto value = i.Split("=");
					if (value.size() > 1)
					{
						if (value[0].Contains("Default"))
						{
							std::string defaultValue = value[1].c_str();
							size_t numCharsToCopy = std::min(defaultValue.size(), sizeof(shaderTextureInfos[infoSize - 1].defaultTexture) - 1);
							defaultValue.copy(shaderTextureInfos[infoSize - 1].defaultTexture, numCharsToCopy);
							shaderTextureInfos[infoSize - 1].defaultTexture[numCharsToCopy] = '\0';
						}
						else if (value[0].Contains("Name"))
						{
							std::string name = value[1].c_str();
							size_t numCharsToCopy = std::min(name.size(), sizeof(shaderTextureInfos[infoSize - 1].name) - 1);
							name.copy(shaderTextureInfos[infoSize - 1].name, numCharsToCopy);
							shaderTextureInfos[infoSize - 1].name[numCharsToCopy] = '\0';
						}
						else if (value[0].Contains("Filter"))
						{
							HString valueStr = value[1].ClearSpace();
							if (valueStr.IsSame("Linear", false))
								shaderTextureInfos[infoSize - 1].msFilter = MSFilter::Linear;
							else if (valueStr.IsSame("Nearest", false))
								shaderTextureInfos[infoSize - 1].msFilter = MSFilter::Nearest;
						}
						else if (value[0].Contains("Group", false))
						{
							std::string group = value[1].c_str();
							size_t numCharsToCopy = std::min(group.size(), sizeof(shaderTextureInfos[infoSize - 1].group) - 1);
							group.copy(shaderTextureInfos[infoSize - 1].group, numCharsToCopy);
							shaderTextureInfos[infoSize - 1].group[numCharsToCopy] = '\0';
						}
						else if (value[0].Contains("Address"))
						{
							HString valueStr = value[1].ClearSpace();
							if (valueStr.IsSame("Wrap", false))
								shaderTextureInfos[infoSize - 1].msAddress = MSAddress::Wrap;
							else if (valueStr.IsSame("Clamp", false))
								shaderTextureInfos[infoSize - 1].msAddress = MSAddress::Clamp;
							else if (valueStr.IsSame("Mirror", false))
								shaderTextureInfos[infoSize - 1].msAddress = MSAddress::Mirror;
							else if (valueStr.IsSame("Border", false))
								shaderTextureInfos[infoSize - 1].msAddress = MSAddress::Border;
						}
					}
				}
			}
		}
		if (!(header.flags & NativeHLSL))//原生HLSL,不进行拓展处理
		{
			//补充
			//Parameter
			if (cBufferMaterialRegisterLineIndexPS != -1 && cBufferMaterialRegisterLineIndexVS == -1)
			{
				line[cBufferMaterialRegisterLineIndexPS] += ":register(b0, space2)";
			}
			else if (cBufferMaterialRegisterLineIndexPS == -1 && cBufferMaterialRegisterLineIndexVS != -1)
			{
				line[cBufferMaterialRegisterLineIndexVS] += ":register(b0, space2)";
			}
			else if(cBufferMaterialRegisterLineIndexPS != -1 && cBufferMaterialRegisterLineIndexVS != -1)
			{
				line[cBufferMaterialRegisterLineIndexVS] += ":register(b0, space2)";
				line[cBufferMaterialRegisterLineIndexPS] += ":register(b0, space3)";
			}
			//Texture
			for (int i = 0; i < textureRegisterLineIndex.size(); i++)
			{
				HString texName = GetTextureNameFromShaderCodeLine(line[textureRegisterLineIndex[i]]);
				line[textureRegisterLineIndex[i]] = line[textureRegisterLineIndex[i]].Split(";")[0];//移除分号
				if (cBufferMaterialRegisterLineIndexVS != -1 && cBufferMaterialRegisterLineIndexPS != 1)
				{
					line[textureRegisterLineIndex[i]] += HString(":register(t") + HString::FromInt(i) + ",space4);";
					//给Texture2D补充一个SamplerState
					line[textureRegisterLineIndex[i]] += "   SamplerState " + texName + "Sampler:register(s" + HString::FromInt(i) + ",space4);";
				}
				else if (cBufferMaterialRegisterLineIndexVS == -1 && cBufferMaterialRegisterLineIndexPS != 1||
						cBufferMaterialRegisterLineIndexVS != -1 && cBufferMaterialRegisterLineIndexPS == 1
					)
				{
					line[textureRegisterLineIndex[i]] += HString(":register(t") + HString::FromInt(i) + ",space3);";
					//给Texture2D补充一个SamplerState
					line[textureRegisterLineIndex[i]] += "   SamplerState " + texName + "Sampler:register(s" + HString::FromInt(i) + ",space3);";
				}
				else
				{
					line[textureRegisterLineIndex[i]] += HString(":register(t") + HString::FromInt(i) + ",space2);";
					//给Texture2D补充一个SamplerState
					line[textureRegisterLineIndex[i]] += "   SamplerState " + texName + "Sampler:register(s" + HString::FromInt(i) + ",space2);";
				}
			}
		}
	}
	//重新组装shader code
	_shaderSrcCode.empty();
	for (auto i : line)
	{
		//OutputDebugStringA((HString("\n") + i).c_str());
		_shaderSrcCode += "\n" + i;
	}
	//
	HString fileName = srcShaderFileFullPath;

	//为每个变体编译ShaderCache
	MessageOut("--");
	HString shaderTypeStr;
	if (shaderType == CompileShaderType::VertexShader)
	{
		shaderTypeStr = ("Vertex");
	}
	else if (shaderType == CompileShaderType::PixelShader)
	{
		shaderTypeStr = ("Pixel");
	}
	else if (shaderType == CompileShaderType::ComputeShader)
	{
		shaderTypeStr = ("Compute");
	}
	MessageOut((HString("-Start Compile ") + shaderTypeStr + " Shader Permutation--"), false, false);
	
	//Global setting
	if (_bEnableShaderDebug == 1)
	{
		header.flags |= EnableShaderDebug;
	}
	//
	std::vector<ShaderParameterInfo> shaderParamInfos = shaderParamInfosVS;
	shaderParamInfos.insert(shaderParamInfos.end(), shaderParamInfosPS.begin(), shaderParamInfosPS.end());
	GenerateCombinations((int)varients.size(), 0, 0, _shaderSrcCode, fileName, entryPoint, shaderType, header, shaderParamInfos, shaderTextureInfos, srcShaderFileFullPath, varients, macroDefines);

}

void Shaderc::ShaderCompiler::PostProcessShaderCache(std::vector<uint32_t> shaderCacheData, ShaderCacheHeader& header, std::vector<ShaderParameterInfo>& params, std::vector < ShaderTextureInfo>& tex)
{
	//二次处理shader cache
	int SV_TargetCount = 0;
	spirv_cross::CompilerReflection reflector(shaderCacheData);
	uint32_t numRenderTargets = 0;
	auto executionModel = reflector.get_execution_model();
	switch (executionModel)
	{
	case spv::ExecutionModelVertex:	//Vertex Shader
	{
		ConsoleDebug::print_endl("PostProcess Shader type: Fragment");
		if (header.flags & NativeHLSL)
		{
			header.shaderTextureCount = (uint8_t)reflector.get_shader_resources().separate_images.size();
			tex.resize(header.shaderTextureCount);
		}
	}
	break;
	case spv::ExecutionModelTessellationControl:
		ConsoleDebug::print_endl("PostProcess Shader type: Tessellation Control");
		break;
	case spv::ExecutionModelTessellationEvaluation:
		ConsoleDebug::print_endl("PostProcess Shader type: Tessellation Evaluation");
		break;
	case spv::ExecutionModelGeometry:
		ConsoleDebug::print_endl("PostProcess Shader type: Geometry");
		break;
	case spv::ExecutionModelFragment: //Pixel Shader
	{
		ConsoleDebug::print_endl("PostProcess Shader type: Fragment");
		for (const auto& entry : reflector.get_shader_resources().stage_outputs)
		{
			HString outputEntryName = entry.name;
			const spirv_cross::SPIRType& type = reflector.get_type(entry.base_type_id);
			if (type.vecsize > 1)//输出格式大于float的都认为是ColorAttachment
			{
				header.colorAttachmentCount++;
			}
		}
		if (header.flags & NativeHLSL)
		{
			header.shaderTextureCount = (uint8_t)reflector.get_shader_resources().separate_images.size();
			tex.resize(header.shaderTextureCount);
		}
	}
	break;
	case spv::ExecutionModelGLCompute:
		ConsoleDebug::print_endl("PostProcess Shader type: Compute");
		break;
	case spv::ExecutionModelRayGenerationNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Ray Generation");
		break;
	case spv::ExecutionModelIntersectionNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Intersection");
		break;
	case spv::ExecutionModelAnyHitNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Any Hit");
		break;
	case spv::ExecutionModelClosestHitNV:
		ConsoleDebug::print_endl("PostProcess Shader type:  Closest Hit");
		break;
	case spv::ExecutionModelMissNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Miss");
		break;
	case spv::ExecutionModelCallableNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Callable");
		break;
	case spv::ExecutionModelTaskNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Task");
		break;
	case spv::ExecutionModelMeshNV:
		ConsoleDebug::print_endl("PostProcess Shader type: Mesh");
		break;
	default:
		std::cout << "Unknown shader type" << std::endl;
	}
}
#endif