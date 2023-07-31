//[Flags]EnableShaderDebug;
//[Varient]USE_COLOR;

#define MVP mul(World,mul(Projection,View))
#define VP  mul(Projection,View)

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
    float4x4 World;
};

struct VSInput
{
    float3 Position     : POSITION;
    float4 Color        : COLOR;
};

struct VSToPS
{
    float4 SVPosition   : SV_POSITION;
    float4 Color        : COLOR;
    float3 WorldPosition:TEXCOORD0;
};

//Vertex shader
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    //Transform MVP
    float4 WorldPosition = mul(World, float4(IN.Position,1.0));
    OUT.SVPosition = mul(VP, mul(World, float4(IN.Position,1.0)));
    OUT.WorldPosition = WorldPosition.xyz;
    //OUT.SVPosition = mul(MVP, float4(IN.Position,1.0));

    OUT.Color = IN.Color;
    return OUT;
}

//Pixel shader
float4 PSMain(VSToPS IN) : SV_Target0
{   
    return IN.Color;
}

