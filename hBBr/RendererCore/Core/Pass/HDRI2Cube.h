#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HRect.h"

class HDRI2Cube :public ComputePass
{
public:
	HDRI2Cube(HString imagePath);
	virtual ~HDRI2Cube();
	virtual void PassInit()override;
	void PassExecute();
	VkExtent2D _cubeMapFaceSize = { 1024 ,1024 };
private:
	PipelineIndex CreatePipeline();
	PipelineIndex _pipelineIndex;

	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _storeDescriptorSetLayout = VK_NULL_HANDLE;
	std::shared_ptr<class DescriptorSet> store_descriptorSet;
	std::shared_ptr<Texture2D> _storeTexture;
	std::shared_ptr<Texture2D> _hdriTexture;
};
