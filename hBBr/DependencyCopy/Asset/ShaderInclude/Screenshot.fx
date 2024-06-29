[Flag]
{
    NativeHLSL; //原生HLSL,不进行拓展编译,需遵循HLSL代码规范编写
    EnableShaderDebug;
};

cbuffer Pass :register(b0,space0)
{
    float4 Parameter0;
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

Texture2D BaseTexture: register(t0,space1);
SamplerState BaseTextureSampler: register(s0,space1);

VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    OUT.SVPosition = float4(IN.Pos,0,1);
    OUT.UV = IN.UV;
    return OUT;
}

float4 PSMain(VSToPS IN) :SV_Target0
{   
    half4 result = 1;
    half4 baseTexture = BaseTexture.SampleBias(BaseTextureSampler, IN.UV, -1);
    
    result = baseTexture;
    return result;
}
