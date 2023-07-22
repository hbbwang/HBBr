#pragma once
//#include "Common.h"
#include <vector>
#include "vulkan/vulkan.h"

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

private:

	std::vector<VkFramebuffer> _frameBuffer;

	VkRenderPass _renderPass;

};