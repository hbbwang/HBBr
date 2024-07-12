#ifndef _COMMON_HLSL
#define _COMMON_HLSL

#define MVP mul(World,mul(Projection,View))
#define VP  mul(Projection,View)

#define GameTime (CameraPos_GameTime.w)
#define CameraPos (CameraPos_GameTime.xyz)
#define CameraDir (CameraDirection.xyz)
#define ScreenSize (ScreenInfo.xy)

cbuffer Pass :register(b0)
{
    float4x4 View;
    float4x4 View_Inv;
    float4x4 Projection;
    float4x4 Projection_Inv;
    float4x4 ViewProj;
    float4x4 ViewProj_Inv;
    float4 ScreenInfo; // screen xy,z near,w zfar
    float4 CameraPos_GameTime;
    float4 CameraDirection;
};

cbuffer Pass :register(b0 ,space1)
{
    float4x4 World;
};

struct VSToPS
{
    float4 SVPosition       : SV_POSITION;

    #if USE_VERTEX_INPUT_COLOR
    float4 Color            : COLOR;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD0
    float4 Texcoord01       : TEXCOORD0;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD1
    float4 Texcoord23       : TEXCOORD1;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD2
    float4 Texcoord45       : TEXCOORD2;
    #endif

    #if USE_VERTEX_INPUT_NORMAL
    float3 WorldNormal      : NORMAL;
    #endif

    #if USE_VERTEX_INPUT_TANGENT
    float3 WorldTangent     : TANGENT;
    #endif

    #if USE_VERTEX_INPUT_NORMAL || USE_VERTEX_INPUT_TANGENT
    float3 WorldBitangent   : BINORMAL;
    #endif
    
    float3 LocalPosition    : TEXCOORD2;
    float3 WorldPosition    : TEXCOORD3;
    float3 CameraVector     : TEXCOORD4;
};

struct PixelShaderParameter
{
    float3  BaseColor;
    float3  WorldNormal;
    float3  WorldTangent;
    float3  WorldBitangent;
    float3  Emissive;
    float   Roughness;
    float   Metallic;
    float   AO;
    float   Specular;
    float   InShadow;
    float4  VertexColor;
    uint    ShadingModelID;
    float2  UV[6];
    //
    float3  LocalPosition;
    float3  WorldPosition;
    float3  CameraVector;
    //World TBN
    float3x3 TangentToWorld;
};

void InitPSParameter(in VSToPS IN , inout PixelShaderParameter Params)
{
    Params.BaseColor = float3(0,0,0);

    #if USE_VERTEX_INPUT_NORMAL
    Params.WorldNormal = normalize(IN.WorldNormal);
    #endif

    #if USE_VERTEX_INPUT_TANGENT
    Params.WorldTangent = normalize(IN.WorldTangent);
    #endif

    #if USE_VERTEX_INPUT_NORMAL || USE_VERTEX_INPUT_TANGENT
    Params.WorldBitangent = normalize(IN.WorldBitangent);
    Params.TangentToWorld = float3x3(
        float3(Params.WorldTangent.x,Params.WorldBitangent.x , Params.WorldNormal.x),
        float3(Params.WorldTangent.y,Params.WorldBitangent.y , Params.WorldNormal.y),
        float3(Params.WorldTangent.z,Params.WorldBitangent.z , Params.WorldNormal.z)  
    );
    #endif

    #if USE_VERTEX_INPUT_COLOR
    Params.VertexColor = IN.Color;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD0
    Params.UV[0] = IN.Texcoord01.xy;
    Params.UV[1] = IN.Texcoord01.zw;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD1
    Params.UV[2] = IN.Texcoord23.xy;
    Params.UV[3] = IN.Texcoord23.zw;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD2
    Params.UV[4] = IN.Texcoord45.xy;
    Params.UV[5] = IN.Texcoord45.zw;
    #endif

    Params.Emissive = float3(0,0,0);
    Params.Roughness = 1.0f;
    Params.Metallic = 0.0f;
    Params.AO = 1.0f;
    Params.Specular = 1.0;
    Params.InShadow = 1.0;
    Params.ShadingModelID = 0;
    Params.LocalPosition = IN.LocalPosition;
    Params.WorldPosition = IN.WorldPosition;
    Params.CameraVector = normalize(IN.CameraVector);
}

