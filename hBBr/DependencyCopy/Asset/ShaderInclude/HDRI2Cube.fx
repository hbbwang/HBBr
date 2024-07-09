[Flag]
{
    NativeHLSL; //原生HLSL,不进行拓展编译,需遵循HLSL代码规范编写
    HideInEditor;//不希望显示在编辑器(例如材质编辑器)
};

cbuffer Pass :register(b0,space0)
{
    float4x4 M;
    float4x4 VP;
    float3 CamPos;
    float XDegree;
    float ZDegree;
};

struct VSInput
{
    float3 Position : POSITION;
};

struct VSToPS
{
    float4 SVPosition       : SV_POSITION;
    float3 CameraView       : TEXCOORD0;
};

Texture2D HDRITexture: register(t0,space1);
SamplerState HDRITextureSampler: register(s0,space1);

VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    float4 WolrdPos = mul(M, float4(IN.Position,1.0));
    OUT.SVPosition = mul(VP, WolrdPos);
    OUT.CameraView = normalize(WolrdPos - CamPos);
    return OUT;
}

float4 PSMain(VSToPS IN) :SV_Target0
{   
    float3 dir = normalize(IN.CameraView.xyz);
    float2 pos = 1.0 / 3.1415927 * float2(atan2(dir.x, dir.y), 2.0 * asin(-dir.z + ZDegree));
    pos = 0.5 * pos + float2(0.5, 0.5);
    pos.x += XDegree / 180;
    return HDRITexture.SampleBias(HDRITextureSampler, pos, -1);;
}
