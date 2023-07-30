//[Flags]EnableShaderDebug

struct VSInput
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float4 color        : COLOR;
    float4 texcoord01   : TEXCOORD0;
    float4 texcoord23   : TEXCOORD1;
};

struct VSToPS
{
    float4 SVPosition   : SV_POSITION;
};

//Vertex shader
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    OUT.SVPosition = float4(IN.position.xyz, 1.0f);
    return OUT;
}

//Pixel shader
float4 PSMain(VSToPS IN) : SV_Target0
{   
    return float4(1.0f,0.0f,0.0f,1.0f);
}

