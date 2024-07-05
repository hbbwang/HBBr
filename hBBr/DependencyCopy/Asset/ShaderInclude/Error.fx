[Flag]//Shader flags
{
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
    float theSin = sin(GameTime * 2);
    float4 WorldPosition = mul(World, float4(IN.Position,1.0)) + float4(0,theSin*0.25,0, 0) ;
    WorldPosition.xyz *= float3(1,saturate((theSin *0.5+0.5)*0.1 + 0.9),1);
    OUT.SVPosition = mul(VP, WorldPosition);
    OUT.WorldPosition = WorldPosition.xyz ;

    //Transform TangentSpace
    OUT.WorldNormal =  normalize(DirectionLocalToWorld(IN.Normal));

    //Others
    OUT.CameraVector = (OUT.WorldPosition.xyz - CameraPos.xyz);

    return OUT;
}


//像素着色器补充
void frag(in VSToPS IN , inout PixelShaderParameter Parameters)
{
    half3 color = saturate((sin(GameTime * 4) * 0.5 + 0.5) * 0.75 + 0.25) * half3(1,0,1);
    half nol = dot(normalize(half3(0.5,1,0)), IN.WorldNormal.xyz) * 0.5 + 0.5 ;
    Parameters.BaseColor = 0;
    Parameters.Emissive = color * nol;
}

#include "Include/BasePassVertexShader.hlsl"
#include "Include/BasePassPixelShader.hlsl"