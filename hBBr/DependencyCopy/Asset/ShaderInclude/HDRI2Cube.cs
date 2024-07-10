[Flag]
{
    NativeHLSL; //原生HLSL,不进行拓展编译,需遵循HLSL代码规范编写
    HideInEditor;//不希望显示在编辑器(例如材质编辑器)
};

//RW =read & write
RWTexture2D<float4> DstTexture : register(u0,space0);
Texture2D<float4>   HDRITexture: register(t1,space0);

// numthreads：创建线程组的大小，也就是一个线程组包含多少个线程，下面的指令表示：指定每个线程组包含64个线程
// id：该线程所在的总的线程结构中的索引
[numthreads( 8, 8, 1 )]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float4 hdriColor = HDRITexture.Load(uint3(id.xy, 0));
	DstTexture[id.xy] = hdriColor;
}