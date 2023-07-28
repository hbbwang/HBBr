#pragma once
//Pass基类
#include "Common.h"
#include "Pipeline.h"
#include <vector>
#include <memory>
#include <map>
#include "glm/glm.hpp"
#include "HString.h"

class Texture;
class VulkanRenderer;

class PassBase
{
	friend class VulkanManager;
	friend class PassManager;
public:
	PassBase(VulkanRenderer* renderer);
	~PassBase();
	virtual void PassBuild() {}
	__forceinline HString GetName()const { return _passName; }
protected:
	virtual void PassInit() {}
	virtual void PassUpdate() {}
	virtual void PassReset() {}
	std::shared_ptr<Texture> GetSceneTexture(uint32_t descIndex);
	std::unique_ptr<Pipeline> _pipeline;
	VulkanRenderer* _renderer = NULL;
	HString _passName = "PassBase";
};

class GraphicsPass : public PassBase
{
public:
	GraphicsPass(VulkanRenderer* renderer) :PassBase(renderer) {}
	~GraphicsPass();
	//Step 1 , Can add multiple attachments.
	virtual void AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkFormat attachmentFormat, VkImageLayout initLayout, VkImageLayout finalLayout);
	//Step 2 , Setup subpass by attachments.
	virtual void AddSubpass(std::vector<uint32_t> inputAttachments, std::vector<uint32_t> colorAttachments, int depthStencilAttachments = -1);
	//Step the last,custom.
	virtual void PassBuild()override {}
	virtual void PassReset()override { _currentFrameBufferSize = { 999999 , 999999 }; }
	virtual void ResetFrameBuffer(VkExtent2D size, std::vector<VkImageView> swapchainImageViews,std::vector<VkImageView> imageViews);
	void CreateRenderPass();
	__forceinline VkRenderPass GetRenderPass()const
	{
		return _renderPass;
	}
protected:
	VkFramebuffer GetFrameBuffer()const;
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

class ComputePass : public PassBase
{
public:
	ComputePass(VulkanRenderer* renderer) :PassBase(renderer) {}
};

/* Opaque pass define */
class OpaquePass :public GraphicsPass
{
public:
	struct PassUniformBuffer
	{
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		glm::vec4 BaseColor;
	};
	struct ObjectUniformBuffer
	{
		glm::mat4 WorldMatrix;
	};
	OpaquePass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	~OpaquePass();
	virtual void PassInit()override;
	virtual void PassBuild()override;
	virtual void PassUpdate()override;
private:
	std::shared_ptr<class DescriptorSet> _descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _descriptorSet_obj;
};

/* Imgui pass define */
class ImguiPass :public GraphicsPass
{
public:
	ImguiPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	~ImguiPass();
	virtual void PassInit()override;
	virtual void PassBuild()override;
	virtual void PassUpdate()override;
};
