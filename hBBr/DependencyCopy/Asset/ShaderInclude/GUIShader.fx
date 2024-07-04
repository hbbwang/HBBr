[Flag]
{
    NativeHLSL; //原生HLSL,不进行拓展编译,需遵循HLSL代码规范编写
    EnableShaderDebug;
    HideInEditor;//不希望显示在编辑器(例如材质编辑器)
};

//GUI Flags
#define IsFont      0x00000001
#define FontShadow  0x00000002

cbuffer GUIPass :register(b0,space0)
{
    float4 UVSetting;
    float4 Color;
	float TextureSizeX;
	float TextureSizeY;
	float ScreenSizeX;
	float ScreenSizeY;
    int Flags;
};

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

Texture2D BaseTexture: register(t0,space1);
SamplerState BaseTextureSampler: register(s0,space1);

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
    half4 result = 1;
    //return IN.Color;
    half4 baseTexture = BaseTexture.SampleBias(BaseTextureSampler,
    IN.UV * (UVSetting.zw / float2(TextureSizeX,TextureSizeY)) + (UVSetting.xy / float2(TextureSizeX,TextureSizeY)) , -1);

    if(Flags & IsFont)
    {
        result = baseTexture.r;
        result = saturate(smoothstep(0.1f , 1.0f , result));
        if(! (Flags & FontShadow) )
        {
            result.rgb = 1.0f;
        }
    }
    else
    {    
        result = baseTexture;
    }
    result *= IN.Color * Color;

    return result;
}
