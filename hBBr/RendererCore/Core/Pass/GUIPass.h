#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "PassBase.h"
#include "HGuid.h"
enum GUIAnchor
{
	TopLeft,
	TopCenter,
	TopRight,
	CenterLeft,
	CenterCenter,
	CenterRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
};

struct GUIVertexData
{
	glm::vec2 Pos;
	glm::vec2 UV;
	glm::vec4 Color;
};

struct GUIUniformBuffer
{
	glm::mat4 Projection;
	glm::vec4 ScaleAndTranslate;
};

struct GUIDrawState
{
	std::vector<GUIVertexData> Data;
	GUIAnchor Anchor;
	HString PipelineTag;
};

class GUIPass :public GraphicsPass
{
public:
	GUIPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~GUIPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;

	void AddImage(float x, float y, float w, float h, GUIAnchor anchor);

private:
	std::shared_ptr<class DescriptorSet> _descriptorSet;
	std::shared_ptr<class Buffer>_vertexBuffer;
	std::vector<GUIDrawState> _drawList;
	std::unordered_map<HString, VkPipeline> _guiPipelines;
	VkPipelineLayout _pipelineLayout;
	GUIUniformBuffer _uniformBuffer;
};
