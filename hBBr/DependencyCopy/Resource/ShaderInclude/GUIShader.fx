//[Flags]EnableShaderDebug;
cbuffer GUIPass :register(b0)
{
    float4x4 ProjectionMatrix;
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
    OUT.SVPosition = mul(ProjectionMatrix, float4(IN.Pos.xy, 0.f, 1.f));
    OUT.SVPosition.y = -OUT.SVPosition.y;
    OUT.Color = IN.Color;
    OUT.UV = IN.UV;
    return OUT;
}

float4 PSMain(VSToPS IN) :SV_Target0
{   
    //return IN.Color;
    half4 baseTexture = BaseTexture.Sample(BaseTextureSampler,IN.UV);
    return baseTexture * IN.Color;
}
