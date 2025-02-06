#pragma once
#include "glm/glm.hpp"
#include "VulkanManager.h"

enum class Pass : uint32_t
{
	PreCommand = 0,
	BasePass = 1,
	DeferredLighting = 2,
	PostProcess = 3,
	Imgui = 4,

	MaxNum = 32
};

std::string GetPassName(Pass& pass);

struct VertexInputLayout
{
	VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	uint32_t inputSize;
	std::vector<VkFormat>inputLayouts;
};