void InitVSToPS(inout VSToPS vs2ps)
{
    vs2ps.SVPosition = float4(0,0,0,1);

    #if USE_VERTEX_INPUT_COLOR
    vs2ps.Color = float4(0,0,0,0);
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD0
    vs2ps.Texcoord01 = float4(0,0,0,0);
    #endif
    
    #if USE_VERTEX_INPUT_TEXCOORD1
    vs2ps.Texcoord23 = float4(0,0,0,0);
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD2
    vs2ps.Texcoord45 = float4(0,0,0,0);
    #endif

    #if USE_VERTEX_INPUT_NORMAL
    vs2ps.WorldNormal = float3(0,0,0);
    #endif

    #if USE_VERTEX_INPUT_TANGENT
    vs2ps.WorldTangent = float3(0,0,0);
    #endif

    #if USE_VERTEX_INPUT_NORMAL || USE_VERTEX_INPUT_TANGENT
    vs2ps.WorldBitangent= float3(0,0,0);
    #endif

    vs2ps.WorldPosition = float3(0,0,0);
    vs2ps.CameraVector = float3(0,0,0);
}

float3 DirectionLocalToWorld(in float3 LocalDir)
{
    float3x3 worldDirMatrix = (float3x3)World;
    return normalize(mul(worldDirMatrix , LocalDir).xyz);
}

//Functions
#define PI  (3.1415926535897932f)

float Square( float x )
{
	return x*x;
}

float2 Square( float2 x )
{
	return x*x;
}

float3 Square( float3 x )
{
	return x*x;
}

float4 Square( float4 x )
{
	return x*x;
}

float Pow2( float x )
{
	return x*x;
}

float2 Pow2( float2 x )
{
	return x*x;
}

float3 Pow2( float3 x )
{
	return x*x;
}

float4 Pow2( float4 x )
{
	return x*x;
}

float Pow3( float x )
{
	return x*x*x;
}

float2 Pow3( float2 x )
{
	return x*x*x;
}

float3 Pow3( float3 x )
{
	return x*x*x;
}

float4 Pow3( float4 x )
{
	return x*x*x;
}

float Pow4( float x )
{
	float xx = x*x;
	return xx * xx;
}

float2 Pow4( float2 x )
{
	float2 xx = x*x;
	return xx * xx;
}

float3 Pow4( float3 x )
{
	float3 xx = x*x;
	return xx * xx;
}

float4 Pow4( float4 x )
{
	float4 xx = x*x;
	return xx * xx;
}

float Pow5( float x )
{
	float xx = x*x;
	return xx * xx * x;
}

float2 Pow5( float2 x )
{
	float2 xx = x*x;
	return xx * xx * x;
}

float3 Pow5( float3 x )
{
	float3 xx = x*x;
	return xx * xx * x;
}

float4 Pow5( float4 x )
{
	float4 xx = x*x;
	return xx * xx * x;
}

float Pow6( float x )
{
	float xx = x*x;
	return xx * xx * xx;
}

float2 Pow6( float2 x )
{
	float2 xx = x*x;
	return xx * xx * xx;
}

float3 Pow6( float3 x )
{
	float3 xx = x*x;
	return xx * xx * xx;
}

float4 Pow6( float4 x )
{
	float4 xx = x*x;
	return xx * xx * xx;
}

float3 ComputeWorldPos(float2 screenUV, float depth) {
	float4 pos = float4(float3((screenUV * 2.0 - 1.0 ), 1.0 ) * depth, 1.0f);
	float4 ret = mul(ViewProj_Inv , pos);
	return ret.xyz / ret.w;
}

//Compute Cube UV ( Texture2D to Cube ) 
float2 GetCubeMapUV(in PixelShaderParameter Parameters)
{
    float XDegree = 0.0f;
    float ZDegree = 0.0f;
    float3 dir = reflect(Parameters.CameraVector.xyz ,Parameters.WorldNormal.xyz ) ;
    float2 pos = 1.0f / PI * float2(atan2(dir.x, dir.z), 2.0 * asin(-dir.y + ZDegree));
    pos = 0.5 * pos + float2(0.5, 0.5);
    pos.x += XDegree / 180;
    return pos;
}

float4 GetTexcoord(float4 Texcoord)
{
    #if IS_VULKAN
        #if USE_BC_FORMAT //Vulkan BC format need to flip Y axis.
            Texcoord.y = 1-Texcoord.y;
            Texcoord.w = 1-Texcoord.w;
        #endif
    #endif
    return Texcoord;
}

float2 GetTexcoord(float2 Texcoord)
{
    #if USE_BC_FORMAT //Vulkan BC format need to flip Y axis.
        Texcoord.y = 1-Texcoord.y;
    #endif
    return Texcoord;
}

float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

#endif