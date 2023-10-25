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
	//x,y,w,h
	alignas(16)
	glm::vec4 UVSetting;
	alignas(4)
	float channel = -1;
	alignas(4)
	float TextureSize = 512;

	bool operator!=(const GUIUniformBuffer& c) const
	{
		return 
			this->UVSetting != c.UVSetting || 
			this->channel != c.channel
			;
	}
};

struct GUIDrawState
{
	GUIAnchor Anchor;
	/* bFixed 填充模式，width和height以百分比为主 */
	bool bFixed;
	glm::vec4 Color;
	glm::vec2 Scale = glm::vec2(1);
	glm::vec2 Translate = glm::vec2(0);
	GUIUniformBuffer uniformBuffer;
	GUIDrawState() {}
	GUIDrawState(GUIAnchor anchor, bool fixed, glm::vec4 color)
	{
		Anchor = anchor;
		bFixed = fixed;
		Color = color;
	}
};

struct GUIPrimitive
{
	std::vector<GUIVertexData> Data;
	std::vector<GUIDrawState> States;
	HString PipelineTag;
	std::vector<wchar_t> fontCharacter;
	Texture* BaseTexture = NULL;
	VkRect2D viewport;
	std::shared_ptr<class DescriptorSet> ub_descriptorSet;
	std::shared_ptr<class DescriptorSet> tex_descriptorSet;
};

class GUIPass :public GraphicsPass
{
public:
	GUIPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~GUIPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;

	void GUIDrawImage(HString tag, Texture* texture, float x, float y, float w, float h,GUIDrawState state);
	void GUIDrawText(HString tag, HString text, float x, float y, float w, float h, GUIDrawState state , float fontSize = 20);

private:
	void CreatePipeline(HString pipelineTag , HString shaderName);
	void ShowPerformance();

	GUIPrimitive* GetPrimitve(HString& tag,GUIDrawState& state , int stateCount,HString pipelineTag, float x, float y, float w, float h);
	std::vector<GUIVertexData> GetGUIPanel(GUIDrawState& state, float x, float y, float w, float h);
	std::shared_ptr<class Buffer>_vertexBuffer;
	std::unordered_map<HString,GUIPrimitive> _drawList;
	std::unordered_map<HString, VkPipeline> _guiPipelines;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _ubDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;
};
