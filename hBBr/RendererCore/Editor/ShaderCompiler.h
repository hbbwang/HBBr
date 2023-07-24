#pragma once
//Shader编译类
#include "Common.h"
#include "HString.h"

//32bit
#define EmptyVariantBit "00000000"

enum class CompileShaderType
{
	VertexShader = 0,
	PixelShader = 1,
	ComputeShader = 2,
};

typedef uint32_t ShaderCompileFlags;
typedef enum ShaderCompileFlagBits
{
	EnableShaderDebug = 0x00000001,		//only shader resource
}EPipelineFlagBits;

//DirectX Compiler
namespace DXCompiler
{
	class ShaderCompiler
	{
	public:
		static void CompileShader(const char* srcShaderFileFullPath, const char* cachePath, const char* entryPoint, CompileShaderType shaderType, ShaderCompileFlags flags = 0);
	};
};