[Flag]
{
    NativeHLSL;
    HideInEditor;
};

//暂定最高64个灯光
#define MaxLightingNum  64
#define CastShadow = 0x00000001

#include "Include/Common.hlsl"
#include "Deferred/DeferredCommon.hlsl"

struct LightingParameter
{
    float3  LightPosition;
	float   LightStrength;
	float3  LightColor;
	float   LightSpecular;
	float3  LightDirection;
	uint    LightType;
	uint    LightFlags;
};

cbuffer Pass :register(b0,space0)
{
    PassUniformBuffer Pass;
    uint ValidLightCount;//有效灯光数量
    LightingParameter LightingParams[MaxLightingNum];
};

struct VSInput
{
    float2 Pos : POSITION;
    float2 UV: TEXCOORD0;
};

struct VSToPS
{
    float4 SVPosition       : SV_POSITION;
    float2 UV               : TEXCOORD0;
};

#include "Include/PostProcessCommon.hlsl"

VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    OUT.SVPosition = float4(IN.Pos,0,1);
    OUT.UV = IN.UV;
    return OUT;
}

float4 PSMain(VSToPS IN) :SV_Target0
{   
    half4 Result = half4(0,0,0,1);
    float2 ScreenUV = IN.UV;
    //Init
    GBufferData GBuffer;
    InitGBufferData(GBuffer);
    DecodeGBufferData(IN.UV, GBuffer);
    if (GBuffer.ShadingModelID == SHADINGMODELID_UNLIT)
    {
        return 0;
    }
    //Diffuse Color
    half3 DiffuseColor = GBuffer.BaseColor * (1 - GBuffer.Metallic);
    //Specular Color F0
    half3 SpecularColor = lerp((0.08f * GBuffer.Specular).xxx , GBuffer.BaseColor.rgb, GBuffer.Metallic.xxx);
    //Calculate World Position
    float3 WorldPosition = ScreenToWorld(ScreenUV, GBuffer.SceneDepth);
    //View Direction
    half3 ViewDir = normalize(CameraPos.xyz - WorldPosition.xyz);
    //BRDF Content
    half NoV = dot(GBuffer.WorldNormal.xyz, ViewDir.xyz);
    //Directional Light
    [loop]
    for(int i = 0; i < ValidLightCount; i++)
    {
        LightingParameter LightParam = LightingParams[i];
        half3 LightDir = LightParam.LightDirection.xyz;
        half NoL = saturate(dot(LightDir, GBuffer.WorldNormal.xyz));
        float3 H = normalize(float3(ViewDir + LightDir));
        half VoL = dot(ViewDir, LightDir);
        float InvLenH = rsqrt( 2 + 2 * VoL);
        half NoH = saturate( (NoL + NoV) * InvLenH);
        half VoH = saturate( InvLenH + InvLenH * VoL);
        //Light Diffuse
        half3 DiffuseLightColor = DiffuseColor * NoL;
        //Specular
        half3 spec = NoL * SpecularGGX(GBuffer.Roughness, SpecularColor * LightParam.LightSpecular, NoL, NoV, VoH, NoH);
        //Light Result
        Result.rgb += (DiffuseLightColor + spec) * LightParam.LightStrength * LightParam.LightColor;
    } 
    return Result;
    //return GBuffer.ShadingModelID;
}
