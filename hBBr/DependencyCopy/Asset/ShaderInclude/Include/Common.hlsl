#ifndef _COMMON_HLSL
#define _COMMON_HLSL

#define GameTime (Pass.CameraPos_GameTime.w)
#define CameraPos (Pass.CameraPos_GameTime.xyz)
#define CameraDir (Pass.CameraDirection.xyz)
#define ScreenSize (Pass.ScreenInfo.xy)
#define NearPlane (Pass.ScreenInfo.z)
#define FarPlane (Pass.ScreenInfo.w)

struct PassUniformBuffer
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

//Functions
#define POW_CLAMP (0.000001f)

#define PI (3.1415926535897932384626433832795)

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

#if 1
	float3 sRGBToLinear(float3 color) 
	{
		color = max(6.10352e-5, color);
		return pow(color, float3(2.4,2.4,2.4));
	}
	float3 linearToSRGB(float3 color) 
	{
		color = max(6.10352e-5, color);
		float f = 1.0 / 2.4;
		return pow(color, float3(f,f,f));
	}
#else
	float3 sRGBToLinear(float3 color) 
	{
		color = max(6.10352e-5, color);
		float3 colorLinear = color / 12.92;
		float3 colorExponential = pow((color + 0.055) / 1.055, float3(2.4,2.4,2.4));
		return mix(colorLinear, colorExponential, step(0.04045, color));
	}
	float3 linearToSRGB(float3 color) 
	{
		color = max(6.10352e-5, color);
		float f = 1.0 / 2.4;
		float3 colorCompressed = (color <= 0.0031308) ? color * 12.92 : pow(color, float3(f,f,f)) * 1.055 - 0.055;
		return colorCompressed * (1.0 / 12.92);
	}
#endif

float4x4 Inverse(float4x4 m)
{
	float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
	float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
	float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
	float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

	float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
	float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
	float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
	float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

	float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
	float idet = 1.0f / det;

	float4x4 ret;

	ret[0][0] = t11 * idet;
	ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
	ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
	ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

	ret[1][0] = t12 * idet;
	ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
	ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
	ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

	ret[2][0] = t13 * idet;
	ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
	ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
	ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

	ret[3][0] = t14 * idet;
	ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
	ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
	ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

	return ret;
}

#endif