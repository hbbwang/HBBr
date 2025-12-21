[Flag]//Shader flags
{
};

[Varient]//变体定义
{
};

#define MATERIAL_SHADINGMODEL_DEFAULT_LIT   1

#include "Include/Config.hlsl"
#include "Include/Common.hlsl"
#include "Include/BasePassCommon.hlsl"

//顶点着色器专用 
cbuffer MaterialVS
{
    [Default=1;Group=Default]
    float UVSize;
};

//像素着色器专用
cbuffer MaterialPS
{
    [Name=Tint;Default=1,1,1,1;Group=Default]
    float4 Tint;
    [Name=Metallic;Default=0;Group=Default]
    float Metallic;
    [Name=Roughness;Default=0.3;Group=Default]
    float Roughness;
};

//顶点着色器的输入布局必须要有，且结构名字也必须是VSInput
struct VSInput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float4 Color        : COLOR;
    float4 Texcoord01   : TEXCOORD0;
    // float4 Texcoord23   : TEXCOORD1;
    // float4 Texcoord45   : TEXCOORD2;
};

//顶点着色器补充
#define DefineVert
void vert(in VSInput IN , inout VSToPS vs2ps)
{
    vs2ps.Texcoord01 *= UVSize;
}

Texture2D BaseTexture
{
    Name = BaseTexture; //Custom name
    Default = UVGrid;   //Default value
    Group = Default;    //Group
    Filter = Linear;
    Address = Wrap;
};

TextureCube EnvironmentTexture
{
    Name = EnvironmentTexture;
    Default = Black;
    Group = Default;
    Filter = Linear;
    Address = Wrap;
};

//像素着色器补充
void frag(in VSToPS IN , inout PixelShaderParameter Parameters)
{
    half4 BaseSample = BaseTexture.SampleBias(BaseTextureSampler,IN.Texcoord01.xy , 0);
    half4 IBL = EnvironmentTexture.SampleLevel(EnvironmentTextureSampler, Parameters.WorldNormal.xyz, 0);

    half3 BaseColor = BaseSample.rgb * Tint.rgb;
    Parameters.BaseColor = BaseColor;
    Parameters.Metallic = Metallic;
    Parameters.Roughness = Roughness;
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"