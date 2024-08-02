#pragma once
//Pass基类
#include "Common.h"
#include "Pipeline.h"
#include <vector>
#include <memory>
#include <map>
#include "HString.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "Primitive.h"
#include "SceneTexture.h"

class Texture2D;
class VulkanRenderer;

class PassBase
{
	friend class VulkanManager;
	friend class PassManager;
public:
	PassBase(class PassManager* manager);
	HBBR_API virtual ~PassBase();
	HBBR_API HBBR_INLINE HString GetName()const { return _passName; }
	HBBR_API HBBR_INLINE double GetPassCPURenderingTime()const { return _cpuTime; }
	HBBR_API HBBR_INLINE double GetPassGPURenderingTime()const { return _gpuTime; }
	HBBR_API HBBR_INLINE void SetPassName(HString name) { _passName = name; }
protected:
	HBBR_API virtual void PassInit() {}
	HBBR_API virtual void PassUpdate() {}
	HBBR_API virtual void Reset() {}
	HBBR_API virtual void PassReset() {}
	HBBR_API virtual void PassBeginUpdate();
	HBBR_API virtual void PassEndUpdate();
	HBBR_API std::shared_ptr<Texture2D> GetSceneTexture(uint32_t descIndex);
	HBBR_API std::shared_ptr<Texture2D> GetSceneTexture(SceneTextureDesc desc);
	class PassManager* _manager = nullptr;
	VulkanRenderer* _renderer = nullptr;
	HString _passName = "PassBase";
	glm::vec4 _markColor = glm::vec4(1,1,1,0.5);

	//性能测试
	uint32_t passIndex = 0;
	HTime _cpuTimer;
	double _cpuTime;
	double _gpuTime;
	bool bStartQuery[4];
};

class GraphicsPass : public PassBase
{
public:
	GraphicsPass(class PassManager* manager) :PassBase(manager) {}
	HBBR_API virtual ~GraphicsPass();
	//Step 1 , Can add multiple attachments.
	HBBR_API virtual void AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkFormat attachmentFormat, VkImageLayout initLayout, VkImageLayout finalLayout);
	//Step 2 , Setup subpass by attachments.
	HBBR_API virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments = -1);
	HBBR_API virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments ,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	//Step the last,custom.
	HBBR_API virtual void PassInit()override {}
	HBBR_API virtual void PassUpdate()override;
	HBBR_API virtual void PassReset()override {}
	HBBR_API void Reset() override {
		_currentFrameBufferSize = { 1 , 1 };//Force update
		PassReset();
	}
	//该函数不需要指定swapchain image，已经包含在内
	HBBR_API virtual void ResetFrameBuffer(VkExtent2D size,std::vector<VkImageView> imageViews);
	//该函数需要手动指定所有images
	HBBR_API virtual void ResetFrameBufferCustom(VkExtent2D size, std::vector<VkImageView> imageViews);

	HBBR_API void CreateRenderPass();

	HBBR_API HBBR_INLINE VkRenderPass GetRenderPass()const
	{
		return _renderPass;
	}
protected:
	HBBR_API virtual void PassRender() {}
	HBBR_API void BeginRenderPass(std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f }, VkExtent2D areaSize = {});
	HBBR_API void EndRenderPass();
	HBBR_API void SetViewport(VkExtent2D viewportSize);
	HBBR_API VkFramebuffer GetFrameBuffer()const;

	VkRenderPass _renderPass = VK_NULL_HANDLE;
	std::vector<VkAttachmentDescription>_attachmentDescs;
	std::vector<VkSubpassDependency>_subpassDependencys;
	std::vector<VkSubpassDescription>_subpassDescs;
	//
	std::vector<VkAttachmentReference> _input_ref;
	std::vector<VkAttachmentReference> _color_ref;
	VkAttachmentReference	_depthStencil_ref;
	std::vector<VkFramebuffer> _framebuffers;
	VkExtent2D _currentFrameBufferSize;
};

class CommandPass : public PassBase
{
public:
	CommandPass(class PassManager* manager) :PassBase(manager) {}
	virtual~CommandPass() {}
};


class ComputePass : public PassBase
{
public:
	ComputePass(class PassManager* manager) :PassBase(manager) {}
	virtual~ComputePass() {}
};
