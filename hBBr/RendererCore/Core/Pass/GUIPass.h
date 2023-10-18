#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "PassBase.h"
#include "HGuid.h"
enum GUIAnchor
{	
	GUIAnchor_TopLeft,
	GUIAnchor_TopCenter,
	GUIAnchor_TopRight,
	GUIAnchor_CenterLeft,
	GUIAnchor_CenterCenter,
	GUIAnchor_CenterRight,
	GUIAnchor_BottomLeft,
	GUIAnchor_BottomCenter,
	GUIAnchor_BottomRight,
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
	GUIAnchor Anchor;
	/* bFixed 填充模式，width和height以百分比为主 */
	bool bFixed;
	glm::vec4 Color;
	GUIDrawState() {}
	GUIDrawState(GUIAnchor anchor, bool fixed , glm::vec4 color)
	{
		Anchor = anchor;
		bFixed = fixed;
		Color = color;
	}
};

struct GUIPrimitive
{
	std::vector<GUIVertexData> Data;
	GUIDrawState State;
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

	void AddImage(float w, float h, GUIDrawState state);

private:
	std::vector<GUIVertexData> GetGUIPanel(GUIDrawState state, float w, float h);
	std::shared_ptr<class DescriptorSet> _descriptorSet;
	std::shared_ptr<class Buffer>_vertexBuffer;
	std::vector<GUIPrimitive> _drawList;
	std::unordered_map<HString, VkPipeline> _guiPipelines;
	VkPipelineLayout _pipelineLayout;
	GUIUniformBuffer _uniformBuffer;
};
