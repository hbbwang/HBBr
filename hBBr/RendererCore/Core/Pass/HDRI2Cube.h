#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HBox.h"
#include "TextureCube.h"

struct HDRI2CubeUnifrom
{
	float CubeMapSize;
};

//这个Pass是个立即生效的pass，且是一次性的，申请内存后就在构造函数里直接执行，如果希望再次重新执行，运行下PassReExecute就行。
//1.先通过Compute Shader，输入一张hdr纹理，生成6张纹理，乃是CubeMap的6面纹理。
//2.根据6面纹理，生成CubeMap dds格式的纹理，Format为BC6U
class HDRI2Cube :public ComputePass
{
public:
	HDRI2Cube();

	virtual ~HDRI2Cube();

	bool PassExecute(std::string hdrImagePath, std::string ddsOutputPath, bool bGenerateMips, int cubeMapSize = -1);

	void ReleasePass();

private:
	std::string _hdrImagePath;

	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout _storeDescriptorSetLayout = VK_NULL_HANDLE;

	std::shared_ptr<class DescriptorSet> store_descriptorSet;

	std::shared_ptr<Texture2D> _storeTexture[6];

	std::shared_ptr<Texture2D> _hdriTexture;

	std::unique_ptr<VMABuffer> _uniformBuffer;
};
