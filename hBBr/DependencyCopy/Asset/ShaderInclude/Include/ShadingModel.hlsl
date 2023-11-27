#ifndef _SHADING_MODELS_HLSL
#define _SHADING_MODELS_HLSL

#ifndef MATERIAL_SHADINGMODEL_UNLIT
#define MATERIAL_SHADINGMODEL_UNLIT 0
#endif

#ifndef MATERIAL_SHADINGMODEL_DEFAULT_LIT
#define MATERIAL_SHADINGMODEL_DEFAULT_LIT 0
#endif

//Shading Model ID define
#define SHADINGMODELID_UNLIT				0
#define SHADINGMODELID_DEFAULT_LIT			1
#define SHADINGMODELID_MASK					0xFF		// 8 bits reserved for ShadingModelID

float EncodeShadingModelID( uint ShadingModelId )
{
    uint Value = (ShadingModelId & SHADINGMODELID_MASK);
	return (float)Value / (float)0xFF;
}

uint DecodeShadingModelID( float InPackedChannel )
{
    return ((uint)round(InPackedChannel * (float)0xFF)) & SHADINGMODELID_MASK;
}


#endif