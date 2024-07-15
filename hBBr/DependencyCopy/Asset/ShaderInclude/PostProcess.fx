[Flag]
{
    NativeHLSL;
    HideInEditor;
};

#include "Include/Common.hlsl"
#include "Deferred/DeferredCommon.hlsl"

//Emissive
Texture2D SceneColor: register(t4,space1);
SamplerState SceneColorSampler: register(s4,space1);

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
    uint DebugMode;
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
    float4 SceneColorInput = SceneColor.Sample(SceneColorSampler, IN.UV);
    float2 ScreenUV = IN.UV;
    //Init
    GBufferData GBuffer;
    InitGBufferData(GBuffer);
    DecodeGBufferData(IN.UV, GBuffer);
    return SceneColorInput;
}
