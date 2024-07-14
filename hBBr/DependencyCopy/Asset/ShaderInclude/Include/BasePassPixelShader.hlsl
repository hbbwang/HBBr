#ifndef _BASE_PASS_PIXEL_SHADER_HLSL
#define _BASE_PASS_PIXEL_SHADER_HLSL

#include "Include/ShadingModel.hlsl"

struct PSOutput
{
    //Emissive
    float3 SceneColor : SV_Target0;
    //GBuffer0 BaseColor & Roughness
    float4 GBuffer0 : SV_Target1;
    //GBuffer1 World Normal & ? 
    float4 GBuffer1 : SV_Target2;
    //GBuffer2 Metallic & Specular & AO & ShadingModel ID
    float4 GBuffer2 : SV_Target3;
    //float4 GBuffer4 : SV_Target4;
};

void InitPSOut(inout PSOutput psInout)
{
    psInout.SceneColor = 0;
    psInout.GBuffer0 = 0;
    psInout.GBuffer1 = 0;
    psInout.GBuffer2 = 0;
}

void SetShadingModelID(inout PixelShaderParameter Parameters)
{
    #if MATERIAL_SHADINGMODEL_DEFAULT_LIT
        Parameters.ShadingModelID = SHADINGMODELID_DEFAULT_LIT;
    #elif MATERIAL_SHADINGMODEL_UNLIT
        Parameters.ShadingModelID = SHADINGMODELID_UNLIT;
    #endif
}

//Pixel shader
PSOutput PSMain(VSToPS IN)
{   
    PSOutput OUT;
    InitPSOut(OUT);
    PixelShaderParameter Parameters;
    InitPSParameter(IN,Parameters);
    SetShadingModelID(Parameters);
    frag(IN,Parameters);

    Parameters.Roughness = saturate(Parameters.Roughness);
    Parameters.Specular =  saturate(Parameters.Specular);
    Parameters.Metallic = saturate(Parameters.Metallic); 
    Parameters.AO = saturate(Parameters.AO);   
    Parameters.InShadow = saturate(Parameters.InShadow);   
    Parameters.WorldNormal = normalize(Parameters.WorldNormal);
    Parameters.BaseColor = max(0.0,Parameters.BaseColor);
    Parameters.Emissive = max(0.0,Parameters.Emissive);

    OUT.GBuffer0 = float4( Parameters.BaseColor , Parameters.Roughness );
    OUT.GBuffer1 = float4( Parameters.WorldNormal * 0.5f + 0.5f , 0.0f );
    OUT.GBuffer2 = float4( Parameters.Metallic , Parameters.Specular , Parameters.AO , EncodeShadingModelID(Parameters.ShadingModelID));
    OUT.SceneColor = float3( Parameters.Emissive);

    return OUT;
}

#endif