#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HRect.h"
#include "TextureCube.h"

struct HDRI2CubeUnifrom
{
	float CubeMapSize;
};

//这个Pass是个立即生效的pass，且是一次性的。
//1.先通过Compute Shader，输入一张hdr纹理，生成6张纹理，乃是CubeMap的6面纹理。
//2.根据6面纹理，生成CubeMap dds格式的纹理，Format为BC6U
class HDRI2Cube :public ComputePass
{
public:
	HDRI2Cube(HString hdrImagePath, HString ddsOutputPath,bool bGenerateMips, int cubeMapSize = -1);

	virtual ~HDRI2Cube();

private:
	HString _ddsOutputPath;
	
	HString _hdrImagePath;

	bool _bGenerateMips;

	uint32_t _cubeMapFaceSize;

	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout _storeDescriptorSetLayout = VK_NULL_HANDLE;

	std::shared_ptr<class DescriptorSet> store_descriptorSet;

	std::shared_ptr<Texture2D> _storeTexture[6];

	std::shared_ptr<Texture2D> _hdriTexture;

	std::unique_ptr<Buffer> _uniformBuffer;


};
