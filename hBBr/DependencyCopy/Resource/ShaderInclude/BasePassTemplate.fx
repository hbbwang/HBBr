//[Flags]EnableShaderDebug;
//[Varient]USE_COLOR;
#include "Include/Common.hlsl"

cbuffer Material :register(b0, space2)
{
    [Name=F_1; Default=1;]
    float F1;
    [Name=F_2; Default=1,1;]
    float2 F2;
    [Name=F_3; Default=1,1,1;]
    float3 F3;
    [Default=1,1,1,1;]
    float4 F4;
};

//[InputLayout]
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

[Filter=Linear; Address=Wrap;]
SamplerState BaseTextureSampler : register(s0,space3);
[Name=BaseTexture; Default=UVGrid;]
Texture2D BaseTexture : register(t0,space3);

//像素着色器补充
void frag(in VSToPS IN , inout PixelShaderParameter Parameters)
{
    //Init
    Parameters.BaseColor        = 0.0f;
    Parameters.Metallic         = 0.0f;
    Parameters.Roughness        = 1.0f;
    Parameters.Emissive         = 0.0f;
    Parameters.WorldNormal      = normalize(IN.WorldNormal);
    Parameters.WorldTangent     = normalize(IN.WorldTangent);
    Parameters.WorldBitangent   = normalize(IN.WorldBitangent);
    Parameters.WorldPosition    = IN.WorldPosition;
    Parameters.LocalPosition    = IN.LocalPosition;
    //
    half4 texSample = BaseTexture.SampleBias(BaseTextureSampler,IN.Texcoord01.xy , 0);
    Parameters.BaseColor        = F4.rgb * F4.a * F3 * F1 * texSample;
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"