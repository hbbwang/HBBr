#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HRect.h"
//GUI Flags
#define IsFont      0x00000001
#define FontShadow  0x00000002

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
	glm::vec4 UVSetting;
	glm::vec4 Color = glm::vec4(1);
	float TextureSizeX = 512;
	float TextureSizeY = 512;
	float ScreenSizeX = 512;
	float ScreenSizeY = 512;
	alignas(16) int Flags = 0;

	bool operator!=(const GUIUniformBuffer& c) const
	{
		return
			this->UVSetting != c.UVSetting ||
			this->Color != c.Color ||
			this->TextureSizeX != c.TextureSizeX ||
			this->TextureSizeY != c.TextureSizeY ||
			this->Flags != c.Flags;
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
	PipelineIndex pipelineIndex;
	std::vector<wchar_t> fontCharacter;
	Texture2D* BaseTexture = nullptr;
	VkRect2D viewport;
	std::shared_ptr<class DescriptorSet> ub_descriptorSet;
	std::shared_ptr<class DescriptorSet> tex_descriptorSet;
};

#define GUIDrawText(text,x,y,w,h,state,fontSize)\
{\
	_GUIDrawText(HString(__FUNCTION__)+HString::FromInt(__LINE__) , text, x, y, w, h, state, fontSize);\
}

#define GUIDrawImage(text,x,y,w,h,state,fontSize)\
{\
	_GUIDrawImage(HString(__FUNCTION__)+HString::FromInt(__LINE__) , texture, x, y, w, h, state);\
}

class GUIPass :public GraphicsPass
{
public:
	GUIPass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~GUIPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;

	GUIPrimitive* GetGUIPrimitive(HString tag) { return &_drawList[tag]; }

	//生成纹理图片，自动生成tag
	void _GUIDrawImage(HString tag, Texture2D* texture, float x, float y, float w, float h, GUIDrawState state);
	
	//生成文字，自定义tag
	void _GUIDrawText(HString tag, HString text, float x, float y, float w, float h, GUIDrawState state , float fontSize = 20);

private:
	PipelineIndex CreatePipeline(HString shaderName);
	void ShowPerformance();

	GUIPrimitive* GetPrimitve(HString& tag,GUIDrawState& state , int stateCount, PipelineIndex index, float x, float y, float w, float h);

	PipelineIndex _guiShaderIndex;

	void SetupPanelAnchor(GUIDrawState state, float x, float y, float w, float h, GUIVertexData* vertexData);
	std::shared_ptr<class VMABuffer>_vertexBuffer;
	std::map<HString,GUIPrimitive> _drawList;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _ubDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;
};
