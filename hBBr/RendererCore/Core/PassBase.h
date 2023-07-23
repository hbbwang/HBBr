#pragma once
//Pass基类
#include "Common.h"
#include "Pipeline.h"
#include <vector>
#include <memory>
class PassBase
{
	friend class PassManager;
public:
	PassBase() {}
	~PassBase() {}
protected:
	virtual void PassInit() {}
	virtual void PassUpdate() {}
	virtual void PassReset() {}
	std::unique_ptr<Pipeline> _pipeline;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
};

class GraphicsPass : public PassBase
{
public:
	GraphicsPass();
};

class ComputePass : public PassBase
{

};