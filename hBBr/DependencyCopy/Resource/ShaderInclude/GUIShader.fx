//[Flags]EnableShaderDebug;
cbuffer GUIPass :register(b0)
{
    float4 UVSetting;
    float channel;
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
    if(channel == 0)
    {
        return baseTexture.r * IN.Color;
    }
    else if(channel == 1)
    {
        return baseTexture.g * IN.Color;
    }
    else if(channel == 2)
    {
        return baseTexture.b * IN.Color;
    }
    else if(channel == 3)
    {
        return baseTexture.a * IN.Color;
    }
    else
    {
        return baseTexture * IN.Color;
    }
}
