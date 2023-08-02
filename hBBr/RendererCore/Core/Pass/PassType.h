#pragma once
#include <vulkan/vulkan.h>
enum class Pass
{
	BasePass = 0,
};

struct VertexInputLayout
{
	VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	uint32_t inputSize;
	std::vector<VkFormat>inputLayouts;
};