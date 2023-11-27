[Flag]//Shader flags
{
    EnableShaderDebug;
};

#define MATERIAL_SHADINGMODEL_UNLIT   1

#include "Include/Config.hlsl"
#include "Include/Common.hlsl"
#include "Include/ShadingModel.hlsl"

[InputLayout]
struct VSInput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
};

#define CustomVSMain
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    InitVSToPS(OUT);

    //Transform MVP
    OUT.LocalPosition = IN.Position;
    float4 WorldPosition = mul(World, float4(IN.Position,1.0));
    OUT.SVPosition = mul(VP, WorldPosition);
    OUT.WorldPosition = WorldPosition.xyz;

    //Transform TangentSpace
    OUT.WorldNormal =  normalize(DirectionLocalToWorld(IN.Normal));

    //Others
    OUT.CameraVector = (OUT.WorldPosition.xyz - CameraPos.xyz);

    return OUT;
}


//像素着色器补充
void frag(in VSToPS IN , inout PixelShaderParameter Parameters)
{
    half3 color = (sin(GameTime) * 0.5 + 0.5) * half3(1,0,1);
    Parameters.BaseColor = 0;
    Parameters.Emissive = color;
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"