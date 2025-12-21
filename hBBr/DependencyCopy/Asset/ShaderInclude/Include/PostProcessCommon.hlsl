#ifndef _POST_PROCESS_COMMON_HLSL
#define _POST_PROCESS_COMMON_HLSL

float3 ScreenToWorld(float2 ScreenUV, float SceneDepth) 
{
	float2 clipSpaceCoords = ScreenUV * 2.0 - 1.0;
	float4 clipSpacePos = float4(clipSpaceCoords, SceneDepth, 1.0f);
	float4 worldPos = mul(Pass.ViewProj_Inv , clipSpacePos);
	worldPos = worldPos / worldPos.w;
	return worldPos.xyz;
}

#endif