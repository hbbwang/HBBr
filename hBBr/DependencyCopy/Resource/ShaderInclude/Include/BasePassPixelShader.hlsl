//#include "Include/Common.hlsl"

struct PSOutput
{
    float4 Color : SV_Target0;
};

void InitPSOut(inout PSOutput psInout)
{
    psInout.Color = 0;
}

//Pixel shader
PSOutput PSMain(VSToPS IN)
{   
    PSOutput OUT;
    InitPSOut(OUT);
    PixelShaderParameter Parameters;
    frag(IN,Parameters);

    OUT.Color = float4(IN.WorldNormal * 0.5 + 0.5 ,1.0f) * (sin(GameTime) * 0.5 + 0.5);

    return OUT;
}

