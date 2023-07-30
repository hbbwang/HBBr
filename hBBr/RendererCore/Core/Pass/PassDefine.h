#pragma once
#include <memory>
#include "PassBase.h"

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
	std::shared_ptr<class Buffer>_vertexBuffer;
	std::shared_ptr<class Buffer>_indexBuffer;
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
private:
	void ShowPerformance();
};
