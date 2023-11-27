//--ShaderFlagsBegin
//  MaterialEditorHidden
//--ShaderFlagsEnd

//--ShaderVariantsBegin
//--ShaderVariantsEnd

//--ShaderParametersBegin
//--ShaderParametersEnd

//--ShaderTexturesBegin
//--ShaderTexturesEnd

//RW =read & write
RWTexture2D<float4> DstTexture : register(u0,space0);

cbuffer CB : register(b0,space1)
{
	float2 TestParam;	// 1.0 / destination dimension
}

// numthreads：创建线程组的大小，也就是一个线程组包含多少个线程，下面的指令表示：指定每个线程组包含64个线程
// id：该线程所在的总的线程结构中的索引
[numthreads( 8, 8, 1 )]
void main(uint3 id : SV_DispatchThreadID)
{
	DstTexture[id.xy] = float4(TestParam,0,1);
}