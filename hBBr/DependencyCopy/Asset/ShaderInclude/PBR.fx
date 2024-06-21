[Flag]//Shader flags
{
    EnableShaderDebug;
};

[Varient]//变体定义
{
};

#define MATERIAL_SHADINGMODEL_DEFAULT_LIT   1

#include "Include/Config.hlsl"
#include "Include/Common.hlsl"
#include "Include/ShadingModel.hlsl"

[MaterialParameter]
cbuffer Material
{
    [Name=Tint; Default=1,1,1,1;]
    float4 Tint;
    [Name=Metallic; Default=0;]
    float Metallic;
    [Name=Roughness; Default=0.3;]
    float Roughness;
};

[InputLayout]
struct VSInput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float4 Color        : COLOR;
    float4 Texcoord01   : TEXCOORD0;
    float4 Texcoord23   : TEXCOORD1;
};

// //顶点着色器补充
// #define DefineVert
// void vert(in VSInput IN , inout VSToPS vs2ps)
// {
// }

Texture2D BaseTexture
{
    Name = BaseTexture;//Custom name
    Default = UVGrid;//Default value
    Filter = Linear;
    Address = Wrap;
};

//像素着色器补充
void frag(in VSToPS IN , inout PixelShaderParameter Parameters)
{
    //Init
    Parameters.BaseColor        = 0.0f;
    Parameters.Metallic         = 0.0f;
    Parameters.Roughness        = 1.0f;
    Parameters.Emissive         = 0.0f;
    //
    half4 BaseSample = BaseTexture.SampleBias(BaseTextureSampler,IN.Texcoord01.xy , 0);
    BaseSample *= Tint;
    Parameters.BaseColor = BaseSample;
    Parameters.Metallic = Metallic;
    Parameters.Roughness = Roughness;
    Parameters.Emissive = BaseSample;
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"