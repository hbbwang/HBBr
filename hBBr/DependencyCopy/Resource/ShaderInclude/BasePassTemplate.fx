//[Flags]EnableShaderDebug;
//[Varient]USE_COLOR;
#include "Include/Common.hlsl"

//[MaterialParameters]
cbuffer Material :register(b0, space2)
{

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

//顶点着色器补充
void vert(in VSInput IN , inout VSToPS vs2ps)
{
    
}

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
    
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"