﻿#pragma once
//Shader编译类
#include "Common.h"
#include <vector>
#include "Shader.h"
//32bit
#define EmptyVariantBit "00000000"

enum class CompileShaderType
{
	VertexShader = 0,
	PixelShader = 1,
	ComputeShader = 2,
};

#if IS_EDITOR
//Shader Compiler(shaderc)
namespace Shaderc
{
	class ShaderCompiler
	{
		public:
		/* Auto find all fx shaders for compile in the path. */
		HBBR_API static void CompileAllShaders(const char* srcShaderPath);

		/* Compile single shader. */
		HBBR_API static void CompileShader(const char* srcShaderFileFullPath,const char* entryPoint, CompileShaderType shaderType);

		HBBR_API static void PostProcessShaderCache(std::vector<uint32_t> shaderCacheData, ShaderCacheHeader& header, std::vector<ShaderParameterInfo>& params, std::vector < ShaderTextureInfo>& tex);

		static int _bEnableShaderDebug;
		HBBR_API static void SetEnableShaderDebug(bool bEnable) {
			_bEnableShaderDebug = bEnable ? 1 : 0;
		}

	};
};
#endif