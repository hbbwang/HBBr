#pragma once
#include <memory>
#include "PassBase.h"

/* Opaque pass define */
class BasePass :public GraphicsPass
{
public:

	BasePass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	~BasePass();

	virtual void PassInit()override;
	virtual void PassBuild()override;
	virtual void PassUpdate()override;
private:
	std::shared_ptr<class DescriptorSet> _descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _descriptorSet_obj;
	std::shared_ptr<class DescriptorSet> _descriptorSet_mat;
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
