//[Flags]EnableShaderDebug;
cbuffer GUIPass :register(b0)
{
    float4 UVSetting;
    float4 channel;
    float TextureSize;
};

//[InputLayout]
struct VSInput
{
    float2 Pos : POSITION;
    float2 UV: TEXCOORD0;
    float4 Color: COLOR;
};

struct VSToPS
{
    float4 SVPosition       : SV_POSITION;
    float4 Color            : COLOR;
    float2 UV               : TEXCOORD0;
};

SamplerState BaseTextureSampler : register(s0,space1);
Texture2D BaseTexture : register(t0,space1);

VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    OUT.SVPosition = float4(IN.Pos,0,1);
    OUT.Color = IN.Color;
    OUT.UV = IN.UV;
    return OUT;
}

float4 PSMain(VSToPS IN) :SV_Target0
{   
    //return IN.Color;
    half4 baseTexture = BaseTexture.Sample(BaseTextureSampler,IN.UV * (UVSetting.zw / TextureSize) + (UVSetting.xy / TextureSize) );
    baseTexture *= channel;
    return baseTexture * IN.Color;
}
