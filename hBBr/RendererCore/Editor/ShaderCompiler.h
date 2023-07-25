#pragma once

//Shader编译类
#include "Common.h"
#include "HString.h"
#include <vector>

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
	EnableShaderDebug = 0x00000001,		//Useful for shader debug in renderdoc .
	HiddenInMaterial = 0x00000002,		//Shader will not be show in Material.
}EPipelineFlagBits;

#if IS_EDITOR
//Shader Compiler(shaderc)
namespace Shaderc
{
	class ShaderCompiler
	{
	public:
		/* Auto find all fx shaders for compile in the path. */
		static void CompileAllShaders(const char* srcShaderPath);

		/* Compile single shader. */
		static void CompileShader(const char* srcShaderFileFullPath,const char* entryPoint, CompileShaderType shaderType, ShaderCompileFlags flags = 0);
	};
};
#endif