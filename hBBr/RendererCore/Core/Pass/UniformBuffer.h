#pragma once
#include "glm/glm.hpp"
#include "VulkanManager.h"

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

#define MaxLightingNum  64

enum LightType : uint32_t
{
	LightType_DirectionalLight = 0,
	LightType_PointLight = 1,
	LightType_SpotLight = 2
};

enum LightingFlagBits : uint32_t
{
	LightingFlag_CastShadow = 0x00000001,
};

struct LightingParameters
{
	//LightPos
	glm::vec3 LightPosition = glm::vec3(0, 0, 0);
	float LightStrength = 1.0f;
	glm::vec3 LightColor = glm::vec3(1, 1, 1);
	float LightSpecular = 1.0f;
	glm::vec3 LightDirection = glm::vec3(0, 1, 0);
	uint32_t LightType = 0;
	alignas(16) uint32_t LightFlags = 0;
}; 

struct LightingUniformBuffer
{
	PassUniformBuffer passUniform;
	alignas(16) uint32_t validLightCount;
	LightingParameters lightParams[MaxLightingNum];
};

struct PostProcessUniformBuffer
{
	PassUniformBuffer passUniform;
	uint32_t debugMode = 0;
};