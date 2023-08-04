#pragma once
#include <vulkan/vulkan.h>

enum class Pass : uint32_t
{
	OpaquePass = 0,

	MaxNum = 32
};

struct VertexInputLayout
{
	VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	uint32_t inputSize;
	std::vector<VkFormat>inputLayouts;
};
