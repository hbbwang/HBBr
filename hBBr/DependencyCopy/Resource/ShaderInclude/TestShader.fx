//[Flags]EnableShaderDebug

struct VSInput
{
    float3 Position     : POSITION;
    float4 Color        : COLOR;
};

struct VSToPS
{
    float4 SVPosition   : SV_POSITION;
    float4 Color        : COLOR;
};

//Vertex shader
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    OUT.SVPosition = float4(IN.Position.xyz, 1.0f);
    OUT.Color = IN.Color;
    return OUT;
}

//Pixel shader
float4 PSMain(VSToPS IN) : SV_Target0
{   
    return IN.Color;
}

