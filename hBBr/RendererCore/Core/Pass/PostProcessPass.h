#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HBox.h"

struct PostProcessVertexData
{
	glm::vec2 Pos;
	glm::vec2 UV;
};

class PostProcessPass :public GraphicsPass
{
public:
	PostProcessPass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~PostProcessPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	std::shared_ptr<class VMABuffer>_vertexBuffer;

	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

	PipelineIndex _shaderIndex;

	std::shared_ptr<class DescriptorSet> _ub_descriptorSet;
	std::shared_ptr<class DescriptorSet> _tex_descriptorSet;

	PipelineIndex CreatePipeline(std::string shaderName);

	std::vector<PostProcessVertexData> vertices;
};