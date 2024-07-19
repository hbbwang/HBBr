#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HRect.h"

struct LightingVertexData
{
	glm::vec2 Pos;
	glm::vec2 UV;
};

class DeferredLightingPass :public GraphicsPass
{
public:
	DeferredLightingPass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~DeferredLightingPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	std::shared_ptr<class Buffer>_vertexBuffer;

	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;

	PipelineIndex _shaderIndex;

	std::shared_ptr<class DescriptorSet> _ub_descriptorSet;
	std::shared_ptr<class DescriptorSet> _tex_descriptorSet;

	PipelineIndex CreatePipeline(HString shaderName);

	std::vector<LightingVertexData> vertices;
};