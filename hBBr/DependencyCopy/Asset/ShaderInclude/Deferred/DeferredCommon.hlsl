#ifndef _DEFERRED_COMMON
#define _DEFERRED_COMMON

#include "Include/BRDF.hlsl"
#include "Include/ShadingModel.hlsl"

Texture2D SceneDepth: register(t0,space1);
SamplerState SceneDepthSampler: register(s0,space1);

//Emissive
Texture2D SceneColor: register(t1,space1);
SamplerState SceneColorSampler: register(s1,space1);

//GBuffer0 BaseColor & Roughness
Texture2D GBuffer0: register(t2,space1);
SamplerState GBuffer0Sampler: register(s2,space1);

//GBuffer1 World Normal & ? 
Texture2D GBuffer1: register(t3,space1);
SamplerState GBuffer1Sampler: register(s3,space1);

//GBuffer2 Metallic & Specular & AO & ShadingModel ID
Texture2D GBuffer2: register(t4,space1);
SamplerState GBuffer2Sampler: register(s4,space1);

struct GBufferData
{
    half3  BaseColor;
    half3  WorldNormal;
    float3 Emissive;
    half   Roughness;
    half   Metallic;
    half   AO;
    half   Specular;
    uint   ShadingModelID;
    float  SceneDepth;
};

void InitGBufferData(inout GBufferData GBuffer)
{
    GBuffer.BaseColor = 0;
    GBuffer.WorldNormal = 0;
    GBuffer.Emissive = 0;
    GBuffer.Roughness = 0;
    GBuffer.Metallic = 0;
    GBuffer.AO = 0;
    GBuffer.Specular = 0;   
    GBuffer.ShadingModelID = 0;   
}

void DecodeGBufferData(half2 UV, inout GBufferData GBuffer)
{
    float SceneDepthInput = SceneDepth.Sample(SceneDepthSampler, UV).r;
    float4 SceneColorInput = SceneColor.Sample(SceneColorSampler, UV);
    float4 GBuffer0Input = GBuffer0.Sample(GBuffer0Sampler, UV);
    float4 GBuffer1Input = GBuffer1.Sample(GBuffer1Sampler, UV);
    float4 GBuffer2Input = GBuffer2.Sample(GBuffer2Sampler, UV);

    GBuffer.BaseColor = GBuffer0Input.rgb;
    GBuffer.WorldNormal = normalize(GBuffer1Input.rgb * 2.0f - 1.0f);
    GBuffer.Emissive = SceneColorInput.rgb;
    GBuffer.Roughness = GBuffer0Input.a;
    GBuffer.Metallic = GBuffer2Input.r;
    GBuffer.Specular = GBuffer2Input.g;   
    GBuffer.AO = GBuffer2Input.b;
    GBuffer.SceneDepth = SceneDepthInput;
    //Decode shader model id
    GBuffer.ShadingModelID = DecodeShadingModelID(GBuffer2Input.a);
}


#endif