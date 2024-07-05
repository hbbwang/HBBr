
#ifndef _BASE_PASS_VERTEX_SHADER_HLSL
#define _BASE_PASS_VERTEX_SHADER_HLSL

#ifndef USE_VERTEX_INPUT_NORMAL
#define USE_VERTEX_INPUT_NORMAL 0
#endif

#ifndef USE_VERTEX_INPUT_TANGENT
#define USE_VERTEX_INPUT_TANGENT 0
#endif

#ifndef USE_VERTEX_INPUT_COLOR
#define USE_VERTEX_INPUT_COLOR 0
#endif

#ifndef USE_VERTEX_INPUT_TEXCOORD0
#define USE_VERTEX_INPUT_TEXCOORD0 0
#endif

#ifndef USE_VERTEX_INPUT_TEXCOORD1
#define USE_VERTEX_INPUT_TEXCOORD1 0
#endif

#ifndef USE_VERTEX_INPUT_TEXCOORD2
#define USE_VERTEX_INPUT_TEXCOORD2 0
#endif

#ifndef CustomVSMain
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    InitVSToPS(OUT);

    //Position(float3) must have
    //Transform MVP
    OUT.LocalPosition = IN.Position;
    float4 WorldPosition = mul(World, float4(IN.Position,1.0));
    OUT.SVPosition = mul(VP, WorldPosition);
    OUT.WorldPosition = WorldPosition.xyz;

    //Transform TangentSpace
    #if USE_VERTEX_INPUT_NORMAL
        OUT.WorldNormal =  normalize(DirectionLocalToWorld(IN.Normal));
    #endif

    #if USE_VERTEX_INPUT_TANGENT
        OUT.WorldTangent =  normalize(DirectionLocalToWorld(IN.Tangent));
    #endif

    #if USE_VERTEX_INPUT_NORMAL || USE_VERTEX_INPUT_TANGENT
        OUT.WorldBitangent = normalize(cross(IN.Normal,IN.Tangent));
    #endif

    //Texcoord 0123
    #if USE_VERTEX_INPUT_TEXCOORD0
        OUT.Texcoord01 = IN.Texcoord01;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD1
        OUT.Texcoord23 = IN.Texcoord23;
    #endif

    #if USE_VERTEX_INPUT_TEXCOORD2
        OUT.Texcoord45 = IN.Texcoord45;
    #endif

    //Vertex Color
    #if USE_VERTEX_INPUT_COLOR
        OUT.Color = IN.Color;
    #endif
    
    //Other
    OUT.CameraVector = (OUT.WorldPosition.xyz - CameraPos.xyz);

    #ifdef DefineVert
        vert(IN,OUT);
    #endif

    return OUT;
}
#endif

#endif