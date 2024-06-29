#pragma once
#include <memory>
#include "PassBase.h"

struct ScreenshotVertexData
{
	glm::vec2 Pos;
	glm::vec2 UV;
};

struct ScreenshotUniformBuffer
{
	glm::vec4 Parameter0;
};

/* Opaque pass define */
class ScreenshotPass :public GraphicsPass
{
public:
	ScreenshotPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~ScreenshotPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;

	static bool bScreenshot;

private:
	std::shared_ptr<class Buffer>_vertexBuffer;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _ubDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;
	PipelineIndex _shaderIndex;
	std::shared_ptr<class DescriptorSet> _ub_descriptorSet;
	std::shared_ptr<class DescriptorSet> _tex_descriptorSet;
	PipelineIndex CreatePipeline(HString shaderName);
};

