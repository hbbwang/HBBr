
#ifndef _COMMON_HLSL
#define _COMMON_HLSL

#define MVP mul(World,mul(Projection,View))
#define VP  mul(Projection,View)

#define GameTime (CameraPos_GameTime.w)
#define CameraPos (CameraPos_GameTime.xyz)
#define CameraVector (CameraDirection.xyz)
#define ScreenSize (ScreenInfo.xy)

cbuffer Pass :register(b0)
{
    float4x4 View;
    float4x4 View_Inv;
    float4x4 Projection;
    float4x4 Projection_Inv;
    float4x4 ViewProj;
    float4x4 ViewProj_Inv;
    float4 ScreenInfo; // screen xy,z near,w zfar
    float4 CameraPos_GameTime;
    float4 CameraDirection;
};

cbuffer Pass :register(b0 ,space1)
{
    float4x4 World;
};

struct VSToPS
{
    float4 SVPosition       : SV_POSITION;
    float4 Color            : COLOR;
    float4 Texcoord01       : TEXCOORD0;
    float4 Texcoord23       : TEXCOORD1;
    float3 WorldNormal      : NORMAL;
    float3 WorldTangent     : TANGENT;
    float3 WorldBitangent   : BINORMAL;
    float3 LocalPosition    : TEXCOORD2;
    float3 WorldPosition    : TEXCOORD3;
};

struct PixelShaderParameter
{
    float3  BaseColor;
    float3  Emissive;
    float   Roughness;
    float   Metallic;
    float3  WorldNormal;
    float3  WorldTangent;
    float3  WorldBitangent;
    float3  WorldPosition;
    float3  LocalPosition;
};

void InitVSToPS(inout VSToPS vs2ps)
{
    vs2ps.SVPosition = float4(0,0,0,1);
    vs2ps.Color = float4(0,0,0,1);
    vs2ps.Texcoord01 = float4(0,0,0,1);
    vs2ps.Texcoord23 = float4(0,0,0,1);
    vs2ps.WorldNormal = float3(0,0,0);
    vs2ps.WorldTangent = float3(0,0,0);
    vs2ps.WorldBitangent= float3(0,0,0);
    vs2ps.WorldPosition = float3(0,0,0);
}

float3 DirectionLocalToWorld(in float3 LocalDir)
{
    float3x3 worldDirMatrix = (float3x3)World;
    return mul(worldDirMatrix , LocalDir).xyz;
}






#endif