#ifndef _BASE_PASS_PIXEL_SHADER_HLSL
#define _BASE_PASS_PIXEL_SHADER_HLSL

#include "Include/Common.hlsl"

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

    OUT.Color = float4((IN.WorldNormal * 0.5 + 0.5) * Parameters.BaseColor ,1.0f);

    return OUT;
}

#endif