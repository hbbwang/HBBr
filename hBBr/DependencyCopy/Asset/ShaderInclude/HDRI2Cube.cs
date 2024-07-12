[Flag]
{
    NativeHLSL; // 原生HLSL,不进行拓展编译,需遵循HLSL代码规范编写
    HideInEditor;// 不希望显示在编辑器(例如材质编辑器)
};

// 输出Cube map 的6个面
RWTexture2D<float4> DstTexture0 : register(u0,space0);
RWTexture2D<float4> DstTexture1 : register(u1,space0);
RWTexture2D<float4> DstTexture2 : register(u2,space0);
RWTexture2D<float4> DstTexture3 : register(u3,space0);
RWTexture2D<float4> DstTexture4 : register(u4,space0);
RWTexture2D<float4> DstTexture5 : register(u5,space0);
// HDRI Texture2D
Texture2D HDRITexture: register(t6,space0);
SamplerState HDRITextureSampler: register(s6,space0);

#define PI 3.14159265358979323846

// 传递一下CubeMap的期望输出大小
cbuffer Pass :register(b7,space0)
{
    float CubeMapSize;
};

// 根据CubeMap的面索引和UV坐标计算3D方向向量
float3 ComputeDirectionVector(uint face, float2 uv)
{
    float3 direction;

    // 将UV坐标映射到[-1, 1]范围
    float2 coord = uv * 2.0 - 1.0;

    // 根据面的索引计算方向向量
    if (face == 0) // 右
        direction = float3(1.0, -coord.y, coord.x);
    else if (face == 1) // 左
        direction = float3(-1.0, -coord.y, -coord.x);
    else if (face == 2) // 上
        direction = float3(-coord.x, 1.0, coord.y);
    else if (face == 3) // 下
        direction = float3(-coord.x, -1.0, -coord.y);
    else if (face == 4) // 前
        direction = float3(-coord.x, -coord.y, 1.0);
    else if (face == 5) // 后
        direction = float3(coord.x, -coord.y, -1.0);

    return -normalize(direction);
}

// 将3D方向向量转换为经纬度坐标
float2 DirectionToLatLong(float3 direction)
{
    float longitude = atan2(direction.z, direction.x);
    float latitude = asin(direction.y);

    return float2(longitude, latitude);
}

// 将经纬度坐标映射到HDRI图像的UV坐标
float2 LatLongToEquirectangularUV(float2 latLong)
{
    float u = (latLong.x + PI) / (2.0 * PI);
    float v = (latLong.y + PI / 2.0) / PI;

    return float2(u, v);
}

[numthreads( 8, 8, 1 )]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    // 计算CubeMap面和mipmap级别
    uint face = 0;
    // 计算UV坐标
    float2 uv = (float2(id.xy) + 0.5) / CubeMapSize;
    {
        face = 1;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture0[id.xy] = hdriColor;
    }
    {
        face = 0;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture1[id.xy] = hdriColor;
    }
    {
        face = 2;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture2[id.xy] = hdriColor;
    }
    {
        face = 3;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture3[id.xy] = hdriColor;
    }
    {
        face = 4;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture4[id.xy] = hdriColor;
    }
    {
        face = 5;
        float3 direction = ComputeDirectionVector(face, uv);
        float2 latLong = DirectionToLatLong(direction);
        float2 equirectangularUV = LatLongToEquirectangularUV(latLong);
        float4 hdriColor = HDRITexture.SampleLevel(HDRITextureSampler, equirectangularUV, 0);
	    DstTexture5[id.xy] = hdriColor;
    }
    
} 