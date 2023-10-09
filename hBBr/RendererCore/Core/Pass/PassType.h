#pragma once
#include "glm/glm.hpp"
#include "VulkanManager.h"

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

struct PassUniformBuffer
{
	glm::mat4 View;
	glm::mat4 View_Inv;
	glm::mat4 Projection;
	glm::mat4 Projection_Inv;
	glm::mat4 ViewProj;
	glm::mat4 ViewProj_Inv;
	glm::vec4 ScreenInfo; // screen xy,z near,w zfar
	glm::vec4 CameraPos_GameTime;//view pos xyz , game time w.
	glm::vec4 CameraDirection;//view dir xyz
};

struct ObjectUniformBuffer
{
	glm::mat4 WorldMatrix;
};
